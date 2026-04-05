/*
  ==============================================================================

    VersionInfo.h
    Created: 5 Apr 2026 12:19:31am
    Author:  tjbac

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace VersionInfo
{
    static inline juce::String currentVersion()
    {
        return JucePlugin_VersionString;
    }

    static inline bool isNewerThan(const juce::String& remoteVersion)
    {
        auto remote = juce::StringArray::fromTokens(remoteVersion, ".", "");
        auto local = juce::StringArray::fromTokens(JucePlugin_VersionString, ".", "");

        for (int i = 0; i < 3; i++)
        {
            int r = i < remote.size() ? remote[i].getIntValue() : 0;
            int l = i < local.size() ? local[i].getIntValue() : 0;

            if (r != l)
                return r > l;
        }

        return false;
    }
}