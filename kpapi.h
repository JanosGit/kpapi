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
#include "kpapiTypes.h"

using namespace Kpa;

class KemperProfilingAmp {
    
public:
    KemperProfilingAmp (const simpleMIDI::HardwareRessource *hardwareRessource) : midi (hardwareRessource){
        
    };
    
    void selectRig (RigNr rig) {
        midi.sendControlChange (rig, 1);
    }
    
    const char *getActiveRigName (RigNr rig) {
        midi.sendSysEx (SysEx::Request::ActiveRigName, SysEx::Request::ActiveRigNameLength);
        
        return nullptr;
    }
    
    char *getRigName (RigNr rig) {
        return nullptr;
    };
    
    
private:
    
    
    
    // A char array used to store strings requensted from the amp
    static const int midiStringBufferLength = 64;
    char midiStringBuffer[midiStringBufferLength];
    
    class MIDIConnection : public SimpleMIDI {
    public:
        MIDIConnection (const simpleMIDI::HardwareRessource *h) : SimpleMIDI (h){
            
        };
        
        void receivedNote (uint8_t note, uint8_t velocity, bool noteOn) override {
            
        };
        
        void receivedControlChange (uint8_t control, uint8_t value) override {
            
        };
        
        void receivedProgrammChange (uint8_t programm) override {
            
        };
        
        void receivedSysEx (uint8_t *sysExBuffer, uint8_t length) override {
            
        };
        
        
    };
    
    MIDIConnection midi;
    
    
};





#endif /* kpapi_h */
