//
//  kpapi.h
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

#include "kpapiSysEx.h"

class ProfilingAmp : private SimpleMIDI::PlatformSpecificImplementation {

    friend class ReverbStomp;
    friend class WahStomp;
    friend class PhaserVibeStomp;
    
public:

    //------------------------------- (Platform dependent) typedefs ----------------------------

    /* On Arduino, strings are returned as char pointers to a buffer inside the class. According to its single threaded
     * architecture, It's assumed, that this will have been completely processed before the next call for a string
     * parameter is sent out. On other platforms with multithreading, std::string support is assumed, so the calls will
     * return a std::string that will contain a copy of the string. For that reasons, there is no global string buffer
     * inside the class but a temporary char buffer will be created before returning a string. The macro
     * KPAPI_TEMP_STRING_BUFFER_IF_NEEDED manages this.
     * */
#ifdef SIMPLE_MIDI_ARDUINO
    typedef const char* returnStringType;
    #define KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
#else
    typedef std::string returnStringType;
    #define KPAPI_TEMP_STRING_BUFFER_IF_NEEDED char stringBuffer[stringBufferLength];
#endif

    //------------------------ Enumerations & static conversion functions --------------------
    /**
     * Used to specify a rig.
     * @see selectRig
     * @see selectPerformanceAndRig
     */
    enum RigNr : uint8_t {
        Rig1 = 50,
        Rig2 = 51,
        Rig3 = 52,
        Rig4 = 53,
        Rig5 = 54
    };

    /**
     * Converts a rig indexed from 0 - 4 to a RigNr. No error checking, ensure that
     * the index passed is inside the valid range.
     */
    static RigNr toRigNr (uint8_t rig) {
        return (RigNr) (rig + 50);
    }

    /**
     * Used to specify a stomp/effect slot.
     * @see toggleStompInSlot
     */
    enum StompSlot : int8_t {
        Unknown = -1,
        Nonexistent = -2,
        First = -3,
        A = 0,
        B = 1,
        C = 2,
        D = 3,
        X = 4,
        Mod = 5,
        Dly = 6,
        Rev = 7
    };

    /**
     * Used to specify stomp/effect types loaded into a slot. There are generic types
     * and specific types.
     * The generic types describe a group of effects, the specific type describes that
     * one specific model. For an example, if there is a "Green Scream", it has the
     * specific type GreenScreamDistortion and the generic type GenericDistortion at the
     * same time.
     */
    enum StompType : uint16_t {
        Empty = 0,

        // Generic types
        GenericMask = 0xFF00,
        GenericDelay = 1 << 8,
        GenericPitchShifterDelay = 2 << 8,
        GenericWah = 3 << 8,
        GenericDistortion = 4 << 8,
        GenericShaper = 5 << 8,
        GenericBooster = 6 << 8,
        GenericEq = 7 << 8,
        GenericDynamics = 8 << 8,
        GenericChorus = 9 << 8,
        GenericPhaser = 10 << 8,
        GenericPitchShifter = 11 << 8,
        GenericEffectsLoop = 12 << 8,

        // Specific types
        SpecificMask = 0x00FF,
        WahWah = 1,
        WahLowPass,
        WahHighPass,
        WahVowel,
        WahPhaser,
        WahFlanger,

        AnalogOctaver = 4,
        WahRingModulator = 9,
        MuffinDistortion = 36,
        GreenScreamDistortion,
        VintageChorus = 65,
        PhaserVibe = 82,
        StudioEqualizer = 98,

        // to be continued...
    };

    /** One of these will be passed to the MIDICommunicationErrorCallback handler function */
    enum MIDICommunicationErrorCode {
        missingActiveSense,
        noResponseBeforeTimeout,
        responseNotMatchingToRequest
    };

    typedef void (*MidiCommErrorCallbackFn)(MIDICommunicationErrorCode);

#ifdef SIMPLE_MIDI_ARDUINO
    /** Arduino only. Creates a ProfilingAmp based on an Arduino HardwareSerial MIDI Connection. */
    ProfilingAmp (HardwareSerial &serial) : SimpleMIDI::PlatformSpecificImplementation (serial),
                                                  stringResponseManager (*this),
                                                  parameterResponseManager (*this) {
        timePointLastTap = 0;
    };

#ifndef SIMPLE_MIDI_NO_SOFT_SERIAL
    /** Arduino only. Creates a ProfilingAmp based on an Arduino SoftwareSerial MIDI Connection. */
    ProfilingAmp (SoftwareSerial &serial) : SimpleMIDI::PlatformSpecificImplementation (serial),
                                                  stringResponseManager (*this),
                                                  parameterResponseManager (*this) {
        timePointLastTap = 0;
    };
#endif //SIMPLE_MIDI_NO_SOFT_SERIAL

    /**
     * On Arduino the MIDI connection needs some initialization in the setup routine. Due to some
     * framework-based restrictions, this initialization cannot take place in the constructor. Call this
     * before using any other function of this class!
     */
    void beginMIDI() {
        begin();
    }

    /* Call this in the loop to react to MIDI Messages comming from the amp.*/
    void receiveMIDI() {
        receive();
    }
#else

    /**
     * All except Arduino. Creates a ProfilingAmp based on any SimpleMIDI::HardwareResource
     * object used for MIDI I/O. Take a look at the SimpleMIDI methods for creating the
     * HardwareResource Object.
     */
    ProfilingAmp (SimpleMIDI::HardwareResource &hardwareRessource) : SimpleMIDI::PlatformSpecificImplementation (hardwareRessource),
                                                                           stringResponseManager (*this),
                                                                           parameterResponseManager (*this) {
        timePointLastTap = std::chrono::system_clock::now();
    };
#endif

#ifdef SIMPLE_MIDI_MULTITHREADED
    /** On multithreaded platforms a midiClockGenerator migth still be running on its own thread */
    ~ProfilingAmp() {
        if (midiClockGenerator != nullptr)
            delete midiClockGenerator;
    };
#endif

    /** Assigns a function that will be called if any midi communication errors occur */
    void setCommunicationErrorCallback (MidiCommErrorCallbackFn midiCommErrorCallbackFn);

    // ---------------- Tempo ---------------------------------------------------
#ifdef SIMPLE_MIDI_MULTITHREADED
    /**
     * Starts a new thread that constantly sends out a midi clock signal to keep the amp in sync.
     * May be much more stable than setTapTempo but will consume more ressources.
     * !! Not available on single threaded environments like Arduino !!
     */
    void startExternalMIDIClocking (uint64_t quarterNoteIntervalInMilliseconds);

    /**
     * Stops the MIDI clock and optionally deletes the thread. If the thread is kept alive the clock will
     * restart faster if startExternalMIDIClocking is called again.
     *
     * @see startExternalMIDIClocking
     */
    void stopExternalMIDIClocking(bool deleteThread = false);

#endif

    /**
     * Sends out two midi clock ticks according to the interval in milliseconds passed. Might not be as stable as
     * the external midi clock generator functions but much more lightweight.
     */
    void setTempo (uint64_t quarterNoteIntervalInMilliseconds);

    /**
     * Sends a Tap Down event. If no tapUp was sent after 3 seconds, the amp will activate the beat scanner.
     * @return The time in milliseconds since the last tap down. If it's more than 3 seconds ago since the last
     *         tap down occured it will return -1
     */
    int16_t tapDown();

    /**
     * Sends a Tap Up event.
     * @see tapDown
     */
    void tapUp();



    // ---------------- Select Rigs and performances ----------------------------
    
    /** Select a rig in the current performance */
    void selectRig (RigNr rig);

    /**
     * Preselect a Performance. You need to send selectRig afterwards to apply the
     * preselection or the amp will leave the preselection mode after a few seconds.
     * @param performanceIdx Should be between 0 and 124
     */
    void preselectPerformance (uint8_t performanceIdx);

    /** Select a perfomance and a rig */
    void selectPerformanceAndRig (uint8_t performanceIdx, RigNr rig);

    /**
     * Selects the next performance. If scroll performance up was sent before, this
     * will stop scrolling and select the current performance.
     */
    void selectNextPerformance();

    /**
     * After sending this command, the amp will start scrolling up through the performances
     * after a short break. When sending selectNextPerformance, the scrolling will
     * stop at that point
     *
     * @see selectNextPerformance
     */
    void startScrollingPerformancesUpwards();

    /**
     * Selects the previous performance. If scroll performance up was sent before, this
     * will stop scrolling and select the current performance.
     */
    void selectPreviousPerformance();

    /**
     * After sending this command, the amp will start scrolling down through the
     * performances after a short break. When sending selectNextPerformance, the scrolling
     * will stop at that point
     *
     * @see selectNextPerformance
     */
    void startScrollingPerformancesBackwards();
    
    // ---------------- Control the effect chain --------------------------------

private:

    enum NRPNPage : int8_t {
        PageUninitialized = -1,
        PageNonexistent = -2,
        Rig = 4,
        Input = 9,
        Amp = 10,
        Eq = 11,
        Cab = 12,
        StompA = 50,
        StompB = 51,
        StompC = 52,
        StompD = 53,
        StompX = 56,
        StompMod = 58,
        StompDly = 60,
        StompReverb = 75,
        SystemGlobal1 = 125,
        SystemGlobal2 = 127
    };

public:

    /**
     * Toggles a stomp/effects slot on or off, no matter which effect is loaded into that slot.
     * For the delay or reverb slot it can be chosen if the reverb tail should be cut or kept.
     */
    void toggleStompInSlot (StompSlot stompSlot, bool onOff, bool withReverbTail = false);

    /**
     * Returns true if the stomp is switched on, false otherwise
     */
    bool getStompToggleState (StompSlot stompSlot);

    /**
     * Returns true if the stomp is switched on, false otherwise
     */
    bool getStompToggleState (NRPNPage stompSlotPage);

    /**
     * Controls the position of the first Wah Pedal in the effects chain. Uses CC for 7-Bit Values,
     * so the range of valid values goes from 0 - 127
     */
    void setWahPedal (uint8_t wahPosition);

    /**
     * Controls the position of the first Volume Pedal in the effects chain. Uses CC for 7-Bit Values,
     * so the range of valid values goes from 0 - 127
     */
    void setVolumePedal (uint8_t volumePedalPosition);

    /**
     * Controls the position of the first Pitch Pedal in the effects chain. Uses CC for 7-Bit Values,
     * so the range of valid values goes from 0 - 127
     */
    void setPitchPedal (uint8_t pitchPedalPosition);

    /**
     * Controls the position of the first Morph Pedal in the effects chain. Uses CC for 7-Bit Values,
     * so the range of valid values goes from 0 - 127
     */
    void setMorphPedal (uint8_t morphPedalPosition);

    // ---------------- Nested classes for the stomps ---------------------------

    /**
     * The base class for all Stomps. Contains some functionality that all stomp types share, like
     * an on/off switch. Note: Stomp instances can only be created by the ProfilingAmp class internally
     * and cannot (and don't need to) be created by the user. Call functions like getWahWahStomp
     * to get a pointer to an instance of a specific effects type if existent.
     */
    class StompBase {
        friend class ProfilingAmp;
    public:
        /**
         * Returns the stomp type. Especially useful for if you have a generic stomp instance pointer and want to
         * know which specific stomp type it is. Note: If you figured out, which specific type a generic stomp
         * instance represents, applying a dynamic_cast to cast the generic stomp type pointer to a specific stomp
         * type pointer will work.
         */
        StompType getStompType();

        /**
         * Should be overriden by derived class to determine if the assumed stomp is still present in that slot.
         * An instance of the base class can never be valid, so this always returns false for StompBase instances;
         * @return
         */
        virtual bool isStillValid() {return false; };

        /** Activates or deactivates a Stomp */
        void toggleOnOff (bool onOff);

    protected:
        StompBase (NRPNPage slotPage, ProfilingAmp &amp) : _slotPage (slotPage), _amp(amp) {};
        StompType stompType = StompType::Empty;
        const NRPNPage _slotPage;
        ProfilingAmp &_amp;
    };

    /** A class describing functions that all Wah Wah types share */
    class GenericWahStomp : public StompBase {
    public:
        /** Checks if there still is any kind of Wah stomp in this slot */
        bool isStillValid() override;

        /** Sets the Wah manual parameter. The value should be in the range 0 - 16383 */
        void setManual (uint16_t manual);

    protected:
        GenericWahStomp (NRPNPage slotPage, ProfilingAmp &amp) : StompBase (slotPage, amp) {
            stompType = (StompType)(stompType | StompType::GenericWah);
        };
    };

    /**
     * Returns a pointer to the generic wah instance in that slot or a nullpointer if none is
     * found in that slot. If no specific slot is passed, a pointer to the first instance of
     * this kind will be returned or a nullpointer if there is no wah in the current rig.
     */
    GenericWahStomp *getGenericWahStomp (StompSlot stompSlot = StompSlot::First);

    /** A class describing the specific functions of the WahWah stomp */
    class WahWahStomp : public GenericWahStomp {
        friend class ProfilingAmp;
    public:
        /** Checks if there still is a WahWah stomp in this slot */
        bool isStillValid() override ;

    private:
        WahWahStomp (NRPNPage slotPage, ProfilingAmp &amp) : GenericWahStomp (slotPage, amp) {
            stompType = (StompType)(stompType|StompType::WahWah);
        };
    };

    /**
     * Returns a pointer to the wah wah instance in that slot or a nullpointer if none is
     * found in that slot. If no specific slot is passed, a pointer to the first instance of
     * this kind will be returned or a nullpointer if there is no wah wah in the current rig.
     */
    WahWahStomp *getWahWahStomp (StompSlot stompSlot = StompSlot::First);

    /**
     * This will update the internal list of stomps which will get cleared after each rig or performance change.
     * It will be called internally as soon as any stomp will be controlled, so you don't need to call this, but
     * you might implement a call to this after each rig change to speed up the access of effect parameters
     * right after a the rig change.
     */
    void scanStompSlots();

    /**
     * Helps searching for stomp types in the effects chain. Searches for a generic Stomp type, e.g.
     * if you pass GenericDistortion it will return the first slot any kind of distortion was found
     * in. If no stomp of this kind is found, it will return StompSlot::Nonexistent, if the type passed
     * was no generic type, it will return StompType::Unknown
     */
    StompSlot getSlotOfFirstGenericStompType (StompType stompTypeToSearchFor);

    /**
     * Helps searching for stomp types in the effects chain. Searches for a specific Stomp type, e.g.
     * if you pass MuffinDistortion it will return the first slot this specific type of stomp was found
     * in. If no stomp of this kind is found, it will return StompSlot::Nonexistent, if the type passed
     * was no specific type, it will return StompType::Unknown
     */
    StompSlot getSlotOfFirstSpecificStompType (StompType stompTypeToSearchFor);

    // ---------------- Getting string parameters for the active rig ------------
    
    /** Returns the name of the currently active rig */
    returnStringType getActiveRigName ();
    
    /** Returns the name of the currently active amp */
    returnStringType getActiveAmpName ();
    
    /** Returns the name of the currently active amp's manufacturer */
    returnStringType getActiveAmpManufacturerName ();
    
    /** Returns the name of the currently active amp model */
    returnStringType getActiveAmpModelName ();
    
    /** Returns the name of the currently active cabinet */
    returnStringType getActiveCabName ();
    
    /** Returns the name of the currently active cabinet's manufacturer */
    returnStringType getActiveCabManufacturerName ();
    
    /** Returns the name of the currently active cab model */
    returnStringType getActiveCabModelName ();
    
    /** Returns the name of the currently active performance */
    returnStringType getActivePerformanceName();
    
    /** Returns the name of a selectable rig in the currently active performance */
    returnStringType getRigName (RigNr rig);
    
private:

    // ======== Managing bidirectional communication=================
    /**
     * A class managing to redirect to content (SysEx-) messages received to the getter function
     * that sent out a request for a parameter.
     * @tparam T Type of data expected, eg. char strings, integer values...
     */
    template<typename T>
    class ResponseMessageManager {
    public:
        enum ErrorCode : uint8_t {
            success = 0,
            timeout = 1,
            stillWaitingForPrevious = 2
        };
#ifdef SIMPLE_MIDI_ARDUINO
        ResponseMessageManager (ProfilingAmp &outerClass) : _outerClass (outerClass) {}

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
        ProfilingAmp &_outerClass;
        bool waitingForResponse = false;
        bool hasReceivedResponse = false;
        T *responseTargetBuffer = nullptr;
        int responseTargetBufferSize = 0;

#else
        ResponseMessageManager (ProfilingAmp &outerClass) : _outerClass (outerClass), waitingForResponse (false) {}

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
        ProfilingAmp &_outerClass;
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

    // Will be called when a response manager doesn't receive the expected response
    MidiCommErrorCallbackFn midiCommunicationError = defaultCommunicationErrorCallback;

    /** Simply prints out some error messages to stderr on non-arduino systems */
    static void defaultCommunicationErrorCallback (MIDICommunicationErrorCode ec);

    static const int stringBufferLength = 64;
#ifdef SIMPLE_MIDI_ARDUINO
    // A char array used to store strings requensted from the amp
    char stringBuffer[stringBufferLength];
#endif

    // ================ Tempo =======================================
#ifdef SIMPLE_MIDI_MULTITHREADED
    MIDIClockGenerator *midiClockGenerator = nullptr;
#endif


#ifdef SIMPLE_MIDI_ARDUINO
    long timePointLastTap;
#else
    std::chrono::time_point<std::chrono::system_clock> timePointLastTap;
#endif

    /**
     * Maps the control change parameters needed to switch a stomp in a specific slot on or
     * off to indexes from 0 - 7
     */
    static constexpr uint8_t stompToggleCC[8] = {17, 18, 19, 20, 22, 24, 26, 28};

    static constexpr uint8_t NRPNValMSB = 6;
    static constexpr uint8_t NRPNValLSB = 38;
    static constexpr uint8_t NRPNValLowResolution = 119;

    /** Maps an index from 0 - 7 to the corresponding NRPN page to control a stomp */
    static constexpr NRPNPage fxSlotNRPNPageMapping[8] = {StompA, StompB, StompC, StompD, StompX, StompMod, StompDly, StompReverb};

    /**
     * Converts a Stomp value to the corresponding NRPNPage. No error checking,
     * you have to ensure that the value passed is valid.
     */
    NRPNPage stompSlotToNRPNPage (StompSlot stomp) {
        if (stomp <= D) {
            return (NRPNPage)(stomp + 50);
        }
        if (stomp <= Dly) {
            return (NRPNPage)((stomp - 3) * 2 + 54);
        }
        return StompReverb;
    }

    enum NRPNParameter : int8_t {
        ParameterUninitialized = -1,
        RigTempo = -2,
        RigVolume = -3,
        RigTempoEnable = -4,
        NoiseGateIntensity = -5,
        InputCleanSense = -6,
        InputDistortionSense = -7,
        AmpOnOff = -8,
        //AmpGain = -9,
                AmpDefinition = -10,
        AmpClarity = -11,
        AmpPowerSagging = -12,
        AmpPick = -13,
        AmpCompressor = -14,
        AmpTubeShape = -15,
        AmpTubeBias = -16,
        AmpDirectMix = -17,
        EqOnOff = -18,
        EqBassGain = -19,
        EqMiddleGain = -20,
        EqTrebleGain = -21,
        EqPresenceGain = -22,
        CabOnOff = -23,
        CabVolume = -24,
        CabHighShift = -25,
        CabLowShift = -26,
        CabCharacter = -27,
        CabPureCabinet = -28,
        StompAType = -29,
        StompBType = -30,
        StompCType = -31,
        StompDType = -32,
        StompXType = -33,
        StompDlyType = -34,
        StompRevType = -35,

        // ======== These numbers can be interpreted as NRPN LSBs directly =======
                StompTypeID = 0,
        OnOff = 3,
        WahManual = 8,
        WahPeak = 9,
        WahRange = 10,
        WahPeakRAnge = 52,
        WahPedalMode = 12,
        WahTouchAttack = 13,
        WahTouchRelease = 14,
        WahTouchBoost = 15,

        DisShaperDrive = 16,
        DisBoosterTone = 17,

        CompGateIntensity = 18,
        CompAttack = 19,
        CompSquash = 33,

        ModRate = 20,
        ModDepth = 21,
        ModFeedback = 22,
        ModCrossover = 23,
        ModHyperChorusAmount = 24,
        ModManual = 25,
        ModPhaserPeakSpread = 26,
        ModPhaserStages = 27,

        RotarySpeedSlowFast = 30,
        RotayDistance = 31,
        RotaryBalance = 32,

        GEQBand1 = 34,
        GEQBand2 = 35,
        GEQBand3 = 36,
        GEQBand4 = 37,
        GEQBand5 = 38,
        GEQBand6 = 39,
        GEQBand7 = 40,
        GEQBand8 = 41,

        PEQLowGain = 42,
        PEQLowFreq = 43,
        PEQHighGain = 44,
        PEQHighFreq = 45,
        PEQPeak1Gain = 46,
        PEQPeak1Freq = 47,
        PEQPeak1Q = 48,
        PEQPeak2Gain = 49,
        PEQPeak2Freq = 50,
        PEQPeak2Q = 51,

        Ducking = 53,

        VoiceMix = 55,

        Detune = 58,
        SmoothChords = 60,
        PureTuning = 61,

        Key = 64,

        FreezeFormants = 65,
        FormantOffset = 66,
        LowCut = 67,
        HighCut = 68,

        DlyMix = 69,
        DlyMixPrePost = 70,
        DlyTime1 = 71,
        DlyTime2 = 72,
        DlyRatio2 = 73,
        DlyRatio3 = 74,
        DlyRatio4 = 75,
        DlyNoteValue1 = 76,
        DlyNoteValue2 = 77,
        DlyNoteValue3 = 78,
        DlyNoteValue4 = 79,
        DlyToTempo = 80,
        DlyVolume1 = 81,
        DlyVolume2 = 82,
        DlyVolume3 = 83,
        DlyVolume4 = 84,
        DlyPan1 = 85,
        DlyPan2 = 86,
        DlyPan3 = 87,
        DlyPan4 = 88,
        DlyVoice1Pitch = 56,
        DlyVoice2Pitch = 57,
        DlyVoice3Pitch = 89,
        DlyVoice4Pitch = 90,
        DlyVoice3Interval = 91,
        DlyVoice4Interval = 92,
        DlyFeedbak = 93,
        DlyInfinityFeedback = 94,
        DlyInfinity = 95,
        DlyFeedback2 = 96,
        DlyFeedbackSyncSwitch = 97,
        DlyLowCut = 98,
        DlyHighCut = 99,
        DlyFilterIntensity = 100,
        DlyModulation = 101,
        DlyChorus = 102,
        DlyFlutterIntensity = 103,
        DlyFlutterShape = 104,
        DlyGrit = 105,
        DlyReverseMix = 106,
        DlySwell = 107,
        DlySmear = 108,
        DlyDucking = 109,

        // ======================================================================
        // to be continued...
    };

    enum ControlChange : uint8_t {
        WahPedal = 1,
        PitchPedal = 4,
        VolumePedal = 7,
        MorphPedal = 11,

        TapTempo = 30, // Value 1 = Down, Value 0 = Up
        Tuner = 31, // Value 1 = Show, 0 = Hide
        RotarySpeed = 33, // Value 0 = Slow, Value 1 = Fast
        DelayInfFeedback = 34,
        DelayHold = 35,

        // Performances and Slots
                PerformancePreselect = 47, // Value 0 - 124 = Performance to preselect
        PerformanceUp = 48, // Value 0 = simply up, Value 1 = start scrolling (stop with 0)
        PerformanceDown = 49, // just as PerformanceUp

        // Effect parameters
                DelayMix = 68,
        DelayFeedback = 69,
        ReverbMix = 70,
        ReverbTime = 71,
        AmpGain = 72,
        GlobalMonitorVolume = 73
    };

    void sendSingleParameterRequest (uint8_t controllerMSB, uint8_t controllerLSB) {
        using namespace kpapi::SysEx;
        char singleParamRequest[] = {SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::SingleParamValueReq, Instance, (char)controllerMSB, (char)controllerLSB, SysExEnd};
        sendSysEx (singleParamRequest, Request::SingleParameterRequestLength);
    }

    // ========== NRPN handling ===============================
    NRPNPage lastNRPNPage = PageUninitialized;
    NRPNParameter lastNRPNParameter = ParameterUninitialized;

    /**
     * Sets a new NRPN page and parameter. Once this is set all following NRPN values sent
     * the profilier will interpret them as a value for this parameter until a new value
     * will be set.
     */
    void setNewNRPNParameter (NRPNPage newPage, NRPNParameter newParameter);

    /**
     * Checks if the page/parameter pair is the current NRPN value, if not sets it and sends the
     * value afterwards.
     */
    void updateLowResNRPN (NRPNPage page, NRPNParameter parameter, uint8_t value);

    /**
     * Checks if the page/parameter pair is the current NRPN value, if not sets it and sends the
     * value afterwards. Send an MSB and LSB
     */
    void updateHighResNRPN (NRPNPage page, NRPNParameter parameter, uint16_t value);

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
    StompBase *getGenericStompInstance (StompType genericStompType, StompSlot stompSlot);

    /**
     * Searches for a specific stomp instance in a particular stomp slot and returns a pointer to
     * this instance. If the stompSlot value passed is StompSlot::First, it scans all slots for an
     * instance of this type and returns a pointer to the first match found. If no instance could be
     * found in the desired slot or in any slot if searching for StompSlot::First, a nullpointer will
     * be returned.
     * This function should only be used if you are searching for a specific stomp type, otherwise
     * getGenericSpecificStompInstance should be used. Generally this will never be used by the user,
     * because the StompBase pointer won't allow any parameter interaction. The corresponding public
     * functions that call this function inside will downcast the pointer to the specific class corresponding
     * to the stomp type so that the user might interact with the stomp.
     */
    StompBase *getSpecificStompInstance (StompType specificStompType, StompSlot stompSlot);
    
// ======== SimpleMIDI meber functions ========================
    void receivedControlChange (uint8_t control, uint8_t value) override;
    
    void receivedProgramChange (uint8_t programm) override;

    void receivedActiveSense() override;
    
    void receivedSysEx (const char *sysExBuffer, const uint16_t length) override;
};























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
    int8_t response[4];
    sendSingleParameterRequest (stompSlotPage, NRPNParameter::OnOff);
    parameterResponseManager.waitingForResponseOrTimeout (response, 4);
    return response[3];
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

ProfilingAmp::StompType ProfilingAmp::StompBase::getStompType() {
    return stompType;
}

void ProfilingAmp::StompBase::toggleOnOff (bool onOff) {
    _amp.updateLowResNRPN (_slotPage, NRPNParameter::OnOff, onOff);
}

bool ProfilingAmp::GenericWahStomp::isStillValid() {
    if (((stompType & StompType::GenericMask) == StompType::GenericWah) && (_slotPage != PageUninitialized)) {
        return true;
    }
    return false;
}

void ProfilingAmp::GenericWahStomp::setManual (uint16_t manual) {
    _amp.updateHighResNRPN (_slotPage, WahManual, manual);
}

ProfilingAmp::GenericWahStomp* ProfilingAmp::getGenericWahStomp (StompSlot stompSlot) {
    return (GenericWahStomp*) getGenericStompInstance (StompType::GenericWah, stompSlot);
}

bool ProfilingAmp::WahWahStomp::isStillValid() {
    if (((stompType & StompType::SpecificMask) == StompType::WahWah) && (_slotPage != PageUninitialized)) {
        return true;
    }
    return false;
}

ProfilingAmp::WahWahStomp* ProfilingAmp::getWahWahStomp (StompSlot stompSlot) {
    return (WahWahStomp*) getSpecificStompInstance (StompType::WahWah, stompSlot);
}

void ProfilingAmp::scanStompSlots() {
    needStompListUpdate = false;
    int8_t response[4];

    // scan all 8 stomp slots
    for (int8_t i = 0; i < 8; i++) {
        NRPNPage pageToScan = fxSlotNRPNPageMapping[i];
        sendSingleParameterRequest (pageToScan, NRPNParameter::StompTypeID);
        parameterResponseManager.waitingForResponseOrTimeout (response, 4);

        // if the response was matching the request, allocate a new stomp in that slot
        char *memBlock = stompMemoryBlock + (i * sizeof (StompBase));
        // todo: load empty stomps at startup
        //delete stompsInCurrentRig[i];
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

ProfilingAmp::StompSlot ProfilingAmp::getSlotOfFirstGenericStompType (StompType stompTypeToSearchFor) {

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

ProfilingAmp::StompSlot ProfilingAmp::getSlotOfFirstSpecificStompType (StompType stompTypeToSearchFor) {

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

ProfilingAmp::returnStringType ProfilingAmp::getActiveRigName () {
    KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
    sendSysEx (kpapi::SysEx::Request::ActiveRigName, kpapi::SysEx::Request::ActiveRigNameLength);
    stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

    return stringBuffer;
}

ProfilingAmp::returnStringType ProfilingAmp::getActiveAmpName () {
    KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
    sendSysEx (kpapi::SysEx::Request::ActiveAmpName, kpapi::SysEx::Request::ActiveAmpNameLength);
    stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

    return stringBuffer;
}

ProfilingAmp::returnStringType ProfilingAmp::getActiveAmpManufacturerName () {
    KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
    sendSysEx (kpapi::SysEx::Request::ActiveAmpManufacturerName, kpapi::SysEx::Request::ActiveAmpManufacturerNameLength);
    stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

    return stringBuffer;
}

ProfilingAmp::returnStringType ProfilingAmp::getActiveAmpModelName () {
    KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
    sendSysEx (kpapi::SysEx::Request::ActiveAmpModelName, kpapi::SysEx::Request::ActiveAmpModelNameLength);
    stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

    return stringBuffer;
}

ProfilingAmp::returnStringType ProfilingAmp::getActiveCabName () {
    KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
    sendSysEx (kpapi::SysEx::Request::ActiveCabName, kpapi::SysEx::Request::ActiveCabNameLength);
    stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

    return stringBuffer;
}

ProfilingAmp::returnStringType ProfilingAmp::getActiveCabManufacturerName () {
    KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
    sendSysEx (kpapi::SysEx::Request::ActiveCabManufacturerName, kpapi::SysEx::Request::ActiveCabManufacturerNameLength);
    stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

    return stringBuffer;
}

ProfilingAmp::returnStringType ProfilingAmp::getActiveCabModelName () {
    KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
    sendSysEx (kpapi::SysEx::Request::ActiveCabModelName, kpapi::SysEx::Request::ActiveCabModelNameLength);
    stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

    return stringBuffer;
}

ProfilingAmp::returnStringType ProfilingAmp::getActivePerformanceName() {
    KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
    sendSysEx (kpapi::SysEx::Request::ActivePerformanceName, kpapi::SysEx::Request::ActivePerformanceNameLength);
    stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

    return stringBuffer;
}

ProfilingAmp::returnStringType ProfilingAmp::getRigName (RigNr rig) {
    using namespace kpapi::SysEx;
    KPAPI_TEMP_STRING_BUFFER_IF_NEEDED
    char rigControllerLSB = (char)rig - 49;

    char rigNameReq[Request::ExtendedStringRequestLength] = {SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::ExtendedStringParamReq, Instance, 0, 0, 1, 0, rigControllerLSB, SysExEnd};

    sendSysEx (rigNameReq, Request::ExtendedStringRequestLength);
    stringResponseManager.waitingForResponseOrTimeout (stringBuffer, stringBufferLength);

    return stringBuffer;
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
    using namespace kpapi;
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


constexpr uint8_t ProfilingAmp::stompToggleCC[];
constexpr ProfilingAmp::NRPNPage ProfilingAmp::fxSlotNRPNPageMapping[];
#endif /* kpapi_h */