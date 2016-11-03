#
#  This file is part of OSKAR.
#
# Copyright (c) 2016, The University of Oxford
# All rights reserved.
#
#  This file is part of the OSKAR package.
#  Contact: oskar at oerc.ox.ac.uk
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#  1. Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#  3. Neither the name of the University of Oxford nor the names of its
#     contributors may be used to endorse or promote products derived from this
#     software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#

from __future__ import absolute_import, division
from . import _measurement_set_lib


class MeasurementSet(object):
    """This class provides a Python interface to a Measurement Set."""

    def __init__(self):
        """The default constructor does nothing. Use create() or open()."""
        self._capsule = 0

    @classmethod
    def create(cls, file_name, num_stations, num_channels, num_pols,
               ref_freq_hz, chan_width_hz, write_autocorr=False,
               write_crosscorr=True):
        """Creates a new, empty Measurement Set with the given name.

        Args:
            file_name (str):      File name of the new Measurement Set.
            num_stations (int):   The number of antennas/stations.
            num_channels (int):   The number of channels in the band.
            num_pols (int):       The number of polarisations (1, 2 or 4).
            ref_freq_hz (float):
                The frequency at the centre of channel 0, in Hz.
            chan_width_hz (float):
                The width of each channel, in Hz.
            write_autocorr (Optional[bool]):
                If set, allow for write of auto-correlation data.
            write_crosscorr (Optional[bool]):
                If set, allow for write of cross-correlation data.
        """
        t = MeasurementSet()
        t._capsule = _measurement_set_lib.create(
            file_name, num_stations, num_channels, num_pols,
            ref_freq_hz, chan_width_hz, write_autocorr, write_crosscorr)
        return t

    def num_channels(self):
        """Returns the number of frequency channels in the Measurement Set."""
        return _measurement_set_lib.num_channels(self._capsule)

    def num_pols(self):
        """Returns the number of polarisations in the Measurement Set."""
        return _measurement_set_lib.num_pols(self._capsule)

    def num_rows(self):
        """Returns the number of rows in the Measurement Set main table."""
        return _measurement_set_lib.num_rows(self._capsule)

    def num_stations(self):
        """Returns the number of stations in the Measurement Set."""
        return _measurement_set_lib.num_stations(self._capsule)

    @classmethod
    def open(cls, file_name):
        """Opens an existing Measurement Set with the given name.

        Args:
            file_name (str):      File name of the Measurement Set to open.
        """
        t = MeasurementSet()
        t._capsule = _measurement_set_lib.open(file_name)
        return t

    def read_coords(self, start_row, num_baselines):
        """Reads baseline coordinate data from the main table.

        This function reads a list of baseline coordinates from
        the main table of the Measurement Set. The coordinates are returned
        as a tuple of numpy arrays (uu, vv, ww).

        Args:
            start_row (int):        The start row index to read (zero-based).
            num_baselines (int):    Number of baselines (rows) to read.

        Returns:
            tuple: (uu, vv, ww) baseline coordinates, in metres.
        """
        return _measurement_set_lib.read_coords(
            self._capsule, start_row, num_baselines)

    def read_vis(self, start_row, start_channel, num_channels, num_baselines,
                 column='DATA'):
        """Reads visibility data from the main table.

        This function reads a block of visibility data from the specified
        column of the main table of the Measurement Set.

        The dimensionality of the returned data block is:
        (num_channels * num_baselines * num_pols),
        with num_pols the fastest varying dimension, then num_baselines,
        and num_channels the slowest.

        Args:
            start_row (int):        The start row index to read (zero-based).
            start_channel (int):    Start channel index to read.
            num_channels (int):     Number of channels to read.
            num_baselines (int):    Number of baselines to read.
            column (Optional[str]): Name of the data column to read.

        Returns:
            array: visibility data block.
        """
        return _measurement_set_lib.read_vis(
            self._capsule, start_row, start_channel,
            num_channels, num_baselines, column)

    def set_phase_centre(self, longitude_rad, latitude_rad):
        """Sets the phase centre.

        Args:
            longitude_rad (float):  Right Ascension of phase centre.
            latitude_rad (float):   Declination of phase centre.
        """
        return _measurement_set_lib.set_phase_centre(
            self._capsule, longitude_rad, latitude_rad)

    def write_coords(self, start_row, num_baselines, uu, vv, ww,
                     exposure_sec, interval_sec, time_stamp):
        """Writes baseline coordinate data to the main table.

        This function writes the supplied list of baseline coordinates to
        the main table of the Measurement Set, extending it if necessary.

        Baseline antenna-pair ordering is implicit:
        a0-a1, a0-a2, a0-a3... a1-a2, a1-a3... a2-a3 etc.
        The supplied number of baselines must be compatible with the number of
        stations in the Measurement Set. Auto-correlations are allowed.

        This function should be called for each time step to write out the
        baseline coordinate data.

        The time stamp is given in units of (MJD) * 86400, i.e. seconds since
        Julian date 2400000.5.

        Args:
            start_row (int):        The start row index to write (zero-based).
            num_baselines (int):    Number of rows to add to the main table.
            uu (float, array-like): Baseline u-coordinates, in metres.
            vv (float, array-like): Baseline v-coordinates, in metres.
            ww (float, array-like): Baseline w-coordinates, in metres.
            exposure_sec (float):   Exposure length per visibility, in seconds.
            interval_sec (float):   Interval length per visibility, in seconds.
            time_stamp (float):     Time stamp of visibility block.
        """
        _measurement_set_lib.write_coords(
            self._capsule, start_row, num_baselines, uu, vv, ww,
            exposure_sec, interval_sec, time_stamp)

    def write_vis(self, start_row, start_channel, num_channels,
                  num_baselines, vis):
        """Writes visibility data to the main table.

        This function writes the given block of visibility data to the
        data column of the Measurement Set, extending it if necessary.

        The dimensionality of the complex vis data block is:
        (num_channels * num_baselines * num_pols),
        with num_pols the fastest varying dimension, then num_baselines,
        and num_channels the slowest.

        Args:
            start_row (int):     The start row index to write (zero-based).
            start_channel (int): Start channel index of the visibility block.
            num_channels (int):  Number of channels in the visibility block.
            num_baselines (int): Number of baselines in the visibility block.
            vis (complex float, array-like): Pointer to visibility block.
        """
        _measurement_set_lib.write_vis(
            self._capsule, start_row, start_channel, num_channels,
            num_baselines, vis)
