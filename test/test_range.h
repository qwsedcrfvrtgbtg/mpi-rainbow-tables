#ifndef __TEST_H
#define __TEST_H

#include <stdio.h>
#include <assert.h>

#include "../src/range.h"

void test_range_next_password1()
{
    range_t range = { 5, 5, "abcdef" };
    char password[6];
    char passwords[][6] = {
        "aaaaa", "aaaab",
        "aaaaf", "aaaba",
        "aaaff", "aabaa",
        "affff", "baaaa",
        "fffff", "" };

    int i;
    for(i=0; i<5; i+=1)
    {
        strcpy(password,passwords[i*2]);
        //printf("We have password:   %s\n",password);
        //printf("Next suppose to be: %s\n",passwords[i*2+1]);
        range_next_password(&range, password);
        //printf("And it is:          %s\n",password);
        assert( !strcmp(password,passwords[i*2+1]) );
    }
}

void test_range_next_password2()
{
    range_t range = { 3, 5, "abcdef" };
    char password[6];
    char passwords[][6] = {
        "aaa", "aab",
        "eff", "faa",
        "fff", "aaaa",
        "ffff", "aaaaa",
        "fffff", "" };

    int i;
    for(i=0; i<5; i+=1)
    {
        strcpy(password,passwords[i*2]);
        //printf("We have password:   %s\n",password);
        //printf("Next suppose to be: %s\n",passwords[i*2+1]);
        range_next_password(&range, password);
        //printf("And it is:          %s\n",password);
        assert( !strcmp(password,passwords[i*2+1]) );
    }
}

void test_range_password_counts1()
{
    range_t r = { 1, 5, "abcdef" };
    int counts[6];
    int expected_counts[6] = {
        1,
        6,
        6*6,
        6*6*6,
        6*6*6*6,
        6*6*6*6*6
    };

    range_password_counts(&r, counts);

    int i;
    for(i=0; i<6; i++)
    {
        assert(counts[i] == expected_counts[i]);
    }
}

void test_range_password_counts2()
{
    range_t r = { 5, 5, "0123456789" };
    int counts[6];
    int expected_counts[6] = {
        1,
        10,    // 10^1
        100,   // 10^2
        1000,  // 10^3
        10000, // 10^4
        100000 // 10^5
    };

    range_password_counts(&r, counts);

    int i;
    for(i=0; i<6; i++)
    {
        assert(counts[i] == expected_counts[i]);
    }
}

void test_range_password_counts3()
{
    range_t r = { 5, 7, "abc" };
    int counts[8];
    int expected_counts[8] = {
        1,
        3,              // 3^1
        3*3,            // 3^2
        3*3*3,          // 3^3
        3*3*3*3,        // 3^4
        3*3*3*3*3,      // 3^5
        3*3*3*3*3*3,    // 3^6
        3*3*3*3*3*3*3   // 3^7
    };

    range_password_counts(&r, counts);

    int i;
    for(i=0; i<8; i++)
    {
        assert(counts[i] == expected_counts[i]);
    }
}

void test_range_password1()
{
    range_t r = { 4, 4, "abcd" };
    char pass[5];
    int indices[6] = { 0, 1, 2, 3, 4, 8 };
    char expected_passwords[][5] = {
        "aaaa", // 0
        "aaab", // 1
        "aaac", // 2
        "aaad", // 3
        "aaba", // 4
        "aaca" // 8
    };

    int counts[5];
    range_password_counts(&r, counts);

    printf("[ ");
    int x;
    for(x=0; x<5; x++) printf("%d ",counts[x]); printf("]\n");

    int i;
    for(i=0; i<6; i++)
    {
        range_password(&r, counts, indices[i], pass);
        printf("  i = %d  index = %d\n  password = %s\n  expected = %s\n", i, indices[i], pass, expected_passwords[i]);
        assert( !strcmp(pass, expected_passwords[i]));
    }
}

void test_range_password2()
{
    range_t r = { 1, 4, "abcd" };
    char pass[5];
    int indices[] = { 0, 3, 4, 19, 20, 83, 84, 339 };
    char expected_passwords[][5] = {
        "a", // 0
        "d", // 3
        "aa", // 4
        "dd", // 19
        "aaa", // 20
        "ddd", // 83
        "aaaa",// 84
        "dddd" // 339
    };

    int counts[5];
    range_password_counts(&r, counts);

    int i;
    for(i=0; i<8; i++)
    {
        range_password(&r, counts, indices[i], pass);
        printf("  i = %d  index = %d\n  password = %s\n  expected = %s\n", i, indices[i], pass, expected_passwords[i]);
        assert( !strcmp(pass, expected_passwords[i]));
    }
}

void test_get_subrange1()
{
    range_t r = { 4, 4, "abcd" };
    char first_pass[5], last_pass[5];
    char boundaries[][5] = {
        "aaaa", "addd",
        "baaa", "bddd",
        "caaa", "cddd",
        "daaa", "dddd"
    };

    int i;
    for(i=0; i<4; i++)
    {
        get_subrange(&r, 4, i+1, first_pass, last_pass);
        printf("Subrange %d/4 is       \"%s\":\"%s\"\n", i+1, first_pass, last_pass);
        printf("Expected subrange \"%s\":\"%s\"\n", boundaries[i*2], boundaries[i*2+1]);
        assert(!strcmp(first_pass, boundaries[i*2]));
        assert(!strcmp(last_pass, boundaries[i*2+1]));
    }
}

void test_get_subrange2()
{
    range_t r = { 1, 3, "abcd" };
    char first_pass[4], last_pass[4];
    char boundaries[][5] = {
        "a", "cd",
        "da", "acd",
        "ada", "bcd",
        "bda", "ccd",
        "cda", "ddd"
    };

    int i;
    for(i=0; i<5; i++)
    {
        get_subrange(&r, 5, i+1, first_pass, last_pass);
        printf("Subrange %d/5 is       \"%s\":\"%s\"\n", i+1, first_pass, last_pass);
        printf("Expected subrange \"%s\":\"%s\"\n", boundaries[i*2], boundaries[i*2+1]);
        assert(!strcmp(first_pass, boundaries[i*2]));
        assert(!strcmp(last_pass, boundaries[i*2+1]));
    }
}

#endif // __TEST_H
