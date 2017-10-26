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

#include <memory>
#include "Capture.h"
#include "gen2ajaTypeMaps.h"

namespace streampunk {

inline Nan::Persistent<v8::Function> &Capture::constructor() {
  static Nan::Persistent<v8::Function> myConstructor;
  return myConstructor;
}

Capture::Capture(uint32_t deviceIndex, uint32_t displayMode, uint32_t pixelFormat) 
: deviceIndex_(deviceIndex),
  displayMode_(displayMode), 
  genericPixelFormat_(pixelFormat),
  audioEnabled_(false)
  //genericPixelFormat_;
  //nativePixelFormat_
  //width_;
  //height_;
{
  async = new uv_async_t;
  uv_async_init(uv_default_loop(), async, FrameCallback);
  uv_mutex_init(&padlock);
  async->data = this;
}

Capture::~Capture() {
  if (!captureCB_.IsEmpty())
    captureCB_.Reset();
}

NAN_MODULE_INIT(Capture::Init) {

  // Prepare constructor template
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("Capture").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Nan::SetPrototypeMethod(tpl, "init", DeviceInit);
  Nan::SetPrototypeMethod(tpl, "doCapture", DoCapture);
  Nan::SetPrototypeMethod(tpl, "stop", StopCapture);
  Nan::SetPrototypeMethod(tpl, "enableAudio", EnableAudio);

  constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("Capture").ToLocalChecked(),
               Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(Capture::New) {

  if (info.IsConstructCall()) {
    // Invoked as constructor: `new Capture(...)`
    uint32_t deviceIndex = info[0]->IsUndefined() ? 0 : Nan::To<uint32_t>(info[0]).FromJust();
    uint32_t displayMode = info[1]->IsUndefined() ? 0 : Nan::To<uint32_t>(info[1]).FromJust();
    uint32_t pixelFormat = info[2]->IsUndefined() ? 0 : Nan::To<uint32_t>(info[2]).FromJust();
    Capture* obj = new Capture(deviceIndex, displayMode, pixelFormat);
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    // Invoked as plain function `Capture(...)`, turn into construct call.
    const int argc = 3;
    v8::Local<v8::Value> argv[argc] = { info[0], info[1], info[2] };
    v8::Local<v8::Function> cons = Nan::New(constructor());
    info.GetReturnValue().Set(Nan::NewInstance(cons, argc, argv).ToLocalChecked());
  }
}

NAN_METHOD(Capture::DeviceInit) {
  Capture* obj = ObjectWrap::Unwrap<Capture>(info.Holder());

  if (obj->initNtv2Capture())
      info.GetReturnValue().Set(Nan::New("made it!").ToLocalChecked());
  else
      info.GetReturnValue().Set(Nan::New("sad :-(").ToLocalChecked());
}

NAN_METHOD(Capture::EnableAudio) {
  Capture* obj = ObjectWrap::Unwrap<Capture>(info.Holder());
  HRESULT result;
  //BMDAudioSampleRate sampleRate = info[0]->IsNumber() ?
  //    (BMDAudioSampleRate) Nan::To<uint32_t>(info[0]).FromJust() : bmdAudioSampleRate48kHz;
  //BMDAudioSampleType sampleType = info[1]->IsNumber() ?
  //    (BMDAudioSampleType) Nan::To<uint32_t>(info[1]).FromJust() : bmdAudioSampleType16bitInteger;
  //uint32_t channelCount = info[2]->IsNumber() ? Nan::To<uint32_t>(info[2]).FromJust() : 2;

  result = obj->setupAudioInput(/*sampleRate, sampleType, channelCount*/);

  switch (result) {
    case E_INVALIDARG:
      info.GetReturnValue().Set(
        Nan::New<v8::String>("audio channel count must be 2, 8 or 16").ToLocalChecked());
      break;
    case S_OK:
      info.GetReturnValue().Set(Nan::New<v8::String>("audio enabled").ToLocalChecked());
      break;
    default:
      info.GetReturnValue().Set(Nan::New<v8::String>("failed to start audio").ToLocalChecked());
      break;
  }
}

NAN_METHOD(Capture::DoCapture) {
  v8::Local<v8::Function> cb = v8::Local<v8::Function>::Cast(info[0]);
  Capture* obj = ObjectWrap::Unwrap<Capture>(info.Holder());
  obj->captureCB_.Reset(cb);

  if (obj->capture())
  {
    info.GetReturnValue().Set(Nan::New("Capture started.").ToLocalChecked());
  }
  else
  {
      info.GetReturnValue().Set(Nan::New("Unable to start capture.").ToLocalChecked());
  }
}


NAN_METHOD(Capture::StopCapture) {
  Capture* obj = ObjectWrap::Unwrap<Capture>(info.Holder());

  if (obj->stop())
  {
    info.GetReturnValue().Set(Nan::New<v8::String>("Capture stopped.").ToLocalChecked());
  }
  else
  {
    info.GetReturnValue().Set(Nan::New<v8::String>("Unable to stop capture.").ToLocalChecked());
  }
}


bool Capture::capture()
{
    bool success = false;

    if (capture_)
    {
        capture_->Run();

        success = true;
    }

    return success;
}


bool Capture::stop()
{
    bool success = false;

    if (capture_)
    {
        capture_->Quit();

        success = true;
    }

    return success;
}


bool Capture::initNtv2Capture()
{
    bool  success(false);
    string deviceSpec(AjaDevice::DEFAULT_DEVICE_SPECIFIER);
    char buffer[10];
    
    if(_itoa_s(deviceIndex_, buffer, 10) == 0)
    {
        deviceSpec = buffer;
    }

    uint32_t                     channelNumber(AjaDevice::DEFAULT_CAPTURE_CHANNEL);                    //    Number of the channel to use
    const NTV2FrameBufferFormat  pixelFormat(getPixelFormat(genericPixelFormat_));
    bool                         multiFormat(false); 
    bool                         captureAncilliaryData(false); 
    AJAStatus                    status(AJA_STATUS_SUCCESS);

    cout << "Capture initializing with pixelFormat " << pixelFormat << endl;

//    Instantiate the NTV2Capture object, using the specified AJA device...
    capture_.reset(new NTV2Capture(&DEFAULT_INIT_PARAMS,
        deviceSpec, true,                             //    With audio?
        ::GetNTV2ChannelForIndex(channelNumber - 1),    //    Channel
        pixelFormat,                                    //    Pixel format
        false,                                          //    Level A/B conversion?
        multiFormat,                                    //    Multi-format mode?
        captureAncilliaryData));                        //    Capture Anc data?

    //    Initialize the capture device...
    status = capture_->Init();
    if (AJA_SUCCESS(status))
    {
        capture_->SetFrameArrivedCallback(this, Capture::_frameArrived);

        success = true;
    }    //    if capture Init succeeded
    else
    {
        cerr << "Capture initialization failed with status " << status << endl;
        capture_.reset();
    }

    return success;
}


HRESULT Capture::setupAudioInput(/*BMDAudioSampleRate sampleRate,
  BMDAudioSampleType sampleType, uint32_t channelCount*/) {

  audioEnabled_ = true;
  // TODO: handle audio properly

  //sampleByteFactor_ = channelCount * (sampleType / 8);
  //HRESULT result = m_deckLinkInput->EnableAudioInput(sampleRate, sampleType, channelCount);

  return S_OK;
}

// Stop video input
bool Capture::cleanupNtv2Capture()
{
    bool success = false;

    if (capture_)
    {
        capture_->Quit();
        capture_.reset();

        success = true;
    }

    return success;
}

bool Capture::initInput() {
  return true;
}


void Capture::frameArrived()
{
  uv_async_send(async);
}


void Capture::_frameArrived(void* context)
{
    Capture* localThis = reinterpret_cast<Capture*>(context);

    localThis->frameArrived();
}


//HRESULT    Capture::VideoInputFormatChanged (BMDVideoInputFormatChangedEvents notificationEvents, IDeckLinkDisplayMode* newDisplayMode, BMDDetectedVideoInputFormatFlags detectedSignalFlags) {
//  return S_OK;
//};

void Capture::TestUV() {
  uv_async_send(async);
}


NAUV_WORK_CB(Capture::FrameCallback) {
  Nan::HandleScope scope;
  Capture *capture = static_cast<Capture*>(async->data);
  Nan::Callback cb(Nan::New(capture->captureCB_));

  v8::Local<v8::Value> bv = Nan::Null();
  v8::Local<v8::Value> ba = Nan::Null();
  uv_mutex_lock(&capture->padlock);

  AVDataBuffer* nextFrame = capture->capture_->LockNextFrame();

  if (nextFrame != nullptr)
  {
    if (nextFrame->fVideoBuffer != nullptr)
    {
        bv = Nan::CopyBuffer(reinterpret_cast<char*>(nextFrame->fVideoBuffer), nextFrame->fVideoBufferSize).ToLocalChecked();
    }

    if (nextFrame->fAudioBuffer != nullptr && capture->audioEnabled_ == true)
    {
        // Transform the output buffer into the desired number of channels - currently this is 2
        const char* audioTransformBuffer(nullptr);
        uint32_t audioTransformBufferSize(0);

        tie(audioTransformBuffer, audioTransformBufferSize) = 
            capture->audioTransform.Transform(reinterpret_cast<char*>(nextFrame->fAudioBuffer), nextFrame->fAudioBufferSize, 16, 2);

        ba = Nan::CopyBuffer(audioTransformBuffer, audioTransformBufferSize).ToLocalChecked();
    }

    capture->capture_->UnlockFrame();
    nextFrame = nullptr;
  }

  v8::Local<v8::Value> argv[2] = { bv, ba };
  cb.Call(2, argv);
}




NTV2FrameBufferFormat Capture::getPixelFormat(uint32_t genericPixelFormat)
{
    NTV2FrameBufferFormat pixelFormat(defaultPixelFormat_);
    NTV2FrameBufferFormat convertedFormat = PIXEL_FORMAT_MAP.ToB(static_cast<GenericPixelFormat>(genericPixelFormat));

    if (convertedFormat != NTV2_FBF_INVALID)
    {
        pixelFormat = convertedFormat;
    }

    return pixelFormat;
}

}
