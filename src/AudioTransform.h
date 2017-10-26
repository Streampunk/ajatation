#pragma once

#include <cstdint>
#include <tuple>
#include <assert.h>
#include "ntv2capture.h"

namespace streampunk
{
 
class AudioTransform
{
    static const uint32_t MAX_BUFFER_SIZE = NTV2_AUDIOSIZE_MAX;
    static const uint32_t AUDIO_INPUT_SAMPLE_SIZE_BYTES = 4; // 24bit audio in 32bit buffers
    static const uint32_t AUDIO_OUTPUT_SAMPLE_SIZE_BYTES = 3; // 24bit audio

public:

    AudioTransform()
    {
    }


    std::tuple<const char*, uint32_t> Transform(const char* inputBuffer, uint32_t inputBufferSize, uint32_t inputChannels, uint32_t outputChannels)
    {
        uint32_t strideBytes = inputChannels * AUDIO_INPUT_SAMPLE_SIZE_BYTES; // where stride is the distance between subsequent samples across all channels

        uint32_t numSamples = inputBufferSize / strideBytes;
        uint32_t bytesWritten = 0;
        uint32_t bytesRead = 0;
        const char* readBuffer = inputBuffer;
        char* writeBuffer = outputBuffer;

        for(uint32_t sampleIdx = 0; sampleIdx < numSamples; sampleIdx++)
        {
            // Convert the 4bytes of each channel data into a 24 bit sample
            // Left Channel
            int sample = *(int *)readBuffer;
            sample = (sample & 0xFF000000) >> 24 | (sample & 0x00FF0000) >> 8 | (sample & 0x0000FF00) << 8 | (sample & 0x000000FF) << 24;
            sample = sample & 0x00FFFFFF;
            memcpy_s(writeBuffer, MAX_BUFFER_SIZE - bytesWritten, (void *)&sample, AUDIO_OUTPUT_SAMPLE_SIZE_BYTES);

            bytesWritten += AUDIO_OUTPUT_SAMPLE_SIZE_BYTES;
            writeBuffer  += AUDIO_OUTPUT_SAMPLE_SIZE_BYTES;

            // Right Channel
            sample = *(int *)&readBuffer[AUDIO_INPUT_SAMPLE_SIZE_BYTES];
            sample = (sample & 0xFF000000) >> 24 | (sample & 0x00FF0000) >> 8 | (sample & 0x0000FF00) << 8 | (sample & 0x000000FF) << 24;
            sample = sample & 0x00FFFFFF;
            memcpy_s(writeBuffer, MAX_BUFFER_SIZE - bytesWritten, (void *)&sample, AUDIO_OUTPUT_SAMPLE_SIZE_BYTES);

            bytesWritten += AUDIO_OUTPUT_SAMPLE_SIZE_BYTES;
            writeBuffer  += AUDIO_OUTPUT_SAMPLE_SIZE_BYTES;
            readBuffer   += strideBytes;
            bytesRead    += strideBytes;
        }

        assert(bytesRead == inputBufferSize);

        return std::make_tuple(outputBuffer, bytesWritten);
    }


private:

    char outputBuffer[MAX_BUFFER_SIZE];
};

}