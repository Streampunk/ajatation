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

#include <stdint.h>
#include <assert.h>
#include <iostream>
#include <iomanip>
#include "BufferStatus.h"

using namespace std;
using namespace std::chrono;

namespace streampunk {

BufferStatus BufferStatus::instance_;

BufferStatus::BufferStatus()
:   timeSinceUpdate_(high_resolution_clock::now()),
    writeIdx_(0)
{
}


void BufferStatus::AddSample(CounterTypes counter, float value)
{
    instance_.AddSampleImpl(counter, value);
}


void BufferStatus::AddSampleImpl(CounterTypes counter, float value)
{
    assert(counter < CounterTypes_LAST);

    Counter& activeCounter = counters_[writeIdx_][counter];
    activeCounter.samples++;
    activeCounter.accumulator += value;
    activeCounter.average = static_cast<float>(activeCounter.accumulator / activeCounter.samples);

    duration<double> timeSpan = high_resolution_clock::now() - timeSinceUpdate_;

    if(timeSpan.count() > 1.0)
    {
        timeSinceUpdate_ = high_resolution_clock::now();

        auto oldIdx = writeIdx_;
        writeIdx_ = (writeIdx_ + 1) % 2;

        DumpCounters(oldIdx);
        
        for(int i = 0; i < CounterTypes_LAST; i++)
        {
            counters_[oldIdx][i].Reset();
        }

    }
}


void BufferStatus::DumpCounters(uint32_t index)
{
    bool output(false);

    if(counters_[index][CaptureCircBuffer].samples > 0)
    {
        cout << "CapCirc: " << setw(4) << setprecision(6) << counters_[index][CaptureCircBuffer].average << "%  ";
        output = true;
    }

    if(counters_[index][CaptureCardBuffer].samples > 0)
    {
        cout << "CapCard: " << setw(4) << setprecision(6) << counters_[index][CaptureCardBuffer].average << "%  ";
        output = true;
    }

    if(counters_[index][PlaybackCircBuffer].samples > 0)
    {
        cout << "PbCirc: " << setw(4) << setprecision(6) << counters_[index][PlaybackCircBuffer].average << "%  ";
        output = true;
    }

    if(counters_[index][PlaybackCardBuffer].samples > 0)
    {
        cout << "PbCard: " << setw(4) << setprecision(6) << counters_[index][PlaybackCardBuffer].average << "%  ";
        output = true;
    }

    if(output)
    {
        cout << endl;
    }
}

}
