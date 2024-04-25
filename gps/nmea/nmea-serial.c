#include "nmea.h"
#include "lpc1768/mstimer/mstimer.h"
#include "lpc1768/uart1.h"
#include "gps/gps.h"

int NmeaCmdAckItem   = -1;
int NmeaCmdAckStatus =  0;

#define BAUD 9600
#define BAUD_STRING(X) STRINGIFY(X) // BAUD_STRING(BAUD) ==> STRINGIFY(19200) - BAUD is expanded to 19200
#define STRINGIFY(X) #X             // STRINGIFY(19200)  ==> "19200" - NB STRINGIFY[BAUD] ==> "BAUD" not "19200" as #X is not expanded!

static int baud = BAUD;
static void nextBaud()
{
    switch (baud)
    {
        case 115200: baud =   4800; break;
        case  57600: baud = 115200; break;
        case  38400: baud =  57600; break;
        case  19200: baud =  38400; break;
        case  14400: baud =  19200; break;
        case   9600: baud =  14400; break;
        case   4800: baud =   9600; break;
        default    : baud =   9600; break;
    }
}
void NmeaSerialInit()
{
    Uart1Init(BAUD);
}

#define WATCHDOG_TIMEOUT_MS 5 * 1000
bool NmeaSerialMessagesStopped = true;
static void checkIfMessagesHaveStopped(bool hadMessage)
{
    static  int msTimerMsgWatchdog = 0;
    static bool messagesHadStopped = true;
    
    if (hadMessage) msTimerMsgWatchdog = MsTimerCount; //Feed the watchdog each time a message is received
    NmeaSerialMessagesStopped = MsTimerRelative(msTimerMsgWatchdog, WATCHDOG_TIMEOUT_MS);
    if ( NmeaSerialMessagesStopped && !messagesHadStopped) GpsLog("NMEA messages have stopped\r\n");
    if (!NmeaSerialMessagesStopped &&  messagesHadStopped) GpsLog("NMEA messages have started\r\n");
    messagesHadStopped = NmeaSerialMessagesStopped;
}

void NmeaSerialMain(void)
{
    //Poll for messages
    bool hadMessage = NmeaMsgMain();
    
    //Work out if there is anything to do
    checkIfMessagesHaveStopped(hadMessage);
    
    static uint32_t msTimerInhibit = 0;
    static bool inhibit = true;
    
    if (MsTimerRelative(msTimerInhibit, 3000)) inhibit = false;
    
    //To distinguish between wrong baud rate and no messages we send test messages
    if (inhibit)
    {
        if (NmeaSerialMessagesStopped)
        {
            static uint32_t msTimerSendTest = 0;
            if (MsTimerRelative(msTimerSendTest, 327))
            {
                msTimerSendTest = MsTimerCount;
                NmeaCmdSend("PMTK000");
            }
        }
    }
    
    if (!inhibit)
    {
        if (NmeaSerialMessagesStopped)
        {
            msTimerInhibit = MsTimerCount;
            inhibit = true;
            nextBaud();
            Uart1SetBaud(baud);
            GpsLog("NMEA auto bauded uart to %d and sending test command 000\r\n", baud);
            NmeaCmdSend("PMTK000");
        }
        else if (baud != BAUD)
        {
            msTimerInhibit = MsTimerCount;
            inhibit = true;
            GpsLog("NMEA sending command 251 to change baud to %d\r\n", BAUD);
            NmeaCmdSend("PMTK251," BAUD_STRING(BAUD));
            Uart1SetBaud(BAUD);
        }
        else if (NmeaTimeMessagesStopped || NmeaFixMessagesStopped)
        {
            msTimerInhibit = MsTimerCount;
            inhibit = true;
            GpsLog("NMEA sending command 314 to include GPZDA time message\r\n");
            NmeaCmdSend("PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0");
        }
    }
}