#include "kpapi.h"




void ProfilingAmp::setCommunicationErrorCallback (MidiCommErrorCallbackFn midiCommErrorCallbackFn) {
    midiCommunicationError = midiCommErrorCallbackFn;
}

// --------------------------- Tempo -------------------------------

void ProfilingAmp::startExternalMIDIClocking (uint64_t quarterNoteIntervalInMilliseconds) {
    if (midiClockGenerator == nullptr)
        midiClockGenerator = new MIDIClockGenerator (*this);

    midiClockGenerator->setIntervall (quarterNoteIntervalInMilliseconds);
}

void ProfilingAmp::stopExternalMIDIClocking(bool deleteThread) {
    midiClockGenerator->stop();

    if (deleteThread) {
        delete midiClockGenerator;
        midiClockGenerator = nullptr;
    }
}


#ifdef SIMPLE_MIDI_ARDUINO

void ProfilingAmp::setTempo (uint64_t quarterNoteIntervalInMilliseconds) {
    quarterNoteIntervalInMilliseconds /= 24; // scaled to midi beat clock intervals
    quarterNoteIntervalInMilliseconds += millis();
    sendMIDIClockTick();

    while (quarterNoteIntervalInMilliseconds < millis());

    sendMIDIClockTick();
}

int16_t ProfilingAmp::tapDown() {
    sendControlChange (ControlChange::TapTempo, 1);

    unsigned long timeNow = millis();
    unsigned long timeDiff = timeNow - timePointLastTap;
    timePointLastTap = timeNow;

    if (timeDiff < 3000)
        return timeDiff;

    return -1;
}

#else

void ProfilingAmp::setTempo (uint64_t quarterNoteIntervalInMilliseconds) {
    quarterNoteIntervalInMilliseconds *= 1000000; // actually nanoseconds now
    quarterNoteIntervalInMilliseconds /= 24; // scaled to midi beat clock intervals
    auto timepoint = std::chrono::system_clock::now() + std::chrono::nanoseconds (quarterNoteIntervalInMilliseconds);
    sendMIDIClockTick();
    std::this_thread::sleep_until (timepoint);
    sendMIDIClockTick();
}

int16_t ProfilingAmp::tapDown() {
    sendControlChange (ControlChange::TapTempo, 1);

    auto timeNow = std::chrono::system_clock::now();
    auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds> (timeNow - timePointLastTap);
    timePointLastTap = timeNow;

    auto timeDiffMs = timeDiff.count();

    if (timeDiffMs < 3000)
        return timeDiffMs;

    return -1;
}

#endif

void ProfilingAmp::tapUp() {
    sendControlChange (ControlChange::TapTempo, 0);
}

// ------------------- Rigs & Performances ------------------

void ProfilingAmp::selectRig (RigNr rig) {
    sendControlChange (rig, 1);
    needStompListUpdate = true;
}

void ProfilingAmp::preselectPerformance (uint8_t performanceIdx) {
    sendControlChange (ControlChange ::PerformancePreselect, performanceIdx);
}

void ProfilingAmp::selectPerformanceAndRig (uint8_t performanceIdx, RigNr rig) {
    sendControlChange (ControlChange ::PerformancePreselect, performanceIdx);
    sendControlChange (rig, 1);
    needStompListUpdate = true;
}

void ProfilingAmp::selectNextPerformance() {
    sendControlChange (ControlChange::PerformanceUp, 0);
    needStompListUpdate = true;
}

void ProfilingAmp::startScrollingPerformancesUpwards() {
    sendControlChange (ControlChange::PerformanceUp, 1);
}

void ProfilingAmp::selectPreviousPerformance() {
    sendControlChange (ControlChange::PerformanceDown, 0);
    needStompListUpdate = true;
}

void ProfilingAmp::startScrollingPerformancesBackwards() {
    sendControlChange (ControlChange::PerformanceDown, 1);
}

// -------------------------- Stomps ----------------------------

void ProfilingAmp::toggleStompInSlot (StompSlot stompSlot, bool onOff, bool withReverbTail) {
    if (withReverbTail) {
        if ((stompSlot == Dly) || (stompSlot == Rev)) {
            sendControlChange (stompToggleCC[stompSlot] + 2, onOff);
        }
        else {
            // if it's no delay or reverb slot, switch it anyway
            sendControlChange (stompToggleCC[stompSlot], onOff);
        }
    }
    else {
        sendControlChange (stompToggleCC[stompSlot], onOff);
    }
}

bool ProfilingAmp::getStompToggleState (StompSlot stompSlot) {
    return getStompToggleState (stompSlotToNRPNPage (stompSlot));
}

bool ProfilingAmp::getStompToggleState (NRPNPage stompSlotPage) {
    return getSingleParameter (stompSlotPage, NRPNParameter::OnOff);
}

void ProfilingAmp::setWahPedal (uint8_t wahPosition) {
    sendControlChange (ControlChange::WahPedal, wahPosition);
}

void ProfilingAmp::setVolumePedal (uint8_t volumePedalPosition) {
    sendControlChange (ControlChange::VolumePedal, volumePedalPosition);
}

void ProfilingAmp::setPitchPedal (uint8_t pitchPedalPosition) {
    sendControlChange (ControlChange::PitchPedal, pitchPedalPosition);
}

void ProfilingAmp::setMorphPedal (uint8_t morphPedalPosition) {
    sendControlChange (ControlChange::MorphPedal, morphPedalPosition);
}

ProfilingAmp::WahWahStomp* ProfilingAmp::getWahWahStomp (StompSlot stompSlot) {
    return (WahWahStomp*) getSpecificStompInstance (StompType::WahWah, stompSlot);
}

void ProfilingAmp::scanStompSlots() {
    needStompListUpdate = false;

    // scan all 8 stomp slots
    for (int8_t i = 0; i < 8; i++) {
        NRPNPage pageToScan = fxSlotNRPNPageMapping[i];
        int16_t stompType = (StompType)getSingleParameter (pageToScan, NRPNParameter::StompTypeID);


        // if the response was matching the request, allocate a new stomp in that slot
        char *memBlock = stompMemoryBlock + (i * sizeof (StompBase));
        // although all stomps have an empty destructor until now, this is just to avoid errors in future
        stompsInCurrentRig[i]->~StompBase();
        if (stompType == -1) {
            // some error. Create an empty stomp to prevent undefined states
            stompsInCurrentRig[i] = new(memBlock) StompBase (pageToScan, *this);
            needStompListUpdate = true;
        }
        else {
            switch ((StompType)stompType) {
                case StompType::WahWah:
                    stompsInCurrentRig[i] = new(memBlock) WahWahStomp (pageToScan, *this);
                    break;

                default:
                    stompsInCurrentRig[i] = new(memBlock) StompBase (pageToScan, *this);
                    break;
            }
        }
    }
}

void ProfilingAmp::initializeStompsInCurrentRig () {

    for (int8_t i = 0; i < 8; i++) {
        NRPNPage pageToFill = fxSlotNRPNPageMapping[i];
        // some stack allocation
        char *memBlock = stompMemoryBlock + (i * sizeof (StompBase));
        stompsInCurrentRig[i] = new (memBlock) StompBase (pageToFill, *this);
    }
}

ProfilingAmp::StompSlot ProfilingAmp::getSlotOfFirstGenericStompType (StompType stompTypeToSearchFor) {

    if (stompTypeToSearchFor <= StompType::SpecificMask)
        return StompSlot::Unknown;

    if (needStompListUpdate) {
        scanStompSlots();
    }

    // scan all stomps in current rig until that generic type was found
    int8_t i = 0;
    for (auto &s : stompsInCurrentRig) {
        StompType maskedType = (StompType)(s->getStompType () & StompType::GenericMask);
        if (maskedType == stompTypeToSearchFor) {
            return (StompSlot)i;
        }
        i++;
    }

    return StompSlot::Nonexistent;
}

ProfilingAmp::StompSlot ProfilingAmp::getSlotOfFirstSpecificStompType (StompType stompTypeToSearchFor) {

    if (stompTypeToSearchFor > StompType::SpecificMask)
        return StompSlot::Unknown;

    if (needStompListUpdate) {
        scanStompSlots();
    }

    // scan all stomps in current rig until that specific type was found
    int8_t i = 0;
    for (auto &s : stompsInCurrentRig) {
        StompType maskedType = (StompType)(s->getStompType () & StompType::SpecificMask);
        if (maskedType == stompTypeToSearchFor) {
            return (StompSlot)i;
        }
        i++;
    }

    return StompSlot::Nonexistent;
}

ProfilingAmp::returnStringType ProfilingAmp::getActiveRigName () {

    constexpr int8_t activeRigNameControllerLSB = 1;
    return getStringParameter (0, activeRigNameControllerLSB);
}

ProfilingAmp::returnStringType ProfilingAmp::getActiveAmpName () {

    constexpr int8_t activeAmpNameControllerLSB = 16;
    return getStringParameter (0, activeAmpNameControllerLSB);
}

ProfilingAmp::returnStringType ProfilingAmp::getActiveAmpManufacturerName () {

    constexpr int8_t activeAmpManufacturerNameControllerLSB = 21;
    return getStringParameter (0, activeAmpManufacturerNameControllerLSB);
}

ProfilingAmp::returnStringType ProfilingAmp::getActiveAmpModelName () {

    constexpr int8_t activeAmpModelNameControllerLSB = 24;
    return getStringParameter (0, activeAmpModelNameControllerLSB);
}

ProfilingAmp::returnStringType ProfilingAmp::getActiveCabName () {

    constexpr int8_t activeCabNameControllerLSB = 32;
    return getStringParameter (0, activeCabNameControllerLSB);
}

ProfilingAmp::returnStringType ProfilingAmp::getActiveCabManufacturerName () {

    constexpr int8_t activeCabManufacturerNameControllerLSB = 37;
    return getStringParameter (0, activeCabManufacturerNameControllerLSB);
}

ProfilingAmp::returnStringType ProfilingAmp::getActiveCabModelName () {

    constexpr int8_t activeCabModelNameControllerLSB = 42;
    return getStringParameter (0, activeCabModelNameControllerLSB);
}

ProfilingAmp::returnStringType ProfilingAmp::getActivePerformanceName() {

    constexpr uint32_t activePerformanceNameControllerNumber = 0x4000;
    return getExtendedStringParameter (activePerformanceNameControllerNumber);
}

ProfilingAmp::returnStringType ProfilingAmp::getRigName (RigNr rig) {

    const uint32_t rigNameControllerNumber = rig + 0x3FCF;
    return getExtendedStringParameter (rigNameControllerNumber);
}

void ProfilingAmp::defaultCommunicationErrorCallback (MIDICommunicationErrorCode ec) {
#ifndef SIMPLE_MIDI_ARDUINO

    std::cerr << "kpapi MIDI communication error: ";
    switch (ec) {
        case missingActiveSense:
            std::cerr << "Missing active Sense";
            break;

        case noResponseBeforeTimeout:
            std::cerr << "No response before timeout";
            break;

        case responseNotMatchingToRequest:
            std::cerr << "The response received was not matching the last request";
            break;
    }

    std::cerr << std::endl;
#endif
}

int16_t ProfilingAmp::getSingleParameter (int8_t pageOrMSB, int8_t parameterOrLSB) {
    int8_t response[4];

    // construct and send the request
    char singleParamRequest[] = {SysExBegin, KemperSysEx::ManCode0, KemperSysEx::ManCode1,
                                 KemperSysEx::ManCode2, KemperSysEx::PtProfiler,
                                 KemperSysEx::DeviceID, FunctionCode::SingleParamValueReq,
                                 KemperSysEx::Instance, (char)pageOrMSB, (char)parameterOrLSB,
                                 SysExEnd};
    sendSysEx (singleParamRequest, sizeof (singleParamRequest));

    // wait for a response
    auto ec = parameterResponseManager.waitingForResponseOrTimeout (response, 4);

    // check if the response is matching
    if ((response[0] == pageOrMSB) && (response[1] == parameterOrLSB)) {
        // put together lsb and msb
        return (response[2] << 7) | response[3];
    }

    // error handling
    if (ec == ResponseMessageManager<int8_t>::ErrorCode::timeout) {
        return -1;
    }

    // if there was no timeout, something went wrong - parameter not matching!!
    midiCommunicationError (MIDICommunicationErrorCode::responseNotMatchingToRequest);
    return -1;
}

ProfilingAmp::returnStringType ProfilingAmp::getStringParameter (int8_t MSB, int8_t LSB) {
    // Use a temporary char buffer on the stack for all platforms that return a std::string
    KPAPI_TEMP_STRING_BUFFER_IF_NEEDED

    // construct and send the request. It's static, so only the two differing bytes need to be replaced with each call
    static char stringRequest[] = {SysExBegin, KemperSysEx::ManCode0, KemperSysEx::ManCode1,
                            KemperSysEx::ManCode2, KemperSysEx::PtProfiler,
                            KemperSysEx::DeviceID, FunctionCode::StringParamReq,
                            KemperSysEx::Instance, 'M', 'L', SysExEnd};
    stringRequest[8] = (char)MSB;
    stringRequest[9] = (char)LSB;

    sendSysEx (stringRequest, sizeof (stringRequest));
    // wait for a response
    stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

    // just return the buffer. If something went wrong, it will be empty
    return stringBuffer;
}

ProfilingAmp::returnStringType ProfilingAmp::getExtendedStringParameter (uint32_t extendedControllerNumber) {
    // Use a temporary char buffer on the stack for all platforms that return a std::string
    KPAPI_TEMP_STRING_BUFFER_IF_NEEDED

    // construct the request. It's static, so only the five differing bytes need to be replaced with each call
    static char extendedStringRequest[] = {SysExBegin, KemperSysEx::ManCode0, KemperSysEx::ManCode1,
                                           KemperSysEx::ManCode2, KemperSysEx::PtProfiler,
                                           KemperSysEx::DeviceID, FunctionCode::ExtendedStringParamReq,
                                           KemperSysEx::Instance, 'M', '1', '2', '3', 'L', SysExEnd};
    // replace values 8 - 12
    for (int8_t i = 12; i > 7; i--) {
        uint8_t maskedContNumbByte = extendedControllerNumber & 0x0000007F;
        extendedStringRequest[i] = (char)maskedContNumbByte;
        extendedControllerNumber >>= 7;
    }

    sendSysEx (extendedStringRequest, sizeof (extendedStringRequest));

    // wait for a response
    stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

    // just return the buffer. If something went wrong, it will be empty
    return stringBuffer;
}

void ProfilingAmp::setNewNRPNParameter (NRPNPage newPage, NRPNParameter newParameter) {
    sendControlChange (99, newPage);
    sendControlChange (98, newParameter);
    lastNRPNPage = newPage;
    lastNRPNParameter = newParameter;
}

void ProfilingAmp::updateLowResNRPN (NRPNPage page, NRPNParameter parameter, uint8_t value) {
    if ((page != lastNRPNPage) || (parameter != lastNRPNParameter)) {
        setNewNRPNParameter (page, parameter);
    }
    sendControlChange (NRPNValLowResolution, value);
}

void ProfilingAmp::updateHighResNRPN (NRPNPage page, NRPNParameter parameter, uint16_t value) {
    if ((page != lastNRPNPage) || (parameter != lastNRPNParameter)) {
        setNewNRPNParameter (page, parameter);
    }
    sendControlChange (NRPNValMSB, value >> 7);
    sendControlChange (NRPNValLSB, value & 0x7F);
}

ProfilingAmp::StompBase* ProfilingAmp::getGenericStompInstance (StompType genericStompType, StompSlot stompSlot) {
    // check if the list is still up to date and get an update otherwise
    if (needStompListUpdate) {
        scanStompSlots ();
    }

    if (stompSlot == StompSlot::First) {
        stompSlot = getSlotOfFirstGenericStompType (genericStompType);
    }

    // check if the slot Index is inside the valid range
    if ((stompSlot >= StompSlot::A) && (stompSlot <= StompSlot::Rev)) {
        if ((stompsInCurrentRig[stompSlot]->getStompType() & StompType::GenericMask) == genericStompType) {
            return stompsInCurrentRig[stompSlot];
        }
    }

    return nullptr;
}

ProfilingAmp::StompBase* ProfilingAmp::getSpecificStompInstance (StompType specificStompType, StompSlot stompSlot) {
    // check if the list is still up to date and get an update otherwise
    if (needStompListUpdate) {
        scanStompSlots ();
    }

    if (stompSlot == StompSlot::First) {
        stompSlot = getSlotOfFirstSpecificStompType (specificStompType);
    }

    // check if the slot Index is inside the valid range
    if ((stompSlot >= StompSlot::A) && (stompSlot <= StompSlot::Rev)) {
        if ((stompsInCurrentRig[stompSlot]->getStompType() & StompType::SpecificMask) == specificStompType) {
            return stompsInCurrentRig[stompSlot];
        }
    }

    return nullptr;
}

void ProfilingAmp::receivedProgramChange (uint8_t programm) {
    // empty
}

void ProfilingAmp::receivedControlChange (uint8_t control, uint8_t value) {
    // empty
}

void ProfilingAmp::receivedActiveSense () {
    // empty
}

void ProfilingAmp::receivedSysEx (const char *sysExBuffer, const uint16_t length) {
    // is it a kemper amp sending the message?
    if ((sysExBuffer[1] == KemperSysEx::ManCode0) &&
            (sysExBuffer[2] == KemperSysEx::ManCode1) &&
            (sysExBuffer[3] == KemperSysEx::ManCode2)) {

        // sysExBuffer[6] contains the function code
        switch (sysExBuffer[6]) {
            case FunctionCode::StringParam: {
                int stringLength = length - 11;

                stringResponseManager.receivedResponse (sysExBuffer + 10, stringLength);

            }
                break;

            case FunctionCode::ExtendedStringParam: {
                int stringLength = length - 14;

                stringResponseManager.receivedResponse (sysExBuffer + 13, stringLength);

            }
                break;

            case FunctionCode::SingleParamChange: {
                parameterResponseManager.receivedResponse ((int8_t*)sysExBuffer + 8, 4);
            }
        }
    }
};


constexpr uint8_t ProfilingAmp::stompToggleCC[];
constexpr ProfilingAmp::NRPNPage ProfilingAmp::fxSlotNRPNPageMapping[];