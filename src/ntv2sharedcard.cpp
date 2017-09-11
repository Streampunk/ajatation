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

#include "ntv2sharedcard.h"

CNTV2SharedCard::CNTV2SharedCard()
:   CNTV2Card()
{
}


CNTV2SharedCard::CNTV2SharedCard (const UWord inDeviceIndex,
                                  const bool  inDisplayError,
                                  const UWord inDeviceType,
                                  const char* pInHostName)
:   CNTV2Card(inDeviceIndex, inDisplayError, inDeviceType, pInHostName),
    captureActive_(false)
{
}


CNTV2SharedCard::~CNTV2SharedCard()
{
}


bool CNTV2SharedCard::SetReference(bool isCapture, NTV2ReferenceSource value)
{
    bool success(true);

    // If this is a capture session, then always set the reference as requested
    if(isCapture)
    {
        captureActive_ = true;
        success = CNTV2Card::SetReference(value);
    }
    else if(captureActive_ == false)
    {
        // Otherwise only set the reference if there isn't a capture session active
        success = CNTV2Card::SetReference(value);
    }

    return success;
}


bool CNTV2SharedCard::SetVideoFormat(bool isCapture, 
                                     NTV2VideoFormat inVideoFormat, 
                                     bool inIsAJARetail, 
                                     bool inKeepVancSettings,
                                     NTV2Channel inChannel)
{
    bool success(true);

    // If this is a capture session, then always set the video format as requested
    if(isCapture)
    {
        captureActive_ = true;
        success = CNTV2Card::SetVideoFormat(inVideoFormat, inIsAJARetail, inKeepVancSettings, inChannel);
    }
    else if(captureActive_ == false)
    {
        // Otherwise only set the format if there isn't a capture session active
        success = CNTV2Card::SetVideoFormat(inVideoFormat, inIsAJARetail, inKeepVancSettings, inChannel);
    }

    return success;
}


bool CNTV2SharedCard::SetReference(NTV2ReferenceSource value)
{
    assert(false);

    return CNTV2Card::SetReference(value);
}


bool CNTV2SharedCard::SetVideoFormat(NTV2VideoFormat inVideoFormat, 
                                     bool inIsAJARetail, 
                                     bool inKeepVancSettings,
                                     NTV2Channel inChannel)
{
    assert(false);

    return CNTV2Card::SetVideoFormat(inVideoFormat, inIsAJARetail, inKeepVancSettings, inChannel);
}