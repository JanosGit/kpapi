//
//  kpapiTypes.h
//  kpapiDev
//
//  Created by Janos Buttgereit on 28.05.17.
//  Copyright © 2017 Janos Buttgereit. All rights reserved.
//

#ifndef kpapiTypes_h
#define kpapiTypes_h

namespace Kpa {

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
            const uint8_t ExtendedStringParamChange = 0x07;
            const uint8_t SingleParamValueReq = 0x41;
            const uint8_t MultiParamValueReq = 0x42;
            const uint8_t StringParamReq = 0x43;
            const uint8_t ExtendedStringParamReq = 0x47;
        }
        
        namespace Request {
            
            const uint32_t ActiveRigNameLength = 11;
            const uint8_t ActiveRigName[ActiveRigNameLength] = {SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0x00, 0x01, SysExEnd};
            
            const uint32_t ActiveAmpNameLength = 11;
            const uint8_t ActiveAmpName[ActiveAmpNameLength] = {SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0x00, 0x10, SysExEnd};

        }
        
    }
    

}

#endif /* kpapiTypes_h */
