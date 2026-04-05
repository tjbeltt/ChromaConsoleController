/*
  ==============================================================================

    VersionInfo.h
    Created: 5 Apr 2026 12:19:31am
    Author:  tjbac

  ==============================================================================
*/

#pragma once

namespace VersionInfo
{
    static constexpr int major = 1;
    static constexpr int minor = 1;
    static constexpr int patch = 0;

    static inline juce::String toString()
    {
        return juce::String(major) + "." + juce::String(minor) + "." + juce::String(patch);
    }

    // Compare against a "major.minor.patch" string
    // Returns true if the remote version is newer
    static inline bool isNewerThan(const juce::String& remoteVersion)
    {
        auto parts = juce::StringArray::fromTokens(remoteVersion, ".", "");
        if (parts.size() < 3) return false;

        int rMajor = parts[0].getIntValue();
        int rMinor = parts[1].getIntValue();
        int rPatch = parts[2].getIntValue();

        if (rMajor != major) return rMajor > major;
        if (rMinor != minor) return rMinor > minor;
        return rPatch > patch;
    }
}