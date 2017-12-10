//
//  kpapiConstants.h
//  kpapiDev
//
//  Created by Janos Buttgereit on 28.05.17.
//  Copyright © 2017 Janos Buttgereit. All rights reserved.
//

#ifndef kpapiConstants_h
#define kpapiConstants_h

#import <cstdint>

namespace Kpa {
    
    enum RigNr : uint8_t {
        Rig1 = 50,
        Rig2 = 51,
        Rig3 = 52,
        Rig4 = 53,
        Rig5 = 54
    };

    /**
     * Converts a rig indexed by 0 - 4 to a RigNr
     */
    RigNr toRigNr (uint8_t rig) {
        return (RigNr) (rig + 50);
    }

    enum Stomp : int8_t {
        Unknown = -1,
        Nonexistent = -2,
        A = 17,
        B = 18,
        C = 19,
        D = 20,
        X = 22,
        Mod = 24,
        Dly = 26,
        Reverb = 28
    };

    /**
     * Converts a index number to the corresponding Stomp parameter. The mapping is:
     * 0 : Stomp::A
     * 1 : Stomp::B
     * 2 : Stomp::C
     * 3 : Stomp::D
     * 4 : Stomp::X
     * 5 : Stomp::Mod
     * 6 : Stomp::Dly
     * 7 : Stomp::Reverb
     */
    Stomp toStomp (uint8_t stomp) {
        if (stomp < 3) {
            return (Stomp) (stomp + 17);
        }
        else {
            stomp -= 3;
            stomp *= 2;
            stomp += 20;
            return (Stomp) stomp;
        }
    }

#ifndef SIMPLE_MIDI_ARDUINO
    // seems like some arduino compatible compilers don't like these enums
    enum ControlChange : uint8_t {
        WahPedal = 1,
        PitchPedal = 4,

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
        StompType = 0,
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

    enum StompType : int8_t {
        Empty = 0,
        Wah = 1,
        AnalogOctaver = 4,
        WahRingModulator = 9,
        Muffin = 36,
        VintageChorus = 65,
        PhaserVibe = 82,
        StudioEqualizer = 98,

        // to be continued...
    };

#else
    namespace ControlChange {

            static const uint8_t WahPedal = 1;
            static const uint8_t PitchPedal = 4;

            static const uint8_t TapTempo = 30; // Value 1 = Down, Value 0 = Up
            static const uint8_t Tuner = 31; // Value 1 = Show, 0 = Hide
            static const uint8_t RotarySpeed = 33; // Value 0 = Slow, Value 1 = Fast
            static const uint8_t DelayInfFeedback = 34;
            static const uint8_t DelayHold = 35;

            // Performances and Slots
            static const uint8_t PerformancePreselect = 47; // Value 0 - 124 = Performance to preselect
            static const uint8_t PerformanceUp = 48; // Value 0 = simply up, Value 1 = start scrolling (stop with 0)
            static const uint8_t PerformanceDown = 49; // just as PerformanceUp

            // Effect parameters
            static const uint8_t DelayMix = 68;
            static const uint8_t DelayFeedback = 69;
            static const uint8_t ReverbMix = 70;
            static const uint8_t ReverbTime = 71;
            static const uint8_t AmpGain = 72;
            static const uint8_t GlobalMonitorVolume = 73;
    }
#endif
    
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


            // Single parameter value requests
            const int SingleParameterRequestLength = 11;

        }
        
    }


}

#endif /* kpapiConstants_h */
