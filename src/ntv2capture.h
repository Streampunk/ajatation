/**
    @file        ntv2capture.h
    @brief        Declares the NTV2Capture class.
    @copyright    Copyright (C) 2012-2016 AJA Video Systems, Inc.  All rights reserved.

    This version of the file is based upon the sample code provided by Aja
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

#ifndef _NTV2CAPTURE_H
#define _NTV2CAPTURE_H

#include "ntv2enums.h"
#include "ntv2devicefeatures.h"
#include "ntv2devicescanner.h"
#include "ntv2democommon.h"
#include "ntv2formatdescriptor.h"
#include "ajabase/common/videotypes.h"
#include "ajabase/common/circularbuffer.h"
#include "ajabase/system/thread.h"
#include "AjaDevice.h"

#define NTV2_AUDIOSIZE_MAX (401 * 1024)
#define NTV2_ANCSIZE_MAX   (0x2000)


/**
    @brief    Instances of me capture frames in real time from a video signal provided to an input of an AJA device.
**/

using namespace streampunk;

class NTV2Capture
{
    //  Public types
    public:

        typedef void(FrameArrivedCallback)(void * pInstance);

    //    Public Instance Methods
    public:
        /**
            @brief    Constructs me using the given settings.
            @note    I'm not completely initialized and ready to use until after my Init method has been called.
            @param[in]    inDeviceSpecifier    Specifies the AJA device to use.  Defaults to "0" (first device found).
            @param[in]    inWithAudio            If true (the default), capture audio in addition to video;  otherwise, don't capture audio.
            @param[in]    inChannel            Specifies the channel to use. Defaults to NTV2_CHANNEL1.
            @param[in]    inPixelFormat        Specifies the pixel format to use for the device's frame buffers. Defaults to 8-bit YUV.
            @param[in]    inDoLvlABConversion    Specifies if level-A/B conversion should be done or not.  Defaults to false (no conversion).
            @param[in]    inMultiFormat        If true, enables multiformat/multichannel mode if the device supports it, and won't acquire
                                            or release the device. If false (the default), acquires/releases exclusive use of the device.
            @param[in]    inWithAnc            If true, captures ancillary data using the new AutoCirculate APIs (if the device supports it).
                                            Defaults to false.
        **/
        NTV2Capture (   const AjaDevice::InitParams* initParams,
                        const std::string            inDeviceSpecifier    = "0",
                        const bool                   inWithAudio          = true,
                        const NTV2Channel            inChannel            = NTV2_CHANNEL1,
                        const NTV2FrameBufferFormat  inPixelFormat        = NTV2_FBF_8BIT_YCBCR,
                        const bool                   inDoLvlABConversion  = false,
                        const bool                   inMultiFormat        = false,
                        const bool                   inWithAnc            = false);

        virtual                        ~NTV2Capture ();

        /**
            @brief    Initializes me and prepares me to Run.
        **/
        virtual AJAStatus            Init (void);

        /**
            @brief    Runs me.
            @note    Do not call this method without first calling my Init method.
        **/
        virtual AJAStatus            Run (void);

        /**
            @brief    Gracefully stops me from running.
        **/
        virtual void                Quit (void);

        /**
            @brief    Provides status information about my input (capture) process.
            @param[out]    outGoodFrames        Receives the number of successfully captured frames.
            @param[out]    outDroppedFrames    Receives the number of dropped frames.
            @param[out]    outBufferLevel        Receives the buffer level (number of captured frames ready to be transferred to the host).
        **/
        virtual void                GetACStatus (ULWord & outGoodFrames, ULWord & outDroppedFrames, ULWord & outBufferLevel);

        /** 
            @brief    Lock the next frame to read the data from it, returns a pointer to the frame data if lock was successful.
            @note   It is essential to call UnlockFrame() after calling this, or the pipeline will become blocked.
         **/
        virtual AVDataBuffer*       LockNextFrame();

        /**
            @brief    Unlock the previously locked frame to free up the circular buffer.
            @param[out]    available frames    If not null, receives the number of available frames.
        **/
        virtual void                UnlockFrame(unsigned int* availableFrames = nullptr);

        /**
            @brief  Set the callback to be invoked when a new frame becomes available.
        **/
        bool SetFrameArrivedCallback(void * const pInstance, FrameArrivedCallback * const callback);

        /**
            @brief  Return the format of the video currently being received.
        **/
        virtual NTV2VideoFormat     GetVideoFormat();

    //    Protected Instance Methods
    protected:
        /**
            @brief    Sets up everything I need for capturing video.
        **/
        virtual AJAStatus        SetupVideo (void);

        /**
            @brief    Sets up everything I need for capturing audio.
        **/
        virtual AJAStatus        SetupAudio (void);

        /**
            @brief    Sets up device routing for capture.
        **/
        virtual void            RouteInputSignal (void);

        /**
            @brief    Sets up my circular buffers.
        **/
        virtual void            SetupHostBuffers (void);

        /**
            @brief    Starts my capture thread.
        **/
        virtual void            StartProducerThread (void);

        /**
            @brief    Repeatedly captures frames using AutoCirculate (until global quit flag set).
        **/
        virtual void            CaptureFrames (void);

        /**
            @brief    Try to start the AutoCirculate buffer, recursively retrying if this doesn't work first time.
        **/
        virtual bool            StartAutoCirculateBuffers(uint32_t retries = 3);

    //    Protected Class Methods
    protected:

        /**
            @brief    This is the capture thread's static callback function that gets called when the capture thread runs.
                      This function gets "Attached" to the AJAThread instance.
            @param[in]    pThread        Points to the AJAThread instance.
            @param[in]    pContext    Context information to pass to the thread.
                                    (For this application, this will point to the NTV2Capture instance.)
        **/
        static void             ProducerThreadStatic (AJAThread * pThread, void * pContext);

        /**
            @brief    log the buffer status
        **/
        virtual void            LogBufferState(ULWord cardBufferFreeSlots);

    //    Private Member Data
    private:
        typedef    AJACircularBuffer <AVDataBuffer *>    MyCircularBuffer;

        AJAThread *                  mProducerThread;                         ///< @brief    My producer thread object -- does the frame capturing
        AJALock *                    mLock;                                   ///< @brief    Global mutex to avoid device frame buffer allocation race condition
        NTV2DeviceID                 mDeviceID;                               ///< @brief    My device identifier
        const std::string            mDeviceSpecifier;                        ///< @brief    The device specifier string
        const NTV2Channel            mInputChannel;                           ///< @brief    My input channel
        NTV2InputSource              mInputSource;                            ///< @brief    The input source I'm using
        NTV2VideoFormat              mVideoFormat;                            ///< @brief    My video format
        NTV2FrameBufferFormat        mPixelFormat;                            ///< @brief    My pixel format
        NTV2FormatDescriptor         mFormatDesc;
        NTV2EveryFrameTaskMode       mSavedTaskMode;                          ///< @brief    Used to restore prior every-frame task mode
        NTV2AudioSystem              mAudioSystem;                            ///< @brief    The audio system I'm using (if any)
        bool                         mDoLevelConversion;                      ///< @brief    Demonstrates a level A to level B conversion
        bool                         mGlobalQuit;                             ///< @brief    Set "true" to gracefully stop
        bool                         mWithAnc;                                ///< @brief    Capture custom anc data?
        uint32_t                     mVideoBufferSize;                        ///< @brief    My video buffer size, in bytes
                                     
        AVDataBuffer                 mAVHostBuffer [CIRCULAR_BUFFER_SIZE];    ///< @brief    My host buffers
        MyCircularBuffer             mAVCircularBuffer;                       ///< @brief    My ring buffer object
                                     
        void *                       mFrameArrivedCallbackContext;
        FrameArrivedCallback *       mFrameArrivedCallback;
        bool                         mFrameLocked;
        AjaDevice::Ref               mDeviceRef;
        const AjaDevice::InitParams* mInitParams;
};    //    NTV2Capture

#endif    //    _NTV2CAPTURE_H
