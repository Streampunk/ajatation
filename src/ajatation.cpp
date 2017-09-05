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

/* TODO - Add license
*/

#define _WINSOCKAPI_

#include <node.h>
#include "node_buffer.h"
#include <stdio.h>
#include <nan.h>

#ifdef WIN32
#include <tchar.h>
#include <conio.h>
#include <objbase.h>		// Necessary for COM
#include <comdef.h>
#endif

#include "Capture.h"
#include "Playback.h"

using namespace v8;

NAN_METHOD(deviceSdkVersion) {

  //TODO Implement properly

  char sdkVer [80];
  int	dlVerMajor, dlVerMinor, dlVerPoint;

  dlVerMajor = 0;
  dlVerMinor = 0;
  dlVerPoint = 0;

  sprintf_s(sdkVer, "TODO: SDK API version: %d.%d.%d", dlVerMajor, dlVerMinor, dlVerPoint);

  info.GetReturnValue().Set(Nan::New(sdkVer).ToLocalChecked());
}

NAN_METHOD(GetFirstDevice) {
  //IDeckLinkIterator* deckLinkIterator;
  //HRESULT	result;
  //IDeckLinkAPIInformation *deckLinkAPIInformation;
  //IDeckLink* deckLink;
  //#ifdef WIN32
  //CoCreateInstance(CLSID_CDeckLinkIterator, NULL, CLSCTX_ALL, IID_IDeckLinkIterator, (void**)&deckLinkIterator);
  //#else
  //deckLinkIterator = CreateDeckLinkIteratorInstance();
  //#endif
  //result = deckLinkIterator->QueryInterface(IID_IDeckLinkAPIInformation, (void**)&deckLinkAPIInformation);
  //if (result != S_OK) {
  //  Nan::ThrowError("Error connecting to DeckLinkAPI.");
  //}
  //if (deckLinkIterator->Next(&deckLink) != S_OK) {
  //  info.GetReturnValue().SetUndefined();
  //  return;
  //}
  #ifdef WIN32
  //BSTR deviceNameBSTR = NULL;
  //result = deckLink->GetModelName(&deviceNameBSTR);
  //if (result == S_OK) {
    //_bstr_t deviceName(deviceNameBSTR, false);
    info.GetReturnValue().Set(Nan::New((char*) "TODO - proper device iteration").ToLocalChecked());
    return;
  //}
  #elif __APPLE__
  CFStringRef deviceNameCFString = NULL;
  result = deckLink->GetModelName(&deviceNameCFString);
  if (result == S_OK) {
    char deviceName [64];
    CFStringGetCString(deviceNameCFString, deviceName, sizeof(deviceName), kCFStringEncodingMacRoman);
    info.GetReturnValue().Set(Nan::New(deviceName).ToLocalChecked());
    return;
  }
  #endif
  info.GetReturnValue().SetUndefined();
}



/* static Local<Object> makeBuffer(char* data, size_t size) {
  HandleScope scope;

  // It ends up being kind of a pain to convert a slow buffer into a fast
  // one since the fast part is implemented in JavaScript.
  Local<Buffer> slowBuffer = Buffer::New(data, size);
  // First get the Buffer from global scope...
  Local<Object> global = Context::GetCurrent()->Global();
  Local<Value> bv = global->Get(String::NewSymbol("Buffer"));
  assert(bv->IsFunction());
  Local<Function> b = Local<Function>::Cast(bv);
  // ...call Buffer() with the slow buffer and get a fast buffer back...
  Handle<Value> argv[3] = { slowBuffer->handle_, Integer::New(size), Integer::New(0) };
  Local<Object> fastBuffer = b->NewInstance(3, argv);

  return scope.Close(fastBuffer);
} */

NAN_MODULE_INIT(Init) {
  Nan::Export(target, "deviceSdkVersion", deviceSdkVersion);
  Nan::Export(target, "getFirstDevice", GetFirstDevice);
  //streampunk::Capture::Init(target);
  streampunk::Playback::Init(target);
  streampunk::Capture::Init(target);
}

NODE_MODULE(ajatation, Init);
