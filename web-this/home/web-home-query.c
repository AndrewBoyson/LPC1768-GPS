#include "web/http/http.h"

void WebHomeQuery(char* pQuery)
{
    while (pQuery)
    {
        char* pName;
        char* pValue;
        pQuery = HttpQuerySplit(pQuery, &pName, &pValue);
        int value = HttpQueryValueAsInt(pValue);
    }
}

