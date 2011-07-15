#ifndef _LIVE_P2PCOMMON2_BASE_PPL_CRYPTO_SHA1_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_CRYPTO_SHA1_H_

/*
SHA-1 in C
By Steve Reid <sreid@sea-to-sky.net>
100% Public Domain

-----------------
23 Apr 2001 version from http://sea-to-sky.net/~sreid/
Modified slightly to take advantage of autoconf.
See sha1.c for full history comments.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned long state[5];
    unsigned long count[2];
    unsigned char buffer[64];
} SHA1_CTX;

void SHA1Transform(unsigned long state[5], unsigned char buffer[64]);
void SHA1Init(SHA1_CTX* context);
void SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned long len); /* JHB */
void SHA1Final(unsigned char digest[20], SHA1_CTX* context);

#ifdef __cplusplus
}
#endif

#endif

