//
//  kpapiSysEx.h
//
//  Created by Janos Buttgereit on 28.05.17.
//  Copyright © 2017 Janos Buttgereit. All rights reserved.
//

#ifndef kpapiSysEx_h
#define kpapiSysEx_h

namespace kpapi {

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

#endif /* kpapiSysEx_h */
