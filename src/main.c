#include <stdio.h>
#include <stdlib.h>
#include <mpi/mpi.h> // -lmpi
#include <openssl/md5.h> // -lcrypto

#include "../c_hashmap-master/hashmap.h"
#include "range.h"
#include "reduction.h"
#include "master.h"
#include "worker.h"

char global_help_text[] =
"mpi-rainbow-tables\n"
"  Program do generowania tablic tęczowych.\n\n"

"  Użycie programu:\n"
"  mpirun -np <liczba_procesorów> mpi-rainbow-tables [opcje]\n\n"

"  Opcje:\n"
"    -c | --max-chain-length <n>     Maksymalna długość łańcucha\n"
"    -m | --min-password-length <n>  Minimalna długość hasła\n"
"    -M | --max-password-length <n>  Maksymalna długość hasła\n\n"

"    -d | --digits                   Dołącza do alfabetu cyfry (domyślnie)\n"
"    -l | --lowercase                Dołącza do alfabetu małe litery\n"
"    -u | --uppercase                Dołącza do alfabetu duże litery\n"
"    -s | --special                  Dołącza do alfabetu znaki specialne\n"
"    -v | --space                    Dołącza do alfabetu spację\n\n"

"    -? | --help                     Wyświetla ten tekst pomocy\n";



/// Przetwarza opcje wywołania programu
void get_options(int argc, char** argv,
                 int* charset_flags,
                 int* max_chain_length,
                 int* min_password_length,
                 int* max_password_length)
{
    // sprawdzanie czy należy wyświelić pomoc
    if( argc > 1)
    {
        if( strcmp(argv[1], "-?") == 0 || strcmp(argv[1], "--help") == 0)
        {
            printf("%s", global_help_text);
            exit(EXIT_SUCCESS);
        }
    }

    // pobieramy opcje
    int i;
    for(i=1; i<argc; i++)
    {
        if( strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--max-chain-length") == 0)
        {
            if(i+1 < argc)
            {
                *max_chain_length = atoi(argv[i+1]);
                i++;
            }
        }

        if( strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--min-password-length") == 0)
        {
            if(i+1 < argc)
            {
                *min_password_length = atoi(argv[i+1]);
                i++;
            }
        }

        if(strcmp(argv[i], "-M") == 0 ||
           strcmp(argv[i], "--max-password-length") == 0)
        {
            if(i + 1 < argc)
            {
                *max_password_length = atoi(argv[i+1]);
                i++;
            }
        }

        if( strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--digits") == 0)
        {
            *charset_flags |= CHARSET_DIGITS;
        }

        if( strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--lowercase") == 0)
        {
            *charset_flags |= CHARSET_LOWER;
        }

        if( strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--uppercase") == 0)
        {
            *charset_flags |= CHARSET_UPPER;
        }

        if( strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--special") == 0)
        {
            *charset_flags |= CHARSET_SPECIAL;
        }

        if( strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--space") == 0)
        {
            *charset_flags |= CHARSET_SPACE;
        }
    }

    if( *charset_flags == 0)
        *charset_flags = CHARSET_DIGITS;
}

int main(int argc, char** argv)
{
    int world_size;
    int world_rank;

    // flagi alfabetu - patrz range.h:range_new_f()
    int charset = 0;
    // maksymalna długość łańcucha
    int max_chain_length = 8;
    // minimalna długość hasła
    int min_password_length = 5;
    // maksymalna długość hasła
    int max_password_length = 5;

    // pobieranie opcji
    get_options(argc, argv,
                &charset, &max_chain_length,
                &min_password_length, &max_password_length);

    // tworzenie zakresu haseł
    range_t range;
    range_new_f(min_password_length, max_password_length, charset , &range);

    // inicjalizacja MPI, pobranie ilości procesów i rangi procesu
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if(world_rank == 0)
    {
        // wyświelenie informacji o ustawieniach programu
        printf("Max chain length set to %d\n", max_chain_length);
        printf("Min password length set to %d\n", min_password_length);
        printf("Max password length set to %d\n", max_password_length);
        printf("Charset consists of |");
        if((charset & CHARSET_DIGITS) == CHARSET_DIGITS)
            printf(" digits |");
        if((charset & CHARSET_LOWER) == CHARSET_LOWER)
            printf(" lowercase letters |");
        if((charset & CHARSET_UPPER) == CHARSET_UPPER)
            printf(" uppercase letters |");
        if((charset & CHARSET_SPECIAL) == CHARSET_SPECIAL)
            printf(" special characters |");
        if((charset & CHARSET_SPACE) == CHARSET_SPACE)
            printf(" space |");
        printf("\n");

        // funckja procesu głównego
        master(world_size, &range);
    }
    else{
        // funckja "robotników"
        worker(world_size, world_rank, &range, max_chain_length);
    }

    MPI_Finalize();

    return 0;
}
