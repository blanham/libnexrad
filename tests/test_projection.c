#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <nexrad/geo.h>

// Internal functions we want to test
extern int _mercator_find_y(double lat, int height);

int main() {
    int height = 512;
    int cy = height / 2;

    // Test normal latitude
    int y_0 = _mercator_find_y(0.0, height);
    assert(y_0 == cy);

    // Test latitude exceeding max
    // Without clamping, log((1 + sin(90)) / (1 - sin(90))) -> log(Inf)
    int y_90 = _mercator_find_y(90.0, height);
    printf("y_90: %d\n", y_90);
    
    // Test latitude below min
    int y_minus_90 = _mercator_find_y(-90.0, height);
    printf("y_minus_90: %d\n", y_minus_90);

    printf("All projection tests passed!\n");
    return 0;
}
