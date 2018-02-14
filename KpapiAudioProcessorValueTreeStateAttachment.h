#pragma once

#include <functional>
#include "../JuceLibraryCode/JuceHeader.h"

class KpapiAudioProcessorValueTreeStateAttachment : public AudioProcessorValueTreeState::Listener {

public:

    /** Adds a parameter to the tree and registers this listener to it */
    KpapiAudioProcessorValueTreeStateAttachment (AudioProcessorValueTreeState &tree,
                                                 const String &parameterID,
                                                 const String &parameterName,
                                                 std::function<void(int16_t)> changeCallback)
                                                 : t (tree), paramID (parameterID) {
        t.createAndAddParameter (paramID, parameterName, String(),  range, 0.0f, nullptr, nullptr);
        t.addParameterListener (paramID, this);
        this->changeCallback = changeCallback;
    }

    ~KpapiAudioProcessorValueTreeStateAttachment() {
        t.removeParameterListener (paramID, this);
    }

    /** Is called by the value tree if a parameter was changed by the host or the UI */
    void parameterChanged (const String &parameterID, float newValue) override {
        if (updating.load() == false) {
            updating = true;
            changeCallback (newValue);
            updating = false;
        }
    }

    /** Call this when a parameter change from the amp was received */
    void setParameterValue (int16_t newValue) {

        if (AudioProcessorParameter* p = t.getParameter (paramID)) {

            const float normalisedNewValue = range.convertTo0to1 (newValue);

            if (p->getValue() != normalisedNewValue)
                updating = true;
                p->setValueNotifyingHost (normalisedNewValue);
                updating = false;
        }
    }

private:
    AudioProcessorValueTreeState &t;
    static const NormalisableRange<float> range

    const String paramID;

    std::function<void(int16_t)> changeCallback;
    std::atomic<bool> updating;
};

NormalisableRange<float> KpapiAudioProcessorValueTreeStateAttachment::range (0.0f, 16383.0f);