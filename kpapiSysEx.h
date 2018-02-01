//
//  kpapiSysEx.h
//
//  Created by Janos Buttgereit on 28.05.17.
//  Copyright © 2017 Janos Buttgereit. All rights reserved.
//

/*

#ifndef kpapiSysEx_h
#define kpapiSysEx_h

namespace kpapi {

    class SysEx {
    public:
        static constexpr char ManCode0 = 0x00;
        static constexpr char ManCode1 = 0x20;
        static constexpr char ManCode2 = 0x33;
        static constexpr char PtProfiler = 0x02;
        static constexpr char DeviceID = 0x7F;
        static constexpr char Instance = 0x00;

        enum FunctionCode : char {
            SingleParamChange = 0x01,
            MultiParamChange = 0x02,
            StringParam = 0x03,
            Blob = 0x04,
            ExtendedParamChange = 0x06,
            ExtendedStringParam = 0x07,
            SingleParamValueReq = 0x41,
            MultiParamValueReq = 0x42,
            StringParamReq = 0x43,
            ExtendedStringParamReq = 0x47
        };

        class Request {
        public:
            // Simple string requests
            static constexpr int StringRequestLength = 11;

            static constexpr int ActiveRigNameLength = StringRequestLength;
            static constexpr char ActiveRigName[ActiveRigNameLength] = {SimpleMIDI::SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 1, SimpleMIDI::SysExEnd};

            static constexpr int ActiveAmpNameLength = StringRequestLength;
            static constexpr char ActiveAmpName[StringRequestLength] = {SimpleMIDI::SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 16, SimpleMIDI::SysExEnd};

            static constexpr int ActiveAmpManufacturerNameLength = StringRequestLength;
            static constexpr char ActiveAmpManufacturerName[StringRequestLength] = {SimpleMIDI::SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 21, SimpleMIDI::SysExEnd};

            static constexpr int ActiveAmpModelNameLength = StringRequestLength;
            static constexpr char ActiveAmpModelName[StringRequestLength] = {SimpleMIDI::SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 24, SimpleMIDI::SysExEnd};

            static constexpr int ActiveCabNameLength = StringRequestLength;
            static constexpr char ActiveCabName[StringRequestLength] = {SimpleMIDI::SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 32, SimpleMIDI::SysExEnd};

            static constexpr int ActiveCabManufacturerNameLength = StringRequestLength;
            static constexpr char ActiveCabManufacturerName[StringRequestLength] = {SimpleMIDI::SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 37, SimpleMIDI::SysExEnd};

            static constexpr int ActiveCabModelNameLength = StringRequestLength;
            static constexpr char ActiveCabModelName[StringRequestLength] = {SimpleMIDI::SysExBegin, ManCode0, ManCode1, ManCode2, PtProfiler, DeviceID, FunctionCode::StringParamReq, Instance, 0, 42, SimpleMIDI::SysExEnd};



            static constexpr int ExtendedStringRequestLength = 14;

            static constexpr int ActivePerformanceNameLength = ExtendedStringRequestLength;
            static constexpr char ActivePerformanceName[ActivePerformanceNameLength] = {SimpleMIDI::SysExBegin, SysEx::ManCode0, SysEx::ManCode1, SysEx::ManCode2, SysEx::PtProfiler, SysEx::DeviceID, SysEx::FunctionCode::ExtendedStringParamReq, SysEx::Instance, 0, 0, 1, 0, 0, SimpleMIDI::SysExEnd};


            // Single parameter value requests
            static constexpr int SingleParameterRequestLength = 11;
        };
    };
}

#endif /* kpapiSysEx_h */
