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

#include "stdafx.h"
#include "CppUnitTest.h"
#include "AjaDevice.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace streampunk;

namespace AjatationTest
{
    string defaultDeviceId("0");
    string otherDeviceId("1");

    const AjaDevice::InitParams otherInitParams = {
        false,                          // Multi-channel
        AJA_FOURCC('A', 'B', 'C', 'D')  // App Signature
    };

    TEST_CLASS(Test_AjaDevice)
    {
    public:
        
        TEST_METHOD(TestBasicLifecycle)
        {
            {
                Assert::AreEqual((int)AjaDevice::GetRefCount(defaultDeviceId), 0);

                AjaDevice::Ref ref1;
                auto result1 = ref1.Initialize(defaultDeviceId, &DEFAULT_INIT_PARAMS);

                Assert::AreEqual((int)AJA_STATUS_SUCCESS, (int)result1);
                Assert::AreEqual(2, (int)AjaDevice::GetRefCount(defaultDeviceId));

                AjaDevice::Ref ref2;
                auto result2 = ref2.Initialize(defaultDeviceId, &otherInitParams);


                Assert::AreEqual((int)AJA_STATUS_BAD_PARAM, (int)result2);
                Assert::AreEqual(2, (int)AjaDevice::GetRefCount(defaultDeviceId));

                {
                    AjaDevice::Ref ref3;
                    auto result3 = ref3.Initialize(defaultDeviceId, &DEFAULT_INIT_PARAMS);

                    Assert::AreEqual((int)AJA_STATUS_SUCCESS, (int)result3);
                    Assert::AreEqual(3, (int)AjaDevice::GetRefCount(defaultDeviceId));
                }

                Assert::AreEqual(2, (int)AjaDevice::GetRefCount(defaultDeviceId));
            }

            Assert::AreEqual((int)AjaDevice::GetRefCount(defaultDeviceId), 0);
        }

    };
}