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

#define _WINSOCKAPI_

#include <node.h>
#include "node_buffer.h"
#include <stdio.h>
#include <nan.h>

#ifdef WIN32
#include <tchar.h>
#include <conio.h>
#endif

#include "ajabase/system/info.h"
#include "Capture.h"
#include "Playback.h"
#include "AjaDevice.h"

using namespace v8;

void PrintSystemInfo()
{
    cout << "AJA Device System Info:" << endl;
    AJASystemInfo info;

    for(uint32_t tag = 0; tag < (uint32_t)AJA_SystemInfoTag_LAST; tag++)
    {
        string label;
        string value;

        info.GetLabel(static_cast<AJASystemInfoTag>(tag), label);
        info.GetValue(static_cast<AJASystemInfoTag>(tag), value);
        
        cout << "    " << label << ": " << value << endl;
    }
}

NAN_METHOD(deviceSdkVersion) 
{
  UWord major(0);
  UWord minor(0);
  UWord point(0);
  UWord build(0);

  char sdkVer [80];

  if(AjaDevice::GetDriverVersion(major, minor, point,build))
  {
    sprintf_s(sdkVer, "Aja Driver Version: %d.%d.%d.%d", major, minor, point,build);
  }
  else
  {
    sprintf_s(sdkVer, "Aja Driver Version: Unavailable - ERROR");
  }

  info.GetReturnValue().Set(Nan::New(sdkVer).ToLocalChecked());
}

NAN_METHOD(getFirstDevice) {
  uint32_t numDevices;
  uint32_t firstDeviceIndex;
  string deviceIdentifier;
  uint64_t deviceSerialNumber;

  if(AjaDevice::GetFirstDevice(numDevices, &firstDeviceIndex, &deviceIdentifier, &deviceSerialNumber))
  {
    info.GetReturnValue().Set(Nan::New(deviceIdentifier).ToLocalChecked());
  }
  else
  {
    info.GetReturnValue().SetUndefined();
  }
}


NAN_MODULE_INIT(Init) {
  Nan::Export(target, "deviceSdkVersion", deviceSdkVersion);
  Nan::Export(target, "getFirstDevice", getFirstDevice);
  streampunk::Playback::Init(target);
  streampunk::Capture::Init(target);
}

NODE_MODULE(ajatation, Init);
