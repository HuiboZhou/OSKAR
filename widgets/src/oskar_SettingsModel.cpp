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

#include "widgets/oskar_SettingsModel.h"
#include "widgets/oskar_SettingsItem.h"
#include <QtGui/QApplication>
#include <QtGui/QBrush>
#include <QtGui/QFontMetrics>
#include <QtGui/QIcon>
#include <QtCore/QVector>
#include <QtCore/QSize>
#include <QtCore/QVariant>
#include <cstdio>

oskar_SettingsModel::oskar_SettingsModel(QObject* parent)
: QAbstractItemModel(parent),
  settings_(NULL),
  rootItem_(NULL)
{
    QString k;
    QStringList options;

    // Set up the root item.
    rootItem_ = new oskar_SettingsItem(QString(), QString(),
            oskar_SettingsItem::LABEL, "Setting", "Value");

    // Simulator settings.
    setLabel("simulator", "Simulator settings");
    k = "simulator/double_precision";
    registerSetting(k, "Use double precision", oskar_SettingsItem::BOOL, false, true);
    setTooltip(k, "Determines whether double precision arithmetic is used");
    registerSetting("simulator/max_sources_per_chunk", "Max. number of sources per chunk", oskar_SettingsItem::INT_POSITIVE, false, 10000);
    registerSetting("simulator/cuda_device_ids", "CUDA device IDs to use", oskar_SettingsItem::INT_CSV_LIST, false, 0);

    // Sky model file settings.
    setLabel("sky", "Sky model settings");
    registerSetting("sky/oskar_source_file", "Input OSKAR source file", oskar_SettingsItem::INPUT_FILE_NAME);
    setLabel("sky/oskar_source_file/filter", "Filter settings");
    registerSetting("sky/oskar_source_file/filter/flux_min", "Flux density min [Jy]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/oskar_source_file/filter/flux_max", "Flux density max [Jy]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/oskar_source_file/filter/radius_inner_deg", "Inner radius from phase centre [deg]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/oskar_source_file/filter/radius_outer_deg", "Outer radius from phase centre [deg]", oskar_SettingsItem::DOUBLE);
#if !(defined(OSKAR_NO_CBLAS) || defined(OSKAR_NO_LAPACK))
    setLabel("sky/oskar_source_file/extended_sources", "Extended source settings");
    registerSetting("sky/oskar_source_file/extended_sources/FWHM_major", "Major axis FWHM [arcsec]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/oskar_source_file/extended_sources/FWHM_minor", "Minor axis FWHM [arcsec]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/oskar_source_file/extended_sources/position_angle", "Position angle [deg]", oskar_SettingsItem::DOUBLE);
#endif

    registerSetting("sky/gsm_file", "Input Global Sky Model file", oskar_SettingsItem::INPUT_FILE_NAME);
    setLabel("sky/gsm_file/filter", "Filter settings");
    registerSetting("sky/gsm_file/filter/flux_min", "Flux density min [Jy]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/gsm_file/filter/flux_max", "Flux density max [Jy]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/gsm_file/filter/radius_inner_deg", "Inner radius from phase centre [deg]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/gsm_file/filter/radius_outer_deg", "Outer radius from phase centre [deg]", oskar_SettingsItem::DOUBLE);
#if !(defined(OSKAR_NO_CBLAS) || defined(OSKAR_NO_LAPACK))
    setLabel("sky/gsm_file/extended_sources", "Extended source settings");
    registerSetting("sky/gsm_file/extended_sources/FWHM_major", "Major axis FWHM [arcsec]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/gsm_file/extended_sources/FWHM_minor", "Minor axis FWHM [arcsec]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/gsm_file/extended_sources/position_angle", "Position angle [deg]", oskar_SettingsItem::DOUBLE);
#endif

    // Sky model generator settings.
    setLabel("sky/generator", "Generators");
    setLabel("sky/generator/random_power_law", "Random, power-law in flux");
    registerSetting("sky/generator/random_power_law/num_sources", "Number of sources", oskar_SettingsItem::INT_UNSIGNED);
    registerSetting("sky/generator/random_power_law/flux_min", "Flux density min [Jy]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_power_law/flux_max", "Flux density max [Jy]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_power_law/power", "Power law index", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_power_law/seed", "Random seed", oskar_SettingsItem::RANDOM_SEED);
    setLabel("sky/generator/random_power_law/filter", "Filter settings");
    registerSetting("sky/generator/random_power_law/filter/flux_min", "Flux density min [Jy]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_power_law/filter/flux_max", "Flux density max [Jy]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_power_law/filter/radius_inner_deg", "Inner radius from phase centre [deg]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_power_law/filter/radius_outer_deg", "Outer radius from phase centre [deg]", oskar_SettingsItem::DOUBLE);
#if !(defined(OSKAR_NO_CBLAS) || defined(OSKAR_NO_LAPACK))
    setLabel("sky/generator/random_power_law/extended_sources", "Extended source settings");
    registerSetting("sky/generator/random_power_law/extended_sources/FWHM_major", "Major axis FWHM [arcsec]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_power_law/extended_sources/FWHM_minor", "Minor axis FWHM [arcsec]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_power_law/extended_sources/position_angle", "Position angle [deg]", oskar_SettingsItem::DOUBLE);
#endif

    setLabel("sky/generator/random_broken_power_law", "Random, broken power-law in flux");
    registerSetting("sky/generator/random_broken_power_law/num_sources", "Number of sources", oskar_SettingsItem::INT_UNSIGNED);
    registerSetting("sky/generator/random_broken_power_law/flux_min", "Flux density min [Jy]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_broken_power_law/flux_max", "Flux density max [Jy]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_broken_power_law/power1", "Power law index 1", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_broken_power_law/power2", "Power law index 2", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_broken_power_law/threshold", "Threshold [Jy]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_broken_power_law/seed", "Random seed", oskar_SettingsItem::RANDOM_SEED);
    setLabel("sky/generator/random_broken_power_law/filter", "Filter settings");
    registerSetting("sky/generator/random_broken_power_law/filter/flux_min", "Flux density min [Jy]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_broken_power_law/filter/flux_max", "Flux density max [Jy]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_broken_power_law/filter/radius_inner_deg", "Inner radius from phase centre [deg]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_broken_power_law/filter/radius_outer_deg", "Outer radius from phase centre [deg]", oskar_SettingsItem::DOUBLE);
#if !(defined(OSKAR_NO_CBLAS) || defined(OSKAR_NO_LAPACK))
    setLabel("sky/generator/random_broken_power_law/extended_sources", "Extended source settings");
    registerSetting("sky/generator/random_broken_power_law/extended_sources/FWHM_major", "Major axis FWHM [arcsec]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_broken_power_law/extended_sources/FWHM_minor", "Minor axis FWHM [arcsec]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/random_broken_power_law/extended_sources/position_angle", "Position angle [deg]", oskar_SettingsItem::DOUBLE);
#endif

    setLabel("sky/generator/healpix", "HEALPix (uniform, all sky) grid");
    registerSetting("sky/generator/healpix/nside", "Nside", oskar_SettingsItem::INT_UNSIGNED);
    setLabel("sky/generator/healpix/filter", "Filter settings");
    registerSetting("sky/generator/healpix/filter/flux_min", "Flux density min [Jy]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/healpix/filter/flux_max", "Flux density max [Jy]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/healpix/filter/radius_inner_deg", "Inner radius from phase centre [deg]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/healpix/filter/radius_outer_deg", "Outer radius from phase centre [deg]", oskar_SettingsItem::DOUBLE);
#if !(defined(OSKAR_NO_CBLAS) || defined(OSKAR_NO_LAPACK))
    setLabel("sky/generator/healpix/extended_sources", "Extended source settings");
    registerSetting("sky/generator/healpix/extended_sources/FWHM_major", "Major axis FWHM [arcsec]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/healpix/extended_sources/FWHM_minor", "Minor axis FWHM [arcsec]", oskar_SettingsItem::DOUBLE);
    registerSetting("sky/generator/healpix/extended_sources/position_angle", "Position angle [deg]", oskar_SettingsItem::DOUBLE);
#endif
    registerSetting("sky/output_sky_file", "Output OSKAR source file", oskar_SettingsItem::OUTPUT_FILE_NAME);

    // Observation settings.
    setLabel("observation", "Observation settings");
    registerSetting("observation/phase_centre_ra_deg", "Phase centre RA [deg]", oskar_SettingsItem::DOUBLE);
    registerSetting("observation/phase_centre_dec_deg", "Phase centre Dec [deg]", oskar_SettingsItem::DOUBLE);
    registerSetting("observation/start_frequency_hz", "Start frequency [Hz]", oskar_SettingsItem::DOUBLE, true);
    registerSetting("observation/num_channels", "Number of frequency channels", oskar_SettingsItem::INT_POSITIVE);
    registerSetting("observation/frequency_inc_hz", "Frequency increment [Hz]", oskar_SettingsItem::DOUBLE);
    registerSetting("observation/start_time_utc", "Start time (UTC)", oskar_SettingsItem::DATE_TIME, true);
    registerSetting("observation/num_time_steps", "Number of time steps", oskar_SettingsItem::INT_POSITIVE);
    registerSetting("observation/length", "Observation length (H:M:S)", oskar_SettingsItem::TIME, true);

    // Telescope model settings.
    setLabel("telescope", "Telescope model settings");
    registerSetting("telescope/config_directory", "Telescope directory", oskar_SettingsItem::TELESCOPE_DIR_NAME, true);
    registerSetting("telescope/longitude_deg", "Longitude [deg]", oskar_SettingsItem::DOUBLE);
    registerSetting("telescope/latitude_deg", "Latitude [deg]", oskar_SettingsItem::DOUBLE);
    registerSetting("telescope/altitude_m", "Altitude [m]", oskar_SettingsItem::DOUBLE);
    registerSetting("telescope/use_common_sky", "Use common sky (short baseline approximation)", oskar_SettingsItem::BOOL, false, true);
    setLabel("telescope/station", "Station settings");
    options.clear();
    options << "AA"; // << "Dish";
    registerSetting("telescope/station/station_type", "Station type", oskar_SettingsItem::OPTIONS, options, false, options[0]);
    registerSetting("telescope/station/use_polarised_elements", "Use polarised elements", oskar_SettingsItem::BOOL, false, true);
    registerSetting("telescope/station/ignore_custom_element_patterns", "Ignore custom element patterns", oskar_SettingsItem::BOOL, false, false);
    registerSetting("telescope/station/evaluate_array_factor", "Evaluate array factor (Jones E)", oskar_SettingsItem::BOOL, false, true);
    registerSetting("telescope/station/evaluate_element_factor", "Evaluate element factor (Jones G)", oskar_SettingsItem::BOOL, false, true);
    registerSetting("telescope/station/normalise_beam", "Normalise array beam", oskar_SettingsItem::BOOL, false, false);
    setLabel("telescope/station/element", "Element settings (overrides)");
    registerSetting("telescope/station/element/gain", "Element gain", oskar_SettingsItem::DOUBLE);
    registerSetting("telescope/station/element/gain_error_fixed", "Element gain std.dev. (systematic)", oskar_SettingsItem::DOUBLE);
    registerSetting("telescope/station/element/gain_error_time", "Element gain std.dev. (time-variable)", oskar_SettingsItem::DOUBLE);
    registerSetting("telescope/station/element/phase_error_fixed_deg", "Element phase std.dev. (systematic) [deg]", oskar_SettingsItem::DOUBLE);
    registerSetting("telescope/station/element/phase_error_time_deg", "Element phase std.dev. (time-variable) [deg]", oskar_SettingsItem::DOUBLE);
    registerSetting("telescope/station/element/position_error_xy_m", "Element (x,y) position std.dev. [m]", oskar_SettingsItem::DOUBLE);
    registerSetting("telescope/station/element/x_orientation_error_deg", "Element X-dipole orientation std.dev. [deg]", oskar_SettingsItem::DOUBLE);
    registerSetting("telescope/station/element/y_orientation_error_deg", "Element Y-dipole orientation std.dev. [deg]", oskar_SettingsItem::DOUBLE);
    registerSetting("telescope/station/element/seed_gain_errors", "Random seed (systematic gain errors)", oskar_SettingsItem::RANDOM_SEED);
    registerSetting("telescope/station/element/seed_phase_errors", "Random seed (systematic phase errors)", oskar_SettingsItem::RANDOM_SEED);
    registerSetting("telescope/station/element/seed_time_variable_errors", "Random seed (time-variable errors)", oskar_SettingsItem::RANDOM_SEED);
    registerSetting("telescope/station/element/seed_position_xy_errors", "Random seed (x,y position errors)", oskar_SettingsItem::RANDOM_SEED);
    registerSetting("telescope/station/element/seed_x_orientation_error", "Random seed (X-dipole orientation errors)", oskar_SettingsItem::RANDOM_SEED);
    registerSetting("telescope/station/element/seed_y_orientation_error", "Random seed (Y-dipole orientation errors)", oskar_SettingsItem::RANDOM_SEED);

    // Element pattern fitting parameters.
    setLabel("telescope/station/element_fit", "Element pattern fitting parameters");
    registerSetting("telescope/station/element_fit/ignore_data_at_pole", "Ignore data at poles", oskar_SettingsItem::BOOL, false, false);
    registerSetting("telescope/station/element_fit/ignore_data_below_horizon", "Ignore data below horizon", oskar_SettingsItem::BOOL, false, true);
    registerSetting("telescope/station/element_fit/overlap_angle_deg", "Overlap angle [deg]", oskar_SettingsItem::DOUBLE, false, 9.0);
    registerSetting("telescope/station/element_fit/weight_boundaries", "Weighting at boundaries", oskar_SettingsItem::DOUBLE, false, 20.0);
    registerSetting("telescope/station/element_fit/weight_overlap", "Weighting in overlap region", oskar_SettingsItem::DOUBLE, false, 4.0);
    //registerSetting("telescope/station/element_fit/use_common_set", "Use common set", oskar_SettingsItem::BOOL, false, true);
    setLabel("telescope/station/element_fit/all", "Common settings (used for all surfaces)");
    registerSetting("telescope/station/element_fit/all/search_for_best_fit", "Search for best fit", oskar_SettingsItem::BOOL, false, true);
    registerSetting("telescope/station/element_fit/all/average_fractional_error", "Average fractional error", oskar_SettingsItem::DOUBLE, false, 0.02);
    registerSetting("telescope/station/element_fit/all/average_fractional_error_factor_increase", "Average fractional error factor increase", oskar_SettingsItem::DOUBLE, false, 1.5);
    registerSetting("telescope/station/element_fit/all/smoothness_factor_reduction", "Smoothness reduction factor", oskar_SettingsItem::DOUBLE, false, 0.9);
    registerSetting("telescope/station/element_fit/all/eps_float", "Epsilon (single precision)", oskar_SettingsItem::DOUBLE, false, 4e-4);
    registerSetting("telescope/station/element_fit/all/eps_double", "Epsilon (double precision)", oskar_SettingsItem::DOUBLE, false, 2e-8);
    registerSetting("telescope/station/element_fit/all/smoothness_factor_override", "Smoothness factor override", oskar_SettingsItem::DOUBLE, false, 1.0);

    // TODO Add parameters for all eight surfaces!

    registerSetting("telescope/output_config_directory", "Output telescope directory", oskar_SettingsItem::OUTPUT_FILE_NAME);

    // Interferometer settings. (Note: Currently loaded into SettingsObservation)
    setLabel("interferometer", "Interferometer settings");
    registerSetting("interferometer/channel_bandwidth_hz", "Channel bandwidth [Hz]", oskar_SettingsItem::DOUBLE);
    registerSetting("interferometer/num_vis_ave", "Number of visibility averages", oskar_SettingsItem::INT_POSITIVE);
    setTooltip("interferometer/num_vis_ave", "Number of averaged evaluations of the full Measurement Equation per visibility dump.");
    registerSetting("interferometer/num_fringe_ave", "Number of fringe averages", oskar_SettingsItem::INT_POSITIVE);
    setTooltip("interferometer/num_fringe_ave", "Number of averaged evaluations of the K-Jones matrix per Measurement Equation evaluation.");
    registerSetting("interferometer/oskar_vis_filename", "Output OSKAR visibility file", oskar_SettingsItem::OUTPUT_FILE_NAME);
#ifndef OSKAR_NO_MS
    registerSetting("interferometer/ms_filename", "Output Measurement Set", oskar_SettingsItem::OUTPUT_FILE_NAME);
#endif
    registerSetting("interferometer/image_output", "Image simulation output",
            oskar_SettingsItem::BOOL);
    setDefault("interferometer/image_output", false);
    setTooltip("interferometer/image_output", "Run the OSKAR imager on "
            "completion of the interferometer simulation. For image settings "
            "see the 'Image settings' group");

    // Beam pattern settings.
    setLabel("beam_pattern", "Beam pattern settings");
    registerSetting("beam_pattern/fov_deg", "Field-of-view [deg]", oskar_SettingsItem::DOUBLE, false, 2.0);
    registerSetting("beam_pattern/size", "Image dimension [pixels]", oskar_SettingsItem::INT_POSITIVE, false, 256);
    registerSetting("beam_pattern/station_id", "Station ID", oskar_SettingsItem::INT_UNSIGNED);
    registerSetting("beam_pattern/filename", "Output OSKAR image file", oskar_SettingsItem::OUTPUT_FILE_NAME);
#ifndef OSKAR_NO_FITS
    registerSetting("beam_pattern/fits_image", "Output FITS image file", oskar_SettingsItem::OUTPUT_FILE_NAME);
#endif

    // Image settings.
    setLabel("image", "Image settings");
    registerSetting("image/fov_deg", "Field-of-view [deg]", oskar_SettingsItem::DOUBLE);
    setDefault("image/fov_deg", 2.0);
    registerSetting("image/size", "Image dimension [pixels]", oskar_SettingsItem::INT_POSITIVE);
    setDefault("image/size", 256);
    options.clear();
    options << "Linear (XX,XY,YX,YY)" << "XX" << "XY" << "YX" << "YY"
            << "Stokes (I,Q,U,V)" << "I" << "Q" << "U" << "V"
            << "PSF";
    registerSetting("image/image_type", "Image type", oskar_SettingsItem::OPTIONS, options);
    setDefault("image/image_type", "I");
    registerSetting("image/channel_snapshots", "Channel snapshots", oskar_SettingsItem::BOOL);
    setDefault("image/channel_snapshots", true);
    registerSetting("image/channel_start", "Channel start", oskar_SettingsItem::INT_UNSIGNED);
    registerSetting("image/channel_end", "Channel end", oskar_SettingsItem::AXIS_RANGE);
    setDefault("image/channel_end", "max");
    registerSetting("image/time_snapshots", "Time snapshots", oskar_SettingsItem::BOOL);
    setDefault("image/time_snapshots", true);
    registerSetting("image/time_start", "Time start", oskar_SettingsItem::INT_UNSIGNED);
    registerSetting("image/time_end", "Time end", oskar_SettingsItem::AXIS_RANGE);
    setDefault("image/time_end", "max");
    options.clear();
    //options << "DFT 2D" << "DFT 3D" << "FFT";
    options << "DFT 2D";
    registerSetting("image/transform_type", "Transform type", oskar_SettingsItem::OPTIONS, options);
    setDefault("image/transform_type", "DFT 2D");
    registerSetting("image/input_vis_data", "Input OSKAR visibility data file", oskar_SettingsItem::INPUT_FILE_NAME);
    registerSetting("image/oskar_image_root", "Output OSKAR image root path", oskar_SettingsItem::OUTPUT_FILE_NAME);
    setTooltip("image/oskar_image_root",
            "Path consisting of the root of the filename used to save the "
            "output image. The full filename will be constructed as:\n"
            "   <root>_type.img");
#ifndef OSKAR_NO_FITS
    registerSetting("image/fits_image_root", "Output FITS image root path", oskar_SettingsItem::OUTPUT_FILE_NAME);
    setTooltip("image/fits_image_root",
            "Path consisting of the root of the filename used to save the "
            "output image. The full filename will be constructed as:\n"
            "   <root>_type.fits");
#endif

}

oskar_SettingsModel::~oskar_SettingsModel()
{
    // Delete any existing settings object.
    if (settings_)
        delete settings_;
    delete rootItem_;
}

int oskar_SettingsModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 2;
}

QVariant oskar_SettingsModel::data(const QModelIndex& index, int role) const
{
    // Check for roles that do not depend on the index.
    if (role == IterationKeysRole)
        return iterationKeys_;
    else if (role == OutputKeysRole)
        return outputKeys_;

    // Get a pointer to the item.
    if (!index.isValid())
        return QVariant();
    oskar_SettingsItem* item = getItem(index);

    // Check for roles common to all columns.
    if (role == Qt::FontRole)
    {
        if (iterationKeys_.contains(item->key()))
        {
            QFont font = QApplication::font();
            font.setBold(true);
            return font;
        }
        QVariant val = item->value();
        if (val.isNull() && item->type() != oskar_SettingsItem::LABEL)
        {
            QFont font = QApplication::font();
            font.setItalic(true);
            return font;
        }
    }
    else if (role == Qt::ForegroundRole)
    {
        QVariant val = item->value();
        if (val.isNull() && item->type() != oskar_SettingsItem::LABEL)
            return QColor(Qt::darkBlue);
    }
    else if (role == Qt::ToolTipRole)
        return item->tooltip();
    else if (role == DefaultRole)
        return item->defaultValue();
    else if (role == KeyRole)
        return item->key();
    else if (role == TypeRole)
        return item->type();
    else if (role == VisibleRole)
        return item->visible() || item->required();
    else if (role == EnabledRole)
        return item->enabled();
    else if (role == OptionsRole)
        return item->options();
    else if (role == IterationNumRole)
        return item->iterationNum();
    else if (role == IterationIncRole)
        return item->iterationInc();
    else if (role == Qt::DecorationRole)
    {
        if (index.column() == 0)
        {
            if (item->type() == oskar_SettingsItem::INPUT_FILE_NAME ||
                    item->type() == oskar_SettingsItem::TELESCOPE_DIR_NAME)
            {
                if (item->required())
                    return QIcon(":/icons/open_required.png");
                return QIcon(":/icons/open.png");
            }
            else if (item->type() == oskar_SettingsItem::OUTPUT_FILE_NAME)
            {
                return QIcon(":/icons/save.png");
            }

            // Check if a generic required item.
            if (item->required() && item->type() != oskar_SettingsItem::LABEL)
                return QIcon(":/icons/required.png");
        }
    }

    // Check for roles in specific columns.
    if (index.column() == 0)
    {
        if (role == Qt::DisplayRole)
        {
            QString label = item->label();
            int iterIndex = iterationKeys_.indexOf(item->key());
            if (iterIndex >= 0)
                label.prepend(QString("[%1] ").arg(iterIndex + 1));
            return label;
        }
    }
    else if (index.column() == 1)
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            QVariant val = item->value();
            if (val.isNull())
                val = item->defaultValue();
            return val;
        }
        else if (role == Qt::CheckStateRole &&
                item->type() == oskar_SettingsItem::BOOL)
        {
            QVariant val = item->value();
            if (val.isNull())
                val = item->defaultValue();
            return val.toBool() ? Qt::Checked : Qt::Unchecked;
        }
        else if (role == Qt::SizeHintRole)
        {
            int width = QApplication::fontMetrics().width(item->label()) + 10;
            return QSize(width, 26);
        }
    }

    return QVariant();
}

Qt::ItemFlags oskar_SettingsModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return 0;

    oskar_SettingsItem* item = getItem(index);
    if (!item->enabled())
        return Qt::ItemIsSelectable;

    if (index.column() == 0 ||
            item->type() == oskar_SettingsItem::LABEL)
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    else if (index.column() == 1 &&
            item->type() == oskar_SettingsItem::BOOL)
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable |
                Qt::ItemIsUserCheckable;
    }

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

const oskar_SettingsItem* oskar_SettingsModel::getItem(const QString& key) const
{
    return hash_.value(key);
}

QVariant oskar_SettingsModel::headerData(int section,
        Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if (section == 0)
            return rootItem_->label();
        else if (section == 1)
            return rootItem_->value();
    }

    return QVariant();
}

QModelIndex oskar_SettingsModel::index(int row, int column,
        const QModelIndex& parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    oskar_SettingsItem* parentItem = getItem(parent);
    oskar_SettingsItem* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex oskar_SettingsModel::index(const QString& key)
{
    QStringList keys = key.split('/');

    // Find the parent, creating groups as necessary.
    QModelIndex parent, child;
    for (int k = 0; k < keys.size() - 1; ++k)
    {
        child = getChild(keys[k], parent);
        if (child.isValid())
            parent = child;
        else
        {
            // Append the group and set it as the new parent.
            append(key, keys[k], oskar_SettingsItem::LABEL, keys[k],
                    false, QVariant(), QStringList(), parent);
            parent = index(rowCount(parent) - 1, 0, parent);
        }
    }

    // Return the model index.
    child = getChild(keys.last(), parent);
    if (!child.isValid())
    {
        append(key, keys.last(), oskar_SettingsItem::LABEL, keys.last(),
                false, QVariant(), QStringList(), parent);
        child = index(rowCount(parent) - 1, 0, parent);
    }
    return child;
}

QMap<int, QVariant> oskar_SettingsModel::itemData(const QModelIndex& index) const
{
    QMap<int, QVariant> d;
    d.insert(KeyRole, data(index, KeyRole));
    d.insert(TypeRole, data(index, TypeRole));
    d.insert(VisibleRole, data(index, VisibleRole));
    d.insert(IterationNumRole, data(index, IterationNumRole));
    d.insert(IterationIncRole, data(index, IterationIncRole));
    d.insert(IterationKeysRole, data(index, IterationKeysRole));
    d.insert(OutputKeysRole, data(index, OutputKeysRole));
    return d;
}

void oskar_SettingsModel::loadSettingsFile(const QString& filename)
{
    if (!filename.isEmpty())
    {
        // Delete any existing settings object.
        if (settings_)
            delete settings_;

        // Create new settings object from supplied filename.
        settings_ = new QSettings(filename, QSettings::IniFormat);

        // Display the contents of the file.
        beginResetModel();
        loadFromParentIndex(QModelIndex());
        endResetModel();
    }
}

QModelIndex oskar_SettingsModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    oskar_SettingsItem* childItem = getItem(index);
    oskar_SettingsItem* parentItem = childItem->parent();

    if (parentItem == rootItem_)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

void oskar_SettingsModel::registerSetting(const QString& key,
        const QString& label, int type, const QStringList& options,
        bool required, const QVariant& defaultValue)
{
    QStringList keys = key.split('/');

    // Find the parent, creating groups as necessary.
    QModelIndex parent, child;
    for (int k = 0; k < keys.size() - 1; ++k)
    {
        child = getChild(keys[k], parent);
        if (child.isValid())
            parent = child;
        else
        {
            // Append the group and set it as the new parent.
            append(key, keys[k], oskar_SettingsItem::LABEL, keys[k],
                    required, defaultValue, QStringList(), parent);
            parent = index(rowCount(parent) - 1, 0, parent);
        }
    }

    // Append the actual setting.
    append(key, keys.last(), type, label, required, defaultValue, options,
            parent);

    // Check if this is an output file.
    if (type == oskar_SettingsItem::OUTPUT_FILE_NAME)
        outputKeys_.append(key);
}

void oskar_SettingsModel::registerSetting(const QString& key,
        const QString& label, int type, bool required,
        const QVariant& defaultValue)
{
    QStringList keys = key.split('/');

    // Find the parent, creating groups as necessary.
    QModelIndex parent, child;
    for (int k = 0; k < keys.size() - 1; ++k)
    {
        child = getChild(keys[k], parent);
        if (child.isValid())
            parent = child;
        else
        {
            // Append the group and set it as the new parent.
            append(key, keys[k], oskar_SettingsItem::LABEL, keys[k],
                    required, defaultValue, QStringList(), parent);
            parent = index(rowCount(parent) - 1, 0, parent);
        }
    }

    // Append the actual setting.
    append(key, keys.last(), type, label, required, defaultValue,
            QStringList(), parent);

    // Check if this is an output file.
    if (type == oskar_SettingsItem::OUTPUT_FILE_NAME)
        outputKeys_.append(key);
}

int oskar_SettingsModel::rowCount(const QModelIndex& parent) const
{
    return getItem(parent)->childCount();
}

void oskar_SettingsModel::saveSettingsFile(const QString& filename)
{
    if (!filename.isEmpty())
    {
        // Delete any existing settings object.
        if (settings_)
            delete settings_;

        // Create new settings object from supplied filename.
        settings_ = new QSettings(filename, QSettings::IniFormat);

        // Set the contents of the file.
        saveFromParentIndex(QModelIndex());
    }
}

bool oskar_SettingsModel::setData(const QModelIndex& idx,
        const QVariant& value, int role)
{
    if (!idx.isValid())
        return false;

    // Get a pointer to the item.
    oskar_SettingsItem* item = getItem(idx);

    // Get model indexes for the row.
    QModelIndex topLeft = idx.sibling(idx.row(), 0);
    QModelIndex bottomRight = idx.sibling(idx.row(), columnCount() - 1);

    // Check for role type.
    if (role == Qt::ToolTipRole)
    {
        item->setTooltip(value.toString());
        emit dataChanged(topLeft, bottomRight);
        return true;
    }
    else if (role == DefaultRole)
    {
        item->setDefaultValue(value);
        emit dataChanged(topLeft, bottomRight);
        return true;
    }
    else if (role == EnabledRole)
    {
        item->setEnabled(value.toBool());
        if (value.toBool())
            settings_->setValue(item->key(), item->value());
        else
            settings_->remove(item->key());
        settings_->sync();
        emit dataChanged(topLeft, bottomRight);
        return true;
    }
    else if (role == IterationNumRole)
    {
        item->setIterationNum(value.toInt());
        emit dataChanged(topLeft, bottomRight);
        return true;
    }
    else if (role == IterationIncRole)
    {
        item->setIterationInc(value);
        emit dataChanged(topLeft, bottomRight);
        return true;
    }
    else if (role == SetIterationRole)
    {
        if (!iterationKeys_.contains(item->key()))
        {
            iterationKeys_.append(item->key());
            emit dataChanged(topLeft, bottomRight);
            return true;
        }
        return false;
    }
    else if (role == ClearIterationRole)
    {
        int i = iterationKeys_.indexOf(item->key());
        if (i >= 0)
        {
            iterationKeys_.removeAt(i);
            emit dataChanged(topLeft, bottomRight);
            foreach (QString k, iterationKeys_)
            {
                QModelIndex idx = index(k);
                emit dataChanged(idx, idx.sibling(idx.row(), columnCount()-1));
            }
            return true;
        }
        return false;
    }
    else if (role == Qt::EditRole || role == Qt::CheckStateRole ||
            role == LoadRole)
    {
        QVariant data = value;
        if (role == Qt::CheckStateRole)
            data = value.toBool() ? QString("true") : QString("false");

        if (idx.column() == 0)
        {
            item->setLabel(data.toString());
            emit dataChanged(idx, idx);
            return true;
        }
        else if (idx.column() == 1)
        {
            // Set the data in the settings file.
            if ((role != LoadRole) && settings_)
            {
                if (data.isNull())
                    settings_->remove(item->key());
                else
                    settings_->setValue(item->key(), data);
                settings_->sync();
            }

            // Set the item data.
            item->setValue(data);
            QModelIndex i(idx);
            while (i.isValid())
            {
                emit dataChanged(i.sibling(i.row(), 0),
                        i.sibling(i.row(), columnCount()-1));
                i = i.parent();
            }
            return true;
        }
    }

    return false;
}

void oskar_SettingsModel::setDefault(const QString& key, const QVariant& value)
{
    QModelIndex idx = index(key);
    setData(idx, value, DefaultRole);
}

void oskar_SettingsModel::setLabel(const QString& key, const QString& label)
{
    QModelIndex idx = index(key);
    setData(idx, label);
}

void oskar_SettingsModel::setTooltip(const QString& key, const QString& tooltip)
{
    QModelIndex idx = index(key);
    setData(idx, tooltip, Qt::ToolTipRole);
}

void oskar_SettingsModel::setValue(const QString& key, const QVariant& value)
{
    // Get the model index.
    QModelIndex idx = index(key);
    idx = idx.sibling(idx.row(), 1);

    // Set the data.
    setData(idx, value, Qt::EditRole);
}

void oskar_SettingsModel::setDisabled(const QString& key, bool value)
{
    // Get the model index.
    QModelIndex idx = index(key);
    idx = idx.sibling(idx.row(), 1);
    setData(idx, value, oskar_SettingsModel::EnabledRole);
}

// Private methods.

void oskar_SettingsModel::append(const QString& key, const QString& subkey,
        int type, const QString& label, bool required,
        const QVariant& defaultValue, const QStringList& options,
        const QModelIndex& parent)
{
    oskar_SettingsItem *parentItem = getItem(parent);

    beginInsertRows(parent, rowCount(), rowCount());
    oskar_SettingsItem* item = new oskar_SettingsItem(key, subkey, type,
            label, QVariant(), required, defaultValue, options, parentItem);
    parentItem->appendChild(item);
    endInsertRows();
    hash_.insert(key, item);
}

QModelIndex oskar_SettingsModel::getChild(const QString& subkey,
        const QModelIndex& parent) const
{
    // Search this parent's children.
    oskar_SettingsItem* item = getItem(parent);
    for (int i = 0; i < item->childCount(); ++i)
    {
        if (item->child(i)->subkey() == subkey)
            return index(i, 0, parent);
    }
    return QModelIndex();
}

oskar_SettingsItem* oskar_SettingsModel::getItem(const QModelIndex& index) const
{
    if (index.isValid())
    {
        oskar_SettingsItem* item =
                static_cast<oskar_SettingsItem*>(index.internalPointer());
        if (item) return item;
    }
    return rootItem_;
}

void oskar_SettingsModel::loadFromParentIndex(const QModelIndex& parent)
{
    int rows = rowCount(parent);
    for (int i = 0; i < rows; ++i)
    {
        QModelIndex idx = index(i, 0, parent);
        if (idx.isValid())
        {
            const oskar_SettingsItem* item = getItem(idx);
            setData(idx.sibling(idx.row(), 1),
                    settings_->value(item->key()), LoadRole);
            loadFromParentIndex(idx);
        }
    }
}

void oskar_SettingsModel::saveFromParentIndex(const QModelIndex& parent)
{
    int rows = rowCount(parent);
    for (int i = 0; i < rows; ++i)
    {
        QModelIndex idx = index(i, 0, parent);
        if (idx.isValid())
        {
            const oskar_SettingsItem* item = getItem(idx);
            if (!item->value().isNull())
                settings_->setValue(item->key(), item->value());
            saveFromParentIndex(idx);
        }
    }
}

oskar_SettingsModelFilter::oskar_SettingsModelFilter(QObject* parent)
: QSortFilterProxyModel(parent),
  hideIfUnset_(false)
{
    setDynamicSortFilter(true);
}

oskar_SettingsModelFilter::~oskar_SettingsModelFilter()
{
}

bool oskar_SettingsModelFilter::hideIfUnset() const
{
    return hideIfUnset_;
}

// Public slots.

void oskar_SettingsModelFilter::setHideIfUnset(bool value)
{
    if (value != hideIfUnset_)
    {
        hideIfUnset_ = value;
        beginResetModel();
        endResetModel();
        invalidateFilter();
    }
}

// Protected methods.

bool oskar_SettingsModelFilter::filterAcceptsRow(int sourceRow,
            const QModelIndex& sourceParent) const
{
    if (!hideIfUnset_) return true;
    QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);
    return sourceModel()->data(idx, oskar_SettingsModel::VisibleRole).toBool();
}
