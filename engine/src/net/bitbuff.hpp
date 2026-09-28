#pragma once

#include "common/common.h"

// Writes bytes to a buffer in network order
class bf_write
{
    public:
        void WriteU32At( u32 val, size_t* pos );
        void WriteU16At( u16 val, size_t* pos );
        void WriteU8At( u8 val, size_t* pos ); 


        char* GetBase( void ) const { return m_pData; };

        size_t GetByteHead( void ) const {return m_bytehead; };
        size_t GetBitHead( void ) const { return m_bithead; };

        void SetBuff( char* buff, size_t len ) {m_pData = buff; m_datalen = len;};

        void ResetHeads( void ) {m_bithead = 0; m_bytehead = 0;};


    private:
        char* m_pData;
        size_t m_datalen;
        size_t m_bithead; // amount of bits written so far
        size_t m_bytehead; // Amount of bytes written so far
};

// Reads to host byte order
class bf_read 
{
    public:
        bool ReadU32At( u32* out, size_t* pos );
        bool ReadU16At( u16* out, size_t* pos );
        bool ReadU8At( u8* out, size_t* pos );


        char* GetBase( void ) const { return m_pData; };
        void SetBuff( char* buff, size_t len ) {m_pData = buff; m_datalen = len;};
    private:
        char* m_pData;
        size_t m_datalen;
        size_t m_bithead; // amount of bits written so far
        size_t m_bytehead; // Amount of bytes written so far
};
