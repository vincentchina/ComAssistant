#ifndef __DATA_CHECKER_H__
#define __DATA_CHECKER_H__

namespace DataChecker
{
enum eEndianAlign{ eAlignByte, eAlignWord, eAlignDWord };
bool toBigEndian( void* data, unsigned int cb, eEndianAlign align = eAlignByte );
unsigned short crc16( const void *data, unsigned int cb );
unsigned int crc32( const void *data, unsigned int cb, bool bLast = true, unsigned int beginValue = 0xffffffff );
unsigned char sum8( const void *data, unsigned int cb );
unsigned short sum16( const void *data, unsigned int cb );
};

#endif
