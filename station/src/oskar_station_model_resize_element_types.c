/*
 * Copyright (c) 2013, The University of Oxford
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

#include "station/oskar_element_model_init.h"
#include "station/oskar_station_model_resize_element_types.h"
#include "station/oskar_station_model_type.h"
#include "station/oskar_station_model_location.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void oskar_station_model_resize_element_types(oskar_StationModel* model,
        int num_element_types, int* status)
{
    void* ptr_temp;
    int i, num_element_types_old, type, location;

    /* Check all inputs. */
    if (!model || !status)
    {
        oskar_set_invalid_argument(status);
        return;
    }

    /* Check if safe to proceed. */
    if (*status) return;

    /* Store the new number of element types. */
    if (model->num_element_types == num_element_types)
        return;
    num_element_types_old = model->num_element_types;
    model->num_element_types = num_element_types;

    /* Resize the element model array. */
    ptr_temp = realloc(model->element_pattern,
            num_element_types * sizeof(oskar_ElementModel));
    if (num_element_types != 0 && !ptr_temp)
    {
        *status = OSKAR_ERR_MEMORY_ALLOC_FAILURE;
        return;
    }

    /* Store the new pointer. */
    model->element_pattern = (oskar_ElementModel*) ptr_temp;

    /* Initialise any new element models. */
    type = oskar_station_model_type(model);
    location = oskar_station_model_location(model);
    for (i = num_element_types_old; i < num_element_types; ++i)
    {
        oskar_element_model_init(&model->element_pattern[i], type, location,
                status);
    }
}

#ifdef __cplusplus
}
#endif
