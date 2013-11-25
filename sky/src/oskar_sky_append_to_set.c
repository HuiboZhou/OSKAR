/*
 * Copyright (c) 2012-2013, The University of Oxford
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

#include <private_sky.h>
#include <oskar_sky.h>

#include <oskar_sky_get_ptr.h>
#include <oskar_sky_init.h>

#include <stdio.h>
#include <stdlib.h>

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifdef __cplusplus
extern "C" {
#endif

void oskar_sky_append_to_set(int* set_size, oskar_Sky*** set_ptr,
        int max_sources_per_model, const oskar_Sky* model, int* status)
{
    int free_space, space_required, num_extra_models, number_to_copy;
    int i, j, type, location, n, n_copy, from_offset;
    oskar_Sky model_ptr, **set;
    size_t new_size;

    /* Check all inputs. */
    if (!set_size || !set_ptr || !model || !status)
    {
        oskar_set_invalid_argument(status);
        return;
    }

    /* Check if safe to proceed. */
    if (*status) return;

    /* Get type and location. */
    type     = oskar_sky_precision(model);
    location = oskar_sky_location(model);
    if (location != OSKAR_LOCATION_CPU)
    {
        *status = OSKAR_ERR_BAD_LOCATION;
        return;
    }

    /* Work out if the set needs to be resized, and if so by how much. */
    free_space = (*set_size == 0) ? 0 :
            max_sources_per_model - (*set_ptr)[*set_size - 1]->num_sources;
    space_required = model->num_sources - free_space;
    num_extra_models = (space_required + max_sources_per_model - 1)
            / max_sources_per_model;

    /* Sanity check. */
    if (num_extra_models < 0)
    {
        *status = OSKAR_ERR_INVALID_ARGUMENT;
        return;
    }

    /* Resize the array of sky model handles. */
    new_size = (*set_size + num_extra_models) * sizeof(oskar_Sky*);
    *set_ptr = realloc((*set_ptr), new_size);
    set = *set_ptr;
    for (i = *set_size; i < *set_size + num_extra_models; ++i)
    {
        set[i] = oskar_sky_create(type, location,
                max_sources_per_model, status);
        /* TODO Please clarify this statement and explain why it's needed. */
        /* Set the number sources to zero as this is the number currently
         * allocated in the model. */
        set[i]->num_sources = 0;
    }

    if (*status) return;

    /* Copy sources from the model into the set. */
    /* Loop over set entries with free space and copy sources into them. */
    number_to_copy = model->num_sources;
    from_offset = 0;
    for (i = (*set_size-1 > 0) ? *set_size-1 : 0; i < *set_size + num_extra_models; ++i)
    {
        oskar_Sky* sky = set[i];
        n = sky->num_sources;
        free_space = max_sources_per_model - n;
        n_copy = MIN(free_space, number_to_copy);
        oskar_sky_get_ptr(&model_ptr, model, from_offset, n_copy, status);
        oskar_sky_insert(sky, &model_ptr, n, status);
        if (*status) break;
        sky->num_sources = n_copy + n;
        number_to_copy  -= n_copy;
        from_offset     += n_copy;
    }

    if (*status) return;

#if !(defined(OSKAR_NO_CBLAS) || defined(OSKAR_NO_LAPACK))
    /* Set the use extended flag if needed. */
    for (j = (*set_size-1 > 0) ? *set_size-1 : 0; j < *set_size + num_extra_models; ++j)
    {
        oskar_Sky* sky = set[j];
        const oskar_Mem *major, *minor;

        /* If any source in the model is extended, set the flag. */
        major = oskar_sky_fwhm_major_const(sky);
        minor = oskar_sky_fwhm_minor_const(sky);
        if (type == OSKAR_DOUBLE)
        {
            const double *maj_, *min_;
            maj_ = oskar_mem_double_const(major, status);
            min_ = oskar_mem_double_const(minor, status);
            for (i = 0; i < sky->num_sources; ++i)
            {
                if (maj_[i] > 0.0 || min_[i] > 0.0)
                {
                    sky->use_extended = OSKAR_TRUE;
                    break;
                }
            }
        }
        else
        {
            const float *maj_, *min_;
            maj_ = oskar_mem_float_const(major, status);
            min_ = oskar_mem_float_const(minor, status);
            for (i = 0; i < sky->num_sources; ++i)
            {
                if (maj_[i] > 0.0 || min_[i] > 0.0)
                {
                    sky->use_extended = OSKAR_TRUE;
                    break;
                }
            }
        }

    }
#endif

    /* Update the set size */
    *set_size += num_extra_models;
}

#ifdef __cplusplus
}
#endif
