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

#include "gen2ajaTypeMaps.h"

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
