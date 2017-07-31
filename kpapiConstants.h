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

    // when a midi request is sent, this is the time interval in miliseconds the send thread polls the "recievedNewMIDI..." flag
    const int waitForStringResponseInterval = 50;
    
    typedef enum : uint8_t {
        Rig1 = 50,
        Rig2 = 51,
        Rig3 = 52,
        Rig4 = 53,
        Rig5 = 54
    } RigNr;
    
    // convert a rig indexed by 0 - 4 to a RigNr
    inline RigNr toRigNr (uint8_t rig) {
        return (RigNr) (rig + 50);
    }
    
    namespace SysEx {
        
        const uint8_t ManCode0 = 0x00;
        const uint8_t ManCode1 = 0x20;
        const uint8_t ManCode2 = 0x33;
        const uint8_t PtProfiler = 0x02;
        const uint8_t DeviceID = 0x7F;
        const uint8_t Instance = 0x00;
        
        namespace FunctionCode {
            
            const uint8_t SingleParamChange = 0x01;
            const uint8_t MultiParamChange = 0x02;
            const uint8_t StringParam = 0x03;
            const uint8_t Blob = 0x04;
            const uint8_t ExtendedParamChange = 0x06;
            const uint8_t ExtendedStringParam = 0x07;
            const uint8_t SingleParamValueReq = 0x41;
            const uint8_t MultiParamValueReq = 0x42;
            const uint8_t StringParamReq = 0x43;
            const uint8_t ExtendedStringParamReq = 0x47;
        }
        
        namespace Request {
            
            // Simple string requests
            const int StringRequestLength = 11;
            
            const int ActiveRigNameLength = StringRequestLength;
            const uint8_t ActiveRigName[ActiveRigNameLength] = {SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 1, SysExEnd};
            
            const int ActiveAmpNameLength = StringRequestLength;
            const uint8_t ActiveAmpName[StringRequestLength] = {SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 16, SysExEnd};
            
            const int ActiveAmpManufacturerNameLength = StringRequestLength;
            const uint8_t ActiveAmpManufacturerName[StringRequestLength] = {SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 21, SysExEnd};
            
            const int ActiveAmpModelNameLength = StringRequestLength;
            const uint8_t ActiveAmpModelName[StringRequestLength] = {SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 24, SysExEnd};
            
            const int ActiveCabNameLength = StringRequestLength;
            const uint8_t ActiveCabName[StringRequestLength] = {SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 32, SysExEnd};
            
            const int ActiveCabManufacturerNameLength = StringRequestLength;
            const uint8_t ActiveCabManufacturerName[StringRequestLength] = {SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 37, SysExEnd};
            
            const int ActiveCabModelNameLength = StringRequestLength;
            const uint8_t ActiveCabModelName[StringRequestLength] = {SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 42, SysExEnd};
            
            
            
            const int ExtendedStringRequestLength = 14;
            
            const int ActivePerformanceNameLength = ExtendedStringRequestLength;
            uint8_t ActivePerformanceName[ActivePerformanceNameLength] = {SysExBegin, SysEx::ManCode0, SysEx::ManCode1, SysEx::ManCode2, SysEx::PtProfiler, SysEx::DeviceID, SysEx::FunctionCode::ExtendedStringParamReq, SysEx::Instance, 0, 0, 1, 0, 0, SysExEnd};

        }
        
    }
    

}

#endif /* kpapiConstants_h */
