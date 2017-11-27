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
#include "kpapiConstants.h"

#include <thread>
#include <chrono>

using namespace Kpa;

class KemperProfilingAmp : private SimpleMIDI::PlatformSpecificImplementation {
    
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


    KemperProfilingAmp (SimpleMIDI::HardwareResource &hardwareRessource) : SimpleMIDI::PlatformSpecificImplementation (hardwareRessource),
                                                                           stringResponseManager (*this) {
#ifdef SIMPLE_MIDI_ARDUINO
        timePointLastTap = 0;
#else
        timePointLastTap = std::chrono::system_clock::now();
#endif
    };

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

        long timeNow = millis;
        long timeDiff = timeNow - timePointLastTap;
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
    }
    
    
    // ---------------- Control the effect chain --------------------------------
    
    /** Control the Wah Pedal */
    void setWah (uint8_t) {
        
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


        ResponseMessageManager (KemperProfilingAmp &outerClass) {

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

            receivedResponse = false;
            waitingForResponse = true;

            while (millis() < timeoutTimepoint) {
                _outerClass.receive();
                if (receivedResponse) {
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
                receivedResponse = true;
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
        bool receivedResponse = false;

        T *responseTargetBuffer = nullptr;
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
                    _outerClass.checkConnectionState ();
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

#ifdef SIMPLE_MIDI_MULTITHREADED
    MIDIClockGenerator *midiClockGenerator = nullptr;
#endif

    void checkConnectionState() {
        std::cerr << "Still connected?" << std::endl;
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
