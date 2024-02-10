// Array of test configurations
#include "test.h"
#include <string.h>

test_data_t *G_test_array[MAX_TESTS];


void init_tests(void)
 {
  memset(&G_test_array,0x0,sizeof(G_test_array));
 }


