#include  <stdint.h>
#include   <stdio.h>

#include "web/http/http.h"
#include "gps/gps.h"
#include "gps/nmea/nmea.h"
#include "settings/settings.h"

void WebNmeaAjax()
{
    HttpOk("text/plain; charset=UTF-8", "no-cache", NULL, NULL);

    char nibble;
    
    nibble = 0;
    if (GpsTrace    ) nibble |= 1;
    if (GpsVerbose  ) nibble |= 2;
    if (NmeaMsgTrace) nibble |= 4;
    if (NmeaCmdTrace) nibble |= 8;
    HttpAddNibbleAsHex(nibble                 ); HttpAddChar('\n');
    HttpAddInt32AsHex (NmeaLatAv              ); HttpAddChar('\n');
    HttpAddInt32AsHex (NmeaLngAv              ); HttpAddChar('\n');
    HttpAddInt32AsHex (NmeaHgtAv              ); HttpAddChar('\n');
    HttpAddInt32AsHex (NmeaHeightDevAboveMslAv); HttpAddChar('\n');
    HttpAddInt32AsHex (NmeaHeightMslAboveWgs  ); HttpAddChar('\n');
    HttpAddInt32AsHex (NmeaDop                ); HttpAddChar('\n');
    HttpAddInt32AsHex (NmeaSatelliteCount     ); HttpAddChar('\n');
    HttpAddInt32AsHex (NmeaFixQuality         ); HttpAddChar('\n');
    HttpAddInt32AsHex (GetSensorHeight()      ); HttpAddChar('\n');
    HttpAddChar('\f');
    
    HttpAddInt32AsHex (NmeaTimeBucketGetSize()); HttpAddChar('\n');
    for (int i = 0; i < NmeaTimeBucketGetSize(); i++)
    {
        HttpAddInt32AsHex(NmeaTimeBucketGetItem(i)); HttpAddChar('\n');
    }
}

