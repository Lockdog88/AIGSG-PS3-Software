#ifndef __24LCXXX_H_
#define __24LCXXX_H_

#include "qm_common.h"

#define     I2C_ADDR_24LCXXX        0x50
#define     __24LC64__

#if defined(__24LC64__)
#define     MAXADR_24LCXXX          8192
#define     PAGE_SIZE_24LCXXX       32
#endif

#if defined(__24LC128__)
#define     MAXADR_24LCXXX          16384
#define     PAGE_SIZE_24LCXXX       64
#endif

#if defined(__24LC256__)
#define     MAXADR_24LCXXX          32768
#define     PAGE_SIZE_24LCXXX       64
#endif

int M24C64_ByteWrite( int mem_addr, uint8_t data );
int M24C64_nByteWrite( int mem_addr, void *data, int size );
int M24C64_PageWrite( int mem_addr, char *data );
int M24C64_nByteRead( int mem_addr, unsigned char *const data, int size );


#endif /* __24LCXXX_H_ */
