/*
  ==============================================================================

    PresetManager.cpp
    Created: 11 Feb 2026 9:08:35pm
    Author:  tjbac

  ==============================================================================
*/

#include "PresetManager.h"
#include "PluginProcessor.h"

PresetManager::PresetManager(juce::AudioProcessor& p) : processor(p)
{
    // Set default preset directory (plugin data folder)
    auto appData = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    presetDirectory = appData.getChildFile(JucePlugin_Manufacturer)
        .getChildFile(JucePlugin_Name)
        .getChildFile("Presets");

   // Create directory if it doesn't exist
    if (!presetDirectory.exists())
        presetDirectory.createDirectory();

    // Load initial preset list and MIDI mappings
    scanPresetsInDirectory();
    loadMidiMappings();
}

PresetManager::~PresetManager()
{
    saveMidiMappings();
}

bool PresetManager::savePreset(const juce::String& presetName, const juce::String& category)
{
    if (presetName.isEmpty())
        return false;

    // Create preset file
    auto presetFile = createPresetFile(presetName, category);

    // Get current processor state
    juce::MemoryBlock stateData;
    processor.getStateInformation(stateData); // likely not to work with current setup

    // Create ValueTree for preset
    juce::ValueTree presetState("PresetState");
    presetState.setProperty("name", presetName, nullptr);
    presetState.setProperty("category", category, nullptr);
    presetState.setProperty("version", JucePlugin_VersionString, nullptr);
    presetState.setProperty("timestamp", juce::Time::getCurrentTime().toISO8601(true), nullptr);

    // Store the state data as binary
    auto stateTree = juce::ValueTree::fromXml(stateData.toString());
    if (!stateTree.isValid())
    {
        // If XML parsing fails, store as base64
        presetState.setProperty("stateData", stateData.toBase64Encoding(), nullptr);
    } else {
        presetState.appendChild(stateTree, nullptr);
    }

    // Write to file
    auto xml = presetState.createXml();
    if (xml == nullptr || !xml->writeToFile(presetFile, {}))
        return false;

    // Update preset list
    {
        const juce::ScopedLock sl(presetLock);
        scanPresetsInDirectory();

        // Find and set as current preset
        for (int i = 0; i < presets.size(); i++)
        {
            if (presets[i].file == presetFile)
            {
                currentPresetIndex = i;
                break;
            }
        }
    }

    // Notify listeners
    Preset savedPreset;
    savedPreset.name = presetName;
    savedPreset.category = category;
    savedPreset.file = presetFile;
    savedPreset.state = presetState;

    notifyPresetSaved(savedPreset);
    notifyPresetListChanged();

    return true;
}

bool PresetManager::getPreserveMidiChannel() const
{
    return preserveMidiChannel.load();
}


void PresetManager::setPreserveMidiChannel(bool shouldPreserve)
{
    preserveMidiChannel.store(shouldPreserve);
}

bool PresetManager::loadPreset(const juce::File& presetFile)
{
    if (!presetFile.existsAsFile())
        return false;

    // Parse Preset File
    auto xml = juce::XmlDocument::parse(presetFile);
    if (xml == nullptr)
        return false;

    auto presetState = juce::ValueTree::fromXml(*xml);
    if (!presetState.isValid())
        return false;

    // Extract State Data
    juce::MemoryBlock stateData;

    if (presetState.hasProperty("stateData"))
    {
        // Load from base64
        auto base64 = presetState.getProperty("stateData").toString();
        stateData.fromBase64Encoding(base64);
    } 
    else 
    {
        // Try to extract from child ValueTree
        for (int i = 0; i < presetState.getNumChildren(); i++)
        {
            auto child = presetState.getChild(i);
            auto childXml = child.createXml();
            if (childXml)
            {
                auto xmlString = childXml->toString();
                stateData.append(xmlString.toRawUTF8(), xmlString.getNumBytesAsUTF8());
                break;
            }
        }
    }

    // Save current MIDI channel before loading preset
    float savedMidiChannelNormalized = 0.0f;
    bool shouldRestore = preserveMidiChannel.load();

    if (shouldRestore)
    {
        auto& chromaProcessor = dynamic_cast<ChromaConsoleControllerAudioProcessor&>(processor);
        if (auto* midiChannelParam = chromaProcessor.parameters.getParameter("midiChannel"))
        {
            savedMidiChannelNormalized = midiChannelParam->getValue();
        }
    }

    // Load Preset state
    // This will overwrite everything including the MIDI channel
    if (stateData.getSize() > 0)
        processor.setStateInformation(stateData.getData(), (int)stateData.getSize());

    // Restore MIDI channel if preservation enabled
    if (shouldRestore)
    {
        auto& chromaProcessor = dynamic_cast<ChromaConsoleControllerAudioProcessor&>(processor);
        if (auto* midiChannelParam = chromaProcessor.parameters.getParameter("midiChannel"))
        {
            midiChannelParam->setValueNotifyingHost(savedMidiChannelNormalized);
        }
    }

    {
        const juce::ScopedLock sl(presetLock);
        for (int i = 0; i < presets.size(); i++)
        {
            if (presets[i].file == presetFile)
            {
                currentPresetIndex = i;
                break;
            }
        }
    }

    // Create preset object for notification
    Preset loadedPreset;
    loadedPreset.name = presetState.getProperty("name", "Unknown").toString();
    loadedPreset.category = presetState.getProperty("category", "").toString();
    loadedPreset.file = presetFile;
    loadedPreset.state = presetState;

    notifyPresetLoaded(loadedPreset);
    notifyCurrentPresetChanged();

    return true;
}

bool PresetManager::loadPresetByIndex(int index)
{
    juce::File fileToLoad;

    {
        const juce::ScopedLock sl(presetLock);

        if (index < 0 || index >= presets.size())
            return false;

        fileToLoad = presets[index].file;
    }

    // Load outside the lock
    return loadPreset(fileToLoad);
}
/*
bool PresetManager::deletePreset(const juce::File& presetFile)
{
    if (!presetFile.existsAsFile())
        return false;

    // Remove Midi Mapping if exists
    {
        const juce::ScopedLock sl(midiMappingLock);
        juce::Array<int> notesToRemove;

        for (juce::HashMap<int, juce::File>::Iterator i(midiNoteToPreset); i.next();)
        {
            if (i.getValue() == presetFile)
                notesToRemove.add(i.getKey());
        }

        for (auto note : notesToRemove)
        {
            midiNoteToPreset.remove(note);
        }
    }

    bool success = presetFile.deleteFile();

    if (success)
    {
        scanPresetsInDirectory();
        notifyPresetListChanged();
    }

    return success;
}
*/

bool PresetManager::deletePreset(const juce::File& presetFile)
{

    if (!presetFile.existsAsFile())
    {
        return false;
    }

    bool success = presetFile.deleteFile();

    if (success)
    {
        // Check if we deleted the currently loaded preset
        bool deletedCurrentPreset = false;
        {
            const juce::ScopedLock sl(presetLock);
            if (currentPresetIndex >= 0 && currentPresetIndex < presets.size())
                deletedCurrentPreset = (presets[currentPresetIndex].file == presetFile);
        }
        scanPresetsInDirectory();
        notifyPresetListChanged();

        if (deletedCurrentPreset)
        {
            {
                const juce::ScopedLock sl(presetLock);
                currentPresetIndex = -1;
            }
            notifyCurrentPresetChanged();
        }
    }

    return success;
}

void PresetManager::loadNextPreset()
{
    juce::File fileToLoad;

    {
        const juce::ScopedLock sl(presetLock);

        if (presets.isEmpty())
            return;

        int nextIndex = (currentPresetIndex + 1) % presets.size();
        fileToLoad = presets[nextIndex].file;
    }

    // Load outside the lock to avoid deadlock
    loadPreset(fileToLoad);
}

void PresetManager::loadPreviousPreset()
{
    juce::File fileToLoad;

    {
        const juce::ScopedLock sl(presetLock);

        if (presets.isEmpty())
            return;

        int prevIndex = currentPresetIndex - 1;
        if (prevIndex < 0)
            prevIndex = presets.size() - 1;

        fileToLoad = presets[prevIndex].file;
    }

    loadPreset(fileToLoad);
}

int PresetManager::getCurrentPresetIndex() const
{
    const juce::ScopedLock sl(presetLock);
    return currentPresetIndex;
}

const PresetManager::Preset* PresetManager::getCurrentPreset() const
{
    const juce::ScopedLock sl(presetLock);
    if (currentPresetIndex >= 0 && currentPresetIndex < presets.size())
        return &presets[currentPresetIndex];

    return nullptr;
}

juce::Array<PresetManager::Preset> PresetManager::getAllPresets() const
{
    const juce::ScopedLock sl(presetLock);
    return presets;
}

juce::Array<PresetManager::Preset> PresetManager::getPresetsByCategory(const juce::String& category) const
{
    const juce::ScopedLock sl(presetLock);

    juce::Array<Preset> filtered;
    for (const auto& preset : presets)
    {
        if (preset.category == category)
            filtered.add(preset);
    }
    
    return filtered;
}

juce::StringArray PresetManager::getCategories() const
{
    const juce::ScopedLock sl(presetLock);

    juce::StringArray categories;
    for (const auto& preset : presets)
    {
        if (preset.category.isNotEmpty() && !categories.contains(preset.category))
            categories.add(preset.category);
    }

    categories.sort(true);
    return categories;
}

void PresetManager::refreshPresetList()
{
    scanPresetsInDirectory();
    notifyPresetListChanged();
}

//=============================================================================
bool PresetManager::setMidiNoteForPreset(const juce::File& presetFile, int midiNote)
{
    if (midiNote < 0 || midiNote > 127)
        return false;

    if (!presetFile.existsAsFile())
        return false;

    const juce::ScopedLock sl(midiMappingLock);
    midiNoteToPreset.set(midiNote, presetFile);
    saveMidiMappings();
    notifyPresetListChanged();

    return true;
}

bool PresetManager::loadPresetFromMidiNote(int midiNote)
{
    const juce::ScopedLock sl(midiMappingLock);
    if (midiNoteToPreset.contains(midiNote))
    {
        auto presetFile = midiNoteToPreset[midiNote];
        return loadPreset(presetFile);
    }

    return false;
}

int PresetManager::getMidiNoteForPreset(const juce::File& presetFile) const
{
    const juce::ScopedLock sl(midiMappingLock);

    for (juce::HashMap<int, juce::File>::Iterator i(midiNoteToPreset); i.next();)
    {
        if (i.getValue() == presetFile)
            return i.getKey();
    }

    return -1;
}

void PresetManager::clearMidiMapping(int midiNote)
{
    const juce::ScopedLock sl(midiMappingLock);
    midiNoteToPreset.remove(midiNote);
    saveMidiMappings();
    notifyPresetListChanged();
}

const juce::HashMap<int, juce::File>& PresetManager::getMidiMappings() const
{
    const juce::ScopedLock sl(midiMappingLock);
    return midiNoteToPreset;
}
//=========================================================================================================
void PresetManager::setPresetDirectory(const juce::File& directory)
{
    presetDirectory = directory;

    if (!presetDirectory.exists())
        presetDirectory.createDirectory();

    scanPresetsInDirectory();
    loadMidiMappings();
    notifyPresetListChanged();
}

//=========================================================================================================
void PresetManager::addListener(Listener* listener)
{
    listeners.add(listener);
}

void PresetManager::removeListener(Listener* listener)
{
    listeners.remove(listener);
}

//==========================================================================================================
juce::ValueTree PresetManager::getState() const
{
    juce::ValueTree state("PresetManagerState");

    state.setProperty("presetDirectory", presetDirectory.getFullPathName(), nullptr);
    state.setProperty("currentPresetIndex", currentPresetIndex, nullptr);
    //state.setProperty("preserveMidiChannel", preserveMidiChannel.load(), nullptr);

    return state;
}

void PresetManager::setState(const juce::ValueTree& state)
{
    if (!state.hasType("PresetManagerState"))
        return;

    auto dir = state.getProperty("presetDirectory").toString();
    if (dir.isNotEmpty())
        setPresetDirectory(juce::File(dir));

    currentPresetIndex = state.getProperty("currentPresetIndex", -1);

    //preserveMidiChannel.store(state.getProperty("preserveMidiChannel", true));
}

//============================================================================================================
void PresetManager::scanPresetsInDirectory()
{
    const juce::ScopedLock sl(presetLock);

    presets.clear();

    if (!presetDirectory.exists())
        return;

    // Scan for preset files recursively
    juce::Array<juce::File> presetFiles;
    presetDirectory.findChildFiles(presetFiles,
        juce::File::findFiles,
        true,
        juce::String("*") + PRESET_EXTENSION);

    for (const auto& file : presetFiles)
    {
        auto preset = loadPresetFromFile(file);
        if (preset.isValid())
        {
            // Check for Midi Mapping
            preset.midiNote = getMidiNoteForPreset(file);
            presets.add(preset);
        }
    }

    // Sort by Category then Name using a comparator struct
    struct PresetComparator
    {
        static int compareElements(const Preset& a, const Preset& b)
        {
            int categoryCompare = a.category.compareIgnoreCase(b.category);
            if (categoryCompare != 0)
                return categoryCompare;
            return a.name.compareIgnoreCase(b.name);
        }
    };

    PresetComparator comparator;
    presets.sort(comparator);
}

juce::File PresetManager::createPresetFile(const juce::String& presetName, const juce::String& category)
{
    auto categoryDir = presetDirectory;

    if (category.isNotEmpty())
    {
        categoryDir = presetDirectory.getChildFile(category);
        categoryDir.createDirectory();
    }

    // Sanitize preset name for filename
    auto safeName = presetName.replaceCharacters("/\\:*?\"<>|", "___________");
    auto fileName = safeName + PRESET_EXTENSION;

    auto file = categoryDir.getChildFile(fileName);

    // If file exists, add number suffix
    int counter = 1;
    while (file.exists())
    {
        fileName = safeName + "_" + juce::String(counter++) + PRESET_EXTENSION;
        file = categoryDir.getChildFile(fileName);
    }

    return file;
}

PresetManager::Preset PresetManager::loadPresetFromFile(const juce::File& file)
{
    Preset preset;
    preset.file = file;

    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr)
        return preset;

    auto state = juce::ValueTree::fromXml(*xml);
    if (!state.isValid())
        return preset;

    preset.name = state.getProperty("name", file.getFileNameWithoutExtension()).toString();
    preset.category = state.getProperty("category", "").toString();
    preset.state = state;

    // If category is empty try to get it from the parent folder
    if (preset.category.isEmpty())
    {
        auto parentDir = file.getParentDirectory();
        if (parentDir != presetDirectory)
            preset.category = parentDir.getFileName(); // ?? feels weird
    }

    return preset;
}

void PresetManager::saveMidiMappings()
{
    const juce::ScopedLock sl(midiMappingLock);

    juce::ValueTree mappings("MidiMappings");

    for (juce::HashMap<int, juce::File>::Iterator i(midiNoteToPreset); i.next();)
    {
        juce::ValueTree item("Mapping");
        item.setProperty("note", i.getKey(), nullptr);
        item.setProperty("preset", i.getValue().getFullPathName(), nullptr);
        mappings.appendChild(item, nullptr);
    }

    auto xml = mappings.createXml();
    if (xml)
    {
        auto mappingFile = presetDirectory.getChildFile(MIDI_MAPPING_FILE);
        xml->writeTo(mappingFile);
    }

}

void PresetManager::loadMidiMappings()
{
    const juce::ScopedLock sl(midiMappingLock);

    midiNoteToPreset.clear();

    auto mappingFile = presetDirectory.getChildFile(MIDI_MAPPING_FILE);
    if (!mappingFile.existsAsFile())
        return;

    auto xml = juce::XmlDocument::parse(mappingFile);
    if (!xml)
        return;

    auto mappings = juce::ValueTree::fromXml(*xml);
    if (!mappings.isValid())
        return;

    for (int i = 0; i < mappings.getNumChildren(); i++)
    {
        auto item = mappings.getChild(i);
        int note = item.getProperty("note", -1);
        auto presetPath = item.getProperty("preset").toString();

        if (note >= 0 && note <= 127 && presetPath.isNotEmpty())
        {
            juce::File presetFile(presetPath);
            if (presetFile.existsAsFile())
                midiNoteToPreset.set(note, presetFile);
        }
    }
}

void PresetManager::notifyPresetLoaded(const Preset& preset)
{
    listeners.call([&](Listener& l) { l.presetLoaded(preset); });
}

void PresetManager::notifyPresetSaved(const Preset& preset)
{
    listeners.call([&](Listener& l) {l.presetSaved(preset); });
}

void PresetManager::notifyPresetListChanged()
{
    listeners.call([](Listener& l) {l.presetListChanged(); });
}

void PresetManager::notifyCurrentPresetChanged()
{
    listeners.call([](Listener& l) {l.currentPresetChanged(); });
}