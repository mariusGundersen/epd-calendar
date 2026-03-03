#include <UMS3.h>

struct Battery
{
    float cellVoltage = -1.0f;
    bool isCharging;
    bool needsCharging()
    {
        return cellVoltage > 0.0f && cellVoltage < 3.5f && !isCharging;
    }
};

Battery getBatteryStatus();