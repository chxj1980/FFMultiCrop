﻿/**
 * Copyright 2019 Matthew Oliver
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "FFFRTestData.h"
#include "FFFrameReader.h"
#include "FFMultiCrop.h"

#include <gtest/gtest.h>
using namespace Fmc;

struct TestParamsEncode
{
    uint32_t m_testDataIndex;
    std::vector<CropOptions> m_cropList;
};

static CropOptions s_options1 = {{{0, 0}, {0, 1}}, {60, 40}, "test-mc-1.mkv"};

static std::vector<TestParamsEncode> g_testDataEncode = {
    {0, {s_options1}},
};

class EncodeTest1 : public ::testing::TestWithParam<TestParamsEncode>
{
protected:
    EncodeTest1() = default;

    void SetUp() override
    {
        Ffr::setLogLevel(Ffr::LogLevel::Error);
        m_cropOps = GetParam().m_cropList;

        // Generate list of crops
        int32_t directionX = 1;
        int32_t directionY = 1;
        int32_t x = 0, y = 0;
        for (auto& j : m_cropOps) {
            for (int32_t i = 0; i < 1000; ++i) {
                if (x >= 1920 || x < 0) {
                    directionX *= -1;
                    x += 2 * directionX;
                }
                if (y >= 1080 || y < 0) {
                    directionY *= -1;
                    --y;
                }
                j.m_cropList.emplace_back(Crop({static_cast<uint32_t>(x), static_cast<uint32_t>(y)}));
                y += directionY;
                x += 2 * directionX;
            }
        }
    }

    void TearDown() override {}

    std::vector<CropOptions> m_cropOps;
};

TEST_P(EncodeTest1, encodeStream)
{
    // Just run an encode and see if output is correct manually
    ASSERT_TRUE(MultiCrop::cropAndEncode(g_testData[GetParam().m_testDataIndex].m_fileName, m_cropOps));

    // Check that we can open encoded file and its parameters are correct
    for (const auto& i : m_cropOps) {
        auto stream = Ffr::Stream::getStream(i.m_fileName);
        ASSERT_NE(stream, nullptr);

        ASSERT_EQ(stream->getWidth(), i.m_resolution.m_width);
        ASSERT_EQ(stream->getHeight(), i.m_resolution.m_height);
        ASSERT_EQ(stream->getTotalFrames(), i.m_cropList.size());
        ASSERT_DOUBLE_EQ(stream->getFrameRate(), g_testData[GetParam().m_testDataIndex].m_frameRate);
    }
}

INSTANTIATE_TEST_SUITE_P(EncodeTestData, EncodeTest1, ::testing::ValuesIn(g_testDataEncode));