/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2023 Sky UK
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef USBPLUGIN_H
#define USBPLUGIN_H

#include <string>
#include <memory>
 
#include <RdkPluginBase.h>

/**
* @brief Dobby Usb plugin.
*
*/
class UsbPlugin : public RdkPluginBase
{
public:
    UsbPlugin(std::shared_ptr<rt_dobby_schema> &containerConfig,
                const std::shared_ptr<DobbyRdkPluginUtils> &utils,
                const std::string &rootfsPath);
    ~UsbPlugin();
 
public:
    inline std::string name() const override
    {
        return mName;
    };

    unsigned hookHints() const override;

public:
    bool postInstallation() override;
    bool postHalt() override;
    bool postStart() override;
 
public:
    std::vector<std::string> getDependencies() const override;

private:
    bool monitor();
    bool doStuff(const std::string& pathToCreate);

    bool addCapability(const std::string& cap);

    const std::string mName;
    const std::string mRootfsPath;
    std::shared_ptr<rt_dobby_schema> mContainerConfig;
    const std::shared_ptr<DobbyRdkPluginUtils> mUtils;
    
    FILE* log;
};
 
#endif // !defined(GPUPLUGIN_H)