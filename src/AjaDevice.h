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

#include <string>
#include <map>
#include <memory>
#include <mutex>
#include "ajantv2\includes\ntv2card.h"
#include "ajabase\common\types.h"

namespace streampunk {

// Class to manage the reservation and release of AJA devices between multiple nodes
class AjaDevice 
{
// Constants
//
public:

    static const string DEFAULT_DEVICE_SPECIFIER;
    static const uint32_t DEFAULT_CAPTURE_CHANNEL = 1;
    static const uint32_t DEFAULT_PLAYBACK_CHANNEL = 3;

// Typedefs and nested classes
//
public:
    
    // Factory method prototype to allow the factory methods to be overridden for testing
    typedef CNTV2Card* (*CNTV2Card_Factory)(void);

    struct InitParams
    {
        bool doMultiChannel;
        ULWord appSignature;
    };

    class Ref
    {
    public:

        // Create a reference - note: for now the init params must be stored in static memory
        Ref();

        // Release() is automatically called within the destructor, when the Ref goes out of scope
        ~Ref();

        // Try to initialize the specified device with the given init params. 
        AJAStatus Initialize(const std::string& deviceSpecifier, const InitParams* initParams);

        // Manually release the referenced device
        void Release();

        // Allow the reference to be checked in a conditional clause
        operator bool() { return (bool)ref_; }

        CNTV2Card* operator->() { assert(ref_); return (ref_ ? ref_->device_.get() : nullptr); }

    private:

        shared_ptr<AjaDevice> ref_;
    };

    // Test function - shouldn't be needed in production code
    static uint32_t GetRefCount(std::string& deviceSpecifier);

    // Overrideable factory function for creating the underlying CNTV2Card class
    static CNTV2Card_Factory NTV2Card_Factory;

    // default CNTV2Card factory function
    static CNTV2Card* DefaultCNTV2CardFactory(void) { return new CNTV2Card; }

    virtual ~AjaDevice();

private:

    AjaDevice(std::string& deviceSpecifier);

    AJAStatus Initialize(const InitParams* initParams);
    void ReleaseDevice();

    std::string            deviceSpecifier_;
    unique_ptr<CNTV2Card>  device_;
    NTV2EveryFrameTaskMode mode_;
    const InitParams*      initParams_;
    NTV2DeviceID           deviceId_;

    static AJAStatus AddRef(std::string deviceSpecifier, shared_ptr<AjaDevice>& ref, const InitParams* initParams);
    static void ReleaseRef(shared_ptr<AjaDevice>& ref);

    static std::map<std::string, shared_ptr<AjaDevice>> references_;
    static std::mutex protectRefCounts_;
    static AJAStatus lastError_;
};

extern const AjaDevice::InitParams DEFAULT_INIT_PARAMS;
}