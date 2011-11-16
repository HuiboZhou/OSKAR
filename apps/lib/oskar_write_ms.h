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

#ifndef OSKAR_WRITE_MS_H_
#define OSKAR_WRITE_MS_H_


/**
 * @file oskar_write_ms.h
 */

#include "oskar_global.h"
#include "apps/lib/oskar_Settings.h"
#include "interferometry/oskar_Visibilities.h"

#include "interferometry/oskar_VisData.h" // DEPRECATED

#ifdef __cplusplus
extern "C" {
#endif

OSKAR_EXPORT
int oskar_write_ms(const char* ms_path, const oskar_Visibilities* vis,
        const oskar_Settings* settings, int overwrite);



// DEPRECATED
OSKAR_EXPORT
void oskar_write_ms_d(const char* ms_path, const oskar_Settings* settings,
        const oskar_VisData_d* vis, const unsigned channel,
        const bool overwrite = true);


// DEPRECATED
OSKAR_EXPORT
void oskar_write_ms_f(const char* ms_path, const oskar_Settings* settings,
        const oskar_VisData_f* vis, const unsigned channel,
        const bool overwrite = true);


#ifdef __cplusplus
}
#endif

#endif // OSKAR_WRITE_MS_H_
