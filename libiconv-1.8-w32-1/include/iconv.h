
#ifndef _LIBICONV_H
#define _LIBICONV_H
#include <stddef.h>
#define LIBICONV_DLL_EXPORTED
#define ICONV_CONST const
typedef void* iconv_t;
extern size_t iconv (iconv_t cd, const char* * inbuf, size_t *inbytesleft, char* * outbuf, size_t *outbytesleft);
extern iconv_t iconv_open (const char* tocode, const char* fromcode);
extern int iconv_close (iconv_t cd);
#define _LIBICONV_VERSION 0x0108
#endif
