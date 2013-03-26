#include "xsltp_config.h"
#include "xsltp_core.h"

#ifndef _XSLTP_STRING_INCLUDED_
#define _XSLTP_STRING_INCLUDED_

typedef enum _XSLTP_URI_ENCODING {
    URL_XALPHAS     = 0x1,     /* Escape all unsafe characters */
    URL_XPALPHAS    = 0x2,     /* As URL_XALPHAS but allows '+' */
    URL_PATH        = 0x4,     /* As URL_XPALPHAS but allows '/' */
    URL_DOSFILE     = 0x8      /* As URL_URLPATH but allows ':' */
} XSLTP_URI_ENCODING;

char *xsltp_strdup(char *str);
char *xsltp_unescape_uri(char * str);
char *xsltp_escape_uri(const char *str, XSLTP_URI_ENCODING mask);

#endif /* _XSLTP_STRING_H_INCLUDED_ */
