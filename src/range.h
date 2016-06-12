#ifndef __RANGE_H
#define __RANGE_H

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

// Flagi używane przy tworzeniu alfabetu.
// Patrz -> funkcja range_new_f()

#define CHARSET_DIGITS   0x01 // 0b00000001
#define CHARSET_LOWER    0x02 // 0b00000010
#define CHARSET_UPPER    0x04 // 0b00000100
#define CHARSET_SPECIAL1 0x08 // 0b00001000
#define CHARSET_SPECIAL2 0x10 // 0b00010000
#define CHARSET_SPECIAL3 0x20 // 0b00100000
#define CHARSET_SPECIAL4 0x40 // 0b01000000
#define CHARSET_SPACE    0x80 // 0b10000000

// Użyteczne mieszanki powyższych.

#define CHARSET_SPECIAL  0x78 // 0b01111000
#define CHARSET_ALL      0xff // 0b11111111
#define CHARSET_LETTERS  0x06 // 0b00000110

/// Reprezentuje zakres haseł do złamania.
//
// Przykładowo:
// range_t r = { 1, 5, "0123456789abcdef" };
// Opisuje zakres haseł od 1 do 5 znaków zawieracjących cyfry hexadecymalne
// (gdzie litery są małe).
typedef struct{
    char  length_min; // minimalna liczba znaków hasła
    char  length_max; // maksymalna liczba znaków hasła
    char  charset[257]; // alfabet znaków
} range_t;



/// Tworzy zakres haseł z użyciem alfabetu w postaci stringu.
/// @length_min Minimalna długość hasła
/// @length_max Maksymalna długość hasła
/// @charset    Alfabet
/// @range      [OUT] Struktura zakresu haseł
void range_new_c(char length_min, char length_max, char* charset, range_t* range)
{
    range->length_min = length_min;
    range->length_max = length_max;
    strncpy(range->charset, charset, 256);
}



/// Tworzy zakres haseł z użyciem flag.
/// @length_min Minimalna długość hasła
/// @length_max Maksymalna długość hasła
/// @flags      Flagi znaków
/// @range      [OUT] Struktura zakresu haseł
//
//  Przykładowe użycie tworzące zakres haseł 3-5 znakowych zawierających
//  cyfry, małe litery i duże litery:
//  range_t r;
//  range_new_f(3, 5, CHARSET_DIGIT | CHARSET_LOWER | CHARSET_UPPER, &r);
void range_new_f(char length_min, char length_max, int flags, range_t* range)
{
    range->length_min = length_min;
    range->length_max = length_max;
    range->charset[0] = '\0';

    // Znaki są dopisywane w określonej kolejności tak, aby kolejność
    // w alfabecie odpowiadała kolejności w zbiorze ASCII.
    // Dzięki temu można porównywać hasła za pomocą funkcji strcmp()
    // i określić na poziomie jej wyniku,
    // które hasło jest w danym zakresie pierwsze.

    // ASCII 32
    if( (flags & CHARSET_SPACE) == CHARSET_SPACE )
    {
        strcat(range->charset, " ");
    }
    // ASCII 33-47 (włącznie)
    if( (flags & CHARSET_SPECIAL1) == CHARSET_SPECIAL1 )
    {
        strcat(range->charset, "!\"#$%&'()*+,-./");
    }
    // ASCII 48-57 (włącznie)
    if( (flags & CHARSET_DIGITS) == CHARSET_DIGITS )
    {
        strcat(range->charset, "0123456789");
    }
    // ASCII 58-64 (włącznie)
    if( (flags & CHARSET_SPECIAL2) == CHARSET_SPECIAL2 )
    {
        strcat(range->charset, ":;<=>?@");
    }
    // ASCII 65-90 (włącznie)
    if( (flags & CHARSET_UPPER) == CHARSET_UPPER )
    {
        strcat(range->charset, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    }
    // ASCII 91-96 (włącznie)
    if( (flags & CHARSET_SPECIAL3) == CHARSET_SPECIAL3 )
    {
        strcat(range->charset, "[\\]^_`");
    }
    // ASCII 97-122 (włącznie)
    if( (flags & CHARSET_LOWER) == CHARSET_LOWER )
    {
        strcat(range->charset, "abcdefghijklmnopqrstuvwxyz");
    }
    // ASCII 91-96 (włącznie)
    if( (flags & CHARSET_SPECIAL4) == CHARSET_SPECIAL4 )
    {
        strcat(range->charset, "{|}~");
    }
}



/// Pobiera nowe hasło w danym zakresie.
/// @range      Zakres haseł
/// @password   [IN] poprzednie hasło / [OUT] nowo pobrane hasło
//
// Przykładowo:
//
// char password[6] = "aaaaa";
// range_t r = { 5, 5, "abcd", 4 };
// range_next_password(r, password);
//
// Po wywołaniu password == "aaaab"
void range_next_password(
                       range_t* range,
                       char* password
                       )
{
    int len = strlen(password);
    int i = len;
    char* next_char = password+len;

    while((*next_char) == '\0' && i>0)
    {
        i--;
        next_char = strchr(range->charset, password[i])+1;
    }

    // sprawdzamy czy dla którejś z pozycji hasła
    // mamy jescze możliwość pobrania wyższego znaku
    if(*next_char)
    {
        password[i++] = *next_char;
        while(i < len)
        {
            password[i++] = range->charset[0];
        }
    }
    else
    {
        // hasło jest wyczerpane
        // sprawdamy czy to maksymalna możliwa długość
        if( len < range->length_max )
        {
            len++;
            for(i=0;i<len;i++)
            {
                password[i] = range->charset[0];
            }
            password[len] = '\0';
        }
        else
        {
            // zwracamy pusty ciąg
            password[0] = '\0';
        }
    }
}



/// Zwraca n-te hasło dla danego zakresu.
/// @range              Zakres haseł
/// @password_counts    Liczebności haseł (uzyskane z range_password_counts())
/// @n                  Numer hasła (liczony od zera)
/// @password           [OUT] Hasło
void range_password(  range_t* range,
                    int* password_counts,
                    int n,
                    char* password  )
{
    /*printf("password_n = %d,", n);*/
    int password_length = range->length_min;
    while( n >= password_counts[password_length])
    {
        n-= password_counts[password_length];
        password_length++;
    }
    /*printf(" password_length = %d\n", password_length);*/

    int i;
    // wypełniamy hasło pierwszymi znakami alfabetu
    for(i=0; i<password_length; i++)
    {
        password[i] = range->charset[0];
    }
    password[password_length] = '\0';

    i = 0; // którą pozycję aktualizujemy
    char* c;
    // pomijamy tyle kombinacji ile się da
    while( i < password_length)
    {
        //printf("Checking possibilities on position %d\n", i);
        c = range->charset;
        //printf( "n = %d, password_counts[k] = %d\n", n, password_counts[password_length-i-2]);
        while( n >= password_counts[password_length-i-1] )
        {
            //printf("n >= password_counts[k] (%d >= %d)\n", n, password_counts[password_length-i-2]);
            //printf("  password_length = %d, i = %d, password_length-i-2 = %d\n", password_length, i , password_length-i-2);
            //printf("Decrementing n and adding c value (up to '%c')\n", *(c+1));
            n -= password_counts[password_length-i-1];
            c++;
        }
        password[i] = *c;
        i++;
    }
    /*c = range->charset;
    while(n > 0)
    {
        c++;
        n--;
    }
    password[password_length-1] = *c;*/
}



/// Zwraca liczebności haseł dla każdej ilości znaków od 1 do range->length_max
/// @range           Zakres znaków
/// @password_counts [OUT] Tablica ilości haseł do poszczególnych długości hasła
//  Liczebności są wpisywane do tablicy password_counts.
//  Każdy element o indeksie k (password_counts[k]) zawiera ilość haseł
//  o długości k.
//  Jeżeli długość alfabetu to A, wtedy password_counts[i] = A^i
int range_password_counts(range_t* range, int* password_counts)
{
    // Całkowita ilośc haseł.
    int password_count_all = 0;

    // Długość alfabetu zakresu
    int charset_length = strlen(range->charset);

    // obliczamy liczbę haseł dla każdego poziomu, oraz całkowitą liczbę haseł
    password_counts[0] = 1;
    password_counts[1] = charset_length;

    if(range->length_min == 1)
        password_count_all = password_counts[1];

    int i = 2;

    while(i <= range->length_max)
    {
        // liczba haseł N znakowych =
        //   liczba haseł N-1 znakowych * długość alfabetu
        password_counts[i] = password_counts[i-1] * charset_length;

        // dodajemy wynik do liczby wszystkich haseł
        if(i >= range->length_min)
            password_count_all += password_counts[i];

        i++;
    }

    return password_count_all;
}



/// Zwraca pierwsze i ostatnie hasło dla danego podzakresu podanego zakresu.
/// @range              Zakres haseł
/// @subrange_count     Ilość podzakresów na jaką chcemy podzielić zakres
/// @subrange_number    Numer podzakresu który chcemy wybrać po podziale
/// @first_password     [OUT] Pierwsze hasło z uzyskanego podzakresu
/// @last_password      [OUT] Ostatnie hasło z uzyskanego podzakresu
//
// Przykładowo:
// range_t r = { 4, 4, "abcd", 4 };  /* Zakres */
// char first_pass[5], last_pass[5]; /* Hasła */
// get_range(r, 2, 1, first_pass, last_pass);
// /* Teraz first_pass == "aaaa", last_pass == "bbbb" */
// get_range(r, 2, 2, first_pass, last_pass);
// /* Teraz first_pass == "caaa", last_pass == "dddd" */
void get_subrange(  range_t* range,
                    int subrange_count,
                    int subrange_number,
                    char* first_password,
                    char* last_password )
{
    // Liczebności haseł. Patrz funkcja range_password_counts()
    int* password_counts = malloc(sizeof(int) * range->length_max+1);

    int password_count_all = range_password_counts(range, password_counts);
    int passwords_per_subrange = password_count_all / subrange_count;
    /*printf("password_count_all = %d\n",password_count_all);
    printf("passwords_per_subrange = %d\n",passwords_per_subrange);
    printf("subrange_count = %d\n",subrange_count);*/

    int* counts = malloc(sizeof(int) * range->length_max+1);
    range_password_counts(range, counts);

    int last_password_index =
        (subrange_number < subrange_count) ?
        (passwords_per_subrange*subrange_number-1) : (password_count_all-1);

    range_password(range, counts, passwords_per_subrange*(subrange_number-1), first_password);
    range_password(range, counts, last_password_index, last_password);

    /*printf("first_pass_index = %d\n", passwords_per_subrange*(subrange_number-1));
    printf("last_pass_index  = %d\n", last_password_index);*/

    // czyszczenie sterty
    free(password_counts);
    free(counts);
}



/// Pobiera liczbę haseł w danym zakresie.
/// @range  Zakres haseł
uint64_t range_password_count_in_range(range_t* range)
{
    uint64_t count = 0;
    int length = range->length_min;
    uint64_t charset_length = strlen(range->charset);
    uint64_t count_in_length = (uint64_t) pow( charset_length, length);

    count += count_in_length;

    while(length < range->length_max)
    {
        count_in_length *= range->length_max;
        count += count_in_length;
        length++;
    }

    return count;
}

#endif // __RANGE_H

