#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "gps/gps.h"
#include "gps/nmea/nmea.h"
#include "clk/clkutc.h"
#include "clk/clkgov.h"
#include "log/log.h"
#include "net/net.h"
#include "net/link/link.h"
#include "net/udp/dns/dns.h"
#include "net/udp/dns/dnslabel.h"
#include "net/udp/dns/dnsname.h"
#include "net/udp/dns/dnsquery.h"
#include "net/udp/dns/dnsreply.h"
#include "net/udp/dns/dnsserver.h"
#include "net/udp/ntp/ntp.h"
#include "net/udp/ntp/ntpclient.h"
#include "net/udp/dhcp/dhcp.h"
#include "net/ip6/icmp/ndp/ns.h"
#include "net/ip6/icmp/ndp/ra.h"
#include "net/ip6/icmp/ndp/rs.h"
#include "net/resolve/nr.h"
#include "net/ip4/icmp/echo4.h"
#include "net/ip6/icmp/echo6.h"
#include "net/ip6/icmp/dest6.h"
#include "net/resolve/ar4.h"
#include "net/resolve/ar6.h"
#include "net/arp/arp.h"
#include "net/ip4/ip4.h"
#include "net/ip6/ip6.h"
#include "net/udp/udp.h"
#include "net/tcp/tcp.h"
#include "net/udp/tftp/tftp.h"
#include "web/web.h"
#include "lpc1768/rtc/rtc.h"

#define GPREG4 (*((volatile unsigned *) 0x40024054))
#define ALMON  (*((volatile unsigned *) 0x40024078))
#define ALYEAR (*((volatile unsigned *) 0x4002407C))
#define ALDOM  (*((volatile unsigned *) 0x4002406C))

/*
ALSEC   6 Alarm value for Seconds      R/W 0x4002 4060 - 
ALMIN   6 Alarm value for Minutes      R/W 0x4002 4064 - 
ALHOUR  5 Alarm value for Hours        R/W 0x4002 4068
ALDOM   5 Alarm value for Day of Month R/W 0x4002 406C *
ALDOW   3 Alarm value for Day of Week  R/W 0x4002 4070
ALDOY   9 Alarm value for Day of Year  R/W 0x4002 4074 -
ALMON   4 Alarm value for Months       R/W 0x4002 4078 *
ALYEAR 12 Alarm value for Years        R/W 0x4002 407C *
*/

enum { iLogUart, iNetStack, iNetNewLine, iNetVerbose, iLink, iDnsName, iDnsQuery, iDnsReply, iDnsServer,
       iNtp, iDhcp, iNsRecvSol, iNsRecvAdv, iNsSendSol, iNr4, iNr, iNtpClient, iClkGov, iEcho4, iEcho6,
       iDest6, iRa, iRs, iAr4, iAr6, iArp, iIp4, iIp6, iUdp, iTcp, iHttp, iTftp };

int GetSensorHeight()
{
    return ALDOM; //9 bits
}
void SetSensorHeight(int value)
{
    ALDOM = value & 0x001F; //5 bits +32
}

void ChgNmeaMsgTrace() { NmeaMsgTrace = !NmeaMsgTrace; }
void ChgNmeaCmdTrace() { NmeaCmdTrace = !NmeaCmdTrace; }
void ChgGpsTrace()     { GpsTrace     = !GpsTrace;     }
void ChgGpsVerbose()   { GpsVerbose   = !GpsVerbose;   }

static bool getBit(int bit)
{
    return GPREG4 & (1U << bit); 
}
static void setBit(int bit, bool value)
{
    if (value) GPREG4 |=   1U << bit ;
    else       GPREG4 &= ~(1U << bit);
}

void SetTraceNetHost (char* text)
{
    int value = strtol(text, NULL, 16);
    NetTraceHost[1] =  value       & 0xFF; //Little endian so low byte
    NetTraceHost[0] = (value >> 8) & 0xFF; //high byte
    ALMON  = value >> 12;    // 4 bits
    ALYEAR = value & 0x3FFF; //12 bits
}
void ChgLogUart        () {       LogUart         =       !LogUart;         setBit(iLogUart,           LogUart       ); }
void ChgTraceNetStack  () {       NetTraceStack   =       !NetTraceStack;   setBit(iNetStack,          NetTraceStack ); }
void ChgTraceNetNewLine() {       NetTraceNewLine =       !NetTraceNewLine; setBit(iNetNewLine,       NetTraceNewLine); }
void ChgTraceNetVerbose() {       NetTraceVerbose =       !NetTraceVerbose; setBit(iNetVerbose,       NetTraceVerbose); }
void ChgTraceLink      () {      LinkTrace        =      !LinkTrace;        setBit(iLink,            LinkTrace       ); }
void ChgTraceDnsName   () {   DnsNameTrace        =   !DnsNameTrace;        setBit(iDnsName,      DnsNameTrace       ); }
void ChgTraceDnsQuery  () {  DnsQueryTrace        =  !DnsQueryTrace;        setBit(iDnsQuery,    DnsQueryTrace       ); }
void ChgTraceDnsReply  () {  DnsReplyTrace        =  !DnsReplyTrace;        setBit(iDnsReply,    DnsReplyTrace       ); }
void ChgTraceDnsServer () { DnsServerTrace        = !DnsServerTrace;        setBit(iDnsServer,  DnsServerTrace       ); }
void ChgTraceNtp       () {       NtpTrace        =       !NtpTrace;        setBit(iNtp,              NtpTrace       ); }
void ChgTraceDhcp      () {      DhcpTrace        =      !DhcpTrace;        setBit(iDhcp,            DhcpTrace       ); }
void ChgTraceNsRecvSol () {        NsTraceRecvSol =        !NsTraceRecvSol; setBit(iNsRecvSol,         NsTraceRecvSol); }
void ChgTraceNsRecvAdv () {        NsTraceRecvAdv =        !NsTraceRecvAdv; setBit(iNsRecvAdv,         NsTraceRecvAdv); }
void ChgTraceNsSendSol () {        NsTraceSendSol =        !NsTraceSendSol; setBit(iNsSendSol,         NsTraceSendSol); }
void ChgTraceNr4       () {       Nr4Trace        =       !Nr4Trace       ; setBit(iNr4,              Nr4Trace       ); }
void ChgTraceNr        () {        NrTrace        =        !NrTrace       ; setBit(iNr,                NrTrace       ); }
void ChgTraceNtpClient () { NtpClientTrace        = !NtpClientTrace       ; setBit(iNtpClient,  NtpClientTrace       ); }
void ChgTraceSync      () {    ClkGovTrace        =    !ClkGovTrace       ; setBit(iClkGov,        ClkGovTrace       ); }
void ChgTraceEcho4     () {     Echo4Trace        =     !Echo4Trace       ; setBit(iEcho4,          Echo4Trace       ); }
void ChgTraceEcho6     () {     Echo6Trace        =     !Echo6Trace       ; setBit(iEcho6,          Echo6Trace       ); }
void ChgTraceDest6     () {     Dest6Trace        =     !Dest6Trace       ; setBit(iDest6,          Dest6Trace       ); }
void ChgTraceRa        () {        RaTrace        =        !RaTrace       ; setBit(iRa,                RaTrace       ); }
void ChgTraceRs        () {        RsTrace        =        !RsTrace       ; setBit(iRs,                RsTrace       ); }
void ChgTraceAr4       () {       Ar4Trace        =       !Ar4Trace       ; setBit(iAr4,              Ar4Trace       ); }
void ChgTraceAr6       () {       Ar6Trace        =       !Ar6Trace       ; setBit(iAr6,              Ar6Trace       ); }
void ChgTraceArp       () {       ArpTrace        =       !ArpTrace       ; setBit(iArp,              ArpTrace       ); }
void ChgTraceIp4       () {       Ip4Trace        =       !Ip4Trace       ; setBit(iIp4,              Ip4Trace       ); }
void ChgTraceIp6       () {       Ip6Trace        =       !Ip6Trace       ; setBit(iIp6,              Ip6Trace       ); }
void ChgTraceUdp       () {       UdpTrace        =       !UdpTrace       ; setBit(iUdp,              UdpTrace       ); }
void ChgTraceTcp       () {       TcpTrace        =       !TcpTrace       ; setBit(iTcp,              TcpTrace       ); }
void ChgTraceHttp      () {       WebTrace        =       !WebTrace       ; setBit(iHttp,             WebTrace       ); }
void ChgTraceTftp      () {      TftpTrace        =      !TftpTrace       ; setBit(iTftp,            TftpTrace       ); }

void ChgDnsSendRequestsViaIp4()  {             DnsSendRequestsViaIp4 =             !DnsSendRequestsViaIp4; }
void ChgNtpSendRequestsViaIp4()  {  NtpClientQuerySendRequestsViaIp4 =  !NtpClientQuerySendRequestsViaIp4; }
void ChgTftpSendRequestsViaIp4() {            TftpSendRequestsViaIp4 =            !TftpSendRequestsViaIp4; }

//Clock settings
void SetClockPpbDivisor       (int  value) { ClkGovFreqDivisor       = value; }
void SetClockPpbChangeMax     (int  value) { ClkGovFreqChangeMaxPpb  = value; }
void SetClockSyncedLimitPpb   (int  value) { ClkGovFreqSyncedLimPpb  = value; }
void SetClockSyncedHysterisPpb(int  value) { ClkGovFreqSyncedHysPpb  = value; }
void SetClockSlewDivisor      (int  value) { ClkGovSlewDivisor       = value; }
void SetClockSlewMaxMs        (int  value) { ClkGovSlewChangeMaxMs   = value; }
void SetClockSyncedLimitNs    (int  value) { ClkGovSlewSyncedLimNs   = value; }
void SetClockSyncedHysterisNs (int  value) { ClkGovSlewSyncedHysNs   = value; }
void SetClockMaxOffsetSecs    (int  value) { ClkGovSlewOffsetMaxSecs = value; }

//NTP settings

void SetNtpClientServerName      ( char* value) { DnsLabelCopy(NtpClientQueryServerName,   value); }
void SetNtpClientInitialInterval ( int   value) { NtpClientQueryInitialInterval    = value ; }
void SetNtpClientNormalInterval  ( int   value) { NtpClientQueryNormalInterval     = value ; }
void SetNtpClientRetryInterval   ( int   value) { NtpClientQueryRetryInterval      = value ; }
void SetNtpClientOffsetMs        ( int   value) { NtpClientReplyOffsetMs           = value ; }
void SetNtpClientMaxDelayMs      ( int   value) { NtpClientReplyMaxDelayMs         = value ; }

int SettingsInit()
{
    ClkGovFreqDivisor       =     1000;
    ClkGovFreqChangeMaxPpb  =     1000;
    ClkGovFreqSyncedLimPpb  =   100000;
    ClkGovFreqSyncedHysPpb  =     1000;
    ClkGovSlewDivisor       =      100;
    ClkGovSlewChangeMaxMs   =       10;
    ClkGovSlewSyncedLimNs   = 10000000; //10ms
    ClkGovSlewSyncedHysNs   =  1000000; //1ms
    ClkGovSlewOffsetMaxSecs =        3;
    
    NtpClientQueryInitialInterval    =   2 ;
    NtpClientQueryNormalInterval     = 600 ;
    NtpClientQueryRetryInterval      =  60 ;
    NtpClientReplyOffsetMs           =   0 ;
    NtpClientReplyMaxDelayMs         = 100 ;
    
               DnsSendRequestsViaIp4 = false;
    NtpClientQuerySendRequestsViaIp4 = false;
              TftpSendRequestsViaIp4 = true;
              
    if (RtcPowerLost()) 
    {
        GPREG4 = 0;
        ALMON  = 0;
        ALYEAR = 0;
        ALDOM  = 0;
    }
    
    NetTraceHost[1] =  ALYEAR & 0xFF;               //Little endian so low byte
    NetTraceHost[0] = (ALMON << 4) + (ALYEAR >> 8); //high byte
    
          LogUart         = getBit(iLogUart);
          NetTraceStack   = getBit(iNetStack);
          NetTraceNewLine = getBit(iNetNewLine);
          NetTraceVerbose = getBit(iNetVerbose);
         LinkTrace        = getBit(iLink);
      DnsNameTrace        = getBit(iDnsName);
     DnsQueryTrace        = getBit(iDnsQuery);
     DnsReplyTrace        = getBit(iDnsReply);
    DnsServerTrace        = getBit(iDnsServer);
          NtpTrace        = getBit(iNtp);
         DhcpTrace        = getBit(iDhcp);
           NsTraceRecvSol = getBit(iNsRecvSol);
           NsTraceRecvAdv = getBit(iNsRecvAdv);
           NsTraceSendSol = getBit(iNsSendSol);
          Nr4Trace        = getBit(iNr4);
           NrTrace        = getBit(iNr);
    NtpClientTrace        = getBit(iNtpClient);
       ClkGovTrace        = getBit(iClkGov);
        Echo4Trace        = getBit(iEcho4);
        Echo6Trace        = getBit(iEcho6);
        Dest6Trace        = getBit(iDest6);
           RaTrace        = getBit(iRa);
           RsTrace        = getBit(iRs);
          Ar4Trace        = getBit(iAr4);
          Ar6Trace        = getBit(iAr6);
          ArpTrace        = getBit(iArp);
          Ip4Trace        = getBit(iIp4);
          Ip6Trace        = getBit(iIp6);
          UdpTrace        = getBit(iUdp);
          TcpTrace        = getBit(iTcp);
          WebTrace        = getBit(iHttp);
         TftpTrace        = getBit(iTftp);

    return 0;
}
