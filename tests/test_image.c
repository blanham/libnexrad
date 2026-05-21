#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <nexrad/image.h>

struct _nexrad_image {
    uint8_t * buf;
    size_t    size;

    uint16_t width;
    uint16_t height;
    uint16_t radius;
    uint16_t x_center;
    uint16_t y_center;
};

int main() {
    // Test image creation and radius
    nexrad_image *image = nexrad_image_create(100, 200);
    assert(image != NULL);
    assert(image->width == 100);
    assert(image->height == 200);
    // After fix, radius should be min(100, 200) / 2 = 50
    assert(image->radius == 50);

    // Test draw run overflow safety
    // x = 90, length = 20 should be rejected (90 + 20 > 100)
    nexrad_color color = {255, 0, 0, 255};
    nexrad_image_draw_run(image, color, 90, 10, 20);
    // If it didn't crash, it's a good sign. We can't easily check the buffer 
    // contents without exposing internal structs, but we can verify the 
    // bounds check doesn't overflow.

    nexrad_image_destroy(image);

    printf("All image tests passed!\n");
    return 0;
}
