#include <stdbool.h>

#include "log/log.h"
#include "gps/gps.h"
#include "gps/nmea/nmea.h"
#include "gps/nmea/uart1.h"

bool NmeaCmdTrace = false;

void NmeaCmdSend(const char* command)
{
    const char* p  = command;
    NmeaCmdAckItem   = -1;
    NmeaCmdAckStatus =  0;
    
    char checksum = 0;
    
    if (NmeaCmdTrace) GpsLog("NMEA sent command $");
    while (Uart1PutC('$'));
    
    if (NmeaCmdTrace) Log(command);
    while (*p)
    {
        if (!Uart1PutC(*p))
        {
            checksum ^= *p;
            p++;
        }
    }
    char csUpper = checksum >> 4;
    char csLower = checksum & 0xF;
    
    if (csUpper >= 10) csUpper += 'A' - 10; else csUpper += '0';
    if (csLower >= 10) csLower += 'A' - 10; else csLower += '0';
    
    if (NmeaCmdTrace) LogChar('*');        while (Uart1PutC('*'));
    if (NmeaCmdTrace) LogChar(csUpper);    while (Uart1PutC(csUpper));
    if (NmeaCmdTrace) LogChar(csLower);    while (Uart1PutC(csLower));
    if (NmeaCmdTrace) LogChar('\r');       while (Uart1PutC('\r'));
    if (NmeaCmdTrace) LogChar('\n');       while (Uart1PutC('\n'));
}