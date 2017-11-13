#include "stdafx.h"
#include "AjaController.h"
#include "NTV2Capture.h"
#include "NTV2Player.h"
#include "gen2ajaTypeMaps.h"

using namespace streampunk;

AjaController::AjaController(UpdateCallback statusCallback, void* context)
:   statusCallback(statusCallback),
    statusCallbackContext(context),
    status({false, false, false, 0, 0, 0, 0, 0, 0})
{
}


AjaController::~AjaController()
{
    StopPlayback();
    StopCapture();
}


bool AjaController::StartCapture(UINT32 deviceId, UINT32 channelId)
{
    bool success(false);

    std::lock_guard<std::mutex> lock(protectState);

    if(!capture)
    {
        TRACE("Initializing Capture...");

        char buffer[16] = {0};
        char* deviceIdString = itoa(deviceId, buffer, 10);
        const NTV2Channel channel(::GetNTV2ChannelForIndex(channelId - 1));

        capture.reset(new NTV2Capture(&DEFAULT_INIT_PARAMS, deviceIdString, true, channel, NTV2_FBF_10BIT_YCBCR));
        capture->SetFrameArrivedCallback(this, AjaController::FrameArrivedCallback);

        auto result = capture->Init();

        if(AJA_SUCCESS(result))
        {
            TRACE("Starting Capture Running...");

            result = capture->Run();

            if(AJA_SUCCESS(result))
            {
                TRACE("Capture Started!");
                success = true;
            }
            else
            {
                TRACE("Failed to start Capture running - error code: %d", result);
                capture.reset();
            }
        }
        else
        {
            TRACE("Failed to initialize Capture - error code: %d", result);
            capture.reset();
        }
    }

    status.capturing = success;

    return success;
}


bool AjaController::StopCapture()
{
    unique_ptr<NTV2Capture> temp;

    // Release the Capture object outside of the lock
    {
        std::lock_guard<std::mutex> lock(protectState);

        temp.reset(capture.release());

        status.capturing = false;
    }

    return true;
}


bool AjaController::IsCapturing()
{
    bool capturing(false);

    std::lock_guard<std::mutex> lock(protectState);

    capturing = (bool)capture;

    return capturing;
}


bool AjaController::StartPlayback(UINT32 deviceId, UINT32 channelId)
{
    bool success(false);

    std::lock_guard<std::mutex> lock(protectState);

    if(!player)
    {
        TRACE("Initializing Playback...");

        char buffer[16] = {0};
        char* deviceIdString = itoa(deviceId, buffer, 10);

        int                noAudio(0);                    //    Disable audio tone?
        const NTV2Channel channel(::GetNTV2ChannelForIndex(channelId - 1));
        const NTV2OutputDestination    outputDest(::NTV2ChannelToOutputDestination(channel));

        player.reset(new NTV2Player(&DEFAULT_INIT_PARAMS, deviceIdString, true, channel, NTV2_FBF_10BIT_YCBCR, outputDest, NTV2_FORMAT_1080i_5994,false, false, false));
        player->SetScheduledFrameCallback(this, AjaController::FrameRequiredCallback);

        auto result = player->Init();

        if(AJA_SUCCESS(result))
        {
            TRACE("Starting Playback Running...");

            result = player->Run();

            if(AJA_SUCCESS(result))
            {
                TRACE("Playback Started!");
                success = true;
            }
            else
            {
                TRACE("Failed to start Playback running - error code: %d", result);
                player.reset();
            }
        }
        else
        {
            TRACE("Failed to initialize Playback - error code: %d", result);
            player.reset();
        }
    }

    status.playing = success;

    return success;
}


bool AjaController::StopPlayback()
{
    unique_ptr<NTV2Player> temp;

    // Release the Capture object outside of the lock
    {
        std::lock_guard<std::mutex> lock(protectState);

        temp.reset(player.release());

        status.playing = false;
    }

    return true;
}


bool AjaController::IsPlaying()
{
    bool playing(false);

    std::lock_guard<std::mutex> lock(protectState);

    playing = (bool)player;

    return playing;
}


bool AjaController::CaptureToPlaybackRouting(bool enabled)
{
    std::lock_guard<std::mutex> lock(protectState);

    status.routing = enabled;

    return true;
}


void AjaController::GetStatus(Status& status)
{
    std::lock_guard<std::mutex> lock(protectState);

    status = this->status;
}


void AjaController::FrameArrivedCallback(void* pInstance)
{
    reinterpret_cast<AjaController*>(pInstance)->FrameArrivedCallback();
}


void AjaController::FrameArrivedCallback()
{
    {
        std::lock_guard<std::mutex> lock(protectState);

        if(capture)
        {
            auto frame = capture->LockNextFrame();

            status.captureFrames++;
            status.captureFrameSize = frame->fVideoBufferSize;

            if(player && status.routing)
            {
                player->ScheduleFrame((char*)frame->fVideoBuffer, frame->fVideoBufferSize, (char*)frame->fAudioBuffer, frame->fAudioBufferSize, &status.playbackFramesAvailable);

                status.playbackFrames++;
                status.playbackFrameSize = frame->fVideoBufferSize;
            }

            capture->UnlockFrame(&status.captureFramesAvailable);
        }
    }

    if(statusCallback)
    {
        statusCallback(statusCallbackContext);
    }
}


void AjaController::FrameRequiredCallback(void* pInstance)
{
    reinterpret_cast<AjaController*>(pInstance)->FrameRequiredCallback();
}


void AjaController::FrameRequiredCallback()
{
    std::lock_guard<std::mutex> lock(protectState);

}
