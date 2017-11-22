/**
    @file        ntv2player.cpp
    @brief        Implementation of NTV2Player class.
    @copyright    Copyright (C) 2013-2017 AJA Video Systems, Inc.  All rights reserved.
**/

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

#include "ntv2player.h"
#include "ntv2utils.h"
#include "ntv2formatdescriptor.h"
#include "ntv2debug.h"
#include "ajabase/common/testpatterngen.h"
#include "ajabase/common/timecode.h"
#include "ajabase/system/memory.h"
#include "ajabase/system/systemtime.h"
#include "ajabase/system/process.h"
#include <algorithm>
#include "utils.h"
#include "BufferStatus.h"

// TEST
//#include <time.h>

#ifdef DEBUG_OUTPUT
#include <iostream>
#include <stdio.h>
#endif

using namespace std;


#define NTV2_ANCSIZE_MAX    (0x2000)
/**
    @brief    The maximum number of bytes of 48KHz audio that can be transferred for a single frame.
            Worst case, assuming 16 channels of audio (max), 4 bytes per sample, and 67 msec per frame
            (assuming the lowest possible frame rate of 14.98 fps)...
            48,000 samples per second requires 3,204 samples x 4 bytes/sample x 16 = 205,056 bytes
            201K will suffice, with 768 bytes to spare
**/
static const uint32_t    AUDIOBYTES_MAX_48K    (201 * 1024);

/**
    @brief    The maximum number of bytes of 96KHz audio that can be transferred for a single frame.
            Worst case, assuming 16 channels of audio (max), 4 bytes per sample, and 67 msec per frame
            (assuming the lowest possible frame rate of 14.98 fps)...
            96,000 samples per second requires 6,408 samples x 4 bytes/sample x 16 = 410,112 bytes
            401K will suffice, with 512 bytes to spare
**/
static const uint32_t    AUDIOBYTES_MAX_96K    (401 * 1024);

/**
    @brief    Used when reserving the AJA device, this specifies the application signature.
**/
static const ULWord        kAppSignature    (AJA_FOURCC ('S','T','P','K'));

const unsigned int ON_DEVICE_BUFFER_SIZE(7);/// Number of device buffers to allocate
const unsigned int TOTAL_BUFFER_SIZE(ON_DEVICE_BUFFER_SIZE + CIRCULAR_BUFFER_SIZE);/// Number of device buffers to allocate
const unsigned int TARGET_FILL_LEVEL(TOTAL_BUFFER_SIZE / 2);
const unsigned int BUFFER_PRE_FILL_MARGIN(2);

/*
#include <iostream>
#include <fstream>
void DumpAudioToFile(uint32_t* buffer, uint32_t bufferSize)
{
	ofstream myfile;

	static bool started(false);

	if (started)
	{
		myfile.open("C:\\Users\\DSI-User\\Documents\\out1.dat", ios::out | ios::app | ios::binary);
	}
	else
	{
		myfile.open("C:\\Users\\DSI-User\\Documents\\out1.dat", ios::out | ios::trunc | ios::binary);
		started = true;
	}

	if (myfile.is_open())
	{
		myfile.write((char*)buffer, bufferSize);
		myfile.close();
	}


}


void DumpAudioInfo(const string &inDeviceSpecifier, char* message, bool audio, uint32_t* buffer, uint32_t bufferSize)
{
    if(audio)
    {
        bool gotData = false;
        for(int i = 0; i < 16 && i < bufferSize; i++)
        {
            if(buffer[i] != 0x00) gotData = true;
        }

        cout << "!! " << inDeviceSpecifier << "-" << message << " (" << dec << bufferSize << ") bytes of " << (gotData ? "+++VALID+++": "---NULL---") << " audio !!" << endl;
    }
    else
    {
        cout << "!! NO AUDIO !!" << endl;
    }
}
*/

NTV2Player::NTV2Player (const AjaDevice::InitParams* initParams,
                        const string &               inDeviceSpecifier,
                        const bool                   inWithAudio,
                        const NTV2Channel            inChannel,
                        const NTV2FrameBufferFormat  inPixelFormat,
                        const NTV2OutputDestination  inOutputDestination,
                        const NTV2VideoFormat        inVideoFormat,
                        const bool                   inEnableVanc,
                        const bool                   inLevelConversion,
                        const bool                   inDoMultiChannel,
                        const AJAAncillaryDataType   inSendHDRType)

    :    mConsumerThread             (NULL),
        mProducerThread              (NULL),
        mLock                        (new AJALock (CNTV2DemoCommon::GetGlobalMutexName ())),
        mCurrentFrame                (0),
        mCurrentSample               (0),
        mToneFrequency               (440.0),
        mDeviceSpecifier             (inDeviceSpecifier),
        mDeviceID                    (DEVICE_ID_NOTFOUND),
        mOutputChannel               (inChannel),
        mOutputDestination           (inOutputDestination),
        mVideoFormat                 (inVideoFormat),
        mPixelFormat                 (inPixelFormat),
        mSavedTaskMode               (NTV2_DISABLE_TASKS),
        mAudioSystem                 (NTV2_AUDIOSYSTEM_1),
        mVancMode                    (NTV2_VANCMODE_OFF),
        mWithAudio                   (inWithAudio),
        mEnableVanc                  (inEnableVanc),
        mGlobalQuit                  (false),
        mDoLevelConversion           (inLevelConversion),
        mVideoBufferSize             (0),
        mAudioBufferSize             (0),
        mTestPatternVideoBuffers     (NULL),
        mNumTestPatterns             (0),
        mCallbackUserData            (NULL),
        mCallback                    (NULL),
        mScheduleFrameCallbackContext(NULL),
        mScheduleFrameCallback       (NULL),
        mAncType                     (inSendHDRType),
        mInitParams                  (initParams),
        mEnableTestPatternFill       (false),
        mOutputStarted               (false),
        mBufferedFrames              (0)
{
    ::memset (mAVHostBuffer, 0, sizeof (mAVHostBuffer));
}


NTV2Player::~NTV2Player (void)
{
    //    Stop my playout and producer threads, then destroy them...
    Quit ();

    if (mDeviceRef)
    {
        //    Unsubscribe from input vertical event...
        mDeviceRef->UnsubscribeInputVerticalEvent(mOutputChannel);
    }

    if (mTestPatternVideoBuffers)
    {
        for (int32_t ndx = 0;  ndx < mNumTestPatterns;  ndx++)
            delete [] mTestPatternVideoBuffers [ndx];
        delete [] mTestPatternVideoBuffers;
        mTestPatternVideoBuffers = NULL;
        mNumTestPatterns = 0;
    }

    for (unsigned int ndx = 0;  ndx < CIRCULAR_BUFFER_SIZE;  ndx++)
    {
        if (mAVHostBuffer [ndx].fVideoBuffer)
        {
            delete [] mAVHostBuffer [ndx].fVideoBuffer;
            mAVHostBuffer [ndx].fVideoBuffer = NULL;
        }
        if (mAVHostBuffer [ndx].fAudioBuffer)
        {
            delete [] mAVHostBuffer [ndx].fAudioBuffer;
            mAVHostBuffer [ndx].fAudioBuffer = NULL;
        }
    }    //    for each buffer in the ring
}    //    destructor


void NTV2Player::Quit (void)
{
    //    Set the global 'quit' flag, and wait for the threads to go inactive...
    mGlobalQuit = true;

    //    Free my threadss...
    if (mProducerThread)
        while (mProducerThread->Active ())
            AJATime::Sleep (10);

    if (mConsumerThread)
        while (mConsumerThread->Active ())
            AJATime::Sleep (10);

    delete mConsumerThread;
    mConsumerThread = NULL;
    delete mProducerThread;
    mProducerThread = NULL;
}    //    Quit


AJAStatus NTV2Player::Init (void)
{
    assert(!mDeviceRef);

    AJAStatus    status = mDeviceRef.Initialize(mDeviceSpecifier, mInitParams);

    if (AJA_SUCCESS(status))
    {
        mDeviceID = mDeviceRef->GetDeviceID();                        //    Keep this ID handy -- it's used frequently

        //    Beware -- some devices (e.g. Corvid1) can only output from FrameStore 2...
        if ((mOutputChannel == NTV2_CHANNEL1) && (!::NTV2DeviceCanDoFrameStore1Display(mDeviceID)))
            mOutputChannel = NTV2_CHANNEL2;
        if (UWord(mOutputChannel) >= ::NTV2DeviceGetNumFrameStores(mDeviceID))
        {
            cerr << "## ERROR:  Cannot use channel '" << mOutputChannel + 1 << "' -- device only supports channel 1"
                << (::NTV2DeviceGetNumFrameStores(mDeviceID) > 1 ? string(" thru ") + string(1, uint8_t(::NTV2DeviceGetNumFrameStores(mDeviceID) + '0')) : "") << endl;
            return AJA_STATUS_UNSUPPORTED;
        }

        //    Set up the video and audio...
        status = SetUpVideo();
        if (AJA_FAILURE(status))
            return status;

        status = SetUpAudio();
        if (AJA_FAILURE(status))
            return status;

        //    Set up the circular buffers, and the test pattern buffers...
        SetUpHostBuffers();
        status = SetUpTestPatternVideoBuffers();
        if (AJA_FAILURE(status))
            return status;

        //    Set up the device signal routing, and playout AutoCirculate...
        RouteOutputSignal();
        SetUpOutputAutoCirculate();

        //    Lastly, prepare my AJATimeCodeBurn instance...
        // REMOVE const NTV2FormatDescriptor    fd(mVideoFormat, mPixelFormat, mVancMode);
        // REMOVE mTCBurner.RenderTimeCodeFont(CNTV2DemoCommon::GetAJAPixelFormat(mPixelFormat), fd.numPixels, fd.numLines);

    }
    return status;

}    //    Init


AJAStatus NTV2Player::SetUpVideo ()
{
    if (!mDeviceRef) return AJA_STATUS_INITIALIZE;
        
    if (mVideoFormat == NTV2_FORMAT_UNKNOWN)
        mDeviceRef->GetVideoFormat(&mVideoFormat, NTV2_CHANNEL1);
        
    if (!::NTV2DeviceCanDoVideoFormat (mDeviceID, mVideoFormat))
        {cerr << "## ERROR:  This device cannot handle '" << ::NTV2VideoFormatToString (mVideoFormat) << "'" << endl;  return AJA_STATUS_UNSUPPORTED;}

    //    Configure the device to handle the requested video format...
    mDeviceRef->SetVideoFormat(false, mVideoFormat, false, false, mOutputChannel);

    if (!::NTV2DeviceCanDo3GLevelConversion (mDeviceID) && mDoLevelConversion && ::IsVideoFormatA (mVideoFormat))
        mDoLevelConversion = false;
    if (mDoLevelConversion)
        mDeviceRef->SetSDIOutLevelAtoLevelBConversion(mOutputChannel, mDoLevelConversion);
        
    //    Set the frame buffer pixel format for all the channels on the device.
    //    If the device doesn't support it, fall back to 8-bit YCbCr...
    if (!::NTV2DeviceCanDoFrameBufferFormat (mDeviceID, mPixelFormat))
    {
        cerr    << "## NOTE:  Device cannot handle '" << ::NTV2FrameBufferFormatString (mPixelFormat) << "' -- using '"
                << ::NTV2FrameBufferFormatString (NTV2_FBF_8BIT_YCBCR) << "' instead" << endl;
        mPixelFormat = NTV2_FBF_8BIT_YCBCR;
    }

    mDeviceRef->SetFrameBufferFormat(mOutputChannel, mPixelFormat);

    mDeviceRef->SetReference(false, NTV2_REFERENCE_FREERUN);

    mDeviceRef->EnableChannel(mOutputChannel);

    if (mEnableVanc && !::IsRGBFormat (mPixelFormat) && NTV2_IS_HD_VIDEO_FORMAT (mVideoFormat))
    {
        //    Try enabling VANC...
        mDeviceRef->SetEnableVANCData(true);        //    Enable VANC for non-SD formats, to pass thru captions, etc.
        if (::Is8BitFrameBufferFormat (mPixelFormat))
        {
            //    8-bit FBFs require VANC bit shift...
            mDeviceRef->SetVANCShiftMode(mOutputChannel, NTV2_VANCDATA_8BITSHIFT_ENABLE);
            mDeviceRef->SetVANCShiftMode(mOutputChannel, NTV2_VANCDATA_8BITSHIFT_ENABLE);
        }
    }    //    if HD video format
    else
        mDeviceRef->SetEnableVANCData(false);    //    No VANC with RGB pixel formats (for now)

    //    Subscribe the output interrupt -- it's enabled by default...
    mDeviceRef->SubscribeOutputVerticalEvent(mOutputChannel);
    if (OutputDestHasRP188BypassEnabled ())
        DisableRP188Bypass ();

    return AJA_STATUS_SUCCESS;

}    //    SetUpVideo


AJAStatus NTV2Player::SetUpAudio ()
{
    const uint16_t    numberOfAudioChannels    (::NTV2DeviceGetMaxAudioChannels (mDeviceID));

    //    Use NTV2_AUDIOSYSTEM_1, unless the device has more than one audio system...
    if (::NTV2DeviceGetNumAudioSystems (mDeviceID) > 1)
        mAudioSystem = ::NTV2ChannelToAudioSystem (mOutputChannel);    //    ...and base it on the channel
    //    However, there are a few older devices that have only 1 audio system, yet 2 frame stores (or must use channel 2 for playout)...
    if (!::NTV2DeviceCanDoFrameStore1Display (mDeviceID))
        mAudioSystem = NTV2_AUDIOSYSTEM_1;

    mDeviceRef->SetNumberAudioChannels(numberOfAudioChannels, mAudioSystem);
    mDeviceRef->SetAudioRate(NTV2_AUDIO_48K, mAudioSystem);

    //    How big should the on-device audio buffer be?   1MB? 2MB? 4MB? 8MB?
    //    For this demo, 4MB will work best across all platforms (Windows, Mac & Linux)...
    mDeviceRef->SetAudioBufferSize(NTV2_AUDIO_BUFFER_BIG, mAudioSystem);

    //    Set the SDI output audio embedders to embed audio samples from the output of mAudioSystem...
    mDeviceRef->SetSDIOutputAudioSystem(mOutputChannel, mAudioSystem);
    mDeviceRef->SetSDIOutputDS2AudioSystem(mOutputChannel, mAudioSystem);

    //    If the last app using the device left it in end-to-end mode (input passthru),
    //    then loopback must be disabled, or else the output will contain whatever audio
    //    is present in whatever signal is feeding the device's SDI input...
    mDeviceRef->SetAudioLoopBack(NTV2_AUDIO_LOOPBACK_OFF, mAudioSystem);

    return AJA_STATUS_SUCCESS;

}    //    SetUpAudio


void NTV2Player::SetUpHostBuffers ()
{
    //    Let my circular buffer know when it's time to quit...
    mAVCircularBuffer.SetAbortFlag (&mGlobalQuit);

    //    Calculate the size of the video buffer, which depends on video format, pixel format, and whether VANC is included or not...
    mDeviceRef->GetVANCMode(mVancMode);
    mVideoBufferSize = GetVideoWriteSize (mVideoFormat, mPixelFormat, mVancMode);

    //    Calculate the size of the audio buffer, which mostly depends on the sample rate...
    NTV2AudioRate    audioRate    (NTV2_AUDIO_48K);
    mDeviceRef->GetAudioRate(audioRate, mAudioSystem);
    mAudioBufferSize = (audioRate == NTV2_AUDIO_96K) ? AUDIOBYTES_MAX_96K : AUDIOBYTES_MAX_48K;

    //    Allocate my buffers...
    for (size_t ndx = 0; ndx < CIRCULAR_BUFFER_SIZE; ndx++)
    {
        mAVHostBuffer [ndx].fVideoBuffer        = reinterpret_cast <uint32_t *> (new uint8_t [mVideoBufferSize]);
        mAVHostBuffer [ndx].fVideoBufferSize    = mVideoBufferSize;
        mAVHostBuffer [ndx].fAudioBuffer        = mWithAudio ? reinterpret_cast <uint32_t *> (new uint8_t [mAudioBufferSize]) : NULL;
        mAVHostBuffer [ndx].fAudioBufferSize    = mWithAudio ? mAudioBufferSize : 0;

        ::memset (mAVHostBuffer [ndx].fVideoBuffer, 0x00, mVideoBufferSize);
        ::memset (mAVHostBuffer [ndx].fAudioBuffer, 0x00, mWithAudio ? mAudioBufferSize : 0);

        mAVCircularBuffer.Add (&mAVHostBuffer [ndx]);
    }    //    for each AV buffer in my circular buffer

}    //    SetUpHostBuffers


void NTV2Player::RouteOutputSignal ()
{
    const NTV2Standard        outputStandard    (::GetNTV2StandardFromVideoFormat (mVideoFormat));
    const UWord                numVideoOutputs    (::NTV2DeviceGetNumVideoOutputs (mDeviceID));
    bool                    isRGB            (::IsRGBFormat (mPixelFormat));

    //    If device has no RGB conversion capability for the desired channel, use YUV instead
    if (UWord (mOutputChannel) > ::NTV2DeviceGetNumCSCs (mDeviceID))
        isRGB = false;

    NTV2OutputCrosspointID    cscVidOutXpt    (::GetCSCOutputXptFromChannel (mOutputChannel,  false/*isKey*/,  isRGB/*isRGB*/));
    NTV2OutputCrosspointID    fsVidOutXpt        (::GetFrameBufferOutputXptFromChannel (mOutputChannel,  isRGB/*isRGB*/,  false/*is425*/));

    if (isRGB)
        mDeviceRef->Connect(::GetCSCInputXptFromChannel(mOutputChannel, false/*isKeyInput*/), fsVidOutXpt);

    //if (mInitParams->doMultiChannel)
    //{
    //    //    Multiformat --- route the one SDI output to the CSC video output (RGB) or FrameStore output (YUV)...
    //    if (::NTV2DeviceHasBiDirectionalSDI (mDeviceID))
    //        mDeviceRef->SetSDITransmitEnable(mOutputChannel, true);
    //
    //    mDeviceRef->Connect(::GetSDIOutputInputXpt(mOutputChannel, false/*isDS2*/), isRGB ? cscVidOutXpt : fsVidOutXpt);
    //    mDeviceRef->SetSDIOutputStandard(mOutputChannel, outputStandard);
    //}
    //else
    {
        if (isRGB)
            mDeviceRef->Connect(::GetCSCInputXptFromChannel(mOutputChannel, false/*isKeyInput*/), fsVidOutXpt);

        /* NTV2_CHANNEL1 below stops the input flow */
        for (NTV2Channel chan (mOutputChannel);  ULWord (chan) < numVideoOutputs;  chan = NTV2Channel (chan + 1))
        {
            if (::NTV2DeviceHasBiDirectionalSDI (mDeviceID))
                mDeviceRef->SetSDITransmitEnable(chan, true);        //    Make it an output

            mDeviceRef->Connect(::GetSDIOutputInputXpt(chan, false/*isDS2*/), isRGB ? cscVidOutXpt : fsVidOutXpt);
            mDeviceRef->SetSDIOutputStandard(chan, outputStandard);
        }    //    for each output spigot

        if (::NTV2DeviceCanDoWidget (mDeviceID, NTV2_WgtAnalogOut1))
            mDeviceRef->Connect(::GetOutputDestInputXpt(NTV2_OUTPUTDESTINATION_ANALOG), isRGB ? cscVidOutXpt : fsVidOutXpt);

        if (::NTV2DeviceCanDoWidget (mDeviceID, NTV2_WgtHDMIOut1)
            || ::NTV2DeviceCanDoWidget (mDeviceID, NTV2_WgtHDMIOut1v2)
            || ::NTV2DeviceCanDoWidget (mDeviceID, NTV2_WgtHDMIOut1v3))
            mDeviceRef->Connect(::GetOutputDestInputXpt(NTV2_OUTPUTDESTINATION_HDMI), isRGB ? cscVidOutXpt : fsVidOutXpt);
    }

}    //    RouteOutputSignal


void NTV2Player::SetUpOutputAutoCirculate ()
{
    const uint32_t    buffersPerChannel (ON_DEVICE_BUFFER_SIZE);        //    Sufficient and safe for all devices & FBFs

    mDeviceRef->AutoCirculateStop(mOutputChannel);
    {
        AJAAutoLock    autoLock (mLock);    //    Avoid AutoCirculate buffer collisions
        mDeviceRef->AutoCirculateInitForOutput(mOutputChannel, buffersPerChannel,
                                               mWithAudio ? mAudioSystem : NTV2_AUDIOSYSTEM_INVALID,    //    Which audio system?
                                               AUTOCIRCULATE_WITH_RP188 | AUTOCIRCULATE_WITH_ANC);                                //    Add RP188 timecode!
    }

}    //    SetUpOutputAutoCirculate


AJAStatus NTV2Player::Run ()
{
    //    Start my consumer and producer threads...
    StartConsumerThread ();
    //StartProducerThread ();

    return AJA_STATUS_SUCCESS;

}    //    Run



//////////////////////////////////////////////
//    This is where the play thread starts

void NTV2Player::StartConsumerThread ()
{
    //    Create and start the playout thread...
    mConsumerThread = new AJAThread ();
    mConsumerThread->Attach (ConsumerThreadStatic, this);
    mConsumerThread->SetPriority (AJA_ThreadPriority_High);
    mConsumerThread->Start ();

}    //    StartConsumerThread


//    The playout thread function
void NTV2Player::ConsumerThreadStatic (AJAThread * pThread, void * pContext)        //    static
{
    (void) pThread;

    //    Grab the NTV2Player instance pointer from the pContext parameter,
    //    then call its PlayFrames method...
    NTV2Player *    pApp    (reinterpret_cast <NTV2Player *> (pContext));
    if (pApp)
        pApp->PlayFrames ();

}    //    ConsumerThreadStatic

void NTV2Player::PlayFrames (void)
{
    AUTOCIRCULATE_TRANSFER        mOutputXferInfo;

    uint32_t*    fAncBuffer = mAncType != AJAAncillaryDataType_Unknown ? reinterpret_cast <uint32_t *> (AJAMemory::AllocateAligned (NTV2_ANCSIZE_MAX, AJA_PAGE_SIZE)) : NULL;
    uint32_t    fAncBufferSize = mAncType != AJAAncillaryDataType_Unknown ? NTV2_ANCSIZE_MAX : 0;
    ::memset((void*)fAncBuffer, 0x00, fAncBufferSize);
    uint32_t    packetSize = 0;
    switch(mAncType)
    {
    case AJAAncillaryDataType_HDR_SDR:
    {
        AJAAncillaryData_HDR_SDR sdrPacket;
        sdrPacket.GenerateTransmitData((uint8_t*)fAncBuffer, fAncBufferSize, packetSize);
        break;
    }
    case AJAAncillaryDataType_HDR_HDR10:
    {
        AJAAncillaryData_HDR_HDR10 hdr10Packet;
        hdr10Packet.GenerateTransmitData((uint8_t*)fAncBuffer, fAncBufferSize, packetSize);
        break;
    }
    case AJAAncillaryDataType_HDR_HLG:
    {
        AJAAncillaryData_HDR_HLG hlgPacket;
        hlgPacket.GenerateTransmitData((uint8_t*)fAncBuffer, fAncBufferSize, packetSize);
        break;
    }
    default:    break;
    }

    while (!mGlobalQuit)
    {
        AUTOCIRCULATE_STATUS    outputStatus;
        mDeviceRef->AutoCirculateGetStatus(mOutputChannel, outputStatus);

        ULWord numAvailableFrames = outputStatus.GetNumAvailableOutputFrames();

        //    Check if there's room for another frame on the card...
        if (numAvailableFrames > 1 && CheckOutputReady())
        {
            LogBufferState(numAvailableFrames);

            //    Wait for the next frame to become ready to "consume"...
            AVDataBuffer *    playData    (mAVCircularBuffer.StartConsumeNextBuffer ());
            if (playData)
            {
                //cout << "!! TEST, " << clock() << ", , TX" << endl;

                //    Include timecode in output signal...
                mOutputXferInfo.SetOutputTimeCode (NTV2_RP188 (playData->fRP188Data), ::NTV2ChannelToTimecodeIndex (mOutputChannel));

                //    Transfer the timecode-burned frame to the device for playout...
                mOutputXferInfo.SetVideoBuffer (playData->fVideoBuffer, playData->fVideoBufferSize);

                //DumpAudioInfo(mDeviceSpecifier, "Transferring to audio card: ", mWithAudio, playData->fAudioBuffer, playData->fAudioBufferSize);

                //DumpAudioToFile(playData->fAudioBuffer, playData->fAudioBufferSize);

                mOutputXferInfo.SetAudioBuffer (mWithAudio ? playData->fAudioBuffer : NULL, mWithAudio ? playData->fAudioBufferSize : 0);
                mOutputXferInfo.SetAncBuffers(fAncBuffer, NTV2_ANCSIZE_MAX, NULL, 0);
                mDeviceRef->AutoCirculateTransfer(mOutputChannel, mOutputXferInfo);
                mAVCircularBuffer.EndConsumeNextBuffer ();    //    Signal that the frame has been "consumed"

                LOG_BUFFER_STATE("Just added to card, requesting next frame");

                // Decrement frames as we've just added one
                --numAvailableFrames;
            }
        }
        else
        {
            mDeviceRef->WaitForOutputVerticalInterrupt(mOutputChannel);
        }

        if (mScheduleFrameCallback)
        {

            // Request enough frames to fill the buffer
            // NOTE: this approach keeps the C++ to JS interface simple: i.e. one callback per request
            // for a new frame; rather than sending back the number of required frames in the callback,
            // which would change the interface compared with other video cards.
            // As this only requests frames when the on-card buffer is full, we should never get into the
            // situation where the input buffer fills up and frames need to be dropped; as there is an
            // additional circular buffer in front of the on-card buffer to provide extra tolerance.
            for (uint32_t i = 0; i < numAvailableFrames; i++)
            {
                mScheduleFrameCallback(mScheduleFrameCallbackContext);
            }
        }
    }    //    loop til quit signaled

    //    Stop AutoCirculate...
    mDeviceRef->AutoCirculateStop(mOutputChannel);
    //delete [] fAncBuffer;

}    //    PlayFrames


void NTV2Player::LogBufferState(ULWord cardBufferFreeSlots)
{
    auto cardBufferUsedSlots = ON_DEVICE_BUFFER_SIZE - cardBufferFreeSlots;
    auto circBufferUsedSlots = mAVCircularBuffer.GetCircBufferCount();

    // Test traces for capturing buffer state:
    //cout << "!! TEST, " << clock() << ", " << cardBufferFreeSlots << ", " << cardBufferUsedSlots << ", " << circBufferUsedSlots << ", " << (cardBufferUsedSlots + circBufferUsedSlots) << endl;
    //cout << "!! TEST: cardf=" << cardBufferFreeSlots << ", cardu=" << cardBufferUsedSlots << ", crcu=" << circBufferUsedSlots << ", tu=" << (cardBufferUsedSlots + circBufferUsedSlots) << endl;

    // Store the total number of used buffer slots to return from the producer thread
    SetUsedBuffers(cardBufferUsedSlots + circBufferUsedSlots);

    float usedCircBufferPercent = (float)(circBufferUsedSlots * 100) / (float)CIRCULAR_BUFFER_SIZE;
    float userCardBufferPercent = (float)(cardBufferUsedSlots * 100) / (float)ON_DEVICE_BUFFER_SIZE;

    BufferStatus::AddSample(BufferStatus::PlaybackCircBuffer, usedCircBufferPercent);
    BufferStatus::AddSample(BufferStatus::PlaybackCardBuffer, userCardBufferPercent);
}




bool NTV2Player::CheckOutputReady()
{
    if(mOutputStarted == false)
    {
        // If we're nearly full, start the output flowing
        if((CIRCULAR_BUFFER_SIZE - mAVCircularBuffer.GetCircBufferCount()) < BUFFER_PRE_FILL_MARGIN)
        {
            mDeviceRef->AutoCirculateStart(mOutputChannel);    //    Start it running
            mOutputStarted = true;
        }
    }

    return mOutputStarted;
}


//////////////////////////////////////////////
//    This is where the producer thread starts

void NTV2Player::StartProducerThread ()
{
    //    Create and start the producer thread...
    mProducerThread = new AJAThread ();
    mProducerThread->Attach (ProducerThreadStatic, this);
    mProducerThread->SetPriority (AJA_ThreadPriority_High);
    mProducerThread->Start ();

}    //    StartProducerThread


void NTV2Player::ProducerThreadStatic (AJAThread * pThread, void * pContext)        //    static
{
    (void) pThread;

    NTV2Player *    pApp    (reinterpret_cast <NTV2Player *> (pContext));
    if (pApp)
        pApp->ProduceFrames ();

}    //    ProducerThreadStatic


AJAStatus NTV2Player::SetUpTestPatternVideoBuffers (void)
{
    AJATestPatternSelect    testPatternTypes []    =    {AJA_TestPatt_ColorBars100,
                                                    AJA_TestPatt_ColorBars75,
                                                    AJA_TestPatt_Ramp,
                                                    AJA_TestPatt_MultiBurst,
                                                    AJA_TestPatt_LineSweep,
                                                    AJA_TestPatt_CheckField,
                                                    AJA_TestPatt_FlatField,
                                                    AJA_TestPatt_MultiPattern};

    mNumTestPatterns = sizeof (testPatternTypes) / sizeof (AJATestPatternSelect);
    mTestPatternVideoBuffers = new uint8_t * [mNumTestPatterns];
    ::memset (mTestPatternVideoBuffers, 0, mNumTestPatterns * sizeof (uint8_t *));

    //    Set up one video buffer for each of the several predefined patterns...
    for (int testPatternIndex = 0;  testPatternIndex < mNumTestPatterns;  testPatternIndex++)
    {
        //    Allocate the buffer memory...
        mTestPatternVideoBuffers [testPatternIndex] = new uint8_t [mVideoBufferSize];

        //    Use a convenient AJA test pattern generator object to populate an AJATestPatternBuffer with test pattern data...
        AJATestPatternBuffer    testPatternBuffer;
        AJATestPatternGen        testPatternGen;
        NTV2FormatDescriptor    formatDesc        (mVideoFormat, mPixelFormat, mVancMode);

        if (!testPatternGen.DrawTestPattern (testPatternTypes [testPatternIndex],
                                            formatDesc.numPixels,
                                            formatDesc.numLines - formatDesc.firstActiveLine,
                                            CNTV2DemoCommon::GetAJAPixelFormat (mPixelFormat),
                                            testPatternBuffer))
        {
            cerr << "## ERROR:  DrawTestPattern failed, formatDesc: " << formatDesc << endl;
            return AJA_STATUS_FAIL;
        }

        const size_t    testPatternSize    (testPatternBuffer.size ());

        if (formatDesc.firstActiveLine)
        {
            //    Fill the VANC area with something valid -- otherwise the device won't emit a correctly-timed signal...
            unsigned    nVancLines    (formatDesc.firstActiveLine);
            uint8_t *    pVancLine    (mTestPatternVideoBuffers [testPatternIndex]);
            while (nVancLines--)
            {
                if (mPixelFormat == NTV2_FBF_10BIT_YCBCR)
                {
                    ::Make10BitBlackLine (reinterpret_cast <UWord *> (pVancLine), formatDesc.numPixels);
                    ::PackLine_16BitYUVto10BitYUV (reinterpret_cast <const UWord *> (pVancLine), reinterpret_cast <ULWord *> (pVancLine), formatDesc.numPixels);
                }
                else if (mPixelFormat == NTV2_FBF_8BIT_YCBCR)
                    ::Make8BitBlackLine (pVancLine, formatDesc.numPixels);
                else
                {
                    cerr << "## ERROR:  Cannot initialize video buffer's VANC area" << endl;
                    return AJA_STATUS_FAIL;
                }
                pVancLine += formatDesc.linePitch * 4;
            }    //    for each VANC line
        }    //    if has VANC area

        //    Copy the contents of the AJATestPatternBuffer into my video buffer...
        uint8_t *    pVideoBuffer    (mTestPatternVideoBuffers [testPatternIndex] + formatDesc.firstActiveLine * formatDesc.linePitch * 4);
        for (size_t ndx = 0; ndx < testPatternSize; ndx++)
            pVideoBuffer [ndx] = testPatternBuffer [ndx];

    }    //    for each test pattern

    //mEnableTestPatternFill = true;

    return AJA_STATUS_SUCCESS;

}    //    SetUpTestPatternVideoBuffers


static const double    gFrequencies []    =    {250.0, 500.0, 1000.0, 2000.0};
static const ULWord    gNumFrequencies        (sizeof (gFrequencies) / sizeof (double));
static const double    gAmplitudes []    =    {    0.10, 0.15,        0.20, 0.25,        0.30, 0.35,        0.40, 0.45,        0.50, 0.55,        0.60, 0.65,        0.70, 0.75,        0.80, 0.85,
                                            0.85, 0.80,        0.75, 0.70,        0.65, 0.60,        0.55, 0.50,        0.45, 0.40,        0.35, 0.30,        0.25, 0.20,        0.15, 0.10};


/**
@brief    Add a frame to the frame buffer, to be played out in its turn.
@param[in]    videoData            pointer to the video frame to queue.
@param[in]    videoDataLength      length of the video data in bytes.
@param[in]    audioData            pointer to the audio frame to queue.
@param[in]    audioDataLength      length of the audio data in bytes.
@param[out]   usedFrames          If not null, receives the number of buffered frames.
**/
bool NTV2Player::ScheduleFrame(
    const char* videoData,
    const size_t videoDataLength,
    const char* audioData,
    const size_t audioDataLength,
    uint32_t* usedFrames)
{
    bool addedFrame = false;

    // Lock the circular buffer to prevent collisions with ProduceFrames()
    AJAAutoLock    autoLock(mLock);    //    Lock to prevent collisions with the Frame Generator 

    LOG_BUFFER_STATE("Scheduling frame");

    AVDataBuffer* frameData(mAVCircularBuffer.StartProduceNextBuffer());

    //cout << "!! TEST, " << clock() << ", RC, ," << endl;

    //  If no frame is available, wait and try again
    if (frameData)
    {
        if (videoDataLength > 0 && videoData != nullptr)
        {
            // TODO: for the time being, blindly copy mis-matched frame data - potentially handle this differently
            uint32_t copyBytes = min(videoDataLength, mVideoBufferSize);

            //    Copy my pre-made test pattern into my video buffer...
            ::memcpy(frameData->fVideoBuffer, videoData, copyBytes);
            frameData->fVideoBufferSize = copyBytes;
        }
        else
        {
            frameData->fVideoBufferSize = 0;
        }

        //DumpAudioInfo(mDeviceSpecifier, "Transferring to circular buffer: ", mWithAudio, (uint32_t*)audioData, audioDataLength);

        if (audioDataLength > 0 && audioData != nullptr)
        {
            // TODO: for the time being, blindly copy mis-matched frame data - potentially handle this differently
            uint32_t copyBytes = min(audioDataLength, mAudioBufferSize);

            //    Copy my pre-made test pattern into my video buffer...
            ::memcpy(frameData->fAudioBuffer, audioData, copyBytes);
            frameData->fAudioBufferSize = audioDataLength;
        }
        else
        {
            frameData->fAudioBufferSize = 0;
        }
        // TODO: Copy other buffers

        //    Signal that I'm done producing the buffer -- it's now available for playout...
        mAVCircularBuffer.EndProduceNextBuffer();

        addedFrame = true;
    }
#ifdef DEBUG_OUTPUT
    else
    {
        std::cout << "No frames available, dropping frame!" << std::endl;
    }
#endif

    if (usedFrames != nullptr)
    {
        *usedFrames = GetUsedBuffers();
    }

    return addedFrame;
}


void NTV2Player::ProduceFrames (void)
{
    ULWord    frequencyIndex        (0);
    double    timeOfLastSwitch    (0.0);
    ULWord    testPatternIndex    (0);

    AJATimeBase    timeBase (CNTV2DemoCommon::GetAJAFrameRate (::GetNTV2FrameRateFromVideoFormat (mVideoFormat)));

    while (!mGlobalQuit)
    {
        AJAAutoLock    autoLock(mLock);    //    Lock the Frame Generator thread to prevent collisions with ScheduleFrame

        // Only produce frames on this thread if the circular buffer is empty, 
        // or if there is a frame callback defined. Otherwise allow the 'ScheduleFrame'
        // method to put frames into the queue
        //
        if (mCallback || (mAVCircularBuffer.GetCircBufferCount() == 0 && mEnableTestPatternFill == true))
        {
            LOG_BUFFER_STATE("Adding colour bar frame");

            AVDataBuffer *    frameData(mAVCircularBuffer.StartProduceNextBuffer());

            //  If no frame is available, wait and try again
            if (!frameData)
            {
                AJATime::Sleep(10);
                continue;
            }

            if (mCallback)
            {
                // Get the frame to play from whomever requested the callback
                mCallback(mCallbackUserData, frameData);
            }
            else
            {
                //    Copy my pre-made test pattern into my video buffer...
                ::memcpy(frameData->fVideoBuffer, mTestPatternVideoBuffers[testPatternIndex], mVideoBufferSize);

                const    NTV2FrameRate    ntv2FrameRate(::GetNTV2FrameRateFromVideoFormat(mVideoFormat));
                const    TimecodeFormat    tcFormat(CNTV2DemoCommon::NTV2FrameRate2TimecodeFormat(ntv2FrameRate));
                const    CRP188            rp188Info(mCurrentFrame++, 0, 0, 10, tcFormat);
                string                    timeCodeString;

                rp188Info.GetRP188Reg(frameData->fRP188Data);
                rp188Info.GetRP188Str(timeCodeString);

                //    Burn the current timecode into the test pattern image that's now in my video buffer...
                mTCBurner.BurnTimeCode(reinterpret_cast <char *> (frameData->fVideoBuffer), timeCodeString.c_str(), 80);

                //    Generate audio tone data...
                frameData->fAudioBufferSize = mWithAudio ? AddTone(frameData->fAudioBuffer) : 0;

                //    Every few seconds, change the test pattern and tone frequency...
                const double    currentTime(timeBase.FramesToSeconds(mCurrentFrame));
                if (currentTime > timeOfLastSwitch + 4.0)
                {
                    frequencyIndex = (frequencyIndex + 1) % gNumFrequencies;
                    testPatternIndex = (testPatternIndex + 1) % mNumTestPatterns;
                    mToneFrequency = gFrequencies[frequencyIndex];
                    timeOfLastSwitch = currentTime;
                }    //    if time to switch test pattern & tone frequency
            }

            //    Signal that I'm done producing the buffer -- it's now available for playout...
            mAVCircularBuffer.EndProduceNextBuffer();
        }
    }    //    loop til mGlobalQuit goes true

}    //    ProduceFrames


void NTV2Player::GetACStatus (ULWord & outGoodFrames, ULWord & outDroppedFrames, ULWord & outBufferLevel)
{
    AUTOCIRCULATE_STATUS    status;
    mDeviceRef->AutoCirculateGetStatus(mOutputChannel, status);
    outGoodFrames = status.acFramesProcessed;
    outDroppedFrames = status.acFramesDropped;
    outBufferLevel = status.acBufferLevel;
}


uint32_t NTV2Player::AddTone (ULWord * pInAudioBuffer)
{
    NTV2FrameRate    frameRate    (NTV2_FRAMERATE_INVALID);
    NTV2AudioRate    audioRate    (NTV2_AUDIO_RATE_INVALID);
    ULWord            numChannels    (0);

    mDeviceRef->GetFrameRate(&frameRate, mOutputChannel);
    mDeviceRef->GetAudioRate(audioRate, mAudioSystem);
    mDeviceRef->GetNumberAudioChannels(numChannels, mAudioSystem);

    //    Set per-channel tone frequencies...
    double    pFrequencies [kNumAudioChannelsMax];
    pFrequencies [0] = (mToneFrequency / 2.0);
    for (ULWord chan (1);  chan < numChannels;  chan++)
        //    The 1.154782 value is the 16th root of 10, to ensure that if mToneFrequency is 2000,
        //    that the calculated frequency of audio channel 16 will be 20kHz...
        pFrequencies [chan] = pFrequencies [chan - 1] * 1.154782;

    //    Because audio on AJA devices use fixed sample rates (typically 48KHz), certain video frame rates will necessarily
    //    result in some frames having more audio samples than others. The GetAudioSamplesPerFrame function
    //    is used to calculate the correct sample count...
    const ULWord    numSamples        (::GetAudioSamplesPerFrame (frameRate, audioRate, mCurrentFrame));
    const double    sampleRateHertz    (audioRate == NTV2_AUDIO_96K ? 96000.0 : 48000.0);

    return ::AddAudioTone (    pInAudioBuffer,        //    audio buffer to fill
                            mCurrentSample,        //    which sample for continuing the waveform
                            numSamples,            //    number of samples to generate
                            sampleRateHertz,    //    sample rate [Hz]
                            gAmplitudes,        //    per-channel amplitudes
                            pFrequencies,        //    per-channel tone frequencies [Hz]
                            31,                    //    bits per sample
                            false,                //    don't byte swap
                            numChannels);        //    number of audio channels to generate
}    //    AddTone


ULWord NTV2Player::GetRP188RegisterForOutput (const NTV2OutputDestination inOutputDest)        //    static
{
    switch (inOutputDest)
    {
        case NTV2_OUTPUTDESTINATION_SDI1:    return kRegRP188InOut1DBB;    break;    //    reg 29
        case NTV2_OUTPUTDESTINATION_SDI2:    return kRegRP188InOut2DBB;    break;    //    reg 64
        case NTV2_OUTPUTDESTINATION_SDI3:    return kRegRP188InOut3DBB;    break;    //    reg 268
        case NTV2_OUTPUTDESTINATION_SDI4:    return kRegRP188InOut4DBB;    break;    //    reg 273
        case NTV2_OUTPUTDESTINATION_SDI5:    return kRegRP188InOut5DBB;    break;    //    reg 29
        case NTV2_OUTPUTDESTINATION_SDI6:    return kRegRP188InOut6DBB;    break;    //    reg 64
        case NTV2_OUTPUTDESTINATION_SDI7:    return kRegRP188InOut7DBB;    break;    //    reg 268
        case NTV2_OUTPUTDESTINATION_SDI8:    return kRegRP188InOut8DBB;    break;    //    reg 273
        default:                            return 0;                    break;
    }    //    switch on output destination

}    //    GetRP188RegisterForOutput


bool NTV2Player::OutputDestHasRP188BypassEnabled (void)
{
    bool            result        (false);
    const ULWord    regNum        (GetRP188RegisterForOutput (mOutputDestination));
    ULWord            regValue    (0);

    //
    //    Bit 23 of the RP188 DBB register will be set if output timecode will be
    //    grabbed directly from an input (bypass source)...
    //
    if (regNum && mDeviceRef->ReadRegister(regNum, &regValue) && regValue & BIT(23))
        result = true;

    return result;

}    //    OutputDestHasRP188BypassEnabled


void NTV2Player::DisableRP188Bypass (void)
{
    const ULWord    regNum    (GetRP188RegisterForOutput (mOutputDestination));

    //
    //    Clear bit 23 of my output destination's RP188 DBB register...
    //
    if (regNum)
        mDeviceRef->WriteRegister(regNum, 0, BIT(23), 23);

}    //    DisableRP188Bypass

#ifdef DEBUG_OUTPUT
void NTV2Player::LogBufferState(const char* location)
{
    AUTOCIRCULATE_STATUS    outputStatus;
    mDevice.AutoCirculateGetStatus(mOutputChannel, outputStatus);

    ULWord numCardBufferFrames         = outputStatus.GetNumAvailableOutputFrames();
    unsigned int numCircBufferFrames = CIRCULAR_BUFFER_SIZE - mAVCircularBuffer.GetCircBufferCount();

    std::printf("%s: CardBufferFree = %d; CircBufferFree = %d\n", location, numCardBufferFrames, numCircBufferFrames);
    //std::cout << location << ": CardBufferFree = " << numCardBufferFrames << "; CircBufferFree = " << numCircBufferFrames << std::endl;
}
#endif

void NTV2Player::GetCallback (void ** const pInstance, NTV2PlayerCallback ** const callback)
{
    *pInstance = mCallbackUserData;
    *callback = mCallback;
}    //    GetCallback


bool NTV2Player::SetCallback(void * const pInUserData, NTV2PlayerCallback * const callback)
{
    mCallbackUserData = pInUserData;
    mCallback = callback;

    return true;
}    //    SetCallback


bool NTV2Player::SetScheduledFrameCallback(void * const pInstance, ScheduledFrameCallback * const callback)
{
    mScheduleFrameCallbackContext = pInstance;
    mScheduleFrameCallback = callback;

    return true;
}    //    SetCallback
