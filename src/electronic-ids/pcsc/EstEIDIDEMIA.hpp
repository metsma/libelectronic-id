/*
 * Copyright (c) 2020-2024 Estonian Information System Authority
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "EIDIDEMIA.hpp"

// ESTEID specification:
// https://installer.id.ee/media/id2019/TD-ID1-Chip-App.pdf

namespace electronic_id
{

class EstEIDIDEMIAV1 : public EIDIDEMIA
{
public:
    using EIDIDEMIA::EIDIDEMIA;

private:
    PinMinMaxLength authPinMinMaxLength() const override { return {4, 12}; }
    PinMinMaxLength signingPinMinMaxLength() const override { return {5, 12}; }
    std::string name() const override { return "EstEID IDEMIA v1"; }
    Type type() const override { return EstEID; }
};

} // namespace electronic_id
