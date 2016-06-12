#ifndef __REDUCTION_H
#define __REDUCTION_H

#include <openssl/md5.h>
#include <stdlib.h>
#include "range.h"


/// Konwertuje hash z postaci binarnej na tekstową.
/// @byte_hash  Hash w postaci binarnej (16 bajtów).
/// @char_hash  [OUT] Hash w postaci tekstowej (32 bajty).
void bytehash_to_string(unsigned char* byte_hash, char* char_hash)
{
    int i;
    for(i=0; i<16; i++)
    {
        sprintf(char_hash+i*2, "%02x", byte_hash[i]);
    }
}


/// Funkcja redukująca hash do hasła.
/// @range      Zakres haseł
/// @hash       Hash hasła (binarny) do zredukowania (16 bajtów)
/// @password   [OUT] Hasło otrzymane z redukcji
void reduction(range_t* range, unsigned char* hash, char* password)
{
    int charset_length = strlen(range->charset);

    int length = (*((int*)hash)) % (range->length_max - range->length_min + 1)
        + range->length_min;

    int i;
    int offset;
    for(i=0; i<length; i++)
    {
        offset = *((unsigned int*)(hash+(i%13))) % charset_length;
        password[i] = range->charset[offset];
    }
    password[length] = '\0';
}

#endif // __REDUCTION_H

