#ifndef _XSLTP_MD5_H_INCLUDED_
#define _XSLTP_MD5_H_INCLUDED_

#include <openssl/md5.h>

typedef MD5_CTX XSLTP_MD5;

#define xsltp_md5_init   MD5_Init
#define xsltp_md5_update MD5_Update
#define xsltp_md5_final  MD5_Final

#endif /* _XSLTP_MD5_H_INCLUDED_ */
