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

#include <cstdint>
#include <tuple>
#include <assert.h>
#include "ntv2capture.h"

namespace streampunk
{

namespace Aja
{

class AudioTransform
{
    static const uint32_t MAX_BUFFER_SIZE = NTV2_AUDIOSIZE_MAX;
    static const uint32_t AUDIO_4BYTE_SAMPLE_SIZE_BYTES = 4; // 24bit audio in 32bit buffers
    static const uint32_t AUDIO_3BYTE_SAMPLE_SIZE_BYTES = 3; // 24bit audio

public:

    AudioTransform()
    {
    }


    std::tuple<const char*, uint32_t> TransformFromCard(const char* inputBuffer, uint32_t inputBufferSize, uint32_t inputChannels, uint32_t outputChannels)
    {
        // For the time being, just to be safe
        assert(inputChannels == 16);
        assert(outputChannels == 2);

        uint32_t strideBytes = inputChannels * AUDIO_4BYTE_SAMPLE_SIZE_BYTES; // where stride is the distance between subsequent samples across all channels

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
            memcpy_s(writeBuffer, MAX_BUFFER_SIZE - bytesWritten, (void *)&sample, AUDIO_3BYTE_SAMPLE_SIZE_BYTES);

            bytesWritten += AUDIO_3BYTE_SAMPLE_SIZE_BYTES;
            writeBuffer  += AUDIO_3BYTE_SAMPLE_SIZE_BYTES;

            // Right Channel
            sample = *(int *)&readBuffer[AUDIO_4BYTE_SAMPLE_SIZE_BYTES];
            sample = (sample & 0xFF000000) >> 24 | (sample & 0x00FF0000) >> 8 | (sample & 0x0000FF00) << 8 | (sample & 0x000000FF) << 24;
            sample = sample & 0x00FFFFFF;
            memcpy_s(writeBuffer, MAX_BUFFER_SIZE - bytesWritten, (void *)&sample, AUDIO_3BYTE_SAMPLE_SIZE_BYTES);

            bytesWritten += AUDIO_3BYTE_SAMPLE_SIZE_BYTES;
            writeBuffer  += AUDIO_3BYTE_SAMPLE_SIZE_BYTES;
            readBuffer   += strideBytes;
            bytesRead    += strideBytes;
        }

        assert(bytesRead == inputBufferSize);

        return std::make_tuple(outputBuffer, bytesWritten);
    }


    std::tuple<const char*, uint32_t> TransformToCard(const char* inputBuffer, uint32_t inputBufferSize, uint32_t inputChannels, uint32_t outputChannels)
    {
        // For the time being, just to be safe
        assert(inputChannels == 2);
        assert(outputChannels == 16);

        uint32_t inputStrideBytes = inputChannels * AUDIO_3BYTE_SAMPLE_SIZE_BYTES;
        uint32_t outputStrideBytes = outputChannels * AUDIO_4BYTE_SAMPLE_SIZE_BYTES; // where stride is the distance between subsequent samples across all channels

        uint32_t numSamples = inputBufferSize / inputStrideBytes;
        uint32_t outputBufferSize = numSamples * outputStrideBytes;

        assert(outputBufferSize <= MAX_BUFFER_SIZE);
        memset(outputBuffer, 0x00, outputBufferSize);

        uint32_t bytesWritten = 0;
        uint32_t bytesRead = 0;
        const char* readBuffer = inputBuffer;
        char* writeBuffer = outputBuffer;

        for(uint32_t sampleIdx = 0; sampleIdx < numSamples; sampleIdx++)
        {
            // Convert the 4bytes of each channel data into a 24 bit sample
            // Left Channel
            int sample = (*(int *)readBuffer) & 0x00FFFFFF;
			sample = (sample & 0xFF000000) >> 24 | (sample & 0x00FF0000) >> 8 | (sample & 0x0000FF00) << 8 | (sample & 0x000000FF) << 24;
			memcpy_s(writeBuffer, MAX_BUFFER_SIZE - bytesWritten, (void *)&sample, AUDIO_4BYTE_SAMPLE_SIZE_BYTES);

            // Right Channel
			sample = (*(int *)&readBuffer[AUDIO_3BYTE_SAMPLE_SIZE_BYTES]) & 0xFFFFFF00;
			sample = (sample & 0xFF000000) >> 24 | (sample & 0x00FF0000) >> 8 | (sample & 0x0000FF00) << 8 | (sample & 0x000000FF) << 24;
			memcpy_s(&writeBuffer[AUDIO_4BYTE_SAMPLE_SIZE_BYTES], MAX_BUFFER_SIZE - bytesWritten, (void *)&sample, AUDIO_4BYTE_SAMPLE_SIZE_BYTES);

            bytesWritten += outputStrideBytes;
            writeBuffer  += outputStrideBytes;
            readBuffer   += inputStrideBytes;
            bytesRead    += inputStrideBytes;
        }

        //cout << "%%% Bytes Read: " << bytesRead << "; input buffer size: " << inputBufferSize << endl;
        //cout << "%%% Bytes Written: " << bytesWritten << "; output buffer size: " << outputBufferSize << endl;
        // ???!!assert(bytesRead == inputBufferSize);
        assert(bytesWritten == outputBufferSize);

        return std::make_tuple(outputBuffer, bytesWritten);
    }

private:

    char outputBuffer[MAX_BUFFER_SIZE];
};

}
}