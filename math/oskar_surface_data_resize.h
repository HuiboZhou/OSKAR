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

#ifndef OSKAR_SURFACE_DATA_RESIZE_H_
#define OSKAR_SURFACE_DATA_RESIZE_H_

/**
 * @file oskar_surface_data_resize.h
 */

#include "oskar_global.h"
#include "math/oskar_SurfaceData.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 * Resizes memory arrays in a surface data structure.
 *
 * @details
 * This function resizes memory arrays in a surface data structure.
 *
 * @param[in,out] data Pointer to data structure.
 * @param[in] size New size of arrays.
 *
 * @return
 * This function returns a code to indicate if there were errors in execution:
 * - A return code of 0 indicates no error.
 * - A positive return code indicates a CUDA error.
 * - A negative return code indicates an OSKAR error.
 */
OSKAR_EXPORT
int oskar_surface_data_resize(oskar_SurfaceData* data, int size);

#ifdef __cplusplus
}
#endif

#endif /* OSKAR_SURFACE_DATA_RESIZE_H_ */
