#include <stdio.h>

#include "test_range.h"
#include "test_result_buffer.h"

int main(int argc, char** argv)
{
    // range.h
    test_range_next_password1();
    test_range_next_password2();

    test_range_password_counts1();
    test_range_password_counts2();
    test_range_password_counts3();

    test_range_password1();
    test_range_password2();

    test_get_subrange1();
    test_get_subrange2();


    // result_buffer.h
    test_result_buffer_init1();

    test_result_buffer_flush1();

    test_result_make1();

    test_result_buffer_write1();

    return 0;
}
