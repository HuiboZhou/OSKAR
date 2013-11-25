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

#ifndef OSKAR_TELESCOPE_ACCESSORS_H_
#define OSKAR_TELESCOPE_ACCESSORS_H_

/**
 * @file oskar_telescope_accessors.h
 */

#include <oskar_global.h>
#include <oskar_mem.h>
#include <oskar_station.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Properties and metadata. */

/**
 * @brief
 * Returns the numerical precision of data stored in the telescope model.
 *
 * @details
 * Returns the numerical precision of data stored in the telescope model.
 *
 * @param[in] model   Pointer to telescope model.
 *
 * @return The data type (OSKAR_SINGLE or OSKAR_DOUBLE).
 */
OSKAR_EXPORT
int oskar_telescope_precision(const oskar_Telescope* model);

/**
 * @brief
 * Returns the memory location of data stored in the telescope model.
 *
 * @details
 * Returns the memory location of data stored in the telescope model.
 *
 * @param[in] model   Pointer to telescope model.
 *
 * @return The memory location (OSKAR_LOCATION_CPU or OSKAR_LOCATION_GPU).
 */
OSKAR_EXPORT
int oskar_telescope_location(const oskar_Telescope* model);

/**
 * @brief
 * Returns the longitude of the telescope centre.
 *
 * @details
 * Returns the geodetic longitude of the interferometer centre in radians.
 *
 * @param[in] model   Pointer to telescope model.
 *
 * @return The longitude in radians.
 */
OSKAR_EXPORT
double oskar_telescope_longitude_rad(const oskar_Telescope* model);

/**
 * @brief
 * Returns the latitude of the telescope centre.
 *
 * @details
 * Returns the geodetic latitude of the interferometer centre in radians.
 *
 * @param[in] model   Pointer to telescope model.
 *
 * @return The latitude in radians.
 */
OSKAR_EXPORT
double oskar_telescope_latitude_rad(const oskar_Telescope* model);

/**
 * @brief
 * Returns the altitude of the telescope centre.
 *
 * @details
 * Returns the altitude of the interferometer centre above the ellipsoid,
 * in metres.
 *
 * @param[in] model   Pointer to telescope model.
 *
 * @return The altitude in metres.
 */
OSKAR_EXPORT
double oskar_telescope_altitude_m(const oskar_Telescope* model);

/**
 * @brief
 * Returns the right ascension of the phase centre.
 *
 * @details
 * Returns the right ascension of the interferometer phase centre in radians.
 *
 * @param[in] model   Pointer to telescope model.
 *
 * @return The right ascension in radians.
 */
OSKAR_EXPORT
double oskar_telescope_ra0_rad(const oskar_Telescope* model);

/**
 * @brief
 * Returns the declination of the phase centre.
 *
 * @details
 * Returns the declination of the interferometer phase centre in radians.
 *
 * @param[in] model   Pointer to telescope model.
 *
 * @return The declination in radians.
 */
OSKAR_EXPORT
double oskar_telescope_dec0_rad(const oskar_Telescope* model);

/**
 * @brief
 * Returns the channel bandwidth in Hz.
 *
 * @details
 * Returns the channel bandwidth in Hz.
 *
 * @param[in] model   Pointer to telescope model.
 *
 * @return The channel bandwidth in Hz.
 */
OSKAR_EXPORT
double oskar_telescope_bandwidth_hz(const oskar_Telescope* model);

/**
 * @brief
 * Returns the time averaging interval in seconds.
 *
 * @details
 * Returns the time averaging interval in seconds.
 *
 * @param[in] model   Pointer to telescope model.
 *
 * @return The time averaging interval in seconds.
 */
OSKAR_EXPORT
double oskar_telescope_time_average_sec(const oskar_Telescope* model);

/**
 * @brief
 * Returns the number of unique baselines in the telescope model.
 *
 * @details
 * Returns the number of unique baselines in the telescope model.
 *
 * @param[in] model Pointer to telescope model.
 *
 * @return The number of baselines.
 */
OSKAR_EXPORT
int oskar_telescope_num_baselines(const oskar_Telescope* model);

/**
 * @brief
 * Returns the number of interferometer stations in the telescope model.
 *
 * @details
 * Returns the number of interferometer stations in the telescope model.
 *
 * @param[in] model Pointer to telescope model.
 *
 * @return The number of stations.
 */
OSKAR_EXPORT
int oskar_telescope_num_stations(const oskar_Telescope* model);

/**
 * @brief
 * Returns a flag to specify whether all stations are identical.
 *
 * @details
 * Returns a flag to specify whether all stations are identical.
 *
 * Note that this flag is only valid after calling
 * oskar_telescope_analyse().
 *
 * @param[in] model Pointer to telescope model.
 *
 * @return True if all stations are identical; false if not.
 */
OSKAR_EXPORT
int oskar_telescope_identical_stations(const oskar_Telescope* model);

/**
 * @brief
 * Returns the flag specifying whether all stations share a common horizon.
 *
 * @details
 * Returns the flag specifying whether all stations share a common horizon.
 *
 * @param[in] model   Pointer to telescope model.
 *
 * @return The boolean flag value.
 */
OSKAR_EXPORT
int oskar_telescope_common_horizon(const oskar_Telescope* model);

/**
 * @brief
 * Returns the maximum number of elements in a station.
 *
 * @details
 * Returns the maximum number of elements in a station.
 *
 * @param[in] model   Pointer to telescope model.
 *
 * @return The maximum number of elements in a station.
 */
OSKAR_EXPORT
int oskar_telescope_max_station_size(const oskar_Telescope* model);

/**
 * @brief
 * Returns the maximum beamforming hierarchy depth.
 *
 * @details
 * Returns the maximum beamforming hierarchy depth.
 *
 * @param[in] model   Pointer to telescope model.
 *
 * @return The maximum beamforming hierarchy depth.
 */
OSKAR_EXPORT
int oskar_telescope_max_station_depth(const oskar_Telescope* model);

/**
 * @brief
 * Returns the random seed for time-variable errors.
 *
 * @details
 * Returns the random seed for time-variable errors.
 *
 * @param[in] model   Pointer to telescope model.
 *
 * @return The random seed.
 */
OSKAR_EXPORT
int oskar_telescope_random_seed(const oskar_Telescope* model);


/* Station models. */

/**
 * @brief
 * Returns a handle to a station model at the given index.
 *
 * @details
 * Returns a handle to a station model at the given index.
 *
 * @param[in] model Pointer to telescope model.
 * @param[in] i     The station model index.
 *
 * @return A handle to a station model.
 */
OSKAR_EXPORT
oskar_Station* oskar_telescope_station(oskar_Telescope* model, int i);

/**
 * @brief
 * Returns a constant handle to a station model at the given index.
 *
 * @details
 * Returns a constant handle to a station model at the given index.
 *
 * @param[in] model Pointer to telescope model.
 * @param[in] i     The station model index.
 *
 * @return A constant handle to a station model.
 */
OSKAR_EXPORT
const oskar_Station* oskar_telescope_station_const(
        const oskar_Telescope* model, int i);


/* Coordinate arrays. */

/**
 * @brief
 * Returns a handle to the station x positions.
 *
 * @details
 * Returns a handle to the station x positions.
 *
 * @param[in] model Pointer to telescope model.
 *
 * @return A handle to the station x positions.
 */
OSKAR_EXPORT
oskar_Mem* oskar_telescope_station_x(oskar_Telescope* model);

/**
 * @brief
 * Returns a constant handle to the station x positions.
 *
 * @details
 * Returns a constant handle to the station x positions.
 *
 * @param[in] model Pointer to telescope model.
 *
 * @return A constant handle to the station x positions.
 */
OSKAR_EXPORT
const oskar_Mem* oskar_telescope_station_x_const(const oskar_Telescope* model);

/**
 * @brief
 * Returns a handle to the station y positions.
 *
 * @details
 * Returns a handle to the station y positions.
 *
 * @param[in] model Pointer to telescope model.
 *
 * @return A handle to the station y positions.
 */
OSKAR_EXPORT
oskar_Mem* oskar_telescope_station_y(oskar_Telescope* model);

/**
 * @brief
 * Returns a constant handle to the station y positions.
 *
 * @details
 * Returns a constant handle to the station y positions.
 *
 * @param[in] model Pointer to telescope model.
 *
 * @return A constant handle to the station y positions.
 */
OSKAR_EXPORT
const oskar_Mem* oskar_telescope_station_y_const(const oskar_Telescope* model);

/**
 * @brief
 * Returns a handle to the station z positions.
 *
 * @details
 * Returns a handle to the station z positions.
 *
 * @param[in] model Pointer to telescope model.
 *
 * @return A handle to the station z positions.
 */
OSKAR_EXPORT
oskar_Mem* oskar_telescope_station_z(oskar_Telescope* model);

/**
 * @brief
 * Returns a constant handle to the station z positions.
 *
 * @details
 * Returns a constant handle to the station z positions.
 *
 * @param[in] model Pointer to telescope model.
 *
 * @return A constant handle to the station z positions.
 */
OSKAR_EXPORT
const oskar_Mem* oskar_telescope_station_z_const(const oskar_Telescope* model);

/**
 * @brief
 * Returns a handle to the horizon plane station x positions.
 *
 * @details
 * Returns a handle to the horizon plane station x positions.
 *
 * @param[in] model Pointer to telescope model.
 *
 * @return A handle to the horizon plane station x positions.
 */
OSKAR_EXPORT
oskar_Mem* oskar_telescope_station_x_hor(oskar_Telescope* model);

/**
 * @brief
 * Returns a constant handle to the horizon plane station x positions.
 *
 * @details
 * Returns a constant handle to the horizon plane station x positions.
 *
 * @param[in] model Pointer to telescope model.
 *
 * @return A constant handle to the horizon plane station x positions.
 */
OSKAR_EXPORT
const oskar_Mem* oskar_telescope_station_x_hor_const(
        const oskar_Telescope* model);

/**
 * @brief
 * Returns a handle to the horizon plane station y positions.
 *
 * @details
 * Returns a handle to the horizon plane station y positions.
 *
 * @param[in] model Pointer to telescope model.
 *
 * @return A handle to the horizon plane station y positions.
 */
OSKAR_EXPORT
oskar_Mem* oskar_telescope_station_y_hor(oskar_Telescope* model);

/**
 * @brief
 * Returns a constant handle to the horizon plane station y positions.
 *
 * @details
 * Returns a constant handle to the horizon plane station y positions.
 *
 * @param[in] model Pointer to telescope model.
 *
 * @return A constant handle to the horizon plane station y positions.
 */
OSKAR_EXPORT
const oskar_Mem* oskar_telescope_station_y_hor_const(
        const oskar_Telescope* model);

/**
 * @brief
 * Returns a handle to the horizon plane station z positions.
 *
 * @details
 * Returns a handle to the horizon plane station z positions.
 *
 * @param[in] model Pointer to telescope model.
 *
 * @return A handle to the horizon plane station z positions.
 */
OSKAR_EXPORT
oskar_Mem* oskar_telescope_station_z_hor(oskar_Telescope* model);

/**
 * @brief
 * Returns a constant handle to the horizon plane station z positions.
 *
 * @details
 * Returns a constant handle to the horizon plane station z positions.
 *
 * @param[in] model Pointer to telescope model.
 *
 * @return A constant handle to the horizon plane station z positions.
 */
OSKAR_EXPORT
const oskar_Mem* oskar_telescope_station_z_hor_const(
        const oskar_Telescope* model);


/* Setters. */

/**
 * @brief
 * Sets the geographic coordinates of the telescope centre.
 *
 * @details
 * Sets the longitude, latitude and altitude of the interferometer centre.
 *
 * @param[in] model          Pointer to telescope model.
 * @param[in] longitude_rad  East-positive longitude, in radians.
 * @param[in] latitude_rad   North-positive geodetic latitude in radians.
 * @param[in] altitude_m     Altitude above ellipsoid in metres.
 */
OSKAR_EXPORT
void oskar_telescope_set_position(oskar_Telescope* model,
        double longitude_rad, double latitude_rad, double altitude_m);

/**
 * @brief
 * Sets the coordinates of the phase centre.
 *
 * @details
 * Sets the right ascension and declination of the interferometer phase centre.
 *
 * @param[in] model   Pointer to telescope model.
 * @param[in] ra_rad  Right ascension in radians.
 * @param[in] dec_rad Declination in radians.
 */
OSKAR_EXPORT
void oskar_telescope_set_phase_centre(oskar_Telescope* model,
        double ra_rad, double dec_rad);

/**
 * @brief
 * Sets the values of the smearing parameters.
 *
 * @details
 * Sets the values of the smearing parameters.
 *
 * @param[in] model            Pointer to telescope model.
 * @param[in] bandwidth_hz     Channel bandwidth, in Hz.
 * @param[in] time_average_sec Time averaging interval, in seconds.
 */
OSKAR_EXPORT
void oskar_telescope_set_smearing_values(oskar_Telescope* model,
        double bandwidth_hz, double time_average_sec);

/**
 * @brief
 * Sets the flag to specify whether stations should share a common horizon.
 *
 * @details
 * Sets the flag to specify whether stations should share a common horizon.
 *
 * @param[in] model    Pointer to telescope model.
 * @param[in] value    If true, stations will share common source positions.
 */
OSKAR_EXPORT
void oskar_telescope_set_common_horizon(oskar_Telescope* model, int value);

/**
 * @brief
 * Sets the random seed used for time-variable station element errors.
 *
 * @details
 * Sets the random seed used for time-variable station element errors.
 *
 * @param[in] model    Pointer to telescope model.
 * @param[in] value    Random seed value.
 */
OSKAR_EXPORT
void oskar_telescope_set_random_seed(oskar_Telescope* model, int value);

#ifdef __cplusplus
}
#endif

#endif /* OSKAR_TELESCOPE_ACCESSORS_H_ */
