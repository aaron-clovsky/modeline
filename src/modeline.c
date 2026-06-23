/*******************************************************************************
 * Yet Another Modeline Calculator
 * Copyright (C) 2026 Aaron Clovsky
 *
 * Based on Switchres by: Chris Kennedy, Antonio Giner,
 *                        Alexandre Wodarczyk and Gil Delescluse
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

/*******************************************************************************
Headers
*******************************************************************************/
#include <modeline.h>
#include <math.h>

/*******************************************************************************
Macros
*******************************************************************************/
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* Prevent warnings when compiled as C vs C++ */
#ifndef __cplusplus
    #define STRUCT_ZERO_INIT 0
#else
    #define STRUCT_ZERO_INIT
#endif

/*******************************************************************************
External Constants
*******************************************************************************/
struct modeline_monitor MODELINE_MONITOR_GENERIC_15KHZ = {
    /* hfreq_min = */ 15625,
    /* hfreq_max = */ 15750,
    /* vfreq_min = */ 49.50,
    /* vfreq_max = */ 65.00,
    /* hfront_porch = */ 2.000,
    /* hsync_pulse = */ 4.700,
    /* hback_porch = */ 8.000,
    /* vfront_porch = */ 0.000064,
    /* vsync_pulse = */ 0.000192,
    /* vback_porch = */ 0.001024,
    /* hsync_polarity = */ 0,
    /* vsync_polarity = */ 0,
    /* progressive_lines_min = */ 192,
    /* progressive_lines_max = */ 288,
    /* interlaced_lines_min = */ 448,
    /* interlaced_lines_max = */ 576,
    /* vertical_blank = */ 0.00128
};

/*******************************************************************************
Internal Functions
*******************************************************************************/
/*
    C99 round() implemented for compatibility with C89
*/
static int round_c89(double number)
{
    return (int)((number < 0.0) ? ceil(number - 0.5) : floor(number + 0.5));
}

/*
    Round towards nearest odd number (for positive inputs only)
*/
static int round_odd(double number)
{
    return (int)(((int)ceil(number) % 2 == 0) ? floor(number) : ceil(number));
}

/*
    Calculate horizontal part of modeline
*/
static int get_line_params(const modeline_monitor * monitor, modeline * mode)
{
    int    hh, hs, he, ht;
    double line_time, char_time, new_char_time;
    double hfront_porch_min, hsync_pulse_min, hback_porch_min;

    hfront_porch_min = monitor->hfront_porch * .90;
    hsync_pulse_min  = monitor->hsync_pulse * .90;
    hback_porch_min  = monitor->hback_porch * .90;

    line_time = 1 / mode->hfreq * 1000000;

    hh = round_c89(mode->hactive);
    hs = he = ht  = 1;
    new_char_time = line_time / (hh + hs + he + ht);

    do {
        double scaled;
        double diff;

        char_time = new_char_time;
        scaled    = hs * char_time;
        diff      = scaled - monitor->hfront_porch;

        if (scaled < hfront_porch_min || fabs(diff + char_time) < fabs(diff)) {
            hs++;
        }

        scaled = he * char_time;
        diff   = scaled - monitor->hsync_pulse;

        if (scaled < hsync_pulse_min || fabs(diff + char_time) < fabs(diff)) {
            he++;
        }

        scaled = ht * char_time;
        diff   = scaled - monitor->hback_porch;

        if (scaled < hback_porch_min || fabs(diff + char_time) < fabs(diff)) {
            ht++;
        }

        new_char_time = line_time / (hh + hs + he + ht);
    } while (new_char_time != char_time);

    mode->hbegin = hh + hs;
    mode->hend   = hh + hs + he;
    mode->htotal = hh + hs + he + ht;

    return 0;
}

/*
    Calculate total vertical lines for a given height
*/
static int vert_lines_for_height(const modeline_monitor * monitor,
                                 int                      height,
                                 double                   vfreq,
                                 double                   borders,
                                 double                   scan_factor)
{
    double vblank_borders;
    double tmp_a, tmp_b;
    int    vvt;

    vblank_borders = monitor->vertical_blank + borders;

    tmp_a = scan_factor * (1.0 - vfreq * vblank_borders);
    tmp_b = vfreq * height / tmp_a * vblank_borders;

    vvt = (int)MAX(1.0, height / scan_factor + round_c89(tmp_b));

    while ((vfreq * vvt < monitor->hfreq_min) &&
           (vfreq * (vvt + 1) < monitor->hfreq_max)) {
        vvt++;
    }

    return vvt;
}

/*
    Calculate expected achievable refresh for this height on this monitor
*/
static double max_vfreq_for_height(const modeline_monitor * monitor,
                                   int                      height,
                                   double                   borders,
                                   double                   scan_factor)
{
    double tmp = round_c89(monitor->hfreq_max *
                           (monitor->vertical_blank + borders));

    return monitor->hfreq_max / (height / scan_factor + tmp);
}

/*
    Calculate new modeline
*/
static int modeline_to_modeline_monitor(modeline_monitor * monitor,
                                        modeline *         mode)
{
    double line_time;
    double pixel_time;
    double interlace_factor;

    /* If Vfreq monitor is empty, create it around the provided vfreq */
    if (monitor->vfreq_min == 0.0f) {
        monitor->vfreq_min = mode->vfreq - 0.2;
    }

    if (monitor->vfreq_max == 0.0f) {
        monitor->vfreq_max = mode->vfreq + 0.2;
    }

    /* Make sure the monitor includes the target vfreq */
    if (mode->vfreq < monitor->vfreq_min || mode->vfreq > monitor->vfreq_max) {
        return 0;
    }

    line_time        = 1 / mode->hfreq;
    pixel_time       = line_time / mode->htotal * 1000000;
    interlace_factor = (mode->interlace != 0) ? 0.5 : 1.0;

    monitor->hfront_porch = pixel_time * (mode->hbegin - mode->hactive);
    monitor->hsync_pulse  = pixel_time * (mode->hend - mode->hbegin);
    monitor->hback_porch  = pixel_time * (mode->htotal - mode->hend);

    /* We floor the vertical fields to remove the half line from interlaced
       modes, because the modeline generator will add it automatically.
       Otherwise it would be added twice. */
    monitor->vfront_porch  = floor((mode->vbegin - mode->vactive) *
                                  interlace_factor);
    monitor->vfront_porch *= line_time;

    monitor->vsync_pulse  = floor((mode->vend - mode->vbegin) *
                                 interlace_factor);
    monitor->vsync_pulse *= line_time;

    monitor->vback_porch  = floor((mode->vtotal - mode->vend) *
                                 interlace_factor);
    monitor->vback_porch *= line_time;

    monitor->vertical_blank = monitor->vfront_porch + monitor->vsync_pulse +
                              monitor->vback_porch;

    monitor->progressive_lines_min = (mode->interlace != 0) ? 0 : mode->vactive;
    monitor->progressive_lines_max = (mode->interlace != 0) ? 0 : mode->vactive;
    monitor->interlaced_lines_min  = (mode->interlace != 0) ? mode->vactive : 0;
    monitor->interlaced_lines_max  = (mode->interlace != 0) ? mode->vactive : 0;

    monitor->hfreq_min = monitor->vfreq_min * mode->vtotal * interlace_factor;
    monitor->hfreq_max = monitor->vfreq_max * mode->vtotal * interlace_factor;

    return 1;
}

/*
    Calculate new modeline
*/
static enum modeline_error modeline_create(const modeline_monitor * monitor,
                                           modeline *               mode)
{
    const double scan_factor = (mode->interlace != 0) ? 2.0 : 1.0;
    double       vfreq_real  = 0;

    /* Calculate expected achievable refresh for this height */
    vfreq_real = MIN(mode->vfreq,
                     max_vfreq_for_height(monitor,
                                          mode->vactive,
                                          0,
                                          scan_factor));

    if (vfreq_real != mode->vfreq) {
        return MODELINE_ERROR_CALC_V_FREQ;
    }

    /*  Modeline generation */
    {
        double vvt_ini        = 0;
        double vblank_lines   = 0;
        double margin         = 0;
        double v_front_porch  = 0;
        double interlace_incr = (scan_factor == 2) ? 0.5 : 0;
        double tmp;

        /* Get resulting refresh */
        mode->vfreq = vfreq_real;

        /* Get total vertical lines */
        vvt_ini = interlace_incr + vert_lines_for_height(monitor,
                                                         mode->vactive,
                                                         mode->vfreq,
                                                         0,
                                                         scan_factor);

        /* Calculate horizontal frequency */
        mode->hfreq = mode->vfreq * vvt_ini;

        /* Fill horizontal part of modeline */
        get_line_params(monitor, mode);

        /* Calculate pixel clock */
        mode->pclock = (uint64_t)(mode->htotal * mode->hfreq);

        /* Vertical blanking */
        mode->vtotal = (int)(vvt_ini * scan_factor);

        vblank_lines = (int)round_c89(mode->hfreq * monitor->vertical_blank) +
                       interlace_incr;

        margin  = mode->vtotal - mode->vactive - vblank_lines * scan_factor;
        margin /= 2;

        v_front_porch = margin + interlace_incr +
                        mode->hfreq * monitor->vfront_porch * scan_factor;

        if (scan_factor == 2) {
            mode->vbegin = mode->vactive + MAX(1, round_odd(v_front_porch));
        } else {
            mode->vbegin = mode->vactive +
                           MAX(1, (int)round_c89(v_front_porch));
        }

        tmp        = mode->hfreq * monitor->vsync_pulse * scan_factor;
        mode->vend = mode->vbegin + MAX(1, (int)round_c89(tmp));

        /* Recalculate final vfreq */
        mode->vfreq = (mode->hfreq / mode->vtotal) * scan_factor;

        if (mode->vfreq < monitor->vfreq_min ||
            mode->vfreq > monitor->vfreq_max) {
            return MODELINE_ERROR_CALC_V_FREQ_CHECK;
        }
    }

    return MODELINE_ERROR_NONE;
}

/*
    Adjust modeline created with modeline_create() by h_size, h_shift, v_shift
*/
static enum modeline_error modeline_adjust(modeline * mode,
                                           double     hfreq_max,
                                           double     h_size,
                                           int        h_shift,
                                           int        v_shift)
{
    enum modeline_error error;
    int                 vactive, vbegin, vend, vtotal;
    int                 v_front_porch, v_back_porch, v_back_porch_ex;
    int                 max_vtotal, border, padding;

    /* H size ajdustment, valid values 0.5-2.0 */
    if (h_size != 1.0f) {
        modeline_monitor monitor = { STRUCT_ZERO_INIT };

        if (h_size > 2.0f) {
            h_size = 2.0f;
        } else if (h_size < 0.5f) {
            h_size = 0.5f;
        }

        (void)modeline_to_modeline_monitor(&monitor, mode);

        monitor.hfront_porch /= h_size;
        monitor.hback_porch  /= h_size;

        error = modeline_create(&monitor, mode);

        if (error) {
            return error;
        }
    }

    /* H shift adjustment, positive or negative value */
    if (h_shift != 0) {
        if (h_shift >= mode->hbegin - mode->hactive) {
            h_shift = mode->hbegin - mode->hactive - 1;
        } else if (h_shift <= mode->hend - mode->htotal) {
            h_shift = mode->hend - mode->htotal + 1;
        }

        mode->hbegin -= h_shift;
        mode->hend   -= h_shift;
    }

    /* V shift adjustment, positive or negative value */
    if (v_shift != 0) {
        vactive = mode->vactive;
        vbegin  = mode->vbegin;
        vend    = mode->vend;
        vtotal  = mode->vtotal;

        if (mode->interlace) {
            vactive >>= 1;
            vbegin  >>= 1;
            vend    >>= 1;
            vtotal  >>= 1;
        }

        v_front_porch = vbegin - vactive;
        v_back_porch  = vend - vtotal;
        max_vtotal    = (int)(hfreq_max / mode->vfreq);
        border        = max_vtotal - vtotal;
        padding       = 0;

        /* v_shift positive */
        if (v_shift >= v_front_porch) {
            int v_front_porch_ex = v_front_porch + border;

            if (v_shift >= v_front_porch_ex) {
                v_shift = v_front_porch_ex - 1;
            }

            padding  = v_shift - v_front_porch + 1;
            vbegin  += padding;
            vend    += padding;
            vtotal  += padding;

            /* v_shift negative */
        } else if (v_shift <= v_back_porch + 1) {
            v_back_porch_ex = v_back_porch - border;

            if (v_shift <= v_back_porch_ex + 1) {
                v_shift = v_back_porch_ex + 2;
            }

            padding  = -(v_shift - v_back_porch - 2);
            vtotal  += padding;
        }

        vbegin -= v_shift;
        vend   -= v_shift;

        if (mode->interlace) {
            vbegin = (vbegin << 1) | (mode->vbegin & 1);
            vend   = (vend << 1) | (mode->vend & 1);
            vtotal = (vtotal << 1) | (mode->vtotal & 1);
        }

        mode->vbegin = vbegin;
        mode->vend   = vend;
        mode->vtotal = vtotal;

        if (padding != 0) {
            modeline_monitor monitor = { STRUCT_ZERO_INIT };

            mode->hfreq = mode->vfreq * mode->vtotal;

            if (mode->interlace) {
                mode->hfreq /= 2;
            }

            (void)modeline_to_modeline_monitor(&monitor, mode);

            error = modeline_create(&monitor, mode);

            if (error) {
                return error;
            }
        }
    }

    return MODELINE_ERROR_NONE;
}

/*******************************************************************************
External Functions
*******************************************************************************/
enum modeline_error modeline_calc(const struct modeline_monitor * monitor,
                                  unsigned int                    width,
                                  unsigned int                    height,
                                  double                          refresh_rate,
                                  double                          h_size,
                                  int                             h_shift,
                                  int                             v_shift,
                                  struct modeline *               mode)
{
    unsigned int        h_shift_abs, v_shift_abs;
    enum modeline_error error;

    /*
        Sanity checks
    */

    if (!monitor) {
        return MODELINE_ERROR_IN_MONITOR_NULL;
    } else if (!mode) {
        return MODELINE_ERROR_IN_MODE_NULL;
    }

    /* 8K feels like a big enough horizontal resolution */
    if (width == 0 || width % 2 != 0 || width > 8192) {
        return MODELINE_ERROR_IN_WIDTH;
    }

    if (monitor->interlaced_lines_min != 0 &&
        monitor->interlaced_lines_max != 0) {
        if (height == 0 || height % 2 != 0 ||
            (int)height < monitor->progressive_lines_min ||
            (int)height > monitor->interlaced_lines_max ||
            ((int)height > monitor->progressive_lines_max &&
             (int)height < monitor->interlaced_lines_min)) {
            return MODELINE_ERROR_IN_HEIGHT;
        }

        if ((int)height > monitor->progressive_lines_max &&
            (int)height > monitor->interlaced_lines_min &&
            (int)height < monitor->interlaced_lines_max) {
            mode->interlace = 1;
        } else {
            mode->interlace = 0;
        }
    } else {
        if (height == 0 || height % 2 != 0 ||
            (int)height < monitor->progressive_lines_min ||
            (int)height > monitor->progressive_lines_max) {
            return MODELINE_ERROR_IN_HEIGHT;
        }

        mode->interlace = 0;
    }

    if (refresh_rate < monitor->vfreq_min ||
        refresh_rate > monitor->vfreq_max) {
        return MODELINE_ERROR_IN_REFRESH_RATE;
    }

    /* Valid range taken from mode_adjust()  */
    if (h_size < 0.5 || h_size > 2.0) {
        return MODELINE_ERROR_IN_H_SIZE;
    }

    /* These two sanity checks are somewhat arbitrary */
    h_shift_abs = (h_shift < 0) ? -h_shift : h_shift;

    if (h_shift_abs > width / 2) {
        return MODELINE_ERROR_IN_H_SHIFT;
    }

    v_shift_abs = (v_shift < 0) ? -v_shift : v_shift;

    if (v_shift_abs > height / 2) {
        return MODELINE_ERROR_IN_V_SHIFT;
    }

    /* Populate initial fields */
    mode->hactive = width;
    mode->vactive = height;
    mode->vfreq   = refresh_rate;

    if (monitor->interlaced_lines_min != 0 &&
        monitor->interlaced_lines_max != 0 &&
        (int)height >= monitor->interlaced_lines_min) {
        mode->interlace = 1;
    } else {
        mode->interlace = 0;
    }

    /* Create modeline */
    error = modeline_create(monitor, mode);

    if (error) {
        return error;
    }

    /* Adjust modeline (if required) */
    if (h_size != 1.0 || h_shift != 0 || v_shift != 0) {
        error = modeline_adjust(mode,
                                monitor->hfreq_max,
                                h_size,
                                h_shift,
                                v_shift);
        if (error) {
            return error;
        }
    }

    return MODELINE_ERROR_NONE;
}

const char * modeline_error_string(enum modeline_error error)
{
    /* clang-format off */
    switch (error) {
        case MODELINE_ERROR_NONE:
            return "No error";
        case MODELINE_ERROR_IN_MONITOR_NULL:
            return "Argument monitor is NULL";
        case MODELINE_ERROR_IN_WIDTH:
            return "Argument width is odd or too large";
        case MODELINE_ERROR_IN_HEIGHT:
            return "Argument height is outside of monitor's valid range";
        case MODELINE_ERROR_IN_REFRESH_RATE:
            return "Argument refresh_rate is outside of monitor's valid range";
        case MODELINE_ERROR_IN_H_SIZE:
            return "Argument h_size is less than 0.5 or greater than 2.0";
        case MODELINE_ERROR_IN_H_SHIFT:
            return "Argument h_shift is greater than half of width height";
        case MODELINE_ERROR_IN_V_SHIFT:
            return "Argument v_shift is greater than half of height";
        case MODELINE_ERROR_IN_MODE_NULL:
            return "Argument mode is NULL";
        case MODELINE_ERROR_CALC_V_FREQ:
            return "Argument refresh_rate is not achievable for this height";
        case MODELINE_ERROR_CALC_V_FREQ_CHECK:
            return "Failed to calculate modeline";
        default: break;
    }
    /* clang-format on */

    return "Unknown error";
}
