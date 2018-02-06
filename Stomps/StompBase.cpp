#include "../kpapi.h"

ProfilingAmp::StompType ProfilingAmp::StompBase::getStompType() {
    return stompType;
}

void ProfilingAmp::StompBase::setToggleState (bool onOff) {
    _amp.updateLowResNRPN (_slotPage, NRPNParameter::OnOff, onOff);
}

bool ProfilingAmp::StompBase::getToggleState() {
    return _amp.getSingleParameter (_slotPage, NRPNParameter::OnOff);
}



