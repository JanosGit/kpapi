//
//  kpapiConstants.h
//  kpapiDev
//
//  Created by Janos Buttgereit on 28.05.17.
//  Copyright © 2017 Janos Buttgereit. All rights reserved.
//

#ifndef kpapiConstants_h
#define kpapiConstants_h

namespace Kpa {
    
    enum RigNr : uint8_t {
        Rig1 = 50,
        Rig2 = 51,
        Rig3 = 52,
        Rig4 = 53,
        Rig5 = 54
    };
    
    // convert a rig indexed by 0 - 4 to a RigNr
    inline RigNr toRigNr (uint8_t rig) {
        return (RigNr) (rig + 50);
    }

    enum ControlChange : uint8_t {
        WahPedal = 1,
        PitchPedal = 4,

        // Stomps
        ToggleAllStomps = 16,
        ToggleStompA = 17,
        ToggleStompB = 18,
        ToggleStompC = 19,
        ToggleStompD = 20,
        ToggleStompX = 22,
        ToggleStompMod = 24,
        ToggleStompDly = 26,
        ToggleStompDlyWithTail = 27,
        ToggleStompReverb = 28,
        ToggleStompReverbWithTail = 29,

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

    /*
    namespace ControlChange {

        const uint8_t WahPedal = 1; // really??

        const uint8_t PitchPedal = 4;

        // Stomps
        // todo: implement
        const uint8_t toggleAllStomps =

        const uint8_t TapTempo = 50;

    }
     */
    
    namespace SysEx {
        
        const char ManCode0 = 0x00;
        const char ManCode1 = 0x20;
        const char ManCode2 = 0x33;
        const char PtProfiler = 0x02;
        const char DeviceID = 0x7F;
        const char Instance = 0x00;
        
        namespace FunctionCode {
            
            const char SingleParamChange = 0x01;
            const char MultiParamChange = 0x02;
            const char StringParam = 0x03;
            const char Blob = 0x04;
            const char ExtendedParamChange = 0x06;
            const char ExtendedStringParam = 0x07;
            const char SingleParamValueReq = 0x41;
            const char MultiParamValueReq = 0x42;
            const char StringParamReq = 0x43;
            const char ExtendedStringParamReq = 0x47;
        }
        
        namespace Request {
            
            // Simple string requests
            const int StringRequestLength = 11;
            
            const int ActiveRigNameLength = StringRequestLength;
            const char ActiveRigName[ActiveRigNameLength] = {SimpleMIDI::SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 1, SimpleMIDI::SysExEnd};
            
            const int ActiveAmpNameLength = StringRequestLength;
            const char ActiveAmpName[StringRequestLength] = {SimpleMIDI::SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 16, SimpleMIDI::SysExEnd};
            
            const int ActiveAmpManufacturerNameLength = StringRequestLength;
            const char ActiveAmpManufacturerName[StringRequestLength] = {SimpleMIDI::SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 21, SimpleMIDI::SysExEnd};
            
            const int ActiveAmpModelNameLength = StringRequestLength;
            const char ActiveAmpModelName[StringRequestLength] = {SimpleMIDI::SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 24, SimpleMIDI::SysExEnd};
            
            const int ActiveCabNameLength = StringRequestLength;
            const char ActiveCabName[StringRequestLength] = {SimpleMIDI::SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 32, SimpleMIDI::SysExEnd};
            
            const int ActiveCabManufacturerNameLength = StringRequestLength;
            const char ActiveCabManufacturerName[StringRequestLength] = {SimpleMIDI::SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 37, SimpleMIDI::SysExEnd};
            
            const int ActiveCabModelNameLength = StringRequestLength;
            const char ActiveCabModelName[StringRequestLength] = {SimpleMIDI::SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 42, SimpleMIDI::SysExEnd};
            
            
            
            const int ExtendedStringRequestLength = 14;
            
            const int ActivePerformanceNameLength = ExtendedStringRequestLength;
            char ActivePerformanceName[ActivePerformanceNameLength] = {SimpleMIDI::SysExBegin, SysEx::ManCode0, SysEx::ManCode1, SysEx::ManCode2, SysEx::PtProfiler, SysEx::DeviceID, SysEx::FunctionCode::ExtendedStringParamReq, SysEx::Instance, 0, 0, 1, 0, 0, SimpleMIDI::SysExEnd};

        }
        
    }


}

#endif /* kpapiConstants_h */
