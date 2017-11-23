//
//  main.cpp
//  kpapiDev
//
//  Created by Janos Buttgereit on 28.05.17.
//  Copyright © 2017 Janos Buttgereit. All rights reserved.
//

#include <iostream>

#include "../../kpapi.h"

int main(int argc, const char * argv[]) {
 
    
    std::vector<SimpleMIDI::HardwareResource> connectedDevices = searchMIDIDevices();

    for (auto &dev : connectedDevices) {
        std::cout << dev.deviceName << std::endl;
    }
    
    KemperProfilingAmp kemperProfilingAmp (connectedDevices[2]);
    
    std::cout << kemperProfilingAmp.getRigName (Kpa::Rig3) << std::endl;

    kemperProfilingAmp.setTapTempo (500);
    
    return 0;
}
