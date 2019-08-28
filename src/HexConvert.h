#ifndef __HEX_CONVERT_H__
#define __HEX_CONVERT_H__

namespace HexConvert{

    static char HEX_CODE[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

    static void HexToText( unsigned char* buff, int count, QString& txt, char fill = ' ' )
    {
        unsigned char* index = buff;
        unsigned char* end = buff + count;
        while ( index < end )
        {
            unsigned char c = *index++;
            txt += HEX_CODE[ ( c >> 4 ) ];
            txt += HEX_CODE[ ( c & 0x0f ) ];
            if( fill != 0 )
                txt += fill;
        }
        if( fill != 0 )
            txt.remove( txt.length() - 1, 1 );
    }

    inline void HexToText( QByteArray& ba, QString& txt, char fill = ' ' )
    {
        return HexToText( (unsigned char*)ba.data(), ba.size(), txt, fill );
    }

    static void TextToHex( QString& txt, QByteArray& ba )
    {
        bool byteToggle = false;
        bool validByte = false;
        char byte = 0;
        char halfByte = 0;
        for ( QString::iterator iter = txt.begin(); iter != txt.end(); ++iter )
        {
            char c = (*iter).toLatin1();
            if( c >= '0' && c <= '9' )
                halfByte = c - '0';
            else if( c >= 'a' && c <= 'f' )
                halfByte = c - 'a' + 10;
            else if( c >= 'A' && c <= 'F' )
                halfByte = c - 'A' + 10;
            else
                continue;
            if( byteToggle )
            {
                byte <<= 4;
                byte |= halfByte;
                ba.push_back( byte );
            }else
            {
                byte = halfByte;
            }
            byteToggle = !byteToggle;
            validByte = byteToggle;
        }
        if( validByte )
        {
            byte <<= 4;
            ba.push_back( byte );
        }
    }

};


#endif
