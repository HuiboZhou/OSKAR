/*
 * Copyright (c) 2011-2013, The University of Oxford
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

#include "station/oskar_element_model_copy.h"
#include "station/oskar_station_model_copy.h"
#include "station/oskar_station_model_init.h"
#include "station/oskar_station_model_location.h"
#include "station/oskar_station_model_type.h"
#include "station/oskar_station_model_resize_element_types.h"
#include "station/oskar_system_noise_model_copy.h"
#include "utility/oskar_mem_copy.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void oskar_station_model_copy(oskar_StationModel* dst,
        const oskar_StationModel* src, int* status)
{
    int i = 0;

    /* Check all inputs. */
    if (!src || !dst || !status)
    {
        oskar_set_invalid_argument(status);
        return;
    }

    /* Check if safe to proceed. */
    if (*status) return;

    /* Copy common station parameters. */
    dst->station_type = src->station_type;
    dst->longitude_rad = src->longitude_rad;
    dst->latitude_rad = src->latitude_rad;
    dst->altitude_m = src->altitude_m;
    dst->beam_longitude_rad = src->beam_longitude_rad;
    dst->beam_latitude_rad = src->beam_latitude_rad;
    dst->beam_coord_type = src->beam_coord_type;
    oskar_system_noise_model_copy(&dst->noise, &src->noise, status);

    /* Copy aperture array data, except num_element_types (done later). */
    dst->num_elements = src->num_elements;
    dst->use_polarised_elements = src->use_polarised_elements;
    dst->normalise_beam = src->normalise_beam;
    dst->enable_array_pattern = src->enable_array_pattern;
    dst->single_element_model = src->single_element_model;
    dst->array_is_3d = src->array_is_3d;
    dst->apply_element_errors = src->apply_element_errors;
    dst->apply_element_weight = src->apply_element_weight;
    dst->coord_units = src->coord_units;
    dst->orientation_x = src->orientation_x;
    dst->orientation_y = src->orientation_y;

    /* Copy Gaussian station beam data. */
    dst->gaussian_beam_fwhm_deg = src->gaussian_beam_fwhm_deg;

    /* Copy memory blocks. */
    oskar_mem_copy(&dst->x_signal, &src->x_signal, status);
    oskar_mem_copy(&dst->y_signal, &src->y_signal, status);
    oskar_mem_copy(&dst->z_signal, &src->z_signal, status);
    oskar_mem_copy(&dst->x_weights, &src->x_weights, status);
    oskar_mem_copy(&dst->y_weights, &src->y_weights, status);
    oskar_mem_copy(&dst->z_weights, &src->z_weights, status);
    oskar_mem_copy(&dst->weight, &src->weight, status);
    oskar_mem_copy(&dst->gain, &src->gain, status);
    oskar_mem_copy(&dst->gain_error, &src->gain_error, status);
    oskar_mem_copy(&dst->phase_offset, &src->phase_offset, status);
    oskar_mem_copy(&dst->phase_error, &src->phase_error, status);
    oskar_mem_copy(&dst->cos_orientation_x, &src->cos_orientation_x, status);
    oskar_mem_copy(&dst->sin_orientation_x, &src->sin_orientation_x, status);
    oskar_mem_copy(&dst->cos_orientation_y, &src->cos_orientation_y, status);
    oskar_mem_copy(&dst->sin_orientation_y, &src->sin_orientation_y, status);
    oskar_mem_copy(&dst->element_type, &src->element_type, status);

    /* Copy element models, if set. */
    if (src->element_pattern)
    {
        /* Ensure enough space for element model data. */
        oskar_station_model_resize_element_types(dst, src->num_element_types,
                status);

        /* Copy the element model data. */
        for (i = 0; i < src->num_element_types; ++i)
        {
            oskar_element_model_copy(&dst->element_pattern[i],
                    &src->element_pattern[i], status);
        }
    }

    /* Copy child stations. */
    if (src->child)
    {
        /* Allocate array to hold child stations, if required. */
        if (!dst->child)
        {
            int location = 0;
            location = oskar_station_model_location(dst);
            dst->child = (oskar_StationModel*) malloc(src->num_elements *
                    sizeof(oskar_StationModel));

            /* Initialise each child station. */
            for (i = 0; i < src->num_elements; ++i)
            {
                int type;
                type = oskar_station_model_type(&src->child[i]);
                oskar_station_model_init(&dst->child[i], type, location,
                        src->child[i].num_elements, status);
            }
        }

        /* Recursively copy each child station. */
        for (i = 0; i < src->num_elements; ++i)
        {
            oskar_station_model_copy(&dst->child[i], &src->child[i], status);
        }
    }
}

#ifdef __cplusplus
}
#endif
