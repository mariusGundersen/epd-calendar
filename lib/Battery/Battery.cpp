#include "Battery.h"

UMS3 ums3;

Battery getBatteryStatus()
{
    ums3.begin();

    delay(500);
    bool isCharging = ums3.getVbusPresent();
    float cellVoltage = ums3.getBatteryVoltage();

    log_d("Battery voltage is %f, is charging is %d", cellVoltage, isCharging);

    return Battery{cellVoltage, isCharging};
}