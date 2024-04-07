#include "nmea.h"

void NmeaMain()
{
    NmeaSerialMain();
    NmeaTimeMain();
    NmeaFixMain();
}
void NmeaInit()
{
    NmeaFixInit();
    NmeaTimeInit();
    NmeaSerialInit();
}