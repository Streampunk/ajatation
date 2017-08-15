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

/* TODO - add license
*/

'use strict';
var os = require('os');
var isWinOrMac = (os.platform() === 'win32') || (os.platform() === 'darwin');
if (!isWinOrMac)
  throw('Ajatation is not currently supported on this platform');

var bindings = require('bindings');
var ajatatorNative = bindings('ajatation');
const util = require('util');
const EventEmitter = require('events');

// var SegfaultHandler = require('../node-segfault-handler');
// SegfaultHandler.registerHandler("crash.log");
/*
function Capture (deviceIndex, displayMode, pixelFormat) {
  if (arguments.length !== 3 || typeof deviceIndex !== 'number' ||
      typeof displayMode !== 'number' || typeof pixelFormat !== 'number' ) {
    this.emit('error', new Error('Capture requires three number arguments: ' +
      'index, display mode and pixel format'));
  } else {
    this.capture = new ajatatorNative.Capture(deviceIndex, displayMode, pixelFormat);
  }
  this.initialised = false;
  EventEmitter.call(this);
}

util.inherits(Capture, EventEmitter);

Capture.prototype.start = function () {
  try {
    if (!this.initialised) {
      this.initialised = this.capture.init() ? true : false;
      if (!this.initialised) {
        console.error('Cannot start capture when no device is present.');
        return 'Cannot start capture when no device is present.';
      }
    }
    this.capture.doCapture((v, a) => {
      this.emit('frame', v, a);
    });
  } catch (err) {
    this.emit('error', err);
  }
}

Capture.prototype.stop = function () {
  try {
    this.capture.stop();
    this.emit('done');
  } catch (err) {
    this.emit('error', err);
  }
}

Capture.prototype.enableAudio = function (sampleRate, sampleType, channelCount) {
  try {
    if (!this.initialised) {
      this.initialised = this.capture.init() ? true : false;
      if (!this.initialised) {
        console.error('Cannot initialise audio when no device is present.');
        return 'Cannot initialise audio when no device is present.';
      }
    }
    return this.capture.enableAudio(
      typeof sampleRate === 'string' ? +sampleRate : sampleRate,
      typeof sampleType === 'string' ? +sampleType: sampleType,
      typeof channelCount === 'string' ? +channelCount : channelCount);
  } catch (err) {
    return "Error when enabling audio: " + err;
  }
}
*/

function Playback (deviceIndex, displayMode, pixelFormat) {
  if (arguments.length !== 3 || typeof deviceIndex !== 'number' ||
      typeof displayMode !== 'number' || typeof pixelFormat !== 'number' ) {
    this.emit('error', new Error('Playback requires three number arguments: ' +
      'index, display mode and pixel format'));
  } else {
    this.playback = new ajatatorNative.Playback(deviceIndex, displayMode, pixelFormat);
  }
  this.initialised = false;
  EventEmitter.call(this);
}

util.inherits(Playback, EventEmitter);

Playback.prototype.start = function () {
  try {
    if (!this.initialised) {
      console.log("*** playback.init", this.playback.init());
      this.initialised = true;
    }
    console.log("*** playback.doPlayback", this.playback.doPlayback(function (x) {
      this.emit('played', x);
    }.bind(this)));
  } catch (err) {
    this.emit('error', err);
  }
}

Playback.prototype.frame = function (f) {
  try {
    if (!this.initialised) {
      this.playback.init();
      this.initialised = true;
    }
    var result = this.playback.scheduleFrame(f);
    // console.log("*** playback.scheduleFrame", result);
    if (typeof result === 'string')
      throw new Error("Problem scheduling frame: " + result);
    else
      return result;
  } catch (err) {
    this.emit('error', err);
  }
}

Playback.prototype.stop = function () {
  try {
    console.log('*** playback stop', this.playback.stop());
    this.emit('done');
  } catch (err) {
    this.emit('error', err);
  }
}

function bmCodeToInt (s) {
  return Buffer.from(s.substring(0, 4)).readUInt32BE(0);
}

function intToBMCode(i) {
  var b = Buffer.allocUnsafe(4).writeUInt32(i, 0);
  return b.toString();
}

function modeWidth (mode) {
  switch (mode) {
    case ajatation.bmdModeNTSC:
    case ajatation.bmdModeNTSC2398:
    case ajatation.bmdModeNTSCp:
    case ajatation.bmdModePAL:
    case ajatation.bmdModePALp:
      return 720;
    case ajatation.bmdModeHD720p50:
    case ajatation.bmdModeHD720p5994:
    case ajatation.bmdModeHD720p60:
      return 1280;
    case ajatation.bmdModeHD1080p2398:
    case ajatation.bmdModeHD1080p24:
    case ajatation.bmdModeHD1080p25:
    case ajatation.bmdModeHD1080p2997:
    case ajatation.bmdModeHD1080p30:
    case ajatation.bmdModeHD1080i50:
    case ajatation.bmdModeHD1080i5994:
    case ajatation.bmdModeHD1080i6000:
    case ajatation.bmdModeHD1080p50:
    case ajatation.bmdModeHD1080p5994:
    case ajatation.bmdModeHD1080p6000:
      return 1920;
    case ajatation.bmdMode2k2398:
    case ajatation.bmdMode2k24:
    case ajatation.bmdMode2k25:
    case ajatation.bmdMode2kDCI2398:
    case ajatation.bmdMode2kDCI24:
    case ajatation.bmdMode2kDCI25:
      return 2048;
    case ajatation.bmdMode4K2160p2398:
    case ajatation.bmdMode4K2160p24:
    case ajatation.bmdMode4K2160p25:
    case ajatation.bmdMode4K2160p2997:
    case ajatation.bmdMode4K2160p30:
    case ajatation.bmdMode4K2160p50:
    case ajatation.bmdMode4K2160p5994:
    case ajatation.bmdMode4K2160p60:
      return 3840;
    case ajatation.bmdMode4kDCI2398:
    case ajatation.bmdMode4kDCI24:
    case ajatation.bmdMode4kDCI25:
      return 4096;
    default:
      return 0;
  }
}

function modeHeight (mode) {
  switch (mode) {
    case ajatation.bmdModeNTSC:
    case ajatation.bmdModeNTSC2398:
    case ajatation.bmdModeNTSCp:
        return 486;
    case ajatation.bmdModePAL:
    case ajatation.bmdModePALp:
      return 576;
    case ajatation.bmdModeHD720p50:
    case ajatation.bmdModeHD720p5994:
    case ajatation.bmdModeHD720p60:
      return 720;
    case ajatation.bmdModeHD1080p2398:
    case ajatation.bmdModeHD1080p24:
    case ajatation.bmdModeHD1080p25:
    case ajatation.bmdModeHD1080p2997:
    case ajatation.bmdModeHD1080p30:
    case ajatation.bmdModeHD1080i50:
    case ajatation.bmdModeHD1080i5994:
    case ajatation.bmdModeHD1080i6000:
    case ajatation.bmdModeHD1080p50:
    case ajatation.bmdModeHD1080p5994:
    case ajatation.bmdModeHD1080p6000:
      return 1080;
    case ajatation.bmdMode2k2398:
    case ajatation.bmdMode2k24:
    case ajatation.bmdMode2k25:
      return 1556;
    case ajatation.bmdMode2kDCI2398:
    case ajatation.bmdMode2kDCI24:
    case ajatation.bmdMode2kDCI25:
      return 1080;
    case ajatation.bmdMode4K2160p2398:
    case ajatation.bmdMode4K2160p24:
    case ajatation.bmdMode4K2160p25:
    case ajatation.bmdMode4K2160p2997:
    case ajatation.bmdMode4K2160p30:
    case ajatation.bmdMode4K2160p50:
    case ajatation.bmdMode4K2160p5994:
    case ajatation.bmdMode4K2160p60:
    case ajatation.bmdMode4kDCI2398:
    case ajatation.bmdMode4kDCI24:
    case ajatation.bmdMode4kDCI25:
      return 2160;
    default:
      return 0;
  };
};

// Returns the duration of a frame as fraction of a second as an array:
//   [<enumverator>, [denominotor>]
function modeGrainDuration (mode) {
  switch (mode) {
    case ajatation.bmdModeNTSC:
      return [1001, 30000];
    case ajatation.bmdModeNTSC2398: // 3:2 pulldown applied on card
      return [1001, 30000];
    case ajatation.bmdModeNTSCp:
      return [1001, 60000];
    case ajatation.bmdModePAL:
      return [1000, 25000];
    case ajatation.bmdModePALp:
      return [1000, 50000];
    case ajatation.bmdModeHD720p50:
      return [1000, 50000];
    case ajatation.bmdModeHD720p5994:
      return [1001, 60000];
    case ajatation.bmdModeHD720p60:
      return [1000, 60000];
    case ajatation.bmdModeHD1080p2398:
      return [1001, 24000];
    case ajatation.bmdModeHD1080p24:
      return [1000, 24000];
    case ajatation.bmdModeHD1080p25:
      return [1000, 25000];
    case ajatation.bmdModeHD1080p2997:
      return [1001, 30000];
    case ajatation.bmdModeHD1080p30:
      return [1000, 30000];
    case ajatation.bmdModeHD1080i50:
      return [1000, 25000];
    case ajatation.bmdModeHD1080i5994:
      return [1001, 60000];
    case ajatation.bmdModeHD1080i6000:
      return [1000, 60000];
    case ajatation.bmdModeHD1080p50:
      return [1000, 50000];
    case ajatation.bmdModeHD1080p5994:
      return [1001, 60000];
    case ajatation.bmdModeHD1080p6000:
      return [1000, 60000];
    case ajatation.bmdMode2k2398:
      return [1001, 24000];
    case ajatation.bmdMode2k24:
      return [1000, 24000];
    case ajatation.bmdMode2k25:
      return [1000, 25000];
    case ajatation.bmdMode2kDCI2398:
      return [1001, 24000];
    case ajatation.bmdMode2kDCI24:
      return [1000, 24000];
    case ajatation.bmdMode2kDCI25:
      return [1000, 25000];
    case ajatation.bmdMode4K2160p2398:
      return [1001, 24000];
    case ajatation.bmdMode4K2160p24:
      return [1000, 24000];
    case ajatation.bmdMode4K2160p25:
      return [1000, 25000];
    case ajatation.bmdMode4K2160p2997:
      return [1001, 30000];
    case ajatation.bmdMode4K2160p30:
      return [1000, 30000];
    case ajatation.bmdMode4K2160p50:
      return [1000, 50000];
    case ajatation.bmdMode4K2160p5994:
      return [1001, 60000];
    case ajatation.bmdMode4K2160p60:
      return [1000, 60000];
    case ajatation.bmdMode4kDCI2398:
      return [1001, 24000];
    case ajatation.bmdMode4kDCI24:
      return [1000, 24000];
    case ajatation.bmdMode4kDCI25:
      return [1000, 25000];
    default:
    return [0, 1];
  };
};

function modeInterlace (mode) {
  switch (mode) {
    case ajatation.bmdModeNTSC:
    case ajatation.bmdModeNTSC2398:
      return true;
    case ajatation.bmdModeNTSCp:
      return false;
    case ajatation.bmdModePAL:
      return true;
    case ajatation.bmdModePALp:
    case ajatation.bmdModeHD720p50:
    case ajatation.bmdModeHD720p5994:
    case ajatation.bmdModeHD720p60:
    case ajatation.bmdModeHD1080p2398:
    case ajatation.bmdModeHD1080p24:
    case ajatation.bmdModeHD1080p25:
    case ajatation.bmdModeHD1080p2997:
    case ajatation.bmdModeHD1080p30:
      return false;
    case ajatation.bmdModeHD1080i50:
    case ajatation.bmdModeHD1080i5994:
    case ajatation.bmdModeHD1080i6000:
      return true;
    case ajatation.bmdModeHD1080p50:
    case ajatation.bmdModeHD1080p5994:
    case ajatation.bmdModeHD1080p6000:
    case ajatation.bmdMode2k2398:
    case ajatation.bmdMode2k24:
    case ajatation.bmdMode2k25:
    case ajatation.bmdMode2kDCI2398:
    case ajatation.bmdMode2kDCI24:
    case ajatation.bmdMode2kDCI25:
    case ajatation.bmdMode4K2160p2398:
    case ajatation.bmdMode4K2160p24:
    case ajatation.bmdMode4K2160p25:
    case ajatation.bmdMode4K2160p2997:
    case ajatation.bmdMode4K2160p30:
    case ajatation.bmdMode4K2160p50:
    case ajatation.bmdMode4K2160p5994:
    case ajatation.bmdMode4K2160p60:
    case ajatation.bmdMode4kDCI2398:
    case ajatation.bmdMode4kDCI24:
    case ajatation.bmdMode4kDCI25:
      return false;
    default:
      return false;
  }
}

function formatDepth (format) {
 switch (format) {
    case ajatation.bmdFormat8BitYUV:
      return 8;
    case ajatation.bmdFormat10BitYUV:
      return 10;
    case ajatation.bmdFormat8BitARGB:
    case ajatation.bmdFormat8BitBGRA:
      return 8;
    case ajatation.bmdFormat10BitRGB:
      return 10;
    case ajatation.bmdFormat12BitRGB:
    case ajatation.bmdFormat12BitRGBLE:
      return 12;
    case ajatation.bmdFormat10BitRGBXLE:
    case ajatation.bmdFormat10BitRGBX:
      return 10;
    default:
      return 0;
  };
};

function formatFourCC (format) {
  switch (format) {
    case ajatation.bmdFormat8BitYUV:
      return 'UYVY';
    case ajatation.bmdFormat10BitYUV:
     return 'v210';
    case ajatation.bmdFormat8BitARGB:
      return 'ARGB';
    case ajatation.bmdFormat8BitBGRA:
      return 'BGRA';
  // Big-endian RGB 10-bit per component with SMPTE video levels (64-960). Packed as 2:10:10:10
    case ajatation.bmdFormat10BitRGB:
      return 'r210';
  // Big-endian RGB 12-bit per component with full range (0-4095). Packed as 12-bit per component
    case ajatation.bmdFormat12BitRGB:
      return 'R12B';
  // Little-endian RGB 12-bit per component with full range (0-4095). Packed as 12-bit per component
    case ajatation.bmdFormat12BitRGBLE:
      return 'R12L';
  // Little-endian 10-bit RGB with SMPTE video levels (64-940)
    case ajatation.bmdFormat10BitRGBXLE:
      return 'R10l';
  // Big-endian 10-bit RGB with SMPTE video levels (64-940)
    case ajatation.bmdFormat10BitRGBX:
      return 'R10b';
  };
};

function fourCCFormat(fourCC) {
  switch (fourCC) {
    case 'UYVY':
      return ajatation.bmdFormat8BitYUV;
    case 'v210':
    case 'pgroup':
      return ajatation.bmdFormat10BitYUV;
    case 'ARGB':
      return ajatation.bmdFormat8BitARGB;
    case 'BGRA':
      return ajatation.bmdFormat8BitBGRA;
  // Big-endian RGB 10-bit per component with SMPTE video levels (64-960). Packed as 2:10:10:10
    case 'r210':
      return ajatation.bmdFormat10BitRGB;
  // Big-endian RGB 12-bit per component with full range (0-4095). Packed as 12-bit per component
    case 'R12B':
      return ajatation.bmdFormat12BitRGB;
  // Little-endian RGB 12-bit per component with full range (0-4095). Packed as 12-bit per component
    case 'R12L':
      return ajatation.bmdFormat12BitRGBLE;
  // Little-endian 10-bit RGB with SMPTE video levels (64-940)
    case 'R10l':
      return ajatation.bmdFormat10BitRGBXLE;
  // Big-endian 10-bit RGB with SMPTE video levels (64-940)
    case 'R10b':
      return ajatation.bmdFormat10BitRGBX;
  }
}

function formatSampling (format) {
  switch (format) {
    case ajatation.bmdFormat8BitYUV:
      return 'YCbCr-4:2:2';
    case ajatation.bmdFormat10BitYUV:
      return 'YCbCr-4:2:2';
    case ajatation.bmdFormat8BitARGB:
      return 'ARGB';
    case ajatation.bmdFormat8BitBGRA:
      return 'BGRA';
  // Big-endian RGB 10-bit per component with SMPTE video levels (64-960). Packed as 2:10:10:10
    case ajatation.bmdFormat10BitRGB:
      return 'RGB';
  // Big-endian RGB 12-bit per component with full range (0-4095). Packed as 12-bit per component
    case ajatation.bmdFormat12BitRGB:
      return 'RGB'
  // Little-endian RGB 12-bit per component with full range (0-4095). Packed as 12-bit per component
    case ajatation.bmdFormat12BitRGBLE:
      return 'RGB';
  // Little-endian 10-bit RGB with SMPTE video levels (64-940)
    case ajatation.bmdFormat10BitRGBXLE:
      return 'RGB';
  // Big-endian 10-bit RGB with SMPTE video levels (64-940)
    case ajatation.bmdFormat10BitRGBX:
      return 'RGB';
    default:
      return '';
  };
};

function formatColorimetry (format) {
  switch (format) {
    case ajatation.bmdFormat8BitYUV:
      return 'BT601-5';
    case ajatation.bmdFormat10BitYUV:
      return 'BT709-2';
    case ajatation.bmdFormat8BitARGB:
      return 'FULL';
    case ajatation.bmdFormat8BitBGRA:
      return 'FULL';
  // Big-endian RGB 10-bit per component with SMPTE video levels (64-960). Packed as 2:10:10:10
    case ajatation.bmdFormat10BitRGB:
      return 'SMPTE240M';
  // Big-endian RGB 12-bit per component with full range (0-4095). Packed as 12-bit per component
    case ajatation.bmdFormat12BitRGB:
      return 'FULL';
  // Little-endian RGB 12-bit per component with full range (0-4095). Packed as 12-bit per component
    case ajatation.bmdFormat12BitRGBLE:
      return 'FULL';
  // Little-endian 10-bit RGB with SMPTE video levels (64-940)
    case ajatation.bmdFormat10BitRGBXLE:
      return 'SMPTE240M';
  // Big-endian 10-bit RGB with SMPTE video levels (64-940)
    case ajatation.bmdFormat10BitRGBX:
      return 'SMPTE240M';
    default:
      return '';
  };
}

var ajatation = {
  /* Enum BMDDisplayMode - Video display modes */
      /* SD Modes */
  bmdModeNTSC                     : bmCodeToInt('ntsc'),
  bmdModeNTSC2398                 : bmCodeToInt('nt23'),	// 3:2 pulldown
  bmdModePAL                      : bmCodeToInt('pal '),
  bmdModeNTSCp                    : bmCodeToInt('ntsp'),
  bmdModePALp                     : bmCodeToInt('palp'),
      /* HD 1080 Modes */
  bmdModeHD1080p2398              : bmCodeToInt('23ps'),
  bmdModeHD1080p24                : bmCodeToInt('24ps'),
  bmdModeHD1080p25                : bmCodeToInt('Hp25'),
  bmdModeHD1080p2997              : bmCodeToInt('Hp29'),
  bmdModeHD1080p30                : bmCodeToInt('Hp30'),
  bmdModeHD1080i50                : bmCodeToInt('Hi50'),
  bmdModeHD1080i5994              : bmCodeToInt('Hi59'),
  bmdModeHD1080i6000              : bmCodeToInt('Hi60'),	// N.B. This _really_ is 60.00 Hz.
  bmdModeHD1080p50                : bmCodeToInt('Hp50'),
  bmdModeHD1080p5994              : bmCodeToInt('Hp59'),
  bmdModeHD1080p6000              : bmCodeToInt('Hp60'),	// N.B. This _really_ is 60.00 Hz.
      /* HD 720 Modes */
  bmdModeHD720p50                 : bmCodeToInt('hp50'),
  bmdModeHD720p5994               : bmCodeToInt('hp59'),
  bmdModeHD720p60                 : bmCodeToInt('hp60'),
      /* 2k Modes */
  bmdMode2k2398                   : bmCodeToInt('2k23'),
  bmdMode2k24                     : bmCodeToInt('2k24'),
  bmdMode2k25                     : bmCodeToInt('2k25'),
      /* DCI Modes (output only) */
  bmdMode2kDCI2398                : bmCodeToInt('2d23'),
  bmdMode2kDCI24                  : bmCodeToInt('2d24'),
  bmdMode2kDCI25                  : bmCodeToInt('2d25'),
      /* 4k Modes */
  bmdMode4K2160p2398              : bmCodeToInt('4k23'),
  bmdMode4K2160p24                : bmCodeToInt('4k24'),
  bmdMode4K2160p25                : bmCodeToInt('4k25'),
  bmdMode4K2160p2997              : bmCodeToInt('4k29'),
  bmdMode4K2160p30                : bmCodeToInt('4k30'),
  bmdMode4K2160p50                : bmCodeToInt('4k50'),
  bmdMode4K2160p5994              : bmCodeToInt('4k59'),
  bmdMode4K2160p60                : bmCodeToInt('4k60'),
      /* DCI Modes (output only) */
  bmdMode4kDCI2398                : bmCodeToInt('4d23'),
  bmdMode4kDCI24                  : bmCodeToInt('4d24'),
  bmdMode4kDCI25                  : bmCodeToInt('4d25'),
      /* Special Modes */
  bmdModeUnknown                  : bmCodeToInt('iunk'),
  /* Enum BMDFieldDominance - Video field dominance */
  bmdUnknownFieldDominance        : 0,
  bmdLowerFieldFirst              : bmCodeToInt('lowr'),
  bmdUpperFieldFirst              : bmCodeToInt('uppr'),
  bmdProgressiveFrame             : bmCodeToInt('prog'),
  bmdProgressiveSegmentedFrame    : bmCodeToInt('psf '),
  /* Enum BMDPixelFormat - Video pixel formats supported for output/input */
  bmdFormat8BitYUV                : bmCodeToInt('2vuy'),
  bmdFormat10BitYUV               : bmCodeToInt('v210'),
  bmdFormat8BitARGB               : 32,
  bmdFormat8BitBGRA               : bmCodeToInt('BGRA'),
  // Big-endian RGB 10-bit per component with SMPTE video levels (64-960). Packed as 2:10:10:10
  bmdFormat10BitRGB               : bmCodeToInt('r210'),
  // Big-endian RGB 12-bit per component with full range (0-4095). Packed as 12-bit per component
  bmdFormat12BitRGB               : bmCodeToInt('R12B'),
  // Little-endian RGB 12-bit per component with full range (0-4095). Packed as 12-bit per component
  bmdFormat12BitRGBLE             : bmCodeToInt('R12L'),
  // Little-endian 10-bit RGB with SMPTE video levels (64-940)
  bmdFormat10BitRGBXLE            : bmCodeToInt('R10l'),
  // Big-endian 10-bit RGB with SMPTE video levels (64-940)
  bmdFormat10BitRGBX              : bmCodeToInt('R10b'),
  /* Enum BMDDisplayModeFlags - Flags to describe the characteristics of an IDeckLinkDisplayMode. */
  bmdDisplayModeSupports3D        : 1 << 0,
  bmdDisplayModeColorspaceRec601  : 1 << 1,
  bmdDisplayModeColorspaceRec709  : 1 << 2,
  bmdAudioSampleRate48kHz	        : 48000,
  bmdAudioSampleType16bitInteger	: 16,
  bmdAudioSampleType32bitInteger	: 32,
  // Convert to and from Black Magic codes.
  intToBMCode : intToBMCode,
  bmCodeToInt : bmCodeToInt,
  // Get parameters from modes and formats
  modeWidth : modeWidth,
  modeHeight : modeHeight,
  modeGrainDuration : modeGrainDuration,
  modeInterlace : modeInterlace,
  formatDepth : formatDepth,
  formatFourCC : formatFourCC,
  fourCCFormat : fourCCFormat,
  formatSampling : formatSampling,
  formatColorimetry : formatColorimetry,
  // access details about the currently connected devices
  //deckLinkVersion: ajatatorNative.deckLinkVersion,
  getFirstDevice: ajatatorNative.getFirstDevice,
  // Raw access to device classes
  //DirectCapture: ajatatorNative.Capture,
  //Capture : Capture,
  Playback : Playback
};

module.exports = ajatation;
