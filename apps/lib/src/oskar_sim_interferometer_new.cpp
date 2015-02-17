/*
 * Copyright (c) 2011-2015, The University of Oxford
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

#include <cuda_runtime_api.h>

#include <oskar_sim_interferometer_new.h>

#include <oskar_settings_load.h>
#include <oskar_settings_log.h>

#include <oskar_set_up_sky.h>
#include <oskar_set_up_telescope.h>
#include <oskar_set_up_vis.h>
#include <oskar_vis_write_ms.h>

#include <oskar_correlate.h>
#include <oskar_cuda_mem_log.h>
#include <oskar_evaluate_jones_R.h>
#include <oskar_evaluate_jones_Z.h>
#include <oskar_evaluate_jones_E.h>
#include <oskar_evaluate_jones_K.h>
#include <oskar_convert_ecef_to_station_uvw.h>
#include <oskar_convert_ecef_to_baseline_uvw.h>
#include <oskar_log.h>
#include <oskar_jones.h>
#include <oskar_convert_mjd_to_gast_fast.h>
#include <oskar_settings_free.h>
#include <oskar_sky.h>
#include <oskar_telescope.h>
#include <oskar_timers.h>
#include <oskar_timer.h>
#include <oskar_vis.h>
#include <oskar_vis_block.h>
#include <oskar_vis_block_write_ms.h>
#include <oskar_vis_header_write_ms.h>
#include <oskar_vis_header.h>
#include <oskar_station_work.h>

#include <oskar_round_robin.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include <cstdlib>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <cstdarg>

/* Per GPU memory and other meta-data */
struct DeviceData
{
    /* Host memory */
    oskar_VisBlock* vis_block_cpu; /* Vis block on host for copy back & write */

    /* Device memory */
    oskar_VisBlock* vis_block[2];  /* Double buffered device memory vis blocks */
    oskar_Mem *u, *v, *w;
    oskar_Sky* sky_chunk; /* The unmodified sky chunk currently being processed */
    oskar_Sky* local_sky; /* A copy of the sky chunk that has been horizon clipped */
    oskar_Telescope* tel; /* A copy of the telescope model */
    oskar_Jones *J, *R, *E, *K, *Z; /* Jones matrices */
    oskar_StationWork* station_work;

    /* Timers */
    oskar_Timers timers;
};

// TODO mmm not sure this is the best name as it also contains a work array
struct OutputHandles
{
    oskar_VisHeader* header;
    oskar_MeasurementSet* ms;
    oskar_Binary* vis;

    oskar_Mem* station_work; // work array for noise addition.
    oskar_Mem* work_uvw;
};

///////////////////////////////////////////////////////////////////////////////

static void record_timing_(int num_devices, int* cuda_device_ids,
        DeviceData* dd, oskar_Log* log)
{
    double elapsed, t_init = 0.0, t_clip = 0.0, t_R = 0.0, t_E = 0.0, t_K = 0.0;
    double t_join = 0.0, t_correlate = 0.0;

    // Record time taken.
    cudaSetDevice(cuda_device_ids[0]);
    elapsed = oskar_timer_elapsed(dd[0].timers.tmr);
    oskar_log_section(log, 'M', "Simulation completed in %.3f sec.", elapsed);

    // Record percentage times.
    for (int i = 0; i < num_devices; ++i) {
        cudaSetDevice(cuda_device_ids[i]);
        t_init += oskar_timer_elapsed(dd[i].timers.tmr_init_copy);
        t_clip += oskar_timer_elapsed(dd[i].timers.tmr_clip);
        t_R += oskar_timer_elapsed(dd[i].timers.tmr_R);
        t_E += oskar_timer_elapsed(dd[i].timers.tmr_E);
        t_K += oskar_timer_elapsed(dd[i].timers.tmr_K);
        t_join += oskar_timer_elapsed(dd[i].timers.tmr_join);
        t_correlate += oskar_timer_elapsed(dd[i].timers.tmr_correlate);
    }
    t_init *= (100.0 / (num_devices * elapsed));
    t_clip *= (100.0 / (num_devices * elapsed));
    t_R *= (100.0 / (num_devices * elapsed));
    t_E *= (100.0 / (num_devices * elapsed));
    t_K *= (100.0 / (num_devices * elapsed));
    t_join *= (100.0 / (num_devices * elapsed));
    t_correlate *= (100.0 / (num_devices * elapsed));
    // Using depth = -1 for a line without a bullet.
    oskar_log_message(log, 'M', -1, "%6.1f%% Chunk copy & initialise.", t_init);
    oskar_log_message(log, 'M', -1, "%6.1f%% Horizon clip.", t_clip);
    oskar_log_message(log, 'M', -1, "%6.1f%% Jones R.", t_R);
    oskar_log_message(log, 'M', -1, "%6.1f%% Jones E.", t_E);
    oskar_log_message(log, 'M', -1, "%6.1f%% Jones K.", t_K);
    oskar_log_message(log, 'M', -1, "%6.1f%% Jones join.", t_join);
    oskar_log_message(log, 'M', -1, "%6.1f%% Jones correlate.", t_correlate);
    oskar_log_line(log, 'M', ' ');
}

static void log_warning_box_(oskar_Log* log, const char* format, ...)
{
    size_t max_len = 55; // Controls the width of the box

    char buf[5000];
    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    std::string msg(buf);
    std::istringstream ss(msg);
    std::string word, line;
    oskar_log_line(log, 'W', ' ');
    oskar_log_line(log, 'W', '*');
    while (std::getline(ss, word, ' ')) {
        if (line.length() > 0)
            line += std::string(1, ' ');
        if ((line.length() + word.length() + 4) >= max_len) {
            int pad = max_len - line.length() - 1;
            int pad_l = (pad / 2) > 1 ? (pad / 2) : 1;
            int pad_r = (pad / 2) > 0 ? (pad / 2) : 0;
            if (pad % 2 == 0)
                pad_r -= 1;
            line = "!" + std::string(pad_l, ' ') + line;
            line += std::string(pad_r, ' ') + "!";
            oskar_log_message(log, 'W', -1, "%s", line.c_str());
            line.clear();
        }
        line += word;
    }
    int pad = max_len - line.length() - 1;
    int pad_l = (pad / 2) > 1 ? (pad / 2) : 1;
    int pad_r = (pad / 2) > 0 ? (pad / 2) : 0;
    if (pad % 2 == 0)
        pad_r -= 1;
    line = "!" + std::string(pad_l, ' ') + line;
    line += std::string(pad_r, ' ') + "!";
    oskar_log_message(log, 'W', -1, "%s", line.c_str());
    oskar_log_line(log, 'W', '*');
    oskar_log_line(log, 'W', ' ');
}

static void set_up_device_data_(DeviceData* d, const oskar_Settings* s,
        const oskar_Telescope* tel, int max_sources_per_chunk,
        int num_times, int* status)
{
    /* Obtain local variables with data dimensions */
    int num_stations        = oskar_telescope_num_stations(tel);
    int num_src             = max_sources_per_chunk;
    int num_times_per_block = num_times;
    int num_channels        = s->obs.num_channels;

    /* Obtain local variable for data type */
    int prec    = oskar_telescope_precision(tel);
    int complx  = prec | OSKAR_COMPLEX;
    int vistype = complx;
    if (oskar_telescope_pol_mode(tel) == OSKAR_POL_MODE_FULL)
        vistype |= OSKAR_MATRIX;

    /* Host memory */
    d->vis_block_cpu = oskar_vis_block_create(vistype, OSKAR_CPU,
            num_times_per_block, num_channels, num_stations, status);

    /* Device memory */
    d->vis_block[0] = oskar_vis_block_create(vistype, OSKAR_GPU,
            num_times_per_block, num_channels, num_stations, status);
    d->vis_block[1] = oskar_vis_block_create(vistype, OSKAR_GPU,
            num_times_per_block, num_channels, num_stations, status);
    d->u = oskar_mem_create(prec, OSKAR_GPU, num_stations, status);
    d->v = oskar_mem_create(prec, OSKAR_GPU, num_stations, status);
    d->w = oskar_mem_create(prec, OSKAR_GPU, num_stations, status);

    d->sky_chunk = oskar_sky_create(prec, OSKAR_GPU, num_src, status);
    d->local_sky = oskar_sky_create(prec, OSKAR_GPU, num_src, status);
    d->tel = oskar_telescope_create_copy(tel, OSKAR_GPU, status);
    d->J = oskar_jones_create(vistype, OSKAR_GPU, num_stations, num_src, status);
    if (oskar_mem_type_is_matrix(vistype)) {
        d->R = oskar_jones_create(vistype, OSKAR_GPU, num_stations, num_src,
                status);
    } else {
        d->R = 0;
    }
    d->E = oskar_jones_create(vistype, OSKAR_GPU, num_stations, num_src, status);
    d->K = oskar_jones_create(complx, OSKAR_GPU, num_stations, num_src, status);
    d->Z = 0;
    d->station_work = oskar_station_work_create(prec, OSKAR_GPU, status);

    /* Timers */
    oskar_timers_create(&d->timers, OSKAR_TIMER_CUDA);
}

static void free_device_data_(DeviceData* d, int* status)
{
    oskar_vis_block_free(d->vis_block_cpu, status);
    oskar_vis_block_free(d->vis_block[0], status);
    oskar_vis_block_free(d->vis_block[1], status);
    oskar_mem_free(d->u, status);
    oskar_mem_free(d->v, status);
    oskar_mem_free(d->w, status);
    oskar_sky_free(d->sky_chunk, status);
    oskar_sky_free(d->local_sky, status);
    oskar_telescope_free(d->tel, status);
    oskar_station_work_free(d->station_work, status);
    oskar_jones_free(d->J, status);
    oskar_jones_free(d->E, status);
    oskar_jones_free(d->K, status);
    oskar_timers_free(&d->timers);
}

/*
 * Simulates one slice of a visibility block
 */
static void simulate_baselines_(DeviceData* d, oskar_Sky* sky,
        const oskar_Settings* settings, int channel_index_block,
        int time_index_block, int time_index_simulation, int active_vis_block,
        int* status)
{
    // Get a handle to the active visibility block.
    oskar_VisBlock* blk = d->vis_block[active_vis_block];

    // Get dimensions dimensions.
    int num_baselines   = oskar_telescope_num_baselines(d->tel);
    int num_stations    = oskar_telescope_num_stations(d->tel);
    int num_src         = oskar_sky_num_sources(sky);
    int num_times_block = oskar_vis_block_num_times(blk);
    int num_channels    = oskar_vis_block_num_channels(blk);

    /* Return if the block time index requested is outside the valid range */
    if (time_index_block >= num_times_block)
        return;

    // Get the time and frequency of the visibility slice being simulated.
    double start_time_block = oskar_vis_block_time_start_mjd_utc_sec(blk);
    double end_time_block = oskar_vis_block_time_end_mjd_utc_sec(blk);
    double time_inc_block = (end_time_block-start_time_block)/(double)num_times_block;
    double t_mjd_sec = start_time_block + time_index_block * time_inc_block;
    double t_mjd = t_mjd_sec / 86400.0;
    // TODO check times
    double gast = oskar_convert_mjd_to_gast_fast(t_mjd + time_inc_block/2.0);

    double start_freq = oskar_vis_block_freq_start_hz(blk);
    double end_freq   = oskar_vis_block_freq_end_hz(blk);
    double freq_inc   = (end_freq-start_freq)/(double)num_channels;
    double frequency  = start_freq + channel_index_block * freq_inc;

    // Scale sky fluxes with spectral index and rotation measure.
    oskar_sky_scale_flux_with_frequency(d->local_sky, frequency, status);

    // Pull pointers out for this time and channel.
    oskar_Mem* u = oskar_mem_create_alias(
            oskar_vis_block_baseline_uu_metres(blk),
            num_baselines * time_index_block, num_baselines, status);
    oskar_Mem* v = oskar_mem_create_alias(
            oskar_vis_block_baseline_vv_metres(blk),
            num_baselines * time_index_block, num_baselines, status);
    oskar_Mem* w = oskar_mem_create_alias(
            oskar_vis_block_baseline_ww_metres(blk),
            num_baselines * time_index_block, num_baselines, status);
    oskar_Mem* amp = oskar_mem_create_alias(oskar_vis_block_amplitude(blk),
            num_baselines * (num_channels * time_index_block + channel_index_block),
            num_baselines, status);

    // Evaluate station u,v,w coordinates.
    const oskar_Mem *x, *y, *z;
    double ra0 = oskar_telescope_phase_centre_ra_rad(d->tel);
    double dec0 = oskar_telescope_phase_centre_dec_rad(d->tel);
    x = oskar_telescope_station_true_x_offset_ecef_metres_const(d->tel);
    y = oskar_telescope_station_true_y_offset_ecef_metres_const(d->tel);
    z = oskar_telescope_station_true_z_offset_ecef_metres_const(d->tel);
    oskar_convert_ecef_to_station_uvw(num_stations, x, y, z, ra0, dec0, gast,
            d->u, d->v, d->w, status);

    // Set dimensions of Jones matrices (this is not a resize!).
    if (d->R)
        oskar_jones_set_size(d->R, num_stations, num_src, status);
    if (d->Z)
        oskar_jones_set_size(d->Z, num_stations, num_src, status);
    oskar_jones_set_size(d->J, num_stations, num_src, status);
    oskar_jones_set_size(d->E, num_stations, num_src, status);
    oskar_jones_set_size(d->K, num_stations, num_src, status);

    // Evaluate station beam (Jones E: may be matrix).
    // Note: The global time index is required for random number generators.
    oskar_timer_resume(d->timers.tmr_E);
    oskar_evaluate_jones_E(d->E, num_src, oskar_sky_l(sky), oskar_sky_m(sky),
            oskar_sky_n(sky), OSKAR_RELATIVE_DIRECTIONS,
            oskar_sky_reference_ra_rad(sky), oskar_sky_reference_dec_rad(sky),
            d->tel, gast, frequency, d->station_work, time_index_simulation, status);
    oskar_timer_pause(d->timers.tmr_E);

#if 0
    // Evaluate ionospheric phase screen (Jones Z: scalar),
    // and join with Jones E.
    // NOTE this is currently only a CPU implementation.
    if (d->Z)
    {
        oskar_evaluate_jones_Z(d->Z, num_src, sky, d->tel,
                &settings->ionosphere, gast, frequency, &(d->workJonesZ),
                status);
        oskar_timer_resume(d->timers.tmr_join);
        oskar_jones_join(d->E, d->Z, d->E, status);
        oskar_timer_pause(d->timers.tmr_join);
    }
#endif

    // Evaluate parallactic angle (Jones R: matrix),
    // and join with Jones Z*E.
    // TODO Move this into station beam evaluation instead.
    if (d->R) {
        oskar_timer_resume(d->timers.tmr_R);
        oskar_evaluate_jones_R(d->R, num_src, oskar_sky_ra_rad_const(sky),
                oskar_sky_dec_rad_const(sky), d->tel, gast, status);
        oskar_timer_pause(d->timers.tmr_R);
        oskar_timer_resume(d->timers.tmr_join);
        oskar_jones_join(d->R, d->E, d->R, status);
        oskar_timer_pause(d->timers.tmr_join);
    }

    // Evaluate interferometer phase (Jones K: scalar).
    oskar_timer_resume(d->timers.tmr_K);
    oskar_evaluate_jones_K(d->K, num_src, oskar_sky_l_const(sky),
            oskar_sky_m_const(sky), oskar_sky_n_const(sky), d->u, d->v, d->w, frequency,
            oskar_sky_I_const(sky), settings->sky.common_flux_filter_min_jy,
            settings->sky.common_flux_filter_max_jy, status);
    oskar_timer_pause(d->timers.tmr_K);

    // Join Jones K with Jones Z*E*R (if it exists),
    // otherwise with Jones Z*E
    oskar_timer_resume(d->timers.tmr_join);
    oskar_jones_join(d->J, d->K, d->R ? d->R : d->E, status);
    oskar_timer_pause(d->timers.tmr_join);

    // Correlate.
    oskar_timer_resume(d->timers.tmr_correlate);

    oskar_correlate(amp, num_src, d->J, sky, d->tel, d->u, d->v, d->w,
            gast, frequency, status);
    oskar_timer_pause(d->timers.tmr_correlate);


    // Free handles to aliased memory.
    oskar_mem_free(u, status);
    oskar_mem_free(v, status);
    oskar_mem_free(w, status);
    oskar_mem_free(amp, status);
}

static void sim_vis_block_(const oskar_Settings* settings, DeviceData* d,
        const oskar_Sky* const * sky_chunks, int total_chunks,
        int block_index, int iactive, int* status)
{
    /* Initialise the visibility block meta-data */
    oskar_VisBlock* vis_block = d->vis_block[iactive];
    oskar_mem_clear_contents(oskar_vis_block_amplitude(vis_block), status);
    int block_length = settings->interferometer.max_time_samples_per_block;
    double obs_start_mjd = settings->obs.start_mjd_utc;
    double dt_dump = settings->obs.dt_dump_days;
    int total_times = settings->obs.num_time_steps;
    int block_start_time_index = block_index*block_length;
    int block_end_time_index = block_start_time_index + block_length;
    if (block_end_time_index >= total_times)
        block_end_time_index = total_times-1;
    int num_times_block = block_end_time_index-block_start_time_index;
    double block_start_time_mjd = obs_start_mjd+block_start_time_index*dt_dump;
    double block_end_time_mjd   = obs_start_mjd+block_end_time_index*dt_dump;
    int num_channels = settings->obs.num_channels;
    double freq_inc = settings->obs.frequency_inc_hz;
    double start_freq = settings->obs.start_frequency_hz;
    double end_freq = start_freq + num_channels*freq_inc;
    // Set the number of active times in the block
    oskar_vis_block_set_num_times(vis_block, num_times_block, status);
    // Set the time range of the block (of active times slices only)
    oskar_vis_block_set_time_range_mjd_utc_sec(vis_block,
            block_start_time_mjd*86400.0, block_end_time_mjd*86400.0);
    // Set the active frequency range of the block.
    oskar_vis_block_set_freq_range_hz(vis_block, start_freq, end_freq);

    /* Get time and chunk counter ranges for different parallelisation modes */
    // TODO non openMP fall-back.
    int tid = omp_get_thread_num(); /* The current thread id */
    int gpuid = tid-1;              /* The GPU ID associated with this thread */
    int parallelisation_type = SPLIT_CHUNK; // TODO from settings
    int num_gpus = settings->sim.num_cuda_devices;
    int num_chunks, start_chunk, start_time, num_times;
    if (parallelisation_type == SPLIT_CHUNK) {
        start_time = 0;
        num_times = block_length;
        oskar_round_robin(total_chunks, num_gpus, gpuid, &num_chunks, &start_chunk);
    }
    else if (parallelisation_type == SPLIT_TIME) {
        num_chunks = total_chunks;
        start_chunk = 0;
        oskar_round_robin(block_length, num_gpus, gpuid, &num_times, &start_time);
    }

    if (*status) return;

    for (int c = start_chunk; c < (start_chunk+num_chunks); ++c)
    {
        // Copy the current sky chunk to the GPU.
        // NOTE this is potentially slightly wasteful as all relevant sky
        // chunks are copied to the GPU each each vis block.
        // Extra no. vis blocks memory copies of each sky chunk.
        oskar_sky_copy(d->sky_chunk, sky_chunks[c], status);

        for (int t = start_time; t < (start_time + num_times); ++t)
        {
            int time_idx = block_index * block_length + t;

            if (settings->sky.apply_horizon_clip) {
                double t_mjd = obs_start_mjd+(time_idx*dt_dump)+dt_dump/2.0;
                double gast = oskar_convert_mjd_to_gast_fast(t_mjd);
                oskar_sky_horizon_clip(d->local_sky, d->sky_chunk,
                        d->tel, gast, d->station_work, status);
            }

            for (int f = 0; f < num_channels; ++f)
            {
                if (*status) return;

                simulate_baselines_(d, d->local_sky, settings, f, t,
                        time_idx, iactive, status);
            }
        }
    }
    printf("status = %i, (%s:%i)\n", *status, __FILE__, __LINE__);
}


static void write_vis_block_(const oskar_Settings* settings,
        DeviceData* d, OutputHandles* out, const oskar_Telescope* telescope,
        int block_index, int iactive, int* status)
{
    // Copy back vis blocks from each GPU
    for (int i = 0; i < settings->sim.num_cuda_devices; ++i) {
        oskar_vis_block_copy(d[i].vis_block_cpu, d[i].vis_block[!iactive], status);
    }
    // Combine into all vis blocks into the block for GPU 0
    oskar_Mem* amp0 = oskar_vis_block_amplitude(d[0].vis_block_cpu);
    for (int i = 1; i < settings->sim.num_cuda_devices; ++i) {
        oskar_Mem* amp = oskar_vis_block_amplitude(d[i].vis_block_cpu);
        oskar_mem_add(amp0, amp0, amp, status);
    }

    // Set baseline uvw coordinates of vis block.
    if (block_index == 0) {
        int type = settings->sim.double_precision ? OSKAR_DOUBLE : OSKAR_SINGLE;
        int num_stations = oskar_telescope_num_stations(telescope);
        out->work_uvw = oskar_mem_create(type, OSKAR_CPU, 3 * num_stations, status);
    }
    double t_start = oskar_vis_block_time_start_mjd_utc_sec(d[0].vis_block_cpu);
    double t_end = oskar_vis_block_time_end_mjd_utc_sec(d[0].vis_block_cpu);
    int num_times = oskar_vis_block_num_times(d[0].vis_block_cpu);
    double dt_dump_sec = (t_end - t_start) / (double)num_times;
    oskar_convert_ecef_to_baseline_uvw(
            oskar_telescope_num_stations(telescope),
            oskar_telescope_station_true_x_offset_ecef_metres_const(telescope),
            oskar_telescope_station_true_y_offset_ecef_metres_const(telescope),
            oskar_telescope_station_true_z_offset_ecef_metres_const(telescope),
            oskar_telescope_phase_centre_ra_rad(telescope),
            oskar_telescope_phase_centre_dec_rad(telescope),
            num_times, t_start/86400.0, dt_dump_sec/86400.0,
            oskar_vis_block_baseline_uu_metres(d[0].vis_block_cpu),
            oskar_vis_block_baseline_vv_metres(d[0].vis_block_cpu),
            oskar_vis_block_baseline_ww_metres(d[0].vis_block_cpu),
            out->work_uvw, status);

    // Add uncorrelated system noise to the combined visibilities.
    if (settings->interferometer.noise.enable) {
        if (block_index == 0) {
            int type = settings->sim.double_precision ? OSKAR_DOUBLE : OSKAR_SINGLE;
            int num_stations = oskar_telescope_num_stations(telescope);
            out->station_work = oskar_mem_create(type, OSKAR_CPU, num_stations,
                    status);
        }
        int seed = settings->interferometer.noise.seed;
        oskar_vis_block_add_system_noise(d[0].vis_block_cpu,
                telescope, seed, block_index, out->station_work, status);
    }

    // Write the combined vis block into the MS.
#ifndef OSKAR_NO_MS
    const char* ms_name = settings->interferometer.ms_filename;
    if (ms_name && !*status) {
        if (block_index == 0) {
            printf("Writing ms block 0\n");
            bool overwrite = true;
            bool force_polarised = settings->interferometer.force_polarised_ms;
            out->ms = oskar_vis_header_write_ms(out->header, ms_name, overwrite,
                    force_polarised, status);
            printf("status = %i, (%s:%i)\n", *status, __FILE__, __LINE__);
        }
        oskar_vis_block_write_ms(d[0].vis_block_cpu, out->header, out->ms, status);
        printf("status = %i, (%s:%i)\n", *status, __FILE__, __LINE__);
    }
#endif
    // Write the combined vis block into the OSKAR vis binary file.
    const char* vis_name = settings->interferometer.oskar_vis_filename;
    if (vis_name && !*status) {
        if (block_index == 0) {
            printf("Writing OSKAR block 0\n");
            printf("status = %i, (%s:%i)\n", *status, __FILE__, __LINE__);
            out->vis = oskar_vis_header_write(out->header, vis_name, status);
            printf("status = %i, (%s:%i)\n", *status, __FILE__, __LINE__);
        }
        oskar_vis_block_write(d[0].vis_block_cpu, out->vis, block_index, status);
    }
    printf("status = %i, (%s:%i)\n", *status, __FILE__, __LINE__);
}

///////////////////////////////////////////////////////////////////////////////

extern "C" void oskar_sim_interferometer_new(const char* settings_file,
        oskar_Log* log, int* status)
{
    // Find out how many GPUs are in the system.
    int num_gpus_avail = 0;
    *status = (int)cudaGetDeviceCount(&num_gpus_avail);
    if (*status) return;

    // Load the settings file.
    oskar_Settings s;
    oskar_log_section(log, 'M', "Loading settings file '%s'", settings_file);
    oskar_settings_load(&s, log, settings_file, status);
    if (*status) return;

    // Log the relevant settings. (TODO fix/automate these functions)
//    oskar_log_set_keep_file(log, s.sim.keep_log_file);
//    oskar_log_set_file_priority(log, s.sim.write_status_to_log_file ?
//            OSKAR_LOG_STATUS : OSKAR_LOG_MESSAGE);
//    oskar_log_settings_simulator(log, &s);
//    oskar_log_settings_sky(log, &s);
//    oskar_log_settings_observation(log, &s);
//    oskar_log_settings_telescope(log, &s);
//    oskar_log_settings_interferometer(log, &s);

    // Check that an output data file has been specified.
    const char* vis_name = s.interferometer.oskar_vis_filename;
    const char* ms_name = s.interferometer.ms_filename;
    if (!(vis_name || ms_name)) {
        oskar_log_error(log, "No output file specified.");
        *status = OSKAR_ERR_SETTINGS;
        return;
    }

    // Obtain (set up) telescope model and sky model chunk array.
    oskar_Telescope* tel = oskar_set_up_telescope(&s, log, status);
    int num_chunks = 0;
    oskar_Sky** sky_chunks = oskar_set_up_sky(&s, log, &num_chunks, status);


    // Initialise each of the requested GPUs and set up per GPU memory.
    int num_gpus = s.sim.num_cuda_devices;
    if (num_gpus_avail < num_gpus) {
        *status = OSKAR_ERR_CUDA_DEVICES;
        return;
    }
    std::vector<DeviceData> d(num_gpus);
    for (int i = 0; i < num_gpus; ++i) {
        *status = (int)cudaSetDevice(s.sim.cuda_device_ids[i]);
        if (*status) return;
        cudaDeviceSynchronize();
        set_up_device_data_(&d[i], &s, tel, s.sim.max_sources_per_chunk,
                s.obs.num_time_steps, status);
    }

    // Check for errors to ensure there are no null pointers.
    if (*status) return;

    /* Work out how many time blocks have to be processed */
    int total_times = s.obs.num_time_steps;
    int time_block_length = s.interferometer.max_time_samples_per_block;
    int num_time_blocks = ceil((float)total_times / (float)time_block_length);

    /* Create output file-handle structure and initialise the visibility header */
    OutputHandles out;
    out.header = oskar_set_up_vis_header(&s, tel, status);
    out.station_work = 0;
    out.work_uvw = 0;

    /* Create and start simulation timer */
    oskar_Timer* total = oskar_timer_create(OSKAR_TIMER_NATIVE);
    oskar_timer_start(total);

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    //-- START OF MULTIRHREADED SIMULATION CODE --------------------------------
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    // TODO guard code for non-openMP fall-back...

    /* Loop over blocks of the observation time index running simulation and
     * fileIO one block at a time. Simulation and fileIO are overlapped by
     * use of double buffering and a dedicated thread for fileIO
     *
     * Thread 0 is used for fileIO.
     * Threads 1 to n (mapped to GPUs) execute the simulation.
     *
     * Note that no write is launched on the first block counter (as no
     * data is ready yet) and no simulation is performed for the last loop
     * counter (which corresponds to the last block + 1) as this iteration
     * simply writes the last block.
     */
    omp_set_num_threads(num_gpus + 1);
#pragma omp parallel
    {
        int tid = omp_get_thread_num(); /* The current thread id */

        for (int iblock = 0; iblock < (num_time_blocks+1); ++iblock)
        {
            int iactive = iblock % 2; /* index of the active simulation vis block */

            if (tid > 0 && iblock < num_time_blocks)
                sim_vis_block_(&s, &d[tid-1], sky_chunks, num_chunks,
                        iblock, iactive, status);

            if (tid == 0 && iblock > 0) {
                // TODO fileIO timers
                write_vis_block_(&s, &d[0], &out, tel, iblock-1, iactive, status);
            }

            /* Barrier to check sim and write are done before starting new block */
# pragma omp barrier
        }
    }
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    //-- END OF MULTIRHREADED SIMULATION CODE ----------------------------------
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------

    // TODO write log file to MS


    /* TODO replace with OSKAR log call */
    printf("\n\nTime taken = %.2f s\n", oskar_timer_elapsed(total));

    /* Print warning about noise addition for un-normalised beams */
    if (s.interferometer.noise.enable) {
        // If there are sources in the simulation and the station beam is not
        // normalised to 1.0 at the phase centre, the values of noise RMS
        // may give a very unexpected S/N ratio!
        // The alternative would be to scale the noise to match the station
        // beam gain but that would require knowledge of the station beam
        // amplitude at the phase centre for each time and channel...
        int have_sources = (num_chunks > 0 && oskar_sky_num_sources(sky_chunks[0]) > 0);
        int amp_calibrated = s.telescope.normalise_beams_at_phase_centre;
        if (have_sources > 0 && !amp_calibrated) {
            log_warning_box_(log, "WARNING: System noise is being added to "
                    "visibilities without station beam normalisation enabled. "
                    "This may lead to an invalid signal to noise ratio.");
        }
    }

    // Record times.
//    record_timing_(num_gpus, s.sim.cuda_device_ids, &(d[0]), log);

    // Free per-GPU memory and reset all devices.
    for (int i = 0; i < num_gpus; ++i) {
        cudaSetDevice(s.sim.cuda_device_ids[i]);
        free_device_data_(&d[i], status);
        cudaDeviceReset();
    }

    // Free CPU sky and telescope models.
    for (int i = 0; i < num_chunks; ++i) {
        oskar_sky_free(sky_chunks[i], status);
    }
    free(sky_chunks);
    oskar_telescope_free(tel, status);

    // Free/close output handles
    oskar_vis_header_free(out.header, status);
    if (s.interferometer.oskar_vis_filename)
        oskar_binary_free(out.vis);
    if (s.interferometer.ms_filename)
        oskar_ms_close(out.ms);
    oskar_mem_free(out.work_uvw, status);
    oskar_mem_free(out.station_work, status);

    if (!*status)
        oskar_log_section(log, 'M', "Run complete.");
}