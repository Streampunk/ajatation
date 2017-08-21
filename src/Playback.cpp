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

/* -LICENSE-START-
 ** TODO: add license
 ** -LICENSE-END-
 */

#include "Playback.h"
#include <string.h>
#include "ntv2player.h"
#include "ajatypes.h"
#include "ntv2utils.h"
#include "ajabase/common/options_popt.h"
#include "ntv2player.h"
#include <signal.h>
#include <iostream>
#include <iomanip>
#include "ajabase/system/systemtime.h"
#include "gen2ajaTypeMaps.h"

using namespace std;

//	Globals
static bool		gGlobalQuit(false);	//	Set this "true" to exit gracefully

static void SignalHandler(int inSignal)
{
	(void)inSignal;
	gGlobalQuit = true;
}

namespace streampunk {

inline Nan::Persistent<v8::Function> &Playback::constructor() {
  static Nan::Persistent<v8::Function> myConstructor;
  return myConstructor;
}

Playback::Playback(uint32_t deviceIndex, uint32_t displayMode, uint32_t pixelFormat)
: displayMode_(displayMode), pixelFormat_(pixelFormat)
{
  async = new uv_async_t;
  uv_async_init(uv_default_loop(), async, FrameCallback);
  uv_mutex_init(&padlock);
  async->data = this;
}

Playback::~Playback() {
  if (!playbackCB_.IsEmpty())
    playbackCB_.Reset();
}

NAN_MODULE_INIT(Playback::Init) {

  // Prepare constructor template
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("Playback").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Nan::SetPrototypeMethod(tpl, "init", DeviceInit);
  Nan::SetPrototypeMethod(tpl, "scheduleFrame", ScheduleFrame);
  Nan::SetPrototypeMethod(tpl, "doPlayback", DoPlayback);
  Nan::SetPrototypeMethod(tpl, "stop", StopPlayback);

  constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("Playback").ToLocalChecked(),
               Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(Playback::New) {
  if (info.IsConstructCall()) {
    // Invoked as constructor: `new Playback(...)`
    uint32_t deviceIndex = info[0]->IsUndefined() ? 0 : Nan::To<uint32_t>(info[0]).FromJust();
    uint32_t displayMode = info[1]->IsUndefined() ? 0 : Nan::To<uint32_t>(info[1]).FromJust();
    uint32_t pixelFormat = info[2]->IsUndefined() ? 0 : Nan::To<uint32_t>(info[2]).FromJust();
    Playback* obj = new Playback(deviceIndex, displayMode, pixelFormat);
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    // Invoked as plain function `Playback(...)`, turn into construct call.
    const int argc = 3;
    v8::Local<v8::Value> argv[argc] = { info[0], info[1], info[2] };
    v8::Local<v8::Function> cons = Nan::New(constructor());
    info.GetReturnValue().Set(Nan::NewInstance(cons, argc, argv).ToLocalChecked());
  }
}

NAN_METHOD(Playback::DeviceInit) {
	Playback* obj = ObjectWrap::Unwrap<Playback>(info.Holder());

	if (obj->initNtv2Player())
        info.GetReturnValue().Set(Nan::New("made it!").ToLocalChecked());
    else
        info.GetReturnValue().Set(Nan::New("sad :-(").ToLocalChecked());
}

NAN_METHOD(Playback::DoPlayback) {
  v8::Local<v8::Function> cb = v8::Local<v8::Function>::Cast(info[0]);
  Playback* obj = ObjectWrap::Unwrap<Playback>(info.Holder());
  obj->playbackCB_.Reset(cb);

  bool result = obj->play();
  // printf("Playback result code %i and timescale %I64d.\n", result, obj->m_timeScale);

  if (result == true) {
    info.GetReturnValue().Set(Nan::New("Playback started.").ToLocalChecked());
  }
  else {
    info.GetReturnValue().Set(Nan::New("Playback failed to start.").ToLocalChecked());
  }
}

NAN_METHOD(Playback::StopPlayback) {
  Playback* obj = ObjectWrap::Unwrap<Playback>(info.Holder());

  obj->stop();

  obj->playbackCB_.Reset();

  info.GetReturnValue().Set(Nan::New("Playback stopped.").ToLocalChecked());
}

NAN_METHOD(Playback::ScheduleFrame) {
  Playback* obj = ObjectWrap::Unwrap<Playback>(info.Holder());
  v8::Local<v8::Object> bufObj = Nan::To<v8::Object>(info[0]).ToLocalChecked();

  char* bufData = node::Buffer::Data(bufObj);
  size_t bufLength = node::Buffer::Length(bufObj);

  obj->scheduleFrame(bufData, bufLength);
}


bool Playback::initNtv2Player()
{
	bool  success(false);
	AJAStatus		status(AJA_STATUS_SUCCESS);
	const string deviceSpec("0");
	const NTV2VideoFormat	videoFormat(getVideoFormat(displayMode_));
	const NTV2FrameBufferFormat	pixelFormat(getPixelFormat(pixelFormat_));

	uint32_t		channelNumber(1);					//	Number of the channel to use
	int				noAudio(0);					//	Disable audio tone?
	const NTV2Channel channel(::GetNTV2ChannelForIndex(channelNumber - 1));
	const NTV2OutputDestination	outputDest(::NTV2ChannelToOutputDestination(channel));
	int				doMultiChannel(0);					//	Enable multi-format?
	AJAAncillaryDataType sendType = AJAAncillaryDataType_Unknown;

	player_.reset( new NTV2Player(deviceSpec, (noAudio ? false : true), channel, pixelFormat, outputDest, videoFormat, false, false, doMultiChannel ? true : false, sendType));

	::signal(SIGINT, SignalHandler);

	//	Initialize the player...
	status = player_->Init();
	if (AJA_SUCCESS(status))
	{
		player_->SetScheduledFrameCallback(this, Playback::_scheduledFrameCompleted);

		success = true;
	}	//	if player Init succeeded
	else
	{
		cerr << "Player initialization failed with status " << status << endl;
		player_.reset();
	}

	return success;
}


bool Playback::shutdownNtv2Player()
{
	bool success = false;

	if (player_)
	{
		player_->Quit();
		player_.reset();

		success = true;
	}

	return success;
}


bool Playback::play()
{
	bool success = false;

	if (player_)
	{
		player_->Run();

		success = true;
	}

	return success;
}


bool Playback::stop()
{
	bool success = false;

	if (player_)
	{
		player_->Quit();

		success = true;
	}

	return success;
}


bool Playback::scheduleFrame(const char* data, const size_t length)
{
	bool success = false;

	if (player_)
	{
		success = player_->ScheduleFrame(data, length, nullptr, 0);
	}

	return success;
}


void Playback::scheduledFrameCompleted()
{
	uv_async_send(async);
}


void Playback::_scheduledFrameCompleted(void* context)
{
	Playback* localThis = reinterpret_cast<Playback*>(context);

	localThis->scheduledFrameCompleted();
}


NTV2VideoFormat Playback::getVideoFormat(uint32_t genericDisplayMode)
{
	NTV2VideoFormat videoFormat(defaultVideoFormat_);
	NTV2VideoFormat convertedFormat = DISPLAY_MODE_MAP.ToB(static_cast<GenericDisplayMode>(genericDisplayMode));

	if (convertedFormat != NTV2_FORMAT_UNKNOWN)
	{
		videoFormat = convertedFormat;
	}

	return videoFormat;
}


NTV2FrameBufferFormat Playback::getPixelFormat(uint32_t genericPixelFormat)
{
	NTV2FrameBufferFormat pixelFormat(defaultPixelFormat_);
	NTV2FrameBufferFormat convertedFormat = PIXEL_FORMAT_MAP.ToB(static_cast<GenericPixelFormat>(genericPixelFormat));


	if (convertedFormat != NTV2_FBF_INVALID)
	{
		pixelFormat = convertedFormat;
	}

	return pixelFormat;
}


NAUV_WORK_CB(Playback::FrameCallback) {
  Nan::HandleScope scope;
  Playback *playback = static_cast<Playback*>(async->data);
  uv_mutex_lock(&playback->padlock);
  if (!playback->playbackCB_.IsEmpty()) {
    Nan::Callback cb(Nan::New(playback->playbackCB_));

    v8::Local<v8::Value> argv[1] = { Nan::New(playback->result_) };
    cb.Call(1, argv);
  } else {
    printf("Frame callback is empty. Assuming finished.\n");
  }
  uv_mutex_unlock(&playback->padlock);
}

}
