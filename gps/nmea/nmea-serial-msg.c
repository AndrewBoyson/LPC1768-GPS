#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "nmea.h"
#include "uart1.h"
#include "gps/gps.h"
#include "gps/pps.h"
#include "log/log.h"

#define MSG_UNKNOWN      0
#define MSG_BUILDING     1
#define MSG_BAD_CHECKSUM 2
#define MSG_INVALID_CHAR 3
#define MSG_OVERRUN_TYPE 4
#define MSG_TOO_BIG      5
#define MSG_GGA          6
#define MSG_ZDA          7
#define MSG_RMC          8
#define MSG_PMTK001      9
#define MSG_PMTK010     10

bool NmeaMsgTrace = false;

static int moduleReadiness = 0;

static struct tm nmeaTm;
static int       nmeaMs;

static int heightDevAboveMsl;
static int heightMslAboveWgs;
static int dop;
static int satelliteCount;
static int fixQuality;

static int nmeaLatDeg;
static int nmeaLatMin;
static int nmeaLatDir;
static int nmeaLngDeg;
static int nmeaLngMin;
static int nmeaLngDir;

static int ackCmd    =  0;
static int ackStatus =  0;

static int makeDegrees(char dir, int degs, int mins)
{//002 ,50.0386, W
    int result = mins * 50 / 3;                      //mins are 0 to 600,000 so multiply by 100 and divide by 6
    result += degs * 10000000;                       //degs are 0 to 180 so multiply by ten million to give +/- 1.8billion (32bit int can hold +/- 2.1 billion)
    if (dir == 'S' || dir == 'W') result = -result;  //dir can be +ve (N or E) or -ve (S or W) so negate if needed
    return result;
}

static void buildLatitudeValue(int letter, int c)
{ //5629.1419,N
    switch(letter)
    {
        case 0: nmeaLatDeg  = (c - '0') *     10; break;
        case 1: nmeaLatDeg += (c - '0') *      1; break;
        case 2: nmeaLatMin  = (c - '0') * 100000; break;
        case 3: nmeaLatMin += (c - '0') *  10000; break;
        case 4:                                   break;
        case 5: nmeaLatMin += (c - '0') *   1000; break;
        case 6: nmeaLatMin += (c - '0') *    100; break;
        case 7: nmeaLatMin += (c - '0') *     10; break;
        case 8: nmeaLngMin += (c - '0') *      1; break;        
    }
}
static void buildLongitudeValue(int letter, int c)
{ //00250.0386,W
    switch(letter)
    {
        case 0: nmeaLngDeg  = (c - '0') *    100; break;
        case 1: nmeaLngDeg += (c - '0') *     10; break;
        case 2: nmeaLngDeg += (c - '0') *      1; break;
        case 3: nmeaLngMin  = (c - '0') * 100000; break;
        case 4: nmeaLngMin += (c - '0') *  10000; break;
        case 5:                                   break;
        case 6: nmeaLngMin += (c - '0') *   1000; break;
        case 7: nmeaLngMin += (c - '0') *    100; break;
        case 8: nmeaLngMin += (c - '0') *     10; break;
        case 9: nmeaLngMin += (c - '0') *      1; break;
    }   
}
static void buildHeight(int letter, int c, int* pHeight)
{ //49.1
//Need to be able to average over 24 hours implies division by 86400 or a right shift by 16 bits.
//Take 2000m as the max allows it to be measured to millionths.
    if (letter == 0) *pHeight = 0;
    if (c == '.') return;
    *pHeight *= 10;
    *pHeight += (c - '0') * 100000;
}
static void buildDop(int letter, int c)
{ //49.1
//Need to be able to average over 24 hours implies division by 86400 or a right shift by 16 bits.
//Take 2000m as the max allows it to be measured to millionths.
    if (letter == 0) dop = 0;
    if (c == '.') return;
    dop *= 10;
    dop += (c - '0');
}
static void buildSatelliteCount(int letter, int c)
{ //10
    switch(letter)
    {
        case 0: satelliteCount  = (c - '0') * 10; break;
        case 1: satelliteCount +=  c - '0';       break;
    }
}
static void buildGGA(int field, int letter, int c)
{ // GGA          Global Positioning System Fix Data
  //$GPGGA,182107.000,5629.1419,N,00250.0386,W,2,10,1.10, 65.4,M,49.1,M,0000,0000*45 Mine
  //$GPGGA,123519    ,4807.038 ,N,01131.000 ,E,1,08,0.9 ,545.4,M,46.9,M,    ,    *47 Example
    switch (field)
    {
        case 1:                                              break; //123519     Fix taken at 12:35:19 UTC
        case 2: buildLatitudeValue(letter, c);               break; //4807.038   Latitude 48 deg 07.038' N
        case 3: nmeaLatDir = c;                              break; //N
        case 4: buildLongitudeValue(letter, c);              break; //01131.000  Longitude 11 deg 31.000' E
        case 5: nmeaLngDir = c;                              break; //E
        case 6: fixQuality = c - '0';                        break; //2          Fix quality '0' = no fix; '1' = GPS; '2' = DGPS
        case 7: buildSatelliteCount(letter, c);              break; //08         Number of satellites being tracked
        case 8: buildDop(letter, c);                         break; //0.9        Horizontal dilution of position
        case 9: buildHeight(letter, c, &heightDevAboveMsl);  break; //545.4      Altitude above mean sea level
        case 10:                                             break; //M          Altitude units
        case 11:buildHeight(letter, c, &heightMslAboveWgs);  break; //46.9       Height of geoid (mean sea level) above WGS84 ellipsoid
        case 12:                                             break; //M          Height of geoid units
        case 13:                                             break; //(empty)    Time in seconds since last DGPS update
        case 14:                                             break; //(empty)    DGPS station ID number
    }
}
static void buildZDA(int field, int letter, int c)
{ //$GPZDA,215042.000,08,01,2017,,*5B
    switch (field)
    {
        case 1:
            switch(letter)
            {
                case 0: nmeaTm.tm_hour  = (c - '0') * 10; break;
                case 1: nmeaTm.tm_hour +=  c - '0';       break;
                case 2: nmeaTm.tm_min   = (c - '0') * 10; break;
                case 3: nmeaTm.tm_min  +=  c - '0';       break;
                case 4: nmeaTm.tm_sec   = (c - '0') * 10; break;
                case 5: nmeaTm.tm_sec  +=  c - '0';       break;
                case 6:                                   break; //decimal point
                case 7: nmeaMs  = (c - '0') * 100;        break;
                case 8: nmeaMs += (c - '0') * 10;         break;
                case 9: nmeaMs +=  c - '0';               break;
            }
            break;
        case 2:
            switch(letter)
            {
                case 0: nmeaTm.tm_mday  = (c - '0') * 10; break;
                case 1: nmeaTm.tm_mday +=  c - '0';       break;
            }
            break;
        case 3:
            switch (letter)
            {
                case 0: nmeaTm.tm_mon   = (c - '0') * 10; break;
                case 1: nmeaTm.tm_mon  +=  c - '0' - 1;   break; //month is 0 to 11
            }
            break;
        case 4:
            switch (letter)
            {
                case 0: nmeaTm.tm_year  = (c - '0') * 1000; break;
                case 1: nmeaTm.tm_year += (c - '0') *  100; break;
                case 2: nmeaTm.tm_year += (c - '0') *   10; break;
                case 3: nmeaTm.tm_year +=  c - '0'  - 1900; break; //year is since 1900
            }
            break;
    }
}
static void buildPMTK010(int field, int letter, int c)
{
    switch (field)
    {
        case 1:
            switch(letter)
            {
                case 0: moduleReadiness  = (c - '0') * 100; break;
                case 1: moduleReadiness += (c - '0') *  10; break;
                case 2: moduleReadiness +=  c - '0'       ; break;
            }
            break;
    }
}
static void buildPMTK001(int field, int letter, int c)
{
    switch (field)
    {
        case 1:
            if (letter == 0) ackCmd =               c - '0';
            else             ackCmd = ackCmd * 10 + c - '0';
            break;
        case 2:
            ackStatus = c - '0';
            break;
    }
}
static int buildMsg(int c)
{
    static int  length = 0;
    static int  field = 0;
    static int  letter = 0;
    static char type[10];
    static int  msgType = MSG_UNKNOWN;
    static bool isChecksum = false;
    static int  calculatedChecksum = 0;
    static int  messageChecksum = 0;
    static bool messageChecksumHasValue = false;
    
    //Check if message too long
    if (c != '$')
    {
        length++;
        if (length > 255) return MSG_TOO_BIG;
    }
    
    switch (c)
    {
        case '$':
            length = 0;
            calculatedChecksum = 0;
            messageChecksum = 0;
            messageChecksumHasValue = false;
            isChecksum = false;
            field = 0;
            letter = 0;
            msgType = MSG_UNKNOWN;
            break;
        case '\r':
            break; //ignore cr
        case '\n':
            if (messageChecksumHasValue && calculatedChecksum == messageChecksum)
            {
                return msgType;
            }
            else
            {
                GpsLog("NMEA ignoring message with bad checksum\r\n");
                return MSG_BAD_CHECKSUM;
            }
        case '*':
            isChecksum = true;
            break;
        case ',':
            calculatedChecksum ^= ',';
            if (field == 0)
            {
                if (memcmp(type, "GPZDA",   5) == 0) msgType = MSG_ZDA;
                if (memcmp(type, "GPGGA",   5) == 0) msgType = MSG_GGA;
                if (memcmp(type, "GPRMC",   5) == 0) msgType = MSG_RMC;
                if (memcmp(type, "PMTK001", 7) == 0) msgType = MSG_PMTK001;
                if (memcmp(type, "PMTK010", 7) == 0) msgType = MSG_PMTK010;
            }
            field++;
            letter = 0;
            break;
        default:
            if (c < ' ' || c > 'Z') return MSG_INVALID_CHAR; //Ignore invalid characters  
            if (isChecksum)
            {
                messageChecksum <<= 4;
                int nibble = c < 'A' ? c - '0' : c - 'A' + 10;
                messageChecksum += nibble;
                messageChecksumHasValue = true;
            }
            else
            {
                calculatedChecksum ^= c;
                if (field == 0)
                {
                    if (letter >= sizeof(type)) return MSG_OVERRUN_TYPE; //Don't overrun the type buffer
                    type[letter] = c;
                }
                else
                {
                    switch (msgType)
                    {
                        case MSG_ZDA:     buildZDA    (field, letter, c); break;
                        case MSG_GGA:     buildGGA    (field, letter, c); break;
                        case MSG_PMTK001: buildPMTK001(field, letter, c); break;
                        case MSG_PMTK010: buildPMTK010(field, letter, c); break;
                    }
                }
            }
            letter++;
            break;
    }
    
    return MSG_BUILDING;
}
static bool handleLine(int msgType, int msgStartedMs)
{    
    switch (msgType)
    {
        case MSG_GGA: //Fix data
            NmeaFixQuality        = fixQuality;
            NmeaDop               = dop;
            NmeaSatelliteCount    = satelliteCount;
            NmeaLat               = makeDegrees(nmeaLatDir, nmeaLatDeg, nmeaLatMin);
            NmeaLng               = makeDegrees(nmeaLngDir, nmeaLngDeg, nmeaLngMin);
            NmeaHeightDevAboveMsl = heightDevAboveMsl;
            NmeaHeightMslAboveWgs = heightMslAboveWgs;
            NmeaFixReceived();
            return true;
        case MSG_ZDA: //Time data related to a PPS
        {
            time64 t = mktime(&nmeaTm);
            if (PpsIsStable() && nmeaMs == 0) NmeaTimeBucketAdd(msgStartedMs); //Add time to bins to allow display of distribution
            NmeaTimeReceived(t, nmeaMs);
            return true;
        }
        case MSG_PMTK001: //Ack response following a command
            GpsLog("NMEA ack of cmd %03d -> %d\r\n", ackCmd, ackStatus);
            NmeaCmdAckItem   = ackCmd;
            NmeaCmdAckStatus = ackStatus;
            return true;
        case MSG_PMTK010: //After a module startup "$PMTK010,001*" is sent when module is configured and "$PMTK010,002*" when module is ready to receive instructions.
            GpsLog("NMEA system readiness %d\r\n", moduleReadiness);
            return true;
    }
    return false;
}

bool NmeaMsgMain() //Returns true if has processed a message
{
    static int msgStartedMs = 0; //Rather than comparing the end of the message with the last pulse this allows us to compare the start
    static int msgStatus = MSG_UNKNOWN;
    static bool hadLine = false;
    while(true)
    {
        int c = Uart1GetC();
        if (c == -1) break;
        if (NmeaMsgTrace) LogChar(c);
        if (c == '$' || msgStatus == MSG_BUILDING)
        {
            msgStatus = buildMsg(c);
            if (c == '$') msgStartedMs = PpsMsSinceLastPulse;
        }
        hadLine = handleLine(msgStatus, msgStartedMs); //Statuses other than the end of a valid message are simply ignored
        if (hadLine) msgStatus = MSG_UNKNOWN;          //This allows extraneous characters when autobauding other than a '$' to be ignored
    }
    return hadLine;
}