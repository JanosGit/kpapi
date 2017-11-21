//
//  main.cpp
//  kpapiDev
//
//  Created by Janos Buttgereit on 28.05.17.
//  Copyright © 2017 Janos Buttgereit. All rights reserved.
//

#include <iostream>

#include "../../kpapi.h"

void controllerNr2ints (uint32_t controllerNr){
    uint8_t controllerNrEncoded[5];
    controllerNrEncoded[0]=controllerNr>>28;
    controllerNrEncoded[1]=(controllerNr>>21) & 0b01111111;
    controllerNrEncoded[2]=(controllerNr>>14) & 0b01111111;
    controllerNrEncoded[3]=(controllerNr>>7) & 0b01111111;
    controllerNrEncoded[4]=controllerNr & 0b01111111;
    
    std::cout << (int)controllerNrEncoded[0] << ", " << (int)controllerNrEncoded[1] << ", " << (int)controllerNrEncoded[2] << ", " << (int)controllerNrEncoded[3] << ", " << (int)controllerNrEncoded[4] << ", " << std::endl;
}

int main(int argc, const char * argv[]) {
 
    
    std::vector<SimpleMIDI::HardwareResource> connectedDevices = searchMIDIDevices();

    for (auto &dev : connectedDevices) {
        std::cout << dev.deviceName << std::endl;
    }
    
    KemperProfilingAmp kemperProfilingAmp (connectedDevices[1]);
    
    std::cout << kemperProfilingAmp.getActivePerformanceName() << std::endl;
    
    return 0;
}
