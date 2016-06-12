#ifndef __MASTER_H
#define __MASTER_H

#include "tags.h"
#include "result_buffer.h"
#include <stdint.h>
#include <unistd.h>

/// Pobiera hasło dla workera na podstawie poprzedniego hasła.
/// @range          Zakres haseł
/// @password_map   Uchwyt do mapy haseł
/// @last_password  Ostatnie hasło w podzakresie haseł workera
/// @password       [IN] Poprzednie hasło / [OUT] nowe hasło
/// @return         0: istnieje następne hasło w podzakresie, -1: nie istnieje
int get_password_for_worker(range_t* range, map_t password_map, char* last_password, char* password)
{
    char* data;
    while( hashmap_get(password_map,password,(void**)&data) == MAP_OK )
    {
        if( strcmp(password, last_password) == 0)
        {
            // podzakres haseł wyczerpany
            return -1;
        }
        // pobieramy następne hasło w danym zakresie
        range_next_password(range, password);
    }

    data = malloc(sizeof(*data) * 6);
    strcpy(data, password);
    hashmap_put(password_map, data, NULL);

    return 0;
}

/// Funkcja procesu głównego.
/// @world_size     Ilość procesów utworzonych przez MPI
/// @range          Zakres haseł
void master(int world_size, range_t* range)
{
    // liczba workerów
    int worker_count = world_size - 1;

    if( worker_count == 0 )
    {
        printf("Nie utworzono wystarczającej liczby procesów.\n");
        return;
    }

    // Hashmapa na hasła
    // Podczas obliczania hasha dla jakiegoś hasła wstawiamy element
    // z kluczem równym hasłu.
    // Później jeżeli chcemy sprawdzić czy hash został już obliczany, pobieramy
    // element na podstawie hasła (które jest kluczem hashmapy).
    // Jeżeli element jest w hashmapie to znaczy że hash być obliczony.
    map_t master_map = hashmap_new();

    // Rozmiar stringów na hasła
    int password_size = range->length_max + 1;

    // Granice haseł dla workerów
    // każda granica to pierwsze hasło i ostatnie hasło
    char** worker_password = malloc(sizeof(*worker_password) * worker_count * 2);
    int i;
    for(i=0; i<worker_count; i++)
    {
        // tablica na pierwsze hasło
        worker_password[i*2] =
            malloc(sizeof(*worker_password[i*2]) * password_size);
        // tablica na ostatnie hasło
        worker_password[i*2+1] =
            malloc(sizeof(*worker_password[i*2+1]) * password_size);

        // pobieranie podzakresu dla workera
        get_subrange(range,
                     worker_count,
                     i+1,
                     worker_password[i*2],
                     worker_password[i*2+1]);

        printf("MS: Worker %d has range \"%s\":\"%s\"\n",
               i+1, worker_password[i*2], worker_password[i*2+1]);
    }

    // hasło do przesyłania
    char* password = malloc(sizeof(char) * password_size);

    // hasło odebrane w IS_COMPUTED
    char* req_ic_password = malloc(sizeof(*req_ic_password) * password_size);

    // Rezultat otrzymany od workera
    // Oprócz hasła zawiera hash (16B) i długość łańcucha (4B) i znak NULL
    // patrz funckja result_buffer.h:result_make()
    int result_size = (password_size + 16 + 4 + 1);
    char* req_r_result = malloc(sizeof(*req_r_result) * result_size );

    int working = worker_count;

    int awaiting_request_for_work = 0;
    int awaiting_is_computed = 0;
    int awaiting_result = 0;

    MPI_Request req_rfw;
    MPI_Request req_ic;
    MPI_Request req_r;

    MPI_Status req_status;
    MPI_Status req_ic_status;

    int req_flag;
    int is_computed;

    int result;

    result_buffer_t result_buffer;
    result_buffer_init(&result_buffer, 4*1024*1024, "rainbow-table" );

    while( working )
    {
        // obsługa REQUEST_FOR_WORK
        if( !awaiting_request_for_work)
        {
            // asynchronicznie odbieramy wiadomość
            MPI_Irecv(&req_flag, 1, MPI_INT, MPI_ANY_SOURCE, TAG_REQUEST_FOR_WORK, MPI_COMM_WORLD, &req_rfw);
            awaiting_request_for_work = 1;
        }
        else
        {
            // jeżeli oczekujemy już na wiadomość to sprawdzamy
            // czy jakaś przyszła
            MPI_Test(&req_rfw, &req_flag, &req_status);
            // jeżeli req_flag == 1 -> otrzymaliśmy wiadomość
            if( req_flag)
            {
                // pobieramy hasło dla workera
                result = get_password_for_worker(range, master_map,
                                        worker_password[(req_status.MPI_SOURCE-1)*2+1],
                                        worker_password[(req_status.MPI_SOURCE-1)*2] );
                if( result == 0 )
                {
                    // wysyłanie hasła do nadawcy
                    MPI_Send(worker_password[(req_status.MPI_SOURCE-1)*2],
                             password_size,
                             MPI_CHAR,
                             req_status.MPI_SOURCE,
                             TAG_PASSWORD,
                             MPI_COMM_WORLD);
                    //printf("MS: Send PASSWORD [%s] to worker %d\n",
                           //worker_password[(req_status.MPI_SOURCE-1)*2],
                           //req_status.MPI_SOURCE);
                }
                else
                {
                    // jeżeli zakres haseł dla workera się wyczerpał
                    // wywyłamy do niego END_OF_WORK
                    MPI_Send(NULL, 0, MPI_INT, req_status.MPI_SOURCE, TAG_END_OF_WORK, MPI_COMM_WORLD);
                    working--;
                }
                awaiting_request_for_work = 0;
            }
        }

        // obługa IS_COMPUTED
        if( !awaiting_is_computed)
        {
            MPI_Irecv(req_ic_password, password_size, MPI_CHAR, MPI_ANY_SOURCE, TAG_IS_COMPUTED, MPI_COMM_WORLD, &req_ic);
            awaiting_is_computed = 1;
        }
        else
        {
            MPI_Test(&req_ic, &req_flag, &req_ic_status);
            if( req_flag)
            {
                // sprawdzamy czy hash dla hasła już został obliczony
                char* p;
                if( hashmap_get(master_map, req_ic_password, (void**)&p) == MAP_OK )
                {
                    is_computed = 1;
                }
                else
                {
                    char* p = malloc(sizeof(*p) * password_size);
                    strcpy(p, req_ic_password);
                    hashmap_put(master_map, p, NULL);
                    is_computed = 0;
                }

                //printf("MS: Send IS_COMPUTED to %d...\n", req_ic_status.MPI_SOURCE);
                //MPI_Isend(&status, 1, MPI_INT, req_ic_status.MPI_SOURCE, TAG_STATUS, MPI_COMM_WORLD, &req_s);
                MPI_Send(&is_computed, 1, MPI_INT, req_ic_status.MPI_SOURCE, TAG_IS_COMPUTED, MPI_COMM_WORLD);
                awaiting_is_computed = 0;
            }
        }

        // obsługa RESULT
        if( !awaiting_result)
        {
            MPI_Irecv(req_r_result, result_size, MPI_CHAR, MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &req_r);
            awaiting_result = 1;
        }
        else
        {
            MPI_Test(&req_r, &req_flag, MPI_STATUS_IGNORE);
            if( req_flag)
            {
                result_buffer_write(&result_buffer, req_r_result);
                awaiting_result = 0;
            }
        }
    }

    if(awaiting_request_for_work)
    {
        printf("MS: Canceling MPI_Irecv REQUEST_FOR_WORK\n");
        MPI_Cancel(&req_rfw);
    }
    if(awaiting_is_computed)
    {
        printf("MS: Canceling MPI_Irecv IS_COMPUTED\n");
        MPI_Cancel(&req_ic);
    }
    if(awaiting_result)
    {
        printf("MS: Canceling MPI_Irecv RESULT\n");
        MPI_Cancel(&req_r);
    }

    // zamykamy bufor
    result_buffer_close(&result_buffer);

    // czyścimy dane per worker
    for(i=0; i<worker_count; i++)
    {
        free(worker_password[i*2]);
        free(worker_password[i*2+1]);
    }
    free(worker_password);
    free(password);
    free(req_ic_password);
    // czyścimy hashmapę master
    hashmap_free(master_map);
}

#endif // __MASTER_H

