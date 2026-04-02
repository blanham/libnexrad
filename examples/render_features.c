#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nexrad/image.h>
#include <nexrad/feature.h>
#include <nexrad/geo.h>

int main() {
    uint16_t width = 400;
    uint16_t height = 400;
    nexrad_image *image = nexrad_image_create(width, height);
    if (!image) {
        fprintf(stderr, "Failed to create image\n");
        return 1;
    }

    /* Define some colors */
    nexrad_color red = {255, 0, 0, 255};
    nexrad_color green = {0, 255, 0, 255};
    nexrad_color white = {255, 255, 255, 255};

    /* Draw a white background (optional, it's black by default) */
    /* for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x++) {
            nexrad_image_draw_pixel(image, white, x, y);
        }
    } */

    /* Create a manual projected feature list */
    nexrad_projected_feature_list *pfl = malloc(sizeof(nexrad_projected_feature_list));
    pfl->count = 2;
    pfl->features = malloc(sizeof(nexrad_projected_feature *) * 2);

    /* Feature 1: A point at (100, 100) */
    nexrad_feature *f1 = calloc(1, sizeof(nexrad_feature));
    f1->id = 1;
    f1->name = strdup("Test Point");
    f1->geometry = calloc(1, sizeof(nexrad_geometry));
    f1->geometry->type = NEXRAD_GEOMETRY_POINT;

    nexrad_projected_feature *pf1 = malloc(sizeof(nexrad_projected_feature));
    pf1->feature = f1;
    pf1->count = 1;
    pf1->points = malloc(sizeof(nexrad_geo_screen_point));
    pf1->points[0].x = 100;
    pf1->points[0].y = 100;
    pf1->points[0].visible = 1;

    /* Feature 2: A triangle (polygon) */
    nexrad_feature *f2 = calloc(1, sizeof(nexrad_feature));
    f2->id = 2;
    f2->name = strdup("Test Triangle");
    f2->geometry = calloc(1, sizeof(nexrad_geometry));
    f2->geometry->type = NEXRAD_GEOMETRY_POLYGON;

    nexrad_projected_feature *pf2 = malloc(sizeof(nexrad_projected_feature));
    pf2->feature = f2;
    pf2->count = 3;
    pf2->points = malloc(sizeof(nexrad_geo_screen_point) * 3);
    pf2->points[0].x = 200; pf2->points[0].y = 200;
    pf2->points[1].x = 300; pf2->points[1].y = 200;
    pf2->points[2].x = 250; pf2->points[2].y = 300;

    pfl->features[0] = pf1;
    pfl->features[1] = pf2;

    /* Draw the features */
    nexrad_image_draw_features(image, pfl, red);

    /* Draw a single line manually */
    nexrad_image_draw_line(image, green, 10, 10, 390, 390);

    /* Save the image */
    if (nexrad_image_save_png(image, "features_test.png") != 0) {
        fprintf(stderr, "Failed to save image\n");
    } else {
        printf("Saved features_test.png\n");
    }

    /* Cleanup */
    /* Note: simplified cleanup for example */
    nexrad_image_destroy(image);
    free(pf1->points);
    free(pf1);
    free(f1->name);
    free(f1->geometry);
    free(f1);
    free(pf2->points);
    free(pf2);
    free(f2->name);
    free(f2->geometry);
    free(f2);
    free(pfl->features);
    free(pfl);

    return 0;
}
