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

#include "ntv2enums.h"
#include "TypeMap.h"

// 'Generic' display modes. These are copied verbatim from the BMDDisplayMode type defined in DeckLinkAPI.h within the Decklink SDK;
// As these were the first modes to be used within the project, and because they use a fairly sensible symbolic ASCII->integer mapping
// scheme they are being promoted to represent the generic values for a card-agnostic API
typedef enum _GenericDisplayMode
{
    bmdModeNTSC = 0x6e747363,
    bmdModeNTSC2398 = 0x6e743233,
    bmdModePAL = 0x70616c20,
    bmdModeNTSCp = 0x6e747370,
    bmdModePALp = 0x70616c70,
    bmdModeHD1080p2398 = 0x32337073,
    bmdModeHD1080p24 = 0x32347073,
    bmdModeHD1080p25 = 0x48703235,
    bmdModeHD1080p2997 = 0x48703239,
    bmdModeHD1080p30 = 0x48703330,
    bmdModeHD1080i50 = 0x48693530,
    bmdModeHD1080i5994 = 0x48693539,
    bmdModeHD1080i6000 = 0x48693630,
    bmdModeHD1080p50 = 0x48703530,
    bmdModeHD1080p5994 = 0x48703539,
    bmdModeHD1080p6000 = 0x48703630,
    bmdModeHD720p50 = 0x68703530,
    bmdModeHD720p5994 = 0x68703539,
    bmdModeHD720p60 = 0x68703630,
    bmdMode2k2398 = 0x326b3233,
    bmdMode2k24 = 0x326b3234,
    bmdMode2k25 = 0x326b3235,
    bmdMode2kDCI2398 = 0x32643233,
    bmdMode2kDCI24 = 0x32643234,
    bmdMode2kDCI25 = 0x32643235,
    bmdMode4K2160p2398 = 0x346b3233,
    bmdMode4K2160p24 = 0x346b3234,
    bmdMode4K2160p25 = 0x346b3235,
    bmdMode4K2160p2997 = 0x346b3239,
    bmdMode4K2160p30 = 0x346b3330,
    bmdMode4K2160p50 = 0x346b3530,
    bmdMode4K2160p5994 = 0x346b3539,
    bmdMode4K2160p60 = 0x346b3630,
    bmdMode4kDCI2398 = 0x34643233,
    bmdMode4kDCI24 = 0x34643234,
    bmdMode4kDCI25 = 0x34643235,
    bmdModeUnknown = 0x69756e6b
} 	GenericDisplayMode;


// 'Generic' pixel formats. These are copied verbatim from the BMDPixelFormat type defined in DeckLinkAPI.h within the Decklink SDK;
// As these were the first values to be used within the project, and because they use a fairly sensible symbolic ASCII->integer mapping
// scheme they are being promoted to represent the generic values for a card-agnostic API
typedef enum _GenericPixelFormat
{
    bmdFormat8BitYUV = 0x32767579,
    bmdFormat10BitYUV = 0x76323130,
    bmdFormat8BitARGB = 32,
    bmdFormat8BitBGRA = 0x42475241,
    bmdFormat10BitRGB = 0x72323130,
    bmdFormat12BitRGB = 0x52313242,
    bmdFormat12BitRGBLE = 0x5231324c,
    bmdFormat10BitRGBXLE = 0x5231306c,
    bmdFormat10BitRGBX = 0x52313062,
    bmdFormatH265 = 0x68657631,
    bmdFormatDNxHR = 0x41566468
} 	GenericPixelFormat;

typedef TypeMap<GenericDisplayMode, NTV2VideoFormat> DisplayModeMap;

DisplayModeMap::Entry _displayModeMapTable[] = // TODO: Implement mapping tests
{
    { bmdModeNTSC,          NTV2_FORMAT_525_5994 },
    { bmdModeNTSC2398,      NTV2_FORMAT_525_2398 },
    { bmdModePAL,           NTV2_FORMAT_625_5000 },
    { bmdModeNTSCp,         NTV2_FORMAT_525psf_2997 },
    { bmdModePALp,          NTV2_FORMAT_625psf_2500 },
    { bmdModeHD1080p2398,   NTV2_FORMAT_1080psf_2398 },
    { bmdModeHD1080p2398,   NTV2_FORMAT_1080p_2398 },
    { bmdModeHD1080p24,     NTV2_FORMAT_1080psf_2400 },
    { bmdModeHD1080p24,     NTV2_FORMAT_1080p_2400 }, // ?
    { bmdModeHD1080p25,     NTV2_FORMAT_1080p_2500 },
    { bmdModeHD1080p25,     NTV2_FORMAT_1080psf_2500_2 },
    { bmdModeHD1080p2997,   NTV2_FORMAT_1080p_2997 },
    { bmdModeHD1080p2997,   NTV2_FORMAT_1080psf_2997_2 }, // ?
    { bmdModeHD1080p30,     NTV2_FORMAT_1080p_3000 },
    { bmdModeHD1080p30,     NTV2_FORMAT_1080psf_3000_2 },
    { bmdModeHD1080i50,     NTV2_FORMAT_1080i_5000 },
    { bmdModeHD1080i5994,   NTV2_FORMAT_1080i_5994 },
    { bmdModeHD1080i6000,   NTV2_FORMAT_1080i_6000 },
    { bmdModeHD1080p50,     NTV2_FORMAT_1080p_5000 },
    { bmdModeHD1080p5994,   NTV2_FORMAT_1080p_5994 },
    { bmdModeHD1080p6000,   NTV2_FORMAT_1080p_6000 },
    { bmdModeHD720p50,      NTV2_FORMAT_720p_5000 },
    { bmdModeHD720p5994,    NTV2_FORMAT_720p_5994 },
    { bmdModeHD720p60,      NTV2_FORMAT_720p_6000 },
    { bmdMode2k2398,        NTV2_FORMAT_2K_2398 },
    { bmdMode2k24,          NTV2_FORMAT_2K_2400 },
    { bmdMode2k25,          NTV2_FORMAT_2K_2500 },
    { bmdMode2kDCI2398,     NTV2_FORMAT_2K_2398 },
    { bmdMode2kDCI24,       NTV2_FORMAT_2K_2400 },
    { bmdMode2kDCI25,       NTV2_FORMAT_2K_2500 },
    { bmdMode4K2160p2398,   NTV2_FORMAT_4x1920x1080psf_2398 },
    { bmdMode4K2160p2398,   NTV2_FORMAT_4x2048x1080p_2398 },
    { bmdMode4K2160p24,     NTV2_FORMAT_4x1920x1080psf_2400 },
    { bmdMode4K2160p24,     NTV2_FORMAT_4x1920x1080p_2400 },
    { bmdMode4K2160p25,     NTV2_FORMAT_4x1920x1080psf_2500 },
    { bmdMode4K2160p25,     NTV2_FORMAT_4x1920x1080p_2500 },
    { bmdMode4K2160p2997,   NTV2_FORMAT_4x1920x1080p_2997 },
    { bmdMode4K2160p30,     NTV2_FORMAT_4x1920x1080p_3000 },
    { bmdMode4K2160p50,     NTV2_FORMAT_4x1920x1080p_5000 },
    { bmdMode4K2160p5994,   NTV2_FORMAT_4x1920x1080p_5994 },
    { bmdMode4K2160p60,     NTV2_FORMAT_4x1920x1080p_6000 },
    { bmdMode4kDCI2398,     NTV2_FORMAT_4x2048x1080p_2398 },
    { bmdMode4kDCI24,       NTV2_FORMAT_4x2048x1080p_2400 },
    { bmdMode4kDCI25,       NTV2_FORMAT_4x2048x1080p_2500 },
    { bmdModeUnknown,       NTV2_FORMAT_UNKNOWN }
};

const DisplayModeMap DISPLAY_MODE_MAP(_displayModeMapTable, bmdModeUnknown, NTV2_FORMAT_UNKNOWN);

typedef TypeMap<GenericPixelFormat, NTV2FrameBufferFormat> PixelFormatMap;

PixelFormatMap::Entry _pixelFormatMapTable[] = // TODO: Implement mapping tests
{
    { bmdFormat8BitYUV, NTV2_FBF_8BIT_YCBCR },
    { bmdFormat10BitYUV, NTV2_FBF_10BIT_YCBCR },
    { bmdFormat8BitARGB, NTV2_FBF_ARGB },
    { bmdFormat8BitBGRA, NTV2_FBF_ABGR },
    { bmdFormat10BitRGB, NTV2_FBF_10BIT_RGB },
    { bmdFormat12BitRGB, NTV2_FBF_INVALID },
    { bmdFormat12BitRGBLE, NTV2_FBF_INVALID },
    { bmdFormat10BitRGBXLE, NTV2_FBF_10BIT_DPX_LITTLEENDIAN },
    { bmdFormat10BitRGBX, NTV2_FBF_10BIT_DPX }
};

const PixelFormatMap PIXEL_FORMAT_MAP(_pixelFormatMapTable, (GenericPixelFormat)0, NTV2_FBF_INVALID);
