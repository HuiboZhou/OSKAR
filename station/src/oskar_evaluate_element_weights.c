/*
 * Copyright (c) 2012, The University of Oxford
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

#include "station/oskar_evaluate_element_weights.h"
#include "station/oskar_evaluate_element_weights_dft.h"
#include "station/oskar_evaluate_element_weights_errors.h"
#include "utility/oskar_mem_element_multiply.h"
#include "utility/oskar_mem_realloc.h"

#ifdef __cplusplus
extern "C" {
#endif

void oskar_evaluate_element_weights(oskar_Mem* weights,
        oskar_Mem* weights_error, const oskar_StationModel* station,
        double x_beam, double y_beam, double z_beam,
        oskar_CurandState* curand_state, int* status)
{
    int num_elements;

    /* Check all inputs. */
    if (!weights || !weights_error || !station || !status)
    {
        oskar_set_invalid_argument(status);
        return;
    }

    /* Check if safe to proceed. */
    if (*status) return;

    /* Resize weights and weights error work arrays if required. */
    num_elements = station->num_elements;
    if (weights->num_elements < num_elements)
        oskar_mem_realloc(weights, num_elements, status);
    if (weights_error->num_elements < num_elements)
        oskar_mem_realloc(weights_error, num_elements, status);

    /* Generate DFT weights. */
    oskar_evaluate_element_weights_dft(weights, num_elements,
            &station->x_weights, &station->y_weights, &station->z_weights,
            x_beam, y_beam, z_beam, status);

    /* Apply time-variable errors. */
    if (station->apply_element_errors)
    {
        /* Generate weights errors. */
        oskar_evaluate_element_weights_errors(weights_error,
                num_elements, &station->gain, &station->gain_error,
                &station->phase_offset, &station->phase_error,
                curand_state, status);

        /* Modify the weights (complex multiply with error vector). */
        oskar_mem_element_multiply(0, weights, weights_error,
                num_elements, status);
    }

    /* Modify the weights using the provided apodisation values. */
    if (station->apply_element_weight)
        oskar_mem_element_multiply(0, weights, &station->weight,
                num_elements, status);
}

#ifdef __cplusplus
}
#endif