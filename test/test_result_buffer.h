#ifndef __TEST_RESULT_BUFFER_H
#define __TEST_RESULT_BUFFER_H

#include "../src/result_buffer.h"
#include <openssl/md5.h>

void test_result_buffer_init1()
{
    printf("test_result_buffer_init1()\n");
    result_buffer_t rb;
    result_buffer_init(&rb, 1024, "test_result_buffer_init");

    int i;
    for(i=0; i<1024; i++)
    {
        rb.buffer[i] = i%128;
    }

    assert(rb.length == 0);
    assert(rb.buffer[0] == '\0');
    assert(rb.size == 1024);
    assert(rb.output != NULL);

    for(i=0; i<1024; i++)
    {
        assert(rb.buffer[i] == i%128);
    }

    result_buffer_close(&rb);
}

void test_result_buffer_flush1()
{
    printf("test_result_buffer_flush1()\n");
    result_buffer_t rb;
    result_buffer_init(&rb, 1024, "test_result_buffer_init");

    char test_string[] = "ncjnc87sdhcnud scd98cn9w\n nec 98sh8oi39 8fjne9f8 weno";

    strncpy(rb.buffer, test_string, 1023);
    rb.length = strlen(test_string);
    result_buffer_flush(&rb);
    result_buffer_close(&rb);

    char buff[1024];
    FILE* f = fopen("test_result_buffer_init", "r");
    fread(buff, sizeof(buff[0]), strlen(test_string), f);
    fclose(f);
    buff[strlen(test_string)] = '\0';

    assert( !strcmp(buff, test_string));
    assert( rb.buffer[0] == '\0');
}

void test_result_buffer_write1()
{
    printf("test_result_buffer_write1()\n");
    result_buffer_t rb;
    result_buffer_init(&rb, 1024, "test_result_buffer_init");

    unsigned char hash[16];
    char hashstring[33];
    char result[64];

    int i;
    for(i=0; i<16; i++)
    {
        MD5((unsigned char*)&i, 4, hash);
        bytehash_to_string(hash, hashstring);
        hashstring[6] = '\0';
        result_make(hashstring, hash, i, result);

        result_buffer_write(&rb, result);
    }

    char pass[64];
    char* buf = rb.buffer;
    for(i=0; i<16; i++)
    {
        strncpy(pass, buf, 5);
        MD5((unsigned char*)&i, 4, hash);
        bytehash_to_string(hash, hashstring);

        assert( strncmp(pass, hashstring, 5)==0 );
        assert( *(buf + 6) == '\t' );
        assert( strncmp(buf+7, hashstring, 32) == 0 );
        assert( *(buf + 39) == '\t' );
        assert( atoi(buf+40) == i );

        buf = strchr(buf, '\n') + 1;
    }

    result_buffer_flush(&rb);
}

void test_result_make1()
{
    printf("test_result_make1()\n");
    char password[7] = "abcdef";
    unsigned char hash[17] = "1020304050abcdef";
    int chain = 111;

    char result[64];

    result_make(password, hash, chain, result);

    char expected_result[64];
    memcpy(expected_result, &chain, 4);
    memcpy(expected_result+4, hash, 16);
    strncpy(expected_result+20, password, 43);

    // porównanie pierwszych 4Bajtów: int - długość łańcucha
    assert( memcmp(expected_result, result, 4) == 0);
    // porównanie bajtów 4-19: char[16] - hash
    assert( memcmp(expected_result+4, result+4, 16) == 0);
    // porównanie haseł
    assert( strcmp(expected_result+20, result+20) == 0);
}

#endif // __TEST_RESULT_BUFFER_H

