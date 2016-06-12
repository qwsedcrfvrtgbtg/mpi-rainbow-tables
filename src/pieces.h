#ifndef __PIECES_H
#define __PIECES_H

/// Zwraca ilość na jaką powinien być podzielony zakres.
uint64_t pieces_count(range_t* range, int worker_count, uint64_t max_mem)
{
    if(worker_count <= 0)
        return 1u;

    // ilość pamięci na jedno hasło
    uint64_t pass_mem_size = range->length_max + 1 + 4 + 1;
    printf("pass_mem_size: %lu\n", pass_mem_size);

    // ile hasłe zmieści się w połowie pamięci
    uint64_t mem_pass_count = max_mem / 2u / pass_mem_size;
    uint64_t total_password_count = range_password_count_in_range(range);

    uint64_t data_pieces = total_password_count / mem_pass_count;

    uint64_t nearest_power = (uint64_t) worker_count;
    while( nearest_power * (uint64_t) worker_count <= data_pieces)
    {
        nearest_power *= (uint64_t) worker_count;
    }
    return nearest_power;
}

#endif // __PIECES_H

