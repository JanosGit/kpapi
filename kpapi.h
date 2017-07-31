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

class KemperProfilingAmp : private SimpleMIDI {
    
public:
    KemperProfilingAmp (const simpleMIDI::HardwareRessource *hardwareRessource) : SimpleMIDI(hardwareRessource) {
        
#ifndef ARDUINO
        receivedNewStringMutex.lock();
#endif
        
    };
    
    /** Select a rig in the current performance */
    void selectRig (RigNr rig) {
        sendControlChange (rig, 1);
    }
    
    // ---------------- Getting string parameters for the active rig ------------
    
    /** Returns the name of the currently active rig */
    const char *getActiveRigName () {
#ifdef ARDUINO
        receivedNewString = false;
#endif
        sendSysEx (SysEx::Request::ActiveRigName, SysEx::Request::ActiveRigNameLength);
        waitForStringResponse();
        return stringBuffer;
    }
    
    /** Returns the name of the currently active amp */
    const char *getActiveAmpName () {
#ifdef ARDUINO
        receivedNewString = false;
#endif
        sendSysEx (SysEx::Request::ActiveAmpName, SysEx::Request::ActiveAmpNameLength);
        waitForStringResponse();
        return stringBuffer;
    }
    
    /** Returns the name of the currently active amp's manufacturer */
    const char *getActiveAmpManufacturerName () {
#ifdef ARDUINO
        receivedNewString = false;
#endif
        sendSysEx (SysEx::Request::ActiveAmpManufacturerName, SysEx::Request::ActiveAmpManufacturerNameLength);
        waitForStringResponse();
        return stringBuffer;
    }
    
    /** Returns the name of the currently active amp model */
    const char *getActiveAmpModelName () {
#ifdef ARDUINO
        receivedNewString = false;
#endif
        sendSysEx (SysEx::Request::ActiveAmpModelName, SysEx::Request::ActiveAmpModelNameLength);
        waitForStringResponse();
        return stringBuffer;
    }
    
    /** Returns the name of the currently active cabinet */
    const char *getActiveCabName () {
#ifdef ARDUINO
        receivedNewString = false;
#endif
        sendSysEx (SysEx::Request::ActiveCabName, SysEx::Request::ActiveCabNameLength);
        waitForStringResponse();
        return stringBuffer;
    }
    
    /** Returns the name of the currently active cabinet's manufacturer */
    const char *getActiveCabManufacturerName () {
#ifdef ARDUINO
        receivedNewString = false;
#endif
        sendSysEx (SysEx::Request::ActiveCabManufacturerName, SysEx::Request::ActiveCabManufacturerNameLength);
        waitForStringResponse();
        return stringBuffer;
    }
    
    /** Returns the name of the currently active cab model */
    const char *getActiveCabModelName () {
#ifdef ARDUINO
        receivedNewString = false;
#endif
        sendSysEx (SysEx::Request::ActiveCabModelName, SysEx::Request::ActiveCabModelNameLength);
        waitForStringResponse();
        return stringBuffer;
    }
    
    /** Returns the name of the currently active performance */
    const char *getActivePerformanceName() {
#ifdef ARDUINO
        receivedNewString = false;
#endif
        sendSysEx (SysEx::Request::ActivePerformanceName, SysEx::Request::ActivePerformanceNameLength);
        waitForStringResponse();
        return stringBuffer;
    }
    
    /** Returns the name of a selectable rig in the currently active performance */
    const char *getRigName (RigNr rig) {
#ifdef ARDUINO
        receivedNewString = false;
#endif
        
        uint8_t rigControllerLSB = (uint8_t)rig - 49;
        
        uint8_t rigNameReq[SysEx::Request::ExtendedStringRequestLength] = {SysExBegin, SysEx::ManCode0, SysEx::ManCode1, SysEx::ManCode2, SysEx::PtProfiler, SysEx::DeviceID, SysEx::FunctionCode::ExtendedStringParamReq, SysEx::Instance, 0, 0, 1, 0, rigControllerLSB, SysExEnd};
        
        sendSysEx (rigNameReq, SysEx::Request::ExtendedStringRequestLength);
        waitForStringResponse();
        return stringBuffer;
    }
    
    
    
    
private:
    
    /** All member functions requesting a string call this to wait until the SysEx, containing the string, was received. This will block until the string buffer can be safely accessed */
    void waitForStringResponse() {
#ifdef ARDUINO
        while (!receivedNewString){
            // just wait until new midi data is there
            receive();
            // this is a very "dirty" approach but should do the job in this case
            //std::this_thread::sleep_for (std::chrono::milliseconds(waitForStringResponseInterval));
            
        }
        // reset the receivedNewString flag
        receivedNewString = false;
#else 
        // this will block the thread until the mutex is unlocked by the midi thread, after the new string is there
        receivedNewStringMutex.lock();
#endif
    }
    
// ======== SimpleMIDI meber functions ========================
    
    void receivedNote (uint8_t note, uint8_t velocity, bool noteOn) override {
        
    };
    
    void receivedControlChange (uint8_t control, uint8_t value) override {
        
    };
    
    void receivedProgrammChange (uint8_t programm) override {
        
    };
    
    void receivedSysEx (const uint8_t *sysExBuffer, const uint16_t length) override {
        // is it a kemper amp sending the message?
        if ((sysExBuffer[1] == SysEx::ManCode0) &&
            (sysExBuffer[2] == SysEx::ManCode1) &&
            (sysExBuffer[3] == SysEx::ManCode2)) {
            
            // sysExBuffer[6] contains the function code
            switch (sysExBuffer[6]) {
                case SysEx::FunctionCode::StringParam: {
                    uint16_t stringLength = length - 11;
                    
                    if (stringLength <= stringBufferLength) {
                        // copy the string content to the string buffer if it fits into it
                        for (uint i = 0; i < stringLength; i++) {
                            stringBuffer[i] = (char)sysExBuffer[i + 10];
                        }
#ifdef ARDUINO
                        receivedNewString = true;
#else
                        // let the other thread, waiting for a response, know that the midi thread has done its job
                        receivedNewStringMutex.unlock();
#endif
                    }
                }
                    break;
                    
                case SysEx::FunctionCode::ExtendedStringParam: {
                    uint8_t stringLength = length - 14;
                    
                    if (stringLength <= stringBufferLength) {
                        // copy the string content to the string buffer if it fits into it
                        for (uint i = 0; i < stringLength; i++) {
                            stringBuffer[i] = (char)sysExBuffer[i + 13];
                        }
#ifdef ARDUINO
                        receivedNewString = true;
#else
                        // let the other thread, waiting for a response, know that the midi thread has done its job
                        receivedNewStringMutex.unlock();
#endif
                    }
                }
                    break;
            }
            
        }
        
    };

#ifdef ARDUINO
    bool receivedNewString = false;
#else
    std::mutex receivedNewStringMutex;
#endif

    // A char array used to store strings requensted from the amp
    static const int stringBufferLength = 64;
    char stringBuffer[stringBufferLength];
    
};





#endif /* kpapi_h */
