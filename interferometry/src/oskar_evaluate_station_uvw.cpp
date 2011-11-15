/*
 * Copyright (c) 2011, The University of Oxford
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

#include "interferometry/oskar_evaluate_station_uvw.h"
#include "interferometry/oskar_telescope_model_location.h"
#include "interferometry/oskar_telescope_model_type.h"
#include "interferometry/oskar_xyz_to_uvw_cuda.h"
#include "interferometry/oskar_xyz_to_uvw.h"

extern "C"
int oskar_evaluate_station_uvw(oskar_Mem* u, oskar_Mem* v, oskar_Mem* w,
        const oskar_TelescopeModel* telescope, double gast)
{
    int type, location, num_stations, error = 0;

    // Assert that the parameters are not NULL.
    if (u == NULL || v == NULL || w == NULL || telescope == NULL)
        return OSKAR_ERR_INVALID_ARGUMENT;

    // Get data type, location and size of the telescope structure.
    type = oskar_telescope_model_type(telescope);
    location = oskar_telescope_model_location(telescope);
    num_stations = telescope->num_stations;

    // Check that the memory is not NULL.
    if (u->is_null() || v->is_null() || w->is_null() ||
            telescope->station_x.is_null() ||
            telescope->station_y.is_null() ||
            telescope->station_z.is_null())
        return OSKAR_ERR_MEMORY_NOT_ALLOCATED;

    // Check that the data dimensions are OK.
    if (u->num_elements() < num_stations ||
            v->num_elements() < num_stations ||
            w->num_elements() < num_stations ||
            telescope->station_x.num_elements() < num_stations ||
            telescope->station_y.num_elements() < num_stations ||
            telescope->station_z.num_elements() < num_stations)
        return OSKAR_ERR_DIMENSION_MISMATCH;

    // Check that the data is in the right location.
    if (u->location() != location || v->location() != location ||
            w->location() != location)
        return OSKAR_ERR_BAD_LOCATION;

    // Check that the data is of the right type.
    if (u->type() != type || v->type() != type || w->type() != type)
        return OSKAR_ERR_TYPE_MISMATCH;

    // Evaluate Greenwich Hour Angle of phase centre.
    const double ha0 = gast - telescope->ra0;
    const double dec0 = telescope->dec0;

    // Evaluate station u,v,w coordinates.
    if (location == OSKAR_LOCATION_GPU)
    {
        if (type == OSKAR_SINGLE)
        {
            error = oskar_xyz_to_uvw_cuda_f(num_stations, telescope->station_x,
                    telescope->station_y, telescope->station_z, (float)ha0,
                    (float)dec0, *u, *v, *w);
        }
        else if (type == OSKAR_DOUBLE)
        {
            error = oskar_xyz_to_uvw_cuda_d(num_stations, telescope->station_x,
                    telescope->station_y, telescope->station_z, ha0, dec0,
                    *u, *v, *w);
        }
        else
        {
            return OSKAR_ERR_BAD_DATA_TYPE;
        }
    }
    else if (location == OSKAR_LOCATION_CPU)
    {
        if (type == OSKAR_SINGLE)
        {
            oskar_xyz_to_uvw_f(num_stations, telescope->station_x,
                    telescope->station_y, telescope->station_z, (float)ha0,
                    (float)dec0, *u, *v, *w);
        }
        else if (type == OSKAR_DOUBLE)
        {
            oskar_xyz_to_uvw_d(num_stations, telescope->station_x,
                    telescope->station_y, telescope->station_z, ha0, dec0,
                    *u, *v, *w);
        }
        else
        {
            return OSKAR_ERR_BAD_DATA_TYPE;
        }
    }
    else
    {
        return OSKAR_ERR_BAD_LOCATION;
    }

    return error;
}