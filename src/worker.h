#ifndef __WORKER_H
#define __WORKER_H

#include "tags.h"


/// Funckja główna robotnika
/// @world_size         Liczba procesów uruchomionych przez MPI
/// @world_rank         Ranga procesu
/// @range              Zakres haseł
/// @max_chain_length   Maksymalna długość łańcucha
void worker(int world_size, int world_rank, range_t* range, int max_chain_length)
{
    int password_size = (range->length_max + 1);

    char* password = malloc(sizeof(*password) * password_size);
    char* first_password = malloc(sizeof(*first_password) * password_size);

    int result_size = password_size + 16 + 5;
    char* result = malloc(sizeof(*result) * result_size);

    unsigned char hash[16];

    int chain_length;
    int is_computed = 0;

    MPI_Status recv_status;


    while( 1 )
    {
        is_computed = 0;
        chain_length = 0;

        MPI_Send(NULL, 0, MPI_INT, 0, TAG_REQUEST_FOR_WORK, MPI_COMM_WORLD);
        //printf("p%d: Sent REQUEST_FOR_WORK\n", world_rank);
        // 1. otrzymujemy hasło do przetworzenia
        MPI_Recv(first_password, password_size, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_status);

        if(recv_status.MPI_TAG == TAG_END_OF_WORK)
        {
            //printf("p%d: Recv END_OF_WORK\n", world_rank);
            break;
        }

        //printf("p%d: Received PASSWORD [%s]\n", world_rank, first_password);

        // kopiowanie hasła
        strcpy(password,first_password);

        while( !is_computed && chain_length < max_chain_length )
        {
            chain_length++;

            // 2. obliczamy hash
            MD5((unsigned char*)password, strlen(password), hash);

            // 3. redukujemy
            reduction(range, hash, password);
            // 4. wysyłamy zapytanie o hasło
            MPI_Send(password, password_size, MPI_CHAR, 0, TAG_IS_COMPUTED, MPI_COMM_WORLD);
            //printf("p%d: Sent IS_COMPUTED ? [%s]\n", world_rank, password);
            // 5. Odbieramy odpowiedź od mastera
            MPI_Recv(&is_computed, 1, MPI_INT, 0, TAG_IS_COMPUTED, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //printf("p%d: Recv IS_COMPUTED   [%s][%d]\n", world_rank, password, is_computed);
        }

        // wysłanie rezultatu do rodzica
        result_make(first_password, hash, chain_length, result);
        MPI_Send(result, result_size, MPI_UNSIGNED_CHAR, 0, TAG_RESULT, MPI_COMM_WORLD);
        //printf("p%d: Sent RESULT\n", world_rank);
    }

    printf("p%d: Work done.\n", world_rank);

    free(password);
    free(result);
}

#endif // __WORKER_H

