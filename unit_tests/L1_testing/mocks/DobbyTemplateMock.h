/* If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2023 Synamedia
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
*/

#pragma once

#include "DobbyTemplate.h"
#include <gmock/gmock.h>

class DobbyTemplateMock : public DobbyTemplateImpl{
public:

     virtual ~DobbyTemplateMock() = default;

     MOCK_METHOD(void, setSettings, (const std::shared_ptr<const IDobbySettings>& settings), (const,override));
     MOCK_METHOD(std::string, apply, (const ctemplate::TemplateDictionaryInterface* dictionary, bool prettyPrint), (override));
     MOCK_METHOD(bool, applyAt, (int dirFd, const std::string& fileName, const ctemplate::TemplateDictionaryInterface* dictionary, bool prettyPrint), (override));
};
