#include  <stdint.h>
#include   <stdio.h>

#include "web/http/http.h"
#include "lpc1768/rtc/rtc.h"
#include "clock/clk/clk.h"
#include "clock/clk/clktime.h"
#include "clock/clk/clkgov.h"
#include "clock/clk/clkutc.h"
#include "gps/pps.h"
#include "gps/nmea/nmea.h"

void WebHomeAjax()
{
    HttpOk("text/plain; charset=UTF-8", "no-cache", NULL, NULL);
    
    clktime now = ClkNowTai();
    clktime fraction = now & ((1UL << CLK_TIME_ONE_SECOND_SHIFT) - 1);
    clktime ms = (fraction * 1000) >> CLK_TIME_ONE_SECOND_SHIFT;
    HttpAddInt16AsHex(ms);
    HttpAddChar('\n');
    
    char nibble;
    nibble = 0;
    if (!NmeaSerialMessagesStopped) nibble |= 0x08; //bit 12
    if (!NmeaFixMessagesStopped   ) nibble |= 0x04; //bit 11
    if (!NmeaTimeMessagesStopped  ) nibble |= 0x02; //bit 10
    if (PpsIsStable()             ) nibble |= 0x01; //bit  9
    HttpAddNibbleAsHex(nibble);
    
    nibble = 0;
    if (NmeaTimeIsStable()        ) nibble |= 0x08; //bit  8
    if (RtcIsSet()                ) nibble |= 0x04; //bit  7
    if (ClkTimeIsSet()            ) nibble |= 0x02; //bit  6
    if (ClkGovIsReceivingTime     ) nibble |= 0x01; //bit  5
    HttpAddNibbleAsHex(nibble);
    
    nibble = 0;
    if (ClkGovRateIsSynced        ) nibble |= 0x08; //bit  4
    if (ClkGovTimeIsSynced        ) nibble |= 0x04; //bit  3
    HttpAddNibbleAsHex(nibble);
    HttpAddChar('\n');
    
    HttpAddInt12AsHex(ClkUtcGetNextEpochMonth1970());
    HttpAddChar('\n');
    
    HttpAddInt16AsHex(ClkUtcGetEpochOffset());
    HttpAddChar('\n');
}

