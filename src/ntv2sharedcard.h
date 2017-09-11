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

#include "ntv2card.h"

// Class to manage the reservation and release of AJA devices between multiple nodes
class CNTV2SharedCard : public CNTV2Card
{
public:

    /**
        @brief    My default constructor.
    **/
    CNTV2SharedCard ();

    /**
        @brief    Constructs me from the given parameters.
        @param[in]    inDeviceIndex    A zero-based index number that identifies which device to open,
                                    which should be the number received from the NTV2DeviceScanner.
        @param[in]    inDisplayError    If true, displays a message box if there's a failure while opening.
                                    This parameter is obsolete and won't be available in the future.
        @param[in]    inDeviceType    Specifies the NTV2DeviceType of the device to open.
                                    This parameter is obsolete and won't be available in the future.
        @param[in]    pInHostName        If non-NULL, must be a valid pointer to a character buffer that
                                    contains the name of a host that has one or more AJA devices.
                                    Defaults to NULL (the local host).
        @nosubgrouping
    **/
    explicit CNTV2SharedCard ( const UWord inDeviceIndex,
                               const bool  inDisplayError = false,
                               const UWord inDeviceType   = DEVICETYPE_NTV2,
                               const char* pInHostName    = 0);
    /**
        @brief    My destructor.
    **/
    virtual ~CNTV2SharedCard();

// Class overrides
public:

    AJA_VIRTUAL bool        SetReference(bool isCapture, NTV2ReferenceSource value);

    /**
        @brief        Configures the AJA device to handle a specific video format.
        @param[in]    inVideoFormat         Specifies the desired video format for the given channel on the device.
                                            It must be a valid NTV2VideoFormat constant.
        @param[in]    inIsAJARetail         Specify 'true' to preserve the current horizontal and vertical timing settings.
                                            Defaults to true on MacOS, false on other platforms.
        @param[in]    inKeepVancSettings    If true, specifies that the device's current VANC settings are to be preserved;
                                            otherwise, they will not be preserved. Defaults to false.
        @param[in]    inChannel             Specifies the NTV2Channel of interest. Defaults to NTV2_CHANNEL1.
                                            For UHD/4K video formats, specify NTV2_CHANNEL1 to configure quadrant channels 1-4,
                                            or NTV2_CHANNEL5 to configure quadrant channels 5-8.
        @return       True if successful; otherwise false.
        @details      This function changes the device configuration to a specific video standard (e.g., 525, 1080, etc.),
                      frame geometry (e.g., 1920x1080, 720x486, etc.) and frame rate (e.g., 59.94 fps, 29.97 fps, etc.),
                      plus a few other settings (e.g., progressive/interlaced, etc.), all based on the given video format.
    **/
    AJA_VIRTUAL bool SetVideoFormat (bool isCapture, NTV2VideoFormat inVideoFormat, bool inIsAJARetail = AJA_RETAIL_DEFAULT, bool inKeepVancSettings = false, NTV2Channel inChannel = NTV2_CHANNEL1);

private:

    // Try to hide this implementation, as for correct operation this needs to co-ordinate between capture and playback devices
    AJA_VIRTUAL bool SetReference (NTV2ReferenceSource value);
    AJA_VIRTUAL bool SetVideoFormat (NTV2VideoFormat inVideoFormat, bool inIsAJARetail = AJA_RETAIL_DEFAULT, bool inKeepVancSettings = false, NTV2Channel inChannel = NTV2_CHANNEL1);

    bool captureActive_;
};