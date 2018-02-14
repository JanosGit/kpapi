#ifdef KPAPI_JUCE_PARAMETERS

#include "kpapi.h"
#include "KpapiAudioProcessorValueTreeStateAttachment.h"

typedef KpapiAudioProcessorValueTreeStateAttachment vta;

void ProfilingAmp::setupValueTree() {
    attachments.ensureStorageAllocated (TreeParameters::numParameters);

    attachments.set (TreeParameters::ampGain, new vta (tree, "ampGain", "Amp Gain", std::bind (&ProfilingAmp::setAmpGain, this)));
    attachments.set (TreeParameters::ampEQBassGain, new vta (tree, "ampEQBassGain", "Amp EQ Bass Gain", std::bind (&ProfilingAmp::setAmpEQBassGain, this)));
    attachments.set (TreeParameters::ampEQMidGain, new vta (tree, "ampEQMidGain", "Amp EQ Mid Gain", std::bind (&ProfilingAmp::setAmpEQMidGain, this)));
    attachments.set (TreeParameters::ampEQTrebleGain, new vta (tree, "ampEQTrebleGain", "Amp EQ Treble Gain", std::bind (&ProfilingAmp::setAmpEQTrebleGain, this)));
    attachments.set (TreeParameters::ampEQPresenceGain, new vta (tree, "ampEQPresenceGain", "Amp EQ Presence Gain", std::bind (&ProfilingAmp::setAmpEQPresenceGain, this)));
}


#endif