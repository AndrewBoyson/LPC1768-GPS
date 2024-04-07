
#include "web/web.h"
#include "web/http/http.h"
#include "web-pages-this.h"

#define DO_HOME_HTML   DO_THIS + 0
#define DO_HOME_AJAX   DO_THIS + 1
#define DO_HOME_SCRIPT DO_THIS + 2
#define DO_NMEA_HTML   DO_THIS + 3
#define DO_NMEA_AJAX   DO_THIS + 4
#define DO_NMEA_SCRIPT DO_THIS + 5

int WebServerThisDecideWhatToDo(char *pPath, char* pLastModified)
{
    if (HttpSameStr(pPath, "/"          )) return DO_HOME_HTML;
    if (HttpSameStr(pPath, "/home-ajax" )) return DO_HOME_AJAX;
    if (HttpSameStr(pPath, "/nmea"      )) return DO_NMEA_HTML;
    if (HttpSameStr(pPath, "/nmea-ajax" )) return DO_NMEA_AJAX;
    
    if (HttpSameStr(pPath, "/home.js"   )) return HttpSameDate(WebHomeScriptDate, WebHomeScriptTime, pLastModified) ? DO_NOT_MODIFIED : DO_HOME_SCRIPT;
    if (HttpSameStr(pPath, "/nmea.js"   )) return HttpSameDate(WebNmeaScriptDate, WebNmeaScriptTime, pLastModified) ? DO_NOT_MODIFIED : DO_NMEA_SCRIPT;
    
    return DO_NOT_FOUND;
}

bool WebServerThisHandleQuery(int todo, char* pQuery)
{
    switch (todo)
    {
        case DO_HOME_AJAX: WebHomeQuery(pQuery); return true;
        case DO_NMEA_HTML: WebNmeaQuery(pQuery); return true;
        case DO_NMEA_AJAX: WebNmeaQuery(pQuery); return true;
    }
    return false;
}
bool WebServerThisPost(int todo, int contentLength, int contentStart, int size, char* pRequestStream, uint32_t positionInRequestStream, bool* pComplete)
{
    return false;
}
bool WebServerThisReply(int todo)
{
    switch (todo)
    {
        case DO_HOME_HTML:   WebHomeHtml  (); return true;
        case DO_HOME_AJAX:   WebHomeAjax  (); return true;
        case DO_HOME_SCRIPT: WebHomeScript(); return true;
        case DO_NMEA_HTML:   WebNmeaHtml  (); return true;
        case DO_NMEA_AJAX:   WebNmeaAjax  (); return true;
        case DO_NMEA_SCRIPT: WebNmeaScript(); return true;
    }
    return false;
}

