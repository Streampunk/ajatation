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

/* TODO: add license
 ** 
 */

#ifndef CAPTURE_H
#define CAPTURE_H

#include <node.h>
#include <node_object_wrap.h>
#include <uv.h>
#include <node_buffer.h>
#include <nan.h>
#include <memory>

#include "ntv2capture.h"
#include "AudioTransform.h"

namespace streampunk {

class Capture : public Nan::ObjectWrap
{
private:
  explicit Capture(uint32_t deviceIndex = 0, uint32_t channelNumber = 0, uint32_t displayMode = 0, uint32_t pixelFormat = 0);
  ~Capture();

  static NAN_METHOD(New);
  static inline Nan::Persistent<v8::Function> &constructor();

  uv_async_t *async;
  uv_mutex_t padlock;

  // setup the AJA Kona interface (video standard, pixel format, callback object, ...)
  bool initNtv2Capture();

  HRESULT setupAudioInput(/*BMDAudioSampleRate sampleRate, BMDAudioSampleType sampleType,
    uint32_t channelCount*/);

  bool initInput();

  bool cleanupNtv2Capture();

  bool capture();
  bool stop();

  NTV2FrameBufferFormat getPixelFormat(uint32_t genericPixelFormat);

  // init() must be called after the constructor.
  // if init() fails, call the destructor
  //bool            init();
  static NAN_METHOD(DeviceInit);

  // start the capture operation. returns when the operation has completed
  static NAN_METHOD(DoCapture);

  static NAN_METHOD(StopCapture);

  static NAN_METHOD(EnableAudio);

  static NAUV_WORK_CB(FrameCallback);

  uint32_t deviceIndex_;
  uint32_t channelNumber_;
  uint32_t displayMode_;
  uint32_t genericPixelFormat_;
  //uint32_t width_;
  //uint32_t height_;
  bool audioEnabled_;

  AudioTransform audioTransform;

  // uint32_t sampleByteFactor_;
  Nan::Persistent<v8::Function> captureCB_;
  // IDeckLinkVideoInputFrame* latestFrame_;
  // IDeckLinkAudioInputPacket* latestAudio_;

  std::unique_ptr<NTV2Capture> capture_;

public:
  static NAN_MODULE_INIT(Init);

  // IDeckLinkInputCallback
  //virtual HRESULT    VideoInputFormatChanged (BMDVideoInputFormatChangedEvents notificationEvents, IDeckLinkDisplayMode* newDisplayMode, BMDDetectedVideoInputFormatFlags detectedSignalFlags);
  //virtual HRESULT    VideoInputFrameArrived (IDeckLinkVideoInputFrame* arrivedFrame, IDeckLinkAudioInputPacket*);
  virtual void TestUV();

  void frameArrived();
  static void _frameArrived(void* context);

  static const NTV2FrameBufferFormat defaultPixelFormat_ = NTV2_FBF_10BIT_YCBCR;
};

} // namespace streampunk

#endif
