
extern int GetSensorHeight(void); extern void SetSensorHeight(int value);

extern void ChgNmeaMsgTrace(void);
extern void ChgNmeaCmdTrace(void);
extern void ChgGpsTrace    (void);
extern void ChgGpsVerbose  (void);

extern int SettingsInit(void);

extern void SetTraceNetHost   (char* pValue);
extern void ChgLogUart        (void);
extern void ChgTraceNetStack  (void);
extern void ChgTraceNetNewLine(void);
extern void ChgTraceNetVerbose(void);
extern void ChgTraceLink      (void);
extern void ChgTraceDnsName   (void);
extern void ChgTraceDnsQuery  (void);
extern void ChgTraceDnsReply  (void);
extern void ChgTraceDnsServer (void);
extern void ChgTraceNtp       (void);
extern void ChgTraceDhcp      (void);
extern void ChgTraceNsRecvSol (void);
extern void ChgTraceNsRecvAdv (void);
extern void ChgTraceNsSendSol (void);
extern void ChgTraceNr4       (void);
extern void ChgTraceNr        (void);
extern void ChgTraceNtpClient (void);
extern void ChgTraceSync      (void);
extern void ChgTraceEcho4     (void);
extern void ChgTraceEcho6     (void);
extern void ChgTraceDest6     (void);
extern void ChgTraceRa        (void);
extern void ChgTraceRs        (void);
extern void ChgTraceAr4       (void);
extern void ChgTraceAr6       (void);
extern void ChgTraceArp       (void);
extern void ChgTraceIp4       (void);
extern void ChgTraceIp6       (void);
extern void ChgTraceUdp       (void);
extern void ChgTraceTcp       (void);
extern void ChgTraceHttp      (void);
extern void ChgTraceTftp      (void);
extern void ChgTraceOneWire   (void); //Declared to stop the compiler complaining - in the future make this a function pointer

extern void ChgDnsSendRequestsViaIp4(void);
extern void ChgNtpSendRequestsViaIp4(void);
extern void ChgTftpSendRequestsViaIp4(void);

extern void SetClockSlewDivisor      (int  value);
extern void SetClockSlewMaxMs        (int  value);
extern void SetClockPpbDivisor       (int  value);
extern void SetClockPpbChangeMax     (int  value);
extern void SetClockSyncedLimitNs    (int  value);
extern void SetClockSyncedLimitPpb   (int  value);
extern void SetClockSyncedHysterisNs (int  value);
extern void SetClockSyncedHysterisPpb(int  value);
extern void SetClockMaxOffsetSecs    (int  value);

extern void SetNtpClientServerName      ( char* value);
extern void SetNtpClientInitialInterval ( int   value);
extern void SetNtpClientNormalInterval  ( int   value);
extern void SetNtpClientRetryInterval   ( int   value);
extern void SetNtpClientOffsetMs        ( int   value);
extern void SetNtpClientMaxDelayMs      ( int   value);
