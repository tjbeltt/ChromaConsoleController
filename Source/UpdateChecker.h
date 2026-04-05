/*
  ==============================================================================

    UpdateChecker.h
    Created: 4 Apr 2026 11:49:10pm
    Author:  tjbac

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "VersionInfo.h"

class UpdateChecker : private juce::Thread
{
public:
    struct Listener
    {
        virtual ~Listener() = default;
        virtual void updateAvailable(const juce::String& newVersion,
            const juce::String& downloadUrl,
            const juce::String& changelog) = 0;
    };

    UpdateChecker(Listener* listener)
        : Thread("UpdateChecker"), listenerPtr(listener) {
    }

    ~UpdateChecker() override { stopThread(2000); }

    void checkForUpdate()
    {
        startThread(juce::Thread::Priority::background);
    }

private:
    void run() override
    {
        // Point this at your version.json (raw GitHub URL or GitHub Pages)
        juce::URL versionUrl("https://raw.githubusercontent.com/tjbeltt/ChromaConsoleController/refs/heads/main/version.json");

        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
            .withConnectionTimeoutMs(5000);

        auto stream = versionUrl.createInputStream(options);

        if (stream == nullptr || threadShouldExit())
            return; // Silently fail — don't bother the user

        auto response = stream->readEntireStreamAsString();
        auto json = juce::JSON::parse(response);

        if (auto* obj = json.getDynamicObject())
        {
            auto latestVersion = obj->getProperty("latest_version").toString();
            auto downloadUrl = obj->getProperty("download_url").toString();
            auto changelog = obj->getProperty("changelog").toString();

            if (VersionInfo::isNewerThan(latestVersion) && listenerPtr != nullptr)
            {
                // Bounce back to the message thread for UI work
                juce::MessageManager::callAsync([this, latestVersion, downloadUrl, changelog]()
                    {
                        if (listenerPtr != nullptr)
                            listenerPtr->updateAvailable(latestVersion, downloadUrl, changelog);
                    });
            }
        }
    }

    Listener* listenerPtr = nullptr;
};