/*
 * Copyright (c) 2016-2017, The University of Oxford
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Oxford nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "imager/private_imager.h"
#include "imager/oskar_imager.h"

#include "imager/private_imager_update_plane_wproj.h"
#include "imager/oskar_grid_wproj.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SAVE_INPUT_DAT 0
#define SAVE_OUTPUT_DAT 0
#define SAVE_GRID 0

void oskar_imager_update_plane_wproj(oskar_Imager* h, size_t num_vis,
        const oskar_Mem* uu, const oskar_Mem* vv, const oskar_Mem* ww,
        const oskar_Mem* amps, const oskar_Mem* weight, oskar_Mem* plane,
        double* plane_norm, size_t* num_skipped, int* status)
{
    int grid_size;
    size_t num_cells;
    if (*status) return;
    grid_size = oskar_imager_plane_size(h);
    num_cells = grid_size * grid_size;
    if (oskar_mem_precision(plane) != h->imager_prec)
        *status = OSKAR_ERR_TYPE_MISMATCH;
    if (oskar_mem_length(plane) < num_cells)
        oskar_mem_realloc(plane, num_cells, status);
    if (*status) return;
    if (h->imager_prec == OSKAR_DOUBLE)
        oskar_grid_wproj_d(h->num_w_planes,
                oskar_mem_int_const(h->w_support, status),
                h->oversample, h->conv_size_half,
                oskar_mem_double_const(h->w_kernels, status), num_vis,
                oskar_mem_double_const(uu, status),
                oskar_mem_double_const(vv, status),
                oskar_mem_double_const(ww, status),
                oskar_mem_double_const(amps, status),
                oskar_mem_double_const(weight, status),
                h->cellsize_rad, h->w_scale,
                grid_size, num_skipped, plane_norm,
                oskar_mem_double(plane, status));
    else
    {
        char *fname = 0;
#if SAVE_INPUT_DAT || SAVE_OUTPUT_DAT || SAVE_GRID
        fname = (char*) calloc(20 +
                h->input_root ? strlen(h->input_root) : 0, 1);
#endif
#if SAVE_INPUT_DAT
        {
            const float cellsize_rad_tmp = (const float) (h->cellsize_rad);
            const float w_scale_tmp = (const float) (h->w_scale);
            const size_t num_w_planes = (size_t) (h->num_w_planes);
            FILE* f;
            sprintf(fname, "%s_INPUT.dat", h->input_root);
            f = fopen(fname, "wb");
            fwrite(&num_w_planes, sizeof(size_t), 1, f);
            fwrite(oskar_mem_void_const(h->w_support),
                    sizeof(int), num_w_planes, f);
            fwrite(&h->oversample, sizeof(int), 1, f);
            fwrite(&h->conv_size_half, sizeof(int), 1, f);
            fwrite(oskar_mem_void_const(h->w_kernels), 2 * sizeof(float),
                    h->num_w_planes * h->conv_size_half * h->conv_size_half, f);
            fwrite(&num_vis, sizeof(size_t), 1, f);
            fwrite(oskar_mem_void_const(uu), sizeof(float), num_vis, f);
            fwrite(oskar_mem_void_const(vv), sizeof(float), num_vis, f);
            fwrite(oskar_mem_void_const(ww), sizeof(float), num_vis, f);
            fwrite(oskar_mem_void_const(amps), 2 * sizeof(float), num_vis, f);
            fwrite(oskar_mem_void_const(weight), sizeof(float), num_vis, f);
            fwrite(&cellsize_rad_tmp, sizeof(float), 1, f);
            fwrite(&w_scale_tmp, sizeof(float), 1, f);
            fwrite(&grid_size, sizeof(int), 1, f);
            fclose(f);
        }
#endif
        oskar_grid_wproj_f(h->num_w_planes,
                oskar_mem_int_const(h->w_support, status),
                h->oversample, h->conv_size_half,
                oskar_mem_float_const(h->w_kernels, status), num_vis,
                oskar_mem_float_const(uu, status),
                oskar_mem_float_const(vv, status),
                oskar_mem_float_const(ww, status),
                oskar_mem_float_const(amps, status),
                oskar_mem_float_const(weight, status),
                (float) (h->cellsize_rad), (float) (h->w_scale),
                grid_size, num_skipped, plane_norm,
                oskar_mem_float(plane, status));
#if SAVE_OUTPUT_DAT
        {
            FILE* f;
            sprintf(fname, "%s_OUTPUT.dat", h->input_root);
            f = fopen(fname, "wb");
            fwrite(num_skipped, sizeof(size_t), 1, f);
            fwrite(plane_norm, sizeof(double), 1, f);
            fwrite(&grid_size, sizeof(int), 1, f);
            fwrite(oskar_mem_void_const(plane), 2 * sizeof(float), num_cells, f);
            fclose(f);
        }
#endif
#if SAVE_GRID
        sprintf(fname, "%s_GRID", h->input_root);
        oskar_mem_write_fits_cube(plane, fname,
                grid_size, grid_size, 1, 0, status);
#endif
        free(fname);
    }
}

#ifdef __cplusplus
}
#endif
