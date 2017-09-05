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

#include <assert.h>
#include "AjaDevice.h"
#include "ajabase\system\process.h"
#include "ajantv2\includes\ntv2devicescanner.h"

using namespace std;

namespace streampunk {

AjaDevice::CNTV2Card_Factory AjaDevice::NTV2Card_Factory = AjaDevice::DefaultCNTV2CardFactory;
const string AjaDevice::DEFAULT_DEVICE_SPECIFIER = "0";

const AjaDevice::InitParams DEFAULT_INIT_PARAMS = {
    false,                          // Multi-channel
    AJA_FOURCC('S', 'T', 'P', 'K')  // App Signature
};

map<string, shared_ptr<AjaDevice>> AjaDevice::references_;
mutex AjaDevice::protectRefCounts_;
AJAStatus AjaDevice::lastError_(AJA_STATUS_SUCCESS);

AjaDevice::Ref::Ref()
{
    {cout << "Creating Aja reference at address:" << (uintptr_t)this << endl; }
}

AjaDevice::Ref::~Ref()
{
    {cout << "Creating Aja reference at address: " << (uintptr_t)this << " reference is: " << (uintptr_t)ref_.get() << endl; }

    if (ref_)
    {
        {cout << "Releasing Aja reference at address: " << (uintptr_t)this << " reference is: " << (uintptr_t)ref_.get() << endl; }

        Release();
    }
}


AJAStatus AjaDevice::Ref::Initialize(const std::string& deviceSpecifier, const InitParams* initParams)
{
    assert(!ref_);

    AJAStatus status(AJA_STATUS_FAIL);

    if (!ref_)
    {
        {cout << "Initializing Aja reference at address:" << (uintptr_t)this << " reference is: " << (uintptr_t)ref_.get() << endl; }
        status = AjaDevice::AddRef(deviceSpecifier, ref_, initParams);
        {cout << "Initialized Aja reference at address:" << (uintptr_t)this << " reference is: " << (uintptr_t)ref_.get() << endl; }
    }

    return status;
}


void AjaDevice::Ref::Release()
{
    assert(ref_);

    if (ref_)
    {
        AjaDevice::ReleaseRef(ref_);
    }
}


uint32_t AjaDevice::GetRefCount(string& deviceSpecifier)
{
    uint32_t refCount(0);

    auto entry = references_.find(deviceSpecifier);

    if (entry != references_.end())
    {
        refCount = entry->second.use_count();
    }

    return refCount;
}


AjaDevice::~AjaDevice()
{
    {cout << "Destroying Aja device " << deviceSpecifier_ << " at address:" << (uintptr_t)this << endl; }
}


AjaDevice::AjaDevice(std::string& deviceSpecifier)
    : deviceSpecifier_(deviceSpecifier),
    mode_(NTV2_TASK_MODE_INVALID),
    initParams_(nullptr),
    deviceId_(DEVICE_ID_NOTFOUND)
{
    {cout << "Creating Aja device " << deviceSpecifier << " at address:" << (uintptr_t)this << endl; }
}


AJAStatus AjaDevice::AddRef(std::string deviceSpecifier, shared_ptr<AjaDevice>& ref, const InitParams* initParams)
{
    AJAStatus status(AJA_STATUS_FAIL);

    lock_guard<mutex> lock(protectRefCounts_);

    auto entry = references_.find(deviceSpecifier);

    // If there is an existing record, increment the ref count,
    // otherwise create and initialize the device
    if (entry != references_.end())
    {
        // All uses of the device must use the same set of initialization parameters
        if (initParams != entry->second->initParams_)
        {
            {cerr << "*** Error: attempt to initialize device with different initialization parameters!" << endl; }
            status = AJA_STATUS_BAD_PARAM;
        }
        else
        {
            {cout << "Adding reference to device " << deviceSpecifier << endl; }

            ref = entry->second;

            status = AJA_STATUS_SUCCESS;
        }
    }
    else
    {
        {cout << "First initialization of device " << deviceSpecifier << endl; }

        shared_ptr<AjaDevice> tempRef(new AjaDevice(deviceSpecifier));

        status = tempRef->Initialize(initParams);

        if (AJA_SUCCESS(status))
        {
            references_[deviceSpecifier] = tempRef;
            ref = tempRef;
        }
    }

    return status;
}


void AjaDevice::ReleaseRef(shared_ptr<AjaDevice>& ref)
{
    lock_guard<mutex> lock(protectRefCounts_);

    assert(ref);

    if(ref)
    {
        auto entry = references_.find(ref->deviceSpecifier_);

        assert(entry != references_.end());

        if (entry != references_.end())
        {
            assert(entry->second.get() == ref.get());

            ref.reset();

            // If there is only the reference in the map remaining, cleanup and remove
            // the record
            if (entry->second.use_count() == 1)
            {
                entry->second->ReleaseDevice();

                references_.erase(entry->second->deviceSpecifier_);
            }
        }
        else
        {
            cerr << "## ERROR:  Attempt to release a device that isn't recognised: " << ref->deviceSpecifier_ << endl;
        }
    }
}


AJAStatus AjaDevice::Initialize(const InitParams*  initParams)
{
    assert(!device_);
    if(device_)
    {
        cerr << "## ERROR:  Device '" << deviceSpecifier_ << "' is already initialized" << endl;
        return AJA_STATUS_FAIL;
    }

    unique_ptr<CNTV2Card> tempDevice(NTV2Card_Factory());

    if (!tempDevice)
    {
        // If memory really is low, don't log a message to make things worse
        return AJA_STATUS_MEMORY;
    }

    // Open the device...
    if (!CNTV2DeviceScanner::GetFirstDeviceFromArgument(deviceSpecifier_, *tempDevice.get()))
    {
        cerr << "## ERROR:  Device '" << deviceSpecifier_ << "' not found" << endl; 
        return AJA_STATUS_OPEN;
    }

    if (!tempDevice->IsDeviceReady(false))
    {
        cerr << "## ERROR:  Device '" << deviceSpecifier_ << "' not ready" << endl;
        return AJA_STATUS_INITIALIZE;
    }

    NTV2EveryFrameTaskMode oldMode;

    if (!initParams->doMultiChannel)
    {
        if (!tempDevice->AcquireStreamForApplication(initParams->appSignature, static_cast <uint32_t> (AJAProcess::GetPid())))
        {
            cerr << "## ERROR:  Device '" << deviceSpecifier_ << "' is acquired by a another application" << endl;
            return AJA_STATUS_BUSY;
        }

        tempDevice->GetEveryFrameServices(&oldMode);    //    Save the current service level
        mode_ = oldMode;
    }

    tempDevice->SetEveryFrameServices(NTV2_OEM_TASKS);            //    Set OEM service level

    // Transfer the device pointer to its permanent location
    device_.reset(tempDevice.release());
    initParams_ = initParams;

    deviceId_ = device_->GetDeviceID();

    if (::NTV2DeviceCanDoMultiFormat(deviceId_))
    {
        if (initParams_->doMultiChannel)
        {
            device_->SetMultiFormatMode(true);
        }
        else
        {
            device_->SetMultiFormatMode(false);
            device_->ClearRouting();
        }
    }
    else
    {
        device_->ClearRouting();
    }

    return AJA_STATUS_SUCCESS;
}


void AjaDevice::ReleaseDevice()
{
    if (!initParams_->doMultiChannel)
    {
        device_->SetEveryFrameServices(mode_);    //    Restore the previously saved service level
        device_->ReleaseStreamForApplication(initParams_->appSignature, static_cast <uint32_t> (AJAProcess::GetPid()));    //    Release the device
    }
}

}