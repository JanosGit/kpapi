//
//  kpapi.h
//  
//
//  Created by Janos Buttgereit on 28.05.17.
//
//

#ifndef kpapi_h
#define kpapi_h

#include "simpleMIDI/simpleMIDI.h"

#ifndef SIMPLE_MIDI_ARDUINO
#include <thread>
#include <chrono>
#endif

#include "kpapiConstants.h"

/*
// Forward declaration of stomps
class StompBase;
class GenericWahStomp;
class WahWahStomp;
 */

using namespace Kpa;

class KemperProfilingAmp : private SimpleMIDI::PlatformSpecificImplementation {

    friend class ReverbStomp;
    friend class WahStomp;
    friend class PhaserVibeStomp;
    
public:

    /* On Arduino, strings are returned as char pointers to a buffer inside the class. According to it's single threaded
     * architecture, It's assumed, that this will have been completely processed before the next call for a string
     * parameter is sent out. On other platforms with multithreading, std::string support is assumed, so the calls will
     * return a std::string that will contain a copy of the string
     * */
#ifdef SIMPLE_MIDI_ARDUINO
    typedef const char* returnStringType;
    #define KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
#else
    typedef std::string returnStringType;
    #define KPAPI_TEMP_STRING_BUFFER_IF_NEEDED char stringBuffer[stringBufferLength];
#endif

#ifdef SIMPLE_MIDI_ARDUINO
    KemperProfilingAmp (HardwareSerial &serial) : SimpleMIDI::PlatformSpecificImplementation (serial),
                                                  stringResponseManager (*this),
                                                  parameterResponseManager (*this) {
        timePointLastTap = 0;
    };
   /*
#ifndef SIMPLE_MIDI_NO_SOFT_SERIAL
    KemperProfilingAmp (SoftwareSerial &serial) : SimpleMIDI::PlatformSpecificImplementation (serial),
                                                  stringResponseManager (*this),
                                                  parameterResponseManager (*this) {
        timePointLastTap = 0;
    };
#endif*/
#else

    KemperProfilingAmp (SimpleMIDI::HardwareResource &hardwareRessource) : SimpleMIDI::PlatformSpecificImplementation (hardwareRessource),
                                                                           stringResponseManager (*this),
                                                                           parameterResponseManager (*this) {
        timePointLastTap = std::chrono::system_clock::now();
    };
#endif

#ifdef SIMPLE_MIDI_ARDUINO
    /**
     * On Arduino the MIDI connection needs some initialization in the setup routine
     */
    void beginMIDI() {
        begin();
    }

    void receiveMIDI() {
        receive();
    }
#endif

#ifdef SIMPLE_MIDI_MULTITHREADED
    ~KemperProfilingAmp() {
        if (midiClockGenerator != nullptr)
            delete midiClockGenerator;
    };
#endif

    // ---------------- Tempo ---------------------------------------------------
#ifdef SIMPLE_MIDI_MULTITHREADED
    /**
     * Starts a new thread that constantly sends out a midi clock signal to keep the amp in sync.
     * May be much more stable than setTapTempo but will consume more ressources.
     * !! Not available on single threaded environments like Arduino !!
     */
    void startExternalMIDIClocking (uint64_t quarterNoteIntervalInMilliseconds) {
        if (midiClockGenerator == nullptr)
            midiClockGenerator = new MIDIClockGenerator (*this);

        midiClockGenerator->setIntervall (quarterNoteIntervalInMilliseconds);
    }

    /**
     * Stops the MIDI clock and optionally deletes the thread. If the thread is kept alive the clock will
     * restart faster if startExternalMIDIClocking is called again.
     *
     * @see startExternalMIDIClocking
     */
    void stopExternalMIDIClocking(bool deleteThread = false) {
        midiClockGenerator->stop();

        if (deleteThread) {
            delete midiClockGenerator;
            midiClockGenerator = nullptr;
        }
    }

#endif

#ifdef SIMPLE_MIDI_ARDUINO

    /**
     * Sends out two midi clock ticks according to the interval in milliseconds passed. Might not be as stable as
     * the external midi clock generator functions but much more lightweight.
     */
    void setTempo (uint64_t quarterNoteIntervalInMilliseconds) {
        quarterNoteIntervalInMilliseconds /= 24; // scaled to midi beat clock intervals
        quarterNoteIntervalInMilliseconds += millis();
        sendMIDIClockTick();

        while (quarterNoteIntervalInMilliseconds < millis());

        sendMIDIClockTick();
    }

private:
    long timePointLastTap;

public:

    /**
     * Sends a Tap Down event. If no tapUp was sent after 3 seconds, the amp will activate the beat scanner.
     * @return The time in milliseconds since the last tap down. If it's more than 3 seconds ago since the last
     *         tap down occured it will return -1
     */
    int16_t tapDown() {
        sendControlChange (ControlChange::TapTempo, 1);

        unsigned long timeNow = millis();
        unsigned long timeDiff = timeNow - timePointLastTap;
        timePointLastTap = timeNow;

        if (timeDiff < 3000)
            return timeDiff;

        return -1;
    }

#else

    /**
     * Sends out two midi clock ticks according to the interval in milliseconds passed. Might not be as stable as
     * the external midi clock generator functions but much more lightweight.
     */
    void setTempo (uint64_t quarterNoteIntervalInMilliseconds) {
        quarterNoteIntervalInMilliseconds *= 1000000; // actually nanoseconds now
        quarterNoteIntervalInMilliseconds /= 24; // scaled to midi beat clock intervals
        auto timepoint = std::chrono::system_clock::now() + std::chrono::nanoseconds (quarterNoteIntervalInMilliseconds);
        sendMIDIClockTick();
        std::this_thread::sleep_until (timepoint);
        sendMIDIClockTick();
    }

private:
    std::chrono::time_point<std::chrono::system_clock> timePointLastTap;

public:

    /**
     * Sends a Tap Down event. If no tapUp was sent after 3 seconds, the amp will activate the beat scanner.
     * @return The time in milliseconds since the last tap down. If it's more than 3 seconds ago since the last
     *         tap down occured it will return -1
     */
    int16_t tapDown() {
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

    /**
     * Sends a Tap Up event.
     * @see tapDown
     */
    void tapUp() {
        sendControlChange (ControlChange::TapTempo, 0);
    }
    // ---------------- Select Rigs and performances ----------------------------
    
    /** Select a rig in the current performance */
    void selectRig (RigNr rig) {
        sendControlChange (rig, 1);
        needStompListUpdate = true;
    }

    /**
     * Preselect a Performance. You need to send selectRig afterwards to apply the
     * preselection or the amp will leave the preselection mode after a few seconds.
     * @param performanceIdx Should be between 0 and 124
     */
    void preselectPerformance (uint8_t performanceIdx) {
        sendControlChange (ControlChange ::PerformancePreselect, performanceIdx);
    }

    /** Select a perfomance and a rig */
    void selectPerformanceAndRig (uint8_t performanceIdx, RigNr rig) {
        sendControlChange (ControlChange ::PerformancePreselect, performanceIdx);
        sendControlChange (rig, 1);
        needStompListUpdate = true;
    }

    /**
     * Selects the next performance. If scroll performance up was sent before, this
     * will stop scrolling and select the current performance.
     */
    void selectNextPerformance() {
        sendControlChange (ControlChange::PerformanceUp, 0);
        needStompListUpdate = true;
    }

    /**
     * After sending this command, the amp will start scrolling up through the performances
     * after a short break. When sending selectNextPerformance, the scrolling will
     * stop at that point
     *
     * @see selectNextPerformance
     */
    void startScrollingPerformancesUpwards() {
        sendControlChange (ControlChange::PerformanceUp, 1);
    }

    /**
     * Selects the previous performance. If scroll performance up was sent before, this
     * will stop scrolling and select the current performance.
     */
    void selectPreviousPerformance() {
        sendControlChange (ControlChange::PerformanceDown, 0);
        needStompListUpdate = true;
    }

    /**
     * After sending this command, the amp will start scrolling down through the
     * performances after a short break. When sending selectNextPerformance, the scrolling
     * will stop at that point
     *
     * @see selectNextPerformance
     */
    void startScrollingPerformancesBackwards() {
        sendControlChange (ControlChange::PerformanceDown, 1);
    }
    
    // ---------------- Control the effect chain --------------------------------

    // ---------------- Nested classes for the stomps ---------------------------
    class StompBase {

    public:

        StompBase (NRPNPage slotPage, KemperProfilingAmp &amp) : _slotPage (slotPage), _amp(amp) {};

        StompType getStompType() {
            return stompType;
        }

        /**
         * Should be overriden by derived class to determine if the assumed stomp is still present in that slot.
         * An instance of the base class can never be valid, so this always returns true for StompBase;
         * @return
         */
        virtual bool isStillValid() {return false; };

        void toggleOnOff (bool onOff) {

            if ((_amp.lastNRPNPage != _slotPage) || (_amp.lastNRPNParameter != OnOff)) {
                _amp.setNewNRPNParameter (_slotPage, OnOff);
            }

            _amp.sendControlChange (<#uint8_t control#>, <#uint8_t value#>)
        }

    protected:
        StompType stompType = StompType::Empty;
        const NRPNPage _slotPage;
        KemperProfilingAmp &_amp;
    };


    class GenericWahStomp : public StompBase {

    public:

        GenericWahStomp (NRPNPage slotPage, KemperProfilingAmp &amp) : StompBase (slotPage, amp) {
            stompType |= StompType::GenericWah;
        };
        /**
         * Checks if there still is a generic Wah stomp in this slot
         * @return
         */
        bool isStillValid() override {
            if (((stompType & StompType::GenericMask) == StompType::GenericWah) && (_slotPage != PageUninitialized)) {
                return true;
            }
            return false;
        }

        /**
         * Sets the Wah manual parameter. The value should be in the range 0 - 16383
         */
        void setManual (uint16_t manual) {

            if ((_amp.lastNRPNPage != _slotPage) || (_amp.lastNRPNParameter != WahManual)) {
                _amp.setNewNRPNParameter (_slotPage, WahManual);
            }

            _amp.sendControlChange (NRPNValMSB, manual >> 7);
            _amp.sendControlChange (NRPNValLSB, manual & 0x7F);
        }
    };

    class WahWahStomp : public GenericWahStomp {

    public:
        /**
         * Constructs a WahWah stomp.
         */
        WahWahStomp (NRPNPage slotPage, KemperProfilingAmp &amp) : GenericWahStomp (slotPage, amp) {
            stompType = (stompType|StompType::WahWah);
        };

        bool isStillValid() override {
            if (((stompType & StompType::SpecificMask) == StompType::WahWah) && (_slotPage != PageUninitialized)) {
                return true;
            }
            return false;
        }
    };

    /**
     * Toggles a stomp slot on or off, no matter which effect is loaded into that slot.
     * For the delay or reverb slot it can be chosen if the reverb tail should be cut or kept.
     */
    void toggleStomp (StompSlot stompSlot, bool onOff, bool withReverbTail = false) {
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

    /**
     * Returns true if the stomp is switched on, false otherwise
     */
    bool getStompToggleState (StompSlot stompSlot) {
        return getStompToggleState (stompSlotToNRPNPage (stompSlot));
    }

    /**
     * Returns true if the stomp is switched on, false otherwise
     */
    bool getStompToggleState (NRPNPage stompSlotPage) {
        int8_t response[4];
        sendSingleParameterRequest (stompSlotPage, NRPNParameter::OnOff);
        parameterResponseManager.waitingForResponseOrTimeout (response, 4);
        return response[3];
    }
    
    /**
     * Controls the position of the first Wah Pedal in the effects chain. Uses CC for 7-Bit Values
     */
    void setWahCoarse (uint8_t wahPosition) {
        sendControlChange (ControlChange::WahPedal, wahPosition);
    }
    
    /**
     * Controls the position of the first Volume Pedal in the effects chain. Uses CC for 7-Bit Values
     */
    void setVolumePedal (uint8_t volumePedalPosition) {
        sendControlChange (ControlChange::VolumePedal, volumePedalPosition);
    }
    
    /**
     * Controls the position of the first Pitch Pedal in the effects chain. Uses CC for 7-Bit Values
     */
    void setPitchPedal (uint8_t pitchPedalPosition) {
        sendControlChange (ControlChange::PitchPedal, pitchPedalPosition);
    }
    
    /**
     * Controls the position of the first Morph Pedal in the effects chain. Uses CC for 7-Bit Values
     */
    void setMorphPedal (uint8_t morphPedalPosition) {
        sendControlChange (ControlChange::MorphPedal, morphPedalPosition);
    }

    /**
     * This will update the internal list of stomps which will get cleared after each rig or performance change.
     * It will be called internally as soon as any stomp will be controlled, so you don't need to call this, but
     * you might implement a call to this after each rig change to speed up the access of effect parameters
     * right after a the rig change.
     */
    void scanStompSlots() {
        needStompListUpdate = false;
        int8_t response[4];

        // scan all 8 stomp slots
        for (int8_t i = 0; i < 8; i++) {
            NRPNPage pageToScan = fxSlotNRPNPageMapping[i];
            sendSingleParameterRequest (pageToScan, NRPNParameter::StompTypeID);
            parameterResponseManager.waitingForResponseOrTimeout (response, 4);

            // if the response was matching the request, allocate a new stomp in that slot
            char *memBlock = stompMemoryBlock + (i * sizeof (StompBase));
            delete stompsInCurrentRig[i];
            if (response[0] == pageToScan) {

                switch ((StompType)response[3]) {
                    case StompType::WahWah:
                        stompsInCurrentRig[i] = new(memBlock) WahWahStomp (pageToScan, *this);
                        break;

                    default:
                        stompsInCurrentRig[i] = new(memBlock) StompBase (pageToScan, *this);
                        break;
                }
            }
            else {
                midiCommunicationError (responseNotMatchingToRequest);
                stompsInCurrentRig[i] = new(memBlock) StompBase (pageToScan, *this);
                needStompListUpdate = true;
            }
        }
    }

    /**
     * Helps searching for stomp types in the effects chain. Searches for a generic Stomp type, e.g.
     * if you pass GenericDistortion it will return the first slot any kind of distortion was found
     * in. If no stomp of this kind is found, it will return StompSlot::Nonexistent, if the type passed
     * was no generic type, it will return StompType::Unknown
     */
    StompSlot getSlotOfFirstGenericStompType (StompType stompTypeToSearchFor) {

        if (stompTypeToSearchFor <= StompType::SpecificMask)
            return StompSlot::Unknown;

        if (needStompListUpdate) {
            scanStompSlots();
        }

        for (int8_t i = 0; i < 0; i++) {
            if ((stompsInCurrentRig[i]->getStompType() & StompType::GenericMask) == stompTypeToSearchFor) {
                return (StompSlot)i;
            }
        }

        return StompSlot::Nonexistent;
    }

    /**
     * Helps searching for stomp types in the effects chain. Searches for a specific Stomp type, e.g.
     * if you pass MuffinDistortion it will return the first slot this specific type of stomp was found
     * in. If no stomp of this kind is found, it will return StompSlot::Nonexistent, if the type passed
     * was no specific type, it will return StompType::Unknown
     */
    StompSlot getSlotOfFirstSpecificStompType (StompType stompTypeToSearchFor) {

        if (stompTypeToSearchFor > StompType::SpecificMask)
            return StompSlot::Unknown;

        if (needStompListUpdate) {
            scanStompSlots();
        }

        for (int8_t i = 0; i < 0; i++) {
            if ((stompsInCurrentRig[i]->getStompType() & StompType::SpecificMask) == stompTypeToSearchFor) {
                return (StompSlot)i;
            }
        }

        return StompSlot::Nonexistent;
    }

    GenericWahStomp *getGenericWah (StompSlot stompSlot = StompSlot::First) {
        return (GenericWahStomp*) getGenericStompInstance (StompType::GenericWah, stompSlot);
    }

    // ---------------- Getting string parameters for the active rig ------------
    
    /** Returns the name of the currently active rig */
    returnStringType getActiveRigName () {
        KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
        sendSysEx (SysEx::Request::ActiveRigName, SysEx::Request::ActiveRigNameLength);
        stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

        return stringBuffer;
    }
    
    /** Returns the name of the currently active amp */
    returnStringType getActiveAmpName () {
        KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
        sendSysEx (SysEx::Request::ActiveAmpName, SysEx::Request::ActiveAmpNameLength);
        stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

        return stringBuffer;
    }
    
    /** Returns the name of the currently active amp's manufacturer */
    returnStringType getActiveAmpManufacturerName () {
        KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
        sendSysEx (SysEx::Request::ActiveAmpManufacturerName, SysEx::Request::ActiveAmpManufacturerNameLength);
        stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

        return stringBuffer;
    }
    
    /** Returns the name of the currently active amp model */
    returnStringType getActiveAmpModelName () {
        KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
        sendSysEx (SysEx::Request::ActiveAmpModelName, SysEx::Request::ActiveAmpModelNameLength);
        stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

        return stringBuffer;
    }
    
    /** Returns the name of the currently active cabinet */
    returnStringType getActiveCabName () {
        KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
        sendSysEx (SysEx::Request::ActiveCabName, SysEx::Request::ActiveCabNameLength);
        stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

        return stringBuffer;
    }
    
    /** Returns the name of the currently active cabinet's manufacturer */
    returnStringType getActiveCabManufacturerName () {
        KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
        sendSysEx (SysEx::Request::ActiveCabManufacturerName, SysEx::Request::ActiveCabManufacturerNameLength);
        stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

        return stringBuffer;
    }
    
    /** Returns the name of the currently active cab model */
    returnStringType getActiveCabModelName () {
        KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
        sendSysEx (SysEx::Request::ActiveCabModelName, SysEx::Request::ActiveCabModelNameLength);
        stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

        return stringBuffer;
    }
    
    /** Returns the name of the currently active performance */
    returnStringType getActivePerformanceName() {
        KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
        sendSysEx (SysEx::Request::ActivePerformanceName, SysEx::Request::ActivePerformanceNameLength);
        stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

        return stringBuffer;
    }
    
    /** Returns the name of a selectable rig in the currently active performance */
    returnStringType getRigName (RigNr rig) {
        KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
        char rigControllerLSB = (char)rig - 49;
        
        char rigNameReq[SysEx::Request::ExtendedStringRequestLength] = {SysExBegin, SysEx::ManCode0, SysEx::ManCode1, SysEx::ManCode2, SysEx::PtProfiler, SysEx::DeviceID, SysEx::FunctionCode::ExtendedStringParamReq, SysEx::Instance, 0, 0, 1, 0, rigControllerLSB, SysExEnd};
        
        sendSysEx (rigNameReq, SysEx::Request::ExtendedStringRequestLength);
        stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

        return stringBuffer;
    }
    
private:
// ======== Managing bidirectional communication=================
    template<typename T>
    class ResponseMessageManager {

    public:

        enum ErrorCode : uint8_t {
            success = 0,
            timeout = 1,
            stillWaitingForPrevious = 2
        };
#ifdef SIMPLE_MIDI_ARDUINO


        ResponseMessageManager (KemperProfilingAmp &outerClass) : _outerClass (outerClass){

        }

        /**
         * This is called by the function that wants to receive a response after having sent out some kind of request.
         * It blocks until the response message was received and stored in the buffer provided by the caller.
         * If a timeout appears, all response buffer fields will be filled with zeros - so in case it's a C string char
         * array, this will be interpreted as an empty string while in case of integer or float values, this will be the
         * numerical value 0.
         * @param responseBuffer Pointer to an array that's filled with the response data.
         * @param responseBufferSize Size of the array to fill (number of array elements, NOT size in Bytes!).
         * @param timeoutInMilliseconds Time to wait for a response.
         *
         * @return errorCode::success if successful, errorCode::timeout if a timeout occured
         *         or errorCode::stillWaitingForPrevious if it's still waiting for a previous message.
         */
        ErrorCode waitingForResponseOrTimeout (T *responseTargetBuffer, int responseTargetBufferSize, int timeoutInMilliseconds = 500) {
            // will this ever happen???
            if (waitingForResponse)
                return stillWaitingForPrevious;

            unsigned long timeoutTimepoint = timeoutInMilliseconds + millis();

            this->responseTargetBuffer = responseTargetBuffer;
            this->responseTargetBufferSize = responseTargetBufferSize;

            hasReceivedResponse = false;
            waitingForResponse = true;

            while (millis() < timeoutTimepoint) {
                _outerClass.receive();
                if (hasReceivedResponse) {
                    waitingForResponse = false;
                    return success;
                }
            }

            waitingForResponse = false;
            return timeout;
        }

/**
         * This is called by the corresponding MIDI handler when a speficic kind of message was received. If no
         * request was sent out before, it returns false and does nothing, otherwise it copies the elements from
         * the source buffer to the target buffer provided by the caller of waitingForResponseOrTimeout.
         * @param responseBuffer The buffer provided by the MIDI handler.
         * @param responseBufferSize The number of elemets to copy to the target buffer (number of array elements, NOT size in Bytes!).
         * @return false if no thread was waiting for a response, true if the response could have been delivered.
         */
        bool receivedResponse (const T *responseSourceBuffer, int responseSourceBufferSize) {

            if (waitingForResponse) {

                if (responseSourceBufferSize > responseTargetBufferSize)
                    responseSourceBufferSize = responseTargetBufferSize;
                memcpy (responseTargetBuffer, responseSourceBuffer, responseSourceBufferSize * sizeof (T));
                hasReceivedResponse = true;
                return true;
            }
            return false;
        }

        /**
         * Returns true if waitingForResponseOrTimeout was called before and no response data
         * was processed until now.
         */
        bool isWaitingForResponse() {
            return waitingForResponse;
        }

    private:
        KemperProfilingAmp &_outerClass;
        bool waitingForResponse = false;
        bool hasReceivedResponse = false;

        T *responseTargetBuffer = NULL;
        int responseTargetBufferSize = 0;

#else
        ResponseMessageManager (KemperProfilingAmp &outerClass) : _outerClass (outerClass), waitingForResponse (false) {

        }

        /**
         * This is called by the thread that wants to receive a response after having sent out some kind of request.
         * It blocks until the response message was received and stored in the buffer provided by the caller.
         * If a timeout appears, all response buffer fields will be filled with zeros - so in case it's a C string char
         * array, this will be interpreted as an empty string while in case of integer or float values, this will be the
         * numerical value 0.
         * @param responseBuffer Pointer to an array that's filled with the response data.
         * @param responseBufferSize Size of the array to fill (number of array elements, NOT size in Bytes!).
         * @param timeoutInMilliseconds Time to wait for a response.
         *
         * @return errorCode::success if successful, errorCode::timeout if a timeout occured
         *         or errorCode::stillWaitingForPrevious if it's still waiting for a previous message.
         */
        ErrorCode waitingForResponseOrTimeout (T *responseTargetBuffer, int responseTargetBufferSize, int timeoutInMilliseconds = 500) {

            // check if a previous caller still waits for a response
            if (waitingForResponse.load())
                return stillWaitingForPrevious;

            // set the flag that we are waiting for a response
            waitingForResponse.store (true);

            // aquire the lock as fast as possible to set the target buffer
            std::unique_lock<std::mutex> lk (responseTargetBufferMutex);

            this->responseTargetBuffer = responseTargetBuffer;
            this->responseTargetBufferSize = responseTargetBufferSize;

            // calculate the timepoint at which a timeout will be thrown
            auto timeoutTimepoint = std::chrono::system_clock::now() + std::chrono::milliseconds (timeoutInMilliseconds);

            // wait for the condition variable
            while (!responseTargetBufferFilled) {
                auto status = cv.wait_until (lk, timeoutTimepoint);
                if (status == std::cv_status::timeout) {
                    waitingForResponse.store (false);
                    // clear the buffer completely in this case
                    std::fill (responseTargetBuffer, responseTargetBuffer + responseTargetBufferSize, 0);
                    _outerClass.midiCommunicationError (noResponseBeforeTimeout);
                    return timeout;
                }
            }

            // if we came here, the response buffers should have been filled by the MIDI thread. Just reset the two
            // bool flags and leave...
            waitingForResponse.store (false);
            responseTargetBufferFilled = false;

            return success;
        }

        /**
         * This is called by the corresponding MIDI handler when a speficic kind of message was received. If no
         * request was sent out before, it returns false and does nothing, otherwise it aquires the mutex to the
         * response target buffer and copies the elements from the source buffer.
         * @param responseBuffer The buffer provided by the MIDI handler.
         * @param responseBufferSize The number of elemets to copy to the target buffer (number of array elements, NOT size in Bytes!).
         * @return false if no thread was waiting for a response, true if the response could have been delivered.
         */
        bool receivedResponse (const T *responseSourceBuffer, int responseSourceBufferSize) {
            if (waitingForResponse.load()) {

                {
                    std::lock_guard<std::mutex> lk(responseTargetBufferMutex);
                    size_t numElementsToCpy = std::min (responseSourceBufferSize, responseTargetBufferSize);
                    memcpy (responseTargetBuffer, responseSourceBuffer, numElementsToCpy * sizeof(T));
                    responseTargetBufferFilled = true;
                }

                cv.notify_one();

                return true;
            }

            return false;
        }

        /**
         * Returns true if waitingForResponseOrTimeout was called before and no response data
         * was processed until now.
         */
        bool isWaitingForResponse() {
            return waitingForResponse.load();
        }


    private:
        KemperProfilingAmp &_outerClass;

        std::mutex responseTargetBufferMutex;
        std::condition_variable cv;

        std::atomic<bool> waitingForResponse;
        bool responseTargetBufferFilled = false;
        T *responseTargetBuffer = nullptr;
        int responseTargetBufferSize = 0;
#endif
    };

    ResponseMessageManager<char> stringResponseManager;
    ResponseMessageManager<int8_t> parameterResponseManager;

#ifdef SIMPLE_MIDI_MULTITHREADED
    MIDIClockGenerator *midiClockGenerator = nullptr;
#endif

    void midiCommunicationError (MIDICommunicationErrorCode ec) {
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
        //std::cerr << "Still connected?" << std::endl;
    }

    void sendSingleParameterRequest (uint8_t controllerMSB, uint8_t controllerLSB) {
        char singleParamRequest[] = {SysExBegin, SysEx::ManCode0, SysEx::ManCode1, SysEx::ManCode2, SysEx::PtProfiler, SysEx::DeviceID, SysEx::FunctionCode::SingleParamValueReq, SysEx::Instance, (char)controllerMSB, (char)controllerLSB, SysExEnd};
        sendSysEx (singleParamRequest, SysEx::Request::SingleParameterRequestLength);
    }
    // ========== NRPN handling ===============================
    NRPNPage lastNRPNPage = PageUninitialized;
    NRPNParameter lastNRPNParameter = ParameterUninitialized;


    // ========== Stomp handling ===============================
    // just in case there will be other kemper amps in future with a differnt stomp slot count, make this one variable
    static const uint8_t numStomps = 8;
    // the memory block for the placement-new allocation of stomps
    char stompMemoryBlock[numStomps * sizeof (WahWahStomp)];
    StompBase *stompsInCurrentRig[numStomps];
    bool needStompListUpdate = true;

    /**
     * Searches for a generic stomp instance in a particular stomp slot and returns a pointer to
     * this instance. If the stompSlot value passed is StompSlot::First, it scans all slots for an
     * instance of this type and returns a pointer to the first match found. If no instance could be
     * found in the desired slot or in any slot if searching for StompSlot::First, a nullpointer will
     * be returned.
     * This function should only be used if you are searching for a generic stomp type, otherwise
     * get specificSpecificStompInstance should be used. Generally this will never be used by the user,
     * because the StompBase pointer won't allow any parameter interaction. The corresponding public
     * functions that call this function inside will downcast the pointer to the specific class corresponding
     * to the stomp type so that the user might interact with the stomp.
     */
    StompBase *getGenericStompInstance (StompType genericStompType, StompSlot stompSlot) {
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

    void setNewNRPNParameter (NRPNPage newPage, NRPNParameter newParameter) {
        sendControlChange (99, newPage);
        sendControlChange (98, newParameter);
        lastNRPNPage = newPage;
        lastNRPNParameter = newParameter;
    }

    
// ======== SimpleMIDI meber functions ========================
    
    void receivedNote (uint8_t note, uint8_t velocity, bool noteOn) override {
        
    };
    
    void receivedControlChange (uint8_t control, uint8_t value) override {
        
    };
    
    void receivedProgramChange (uint8_t programm) override {
        
    };
    
    void receivedSysEx (const char *sysExBuffer, const uint16_t length) override {
        // is it a kemper amp sending the message?
        if ((sysExBuffer[1] == SysEx::ManCode0) &&
            (sysExBuffer[2] == SysEx::ManCode1) &&
            (sysExBuffer[3] == SysEx::ManCode2)) {
            
            // sysExBuffer[6] contains the function code
            switch (sysExBuffer[6]) {
                case SysEx::FunctionCode::StringParam: {
                    int stringLength = length - 11;

                    stringResponseManager.receivedResponse (sysExBuffer + 10, stringLength);

                }
                    break;
                    
                case SysEx::FunctionCode::ExtendedStringParam: {
                    int stringLength = length - 14;

                    stringResponseManager.receivedResponse (sysExBuffer + 13, stringLength);

                }
                    break;

                case SysEx::FunctionCode::SingleParamChange: {
                    parameterResponseManager.receivedResponse ((int8_t*)sysExBuffer + 8, 4);
                }
            }
            
        }
        
    };


    // A char array used to store strings requensted from the amp
    static const int stringBufferLength = 64;
#ifdef SIMPLE_MIDI_ARDUINO
    char stringBuffer[stringBufferLength];
#endif
    
};



#endif /* kpapi_h */
