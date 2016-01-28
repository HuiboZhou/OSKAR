/*
 * Copyright (c) 2015, The University of Oxford
 * All rights reserved.
 *
 * This file is part of the OSKAR package.
 * Contact: oskar at oerc.ox.ac.uk
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

#ifndef OSKAR_SETTINGS_TYPE_RANDOM_SEED_HPP_
#define OSKAR_SETTINGS_TYPE_RANDOM_SEED_HPP_

#include <oskar_AbstractSettingsType.hpp>

namespace oskar {

// TODO(BM) inherit IntRangeExt?
class RandomSeed : public AbstractSettingsType
{
public:
    RandomSeed();
    virtual ~RandomSeed();

    bool init(const std::string& s);
    bool set_default(const std::string& value);
    std::string get_default() const;
    bool set_value(const std::string& value);
    std::string get_value() const;
    bool is_default() const;

    int value() const { return value_; }
    int default_value() const { return default_; }

    bool operator==(const RandomSeed& other) const;
    bool operator>(const RandomSeed& other) const;

private:
    bool from_string_(const std::string& s, int& value) const;
    int default_, value_;
};

} // namespace oskar
#endif /* OSKAR_SETTINGS_TYPE_RANDOM_SEED_HPP_ */