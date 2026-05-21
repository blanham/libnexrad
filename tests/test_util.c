#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "util.h"

int main() {
    // Test bswap64
    uint64_t val = 0x0102030405060708ULL;
    uint64_t swapped = bswap64(val);
    printf("val: 0x%016llx, swapped: 0x%016llx\n", (unsigned long long)val, (unsigned long long)swapped);
    assert(swapped == 0x0807060504030201ULL);

    // Test safecpy
    char dest[10];
    // Case 1: destlen == 0 should return -1 and not crash/underflow
    int res = safecpy(dest, "hello", 0, 5);
    assert(res == -1);

    // Case 2: normal copy
    char dest2[5];
    safecpy(dest2, "hello", 5, 5);
    printf("dest2: '%s'\n", dest2);
    assert(strcmp(dest2, "hell") == 0);

    // Test nexrad_bswap_float
    float f = 1.0f; // 0x3f800000
    float f_swapped = nexrad_bswap_float(f);
    uint32_t f_bits;
    memcpy(&f_bits, &f_swapped, 4);
    printf("f: %f, f_swapped bits: 0x%08x\n", f, f_bits);
    assert(f_bits == 0x0000803f);

    printf("All util tests passed!\n");
    return 0;
}
