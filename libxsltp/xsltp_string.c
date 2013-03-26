#include "xsltp_config.h"
#include "xsltp_core.h"

#define HEX_ESCAPE '%'
#define ACCEPTABLE(a) ( a>=32 && a<128 && ((isAcceptable[a-32]) & mask))

static unsigned char isAcceptable[96] =
{/* 0x0 0x1 0x2 0x3 0x4 0x5 0x6 0x7 0x8 0x9 0xA 0xB 0xC 0xD 0xE 0xF */
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xF,0xE,0x0,0xF,0xF,0xC, /* 2x  !"#$%&'()*+,-./   */
    0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0x8,0x0,0x0,0x0,0x0,0x0, /* 3x 0123456789:;<=>?   */
    0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF, /* 4x @ABCDEFGHIJKLMNO   */
    0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0x0,0x0,0x0,0x0,0xF, /* 5X PQRSTUVWXYZ[\]^_   */
    0x0,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF, /* 6x `abcdefghijklmno   */
    0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0x0,0x0,0x0,0x0,0x0  /* 7X pqrstuvwxyz{\}~DEL */
};
static char *hex = "0123456789ABCDEF";

char *
xsltp_strdup(char *src)
{
    int len = strlen(src);
    char *dst = (char *) xsltp_malloc(len + 1);
    memcpy(dst, src, len);
    dst[len] = '\0';
    return dst;
}

char *
xsltp_escape_uri(const char *str, XSLTP_URI_ENCODING mask)
{
    const char *p;
    char       *q;
    char       *result;
    int        unacceptable = 0;

    if (!str)
        return NULL;

    for (p = str; *p; p++)
        if (!ACCEPTABLE((unsigned char) *p))
            unacceptable++;

    if ((result = (char  *) xsltp_malloc(p - str + unacceptable + unacceptable + 1)) == NULL)
        return NULL;

    for (q = result, p = str; *p; p++) {
        unsigned char a = *p;
        if (!ACCEPTABLE(a)) {
            *q++ = HEX_ESCAPE; /* Means hex commming */
            *q++ = hex[a >> 4];
            *q++ = hex[a & 15];
        } else {
            *q++ = *p;
        }
    }

    *q++ = 0; /* Terminate */

    return result;
}

static char
xsltp_ascii_hex_to_char(char c)
{
    return  c >= '0' && c <= '9' ?  c - '0'
            : c >= 'A' && c <= 'F'? c - 'A' + 10
            : c - 'a' + 10;	/* accept small letters just in case */
}

char *
xsltp_unescape_uri(char * str)
{
    char *p = str;
    char *q = str;

    if (!str)
        return NULL;

    while (*p) {
        if (*p == HEX_ESCAPE) {
            p++;
            if (*p) *q = xsltp_ascii_hex_to_char(*p++) * 16;
            if (*p) *q = *q + xsltp_ascii_hex_to_char(*p), ++p;
            q++;
        } else {
            *q++ = *p++;
        }
    }

    *q++ = 0;

    return str;
}

