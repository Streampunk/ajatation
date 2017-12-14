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

#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <node.h>
#include <node_object_wrap.h>
#include <uv.h>
#include <node_buffer.h>
#include <nan.h>
#include <memory>

#include "ntv2player.h"
#include "AudioTransform.h"

namespace streampunk {

class Playback : public Nan::ObjectWrap
{
public:
    static NAN_MODULE_INIT(Init);

private:
    explicit Playback(uint32_t deviceIndex = 0, uint32_t channelNumber = 3, uint32_t displayMode = 0, uint32_t pixelFormat = 0);
    ~Playback();

    static NAN_METHOD(New);
    static inline Nan::Persistent<v8::Function> &constructor();

    uv_async_t *async;
    uv_mutex_t padlock;


    static NAN_METHOD(DeviceInit);

    static NAN_METHOD(DoPlayback);

    static NAN_METHOD(StopPlayback);

    static NAN_METHOD(ScheduleFrame);

    static NAUV_WORK_CB(FrameCallback);
    Nan::Persistent<v8::Function> playbackCB_;
    uint32_t result_;

    std::unique_ptr<NTV2Player> player_;

private:

    bool initNtv2Player();
    bool shutdownNtv2Player();
    bool play();
    bool stop();
    bool scheduleFrame(const char* videoData, const size_t videoDataLength, const char* audioData, const size_t audioDataLength, uint32_t& bufferedFrames);

    void scheduledFrameCompleted();
    static void _scheduledFrameCompleted(void* context);

    NTV2VideoFormat getVideoFormat(uint32_t genericDisplayMode);
    NTV2FrameBufferFormat getPixelFormat(uint32_t genericPixelFormat);

    uint32_t deviceIndex_;
    uint32_t channelNumber_;
    uint32_t displayMode_;
    uint32_t pixelFormat_;

    AudioTransform audioTransform;

    static const NTV2VideoFormat defaultVideoFormat_ = NTV2_FORMAT_1080i_5994;
    static const NTV2FrameBufferFormat defaultPixelFormat_ = NTV2_FBF_10BIT_YCBCR;
};

}

#endif
