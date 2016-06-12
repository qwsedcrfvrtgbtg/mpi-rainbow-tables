#ifndef __RESULT_BUFFER_H
#define __RESULT_BUFFER_H

#include <stdio.h>
#include "reduction.h"

/// Struktura reprezentuje bufor rezultatów.
//  Służy do buforowania wyjścia do pliku.
typedef struct{
    char* buffer;
    int size;
    int length;
    FILE* output;
} result_buffer_t;

/// Tworzy rezultat do przesłania do procesu głównego.
/// @password       Hasło od którego zaczął się łańcuch
/// @hash           Końcowy hash łańcucha (16B)
/// @chain_length   Długość łańcucha
/// @result         [OUT] Zawartość rezultatu
void result_make(char* password, unsigned char* hash, int chain_length, char* result)
{
    // bajty 0-3: int chain length
    memcpy(result, &chain_length, 4);
    // bajty 4-19: char hash[16]
    memcpy(result+4, hash, 16);
    // bajty 20+: string z hasłem
    strcpy(result+20, password);
}



/// Inicjalizuje bufor rezultatów.
/// @rb         Bufor rezultatów
/// @size       Rozmiar bufora (w bajtach)
/// @filename   Ścieżka pliku wyjściowego bufora
void result_buffer_init(result_buffer_t* rb, int size, char* filename)
{
    rb->size = size;
    rb->buffer = malloc(sizeof(*(rb->buffer)) * size);
    rb->length = 0;
    rb->buffer[0] = '\0';
    rb->output = fopen(filename, "w");
}



/// Oczyszcza bufor z danych (zapisuje do pliku)
/// @rb     Bufor rezultatów
void result_buffer_flush(result_buffer_t* rb)
{
    printf("Flushing %d bytes.\n", rb->length);
    fputs(rb->buffer, rb->output);
    rb->buffer[0] = '\0';
    rb->length = 0;
}



/// Zapisuje rezultat do bufora.
/// @rb     Bufor rezultatów
/// @result Rezultat uzyskany za pomocą funkcji result_make()
void result_buffer_write(result_buffer_t* rb, char* result)
{
    int password_length = strlen(result);

    // sprawdzamy czy nowy rezultat zmieści się do bufora
    // długość + hasło + '\t' + hash + '\t' + 4B(liczba) + '\n' + NULL
    if(rb->length + password_length + 1 + 32 + 1 + 12 + 1 + 1 >= rb->size)
    {
        result_buffer_flush(rb);
    }

    // Zmienne na dane wyciągnięte z result
    int chain_length;
    unsigned char hash[16];
    char hashstring[33];
    char password[64];

    // wyciąganie danych z result
    memcpy(&chain_length, result, 4);
    memcpy(hash, result+4, 16);
    strncpy(password, result+20, 63);
    // konwersja bytehasha na string
    bytehash_to_string(hash, hashstring);
    // zapis do bufora
    int l = sprintf(rb->buffer + rb->length, "%s\t%s\t%d\n", password, hashstring, chain_length);
    rb->length += l;
}



/// Zamknięcie bufora.
/// @rb     Bufor rezultatów
void result_buffer_close(result_buffer_t* rb)
{
    result_buffer_flush(rb);
    fclose(rb->output);
    free(rb->buffer);
}

#endif // __RESULT_BUFFER_H

