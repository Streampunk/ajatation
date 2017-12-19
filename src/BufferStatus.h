/* Copyright 2017 Streampunk Media Ltd.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#pragma once

#include <chrono>

namespace streampunk {

// Class to manage the recording of buffer statistics for performance tuning the AV pipeline
class BufferStatus 
{
// Types
//
public:

    struct Counter
    {
        uint32_t samples;
        float average;
        double accumulator;

        Counter()
        :   samples(0), average(0.0), accumulator(0.0)
        {}

        void Reset()
        {
            samples = 0;
            average = 0.0;
            accumulator = 0.0;
        }
    };

    enum CounterTypes
    {
        CaptureCircBuffer = 0,
        CaptureCardBuffer,
        PlaybackCircBuffer,
        PlaybackCardBuffer,
        CounterTypes_LAST
    };

// Methods
//
public:

    static void AddSample(CounterTypes counter, float value);

    ~BufferStatus() {}

private:

    BufferStatus();

    void AddSampleImpl(CounterTypes counter, float value);
    void DumpCounters(uint32_t index);

    // Double-buffer the counters so that they don't have to be synchronised for writing/reading.
    // There will be occasional collisions, but this isn't super-critical, and it is better to
    // not need to put any thread-synchronization around this as it is just a debug feature
    Counter counters_[2][CounterTypes_LAST];
    std::chrono::high_resolution_clock::time_point timeSinceUpdate_;
    uint32_t writeIdx_;

    static BufferStatus instance_;
};
}