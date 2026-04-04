/*
  ==============================================================================

    PresetManager.h
    Created: 11 Feb 2026 9:08:35pm
    Author:  tjbac

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class PresetManager : public juce::ValueTree::Listener
{
public:
    struct Preset
    {
        juce::String name;
        juce::String category;
        juce::File file;
        juce::ValueTree state;
        int midiNote = -1; // -1 means no MIDI mapping

        bool isValid() const { return file.existsAsFile(); }
    };

    // Listener interface for preset changes
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void presetLoaded(const Preset& preset) {}
        virtual void presetSaved(const Preset& preset) {}
        virtual void presetListChanged() {}
        virtual void currentPresetChanged() {}
    };
    
    //=========================
    PresetManager(juce::AudioProcessor& processor);
    ~PresetManager() override;

    //=========================
    // Core preset operations
    bool savePreset(const juce::String& presetName, const juce::String& category = "User");
    bool writePresetToFile(const juce::File& presetFile, const juce::String& presetName, const juce::String& category);
    bool loadPreset(const juce::File& presetFile);
    bool loadPresetByIndex(int index);
    bool deletePreset(const juce::File& presetFile);

    //============================
    // Preset Navigation
    void loadNextPreset();
    void loadPreviousPreset();
    int getCurrentPresetIndex() const;
    const Preset* getCurrentPreset() const;

    //================================
    // Preset Bank Management
    juce::Array<Preset> getAllPresets() const;
    juce::Array<Preset> getPresetsByCategory(const juce::String& category) const;
    juce::StringArray getCategories() const;
    void refreshPresetList();
    std::pair<juce::File, juce::String> getIncrementedPresetFile(const juce::String& presetName, const juce::String& category);

    //================================
    // Midi Mapping
    bool setMidiNoteForPreset(const juce::File& presetFile, int midiNote);
    bool loadPresetFromMidiNote(int midiNote);
    int getMidiNoteForPreset(const juce::File& presetFile) const;
    void clearMidiMapping(int midiNote);
    const juce::HashMap<int, juce::File>& getMidiMappings() const;
    void setPreserveMidiChannel(bool shouldPreserve);
    bool getPreserveMidiChannel() const;

    //============================
    // Directories
    juce::File getPresetDirectory() const { return presetDirectory; }
    void setPresetDirectory(const juce::File& directory);

    //==========================
    // Listener Management
    void addListener(Listener* listener);
    void removeListener(Listener* listener);

    //=============================
    // State Management
    juce::ValueTree getState() const;
    void setState(const juce::ValueTree& state);

private:
    //===========================================
    // ValueTree::Listener Callbacks
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override {}
    void valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) override {}
    void valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int) override {}
    void valueTreeChildOrderChanged(juce::ValueTree&, int, int) override {}
    void valueTreeParentChanged(juce::ValueTree&) override {}

    //=============================================
    // Internal Methods
    void scanPresetsInDirectory();
    juce::File createPresetFile(const juce::String& presetName, const juce::String& category);
    Preset loadPresetFromFile(const juce::File& file);
    void saveMidiMappings();
    void loadMidiMappings();
    void notifyPresetLoaded(const Preset& preset);
    void notifyPresetSaved(const Preset& preset);
    void notifyPresetListChanged();
    void notifyCurrentPresetChanged();

    //========================================================
    juce::AudioProcessor& processor;
    juce::File presetDirectory;

    mutable juce::CriticalSection presetLock;
    juce::Array<Preset> presets;
    int currentPresetIndex = -1;

    mutable juce::CriticalSection midiMappingLock;
    juce::HashMap<int, juce::File> midiNoteToPreset; // Midi Note -> Preset File

    juce::ListenerList<Listener> listeners;

    std::atomic<bool> preserveMidiChannel{ true };

    static constexpr const char* PRESET_EXTENSION = ".ccpreset";
    static constexpr const char* MIDI_MAPPING_FILE = "midi_mappings.xml";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};