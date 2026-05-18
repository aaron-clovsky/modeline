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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************************************
Macros
*******************************************************************************/
/* Prevent warnings when compiled as C vs C++ */
#ifndef __cplusplus
    #define STRUCT_ZERO_INIT 0
#else
    #define STRUCT_ZERO_INIT
#endif

/*******************************************************************************
Utilities
*******************************************************************************/
void modeline_print(modeline * mode)
{
    printf("\"%dx%d_%d%s %.6fKHz %.6fHz\"\n"
           " %.6f %d %d %d %d %d %d %d %d %s\n",
           mode->hactive,
           mode->vactive,
           (int)mode->vfreq,
           mode->interlace ? "i" : "",
           mode->hfreq / 1000,
           mode->vfreq,
           (double)mode->pclock / 1000000.0,
           mode->hactive,
           mode->hbegin,
           mode->hend,
           mode->htotal,
           mode->vactive,
           mode->vbegin,
           mode->vend,
           mode->vtotal,
           mode->interlace ? "interlace" : "");
}

/*******************************************************************************
main()
*******************************************************************************/
int main(int argc, char ** argv)
{
    enum modeline_error      error;
    const modeline_monitor * monitor = &MODELINE_MONITOR_GENERIC_15KHZ;
    unsigned int             width;
    unsigned int             height;
    double                   refresh_rate;
    double                   h_size;
    int                      h_shift;
    int                      v_shift;
    modeline                 mode = { STRUCT_ZERO_INIT };

    if (argc < 4 || argc > 7) {
        printf("Usage: %s <width> <height> <refresh> "
               "[h_size] [h_shift] [v_shift]\n",
               argv[0]);
        exit(2);
    }

    width        = atoi(argv[1]);
    height       = atoi(argv[2]);
    refresh_rate = atof(argv[3]);

    h_size  = 1.0;
    h_shift = 0;
    v_shift = 0;

    if (argc >= 5) {
        h_size = atof(argv[4]);
    }
    if (argc >= 6) {
        h_shift = atoi(argv[5]);
    }
    if (argc >= 7) {
        v_shift = atoi(argv[6]);
    }

    error = modeline_calc(monitor,
                          width,
                          height,
                          refresh_rate,
                          h_size,
                          h_shift,
                          v_shift,
                          &mode);

    if (error) {
        fprintf(stderr, "Error: %s\n", modeline_error_string(error));
        exit(1);
    }

    modeline_print(&mode);

    return 0;
}
