#include "web/http/http.h"
#include "settings/settings.h"
#include "gps/gps.h"

void WebNmeaQuery(char* pQuery)
{
    while (pQuery)
    {
        char* pName;
        char* pValue;
        pQuery = HttpQuerySplit(pQuery, &pName, &pValue);
        
        int value = HttpQueryValueAsInt(pValue);
            
        if (HttpSameStr(pName, "reset"       )) GpsInit();
        if (HttpSameStr(pName, "sensorheight")) SetSensorHeight(value);
        if (HttpSameStr(pName, "nmeamsgtrace")) ChgNmeaMsgTrace();
        if (HttpSameStr(pName, "nmeacmdtrace")) ChgNmeaCmdTrace();
        if (HttpSameStr(pName,     "gpstrace")) ChgGpsTrace();
        if (HttpSameStr(pName,   "gpsverbose")) ChgGpsVerbose();
    }
}
