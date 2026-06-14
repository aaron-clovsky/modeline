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
#include <stdint.h>

/*******************************************************************************
Macros
*******************************************************************************/
#ifdef LIBMODELINE_EXPORTS
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT
#endif

/*******************************************************************************
Types
*******************************************************************************/
typedef struct modeline {
    uint64_t pclock;
    int      hactive;
    int      hbegin;
    int      hend;
    int      htotal;
    int      vactive;
    int      vbegin;
    int      vend;
    int      vtotal;
    int      interlace;
    double   vfreq;
    double   hfreq;
} modeline;

typedef struct modeline_monitor {
    double hfreq_min;
    double hfreq_max;
    double vfreq_min;
    double vfreq_max;
    double hfront_porch;
    double hsync_pulse;
    double hback_porch;
    double vfront_porch;
    double vsync_pulse;
    double vback_porch;
    int    hsync_polarity;
    int    vsync_polarity;
    int    progressive_lines_min;
    int    progressive_lines_max;
    int    interlaced_lines_min;
    int    interlaced_lines_max;
    double vertical_blank;
} modeline_monitor;

typedef enum modeline_error {
    MODELINE_ERROR_NONE              = 0,
    MODELINE_ERROR_IN_MONITOR_NULL   = -1,
    MODELINE_ERROR_IN_WIDTH          = -2,
    MODELINE_ERROR_IN_HEIGHT         = -3,
    MODELINE_ERROR_IN_REFRESH_RATE   = -4,
    MODELINE_ERROR_IN_H_SIZE         = -5,
    MODELINE_ERROR_IN_H_SHIFT        = -6,
    MODELINE_ERROR_IN_V_SHIFT        = -7,
    MODELINE_ERROR_IN_MODE_NULL      = -8,
    MODELINE_ERROR_CALC_V_FREQ       = -9,
    MODELINE_ERROR_CALC_V_FREQ_CHECK = -10
} modeline_error;

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
External Constants
*******************************************************************************/
DLL_EXPORT extern struct modeline_monitor MODELINE_MONITOR_GENERIC_15KHZ;

/*******************************************************************************
External Functions
*******************************************************************************/
DLL_EXPORT enum modeline_error
    modeline_calc(const struct modeline_monitor * monitor,
                  unsigned int                    width,
                  unsigned int                    height,
                  double                          refresh_rate,
                  double                          h_size,
                  int                             h_shift,
                  int                             v_shift,
                  struct modeline *               mode);

DLL_EXPORT const char * modeline_error_string(enum modeline_error error);

#ifdef __cplusplus
}
#endif
