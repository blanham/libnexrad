/*
 * Copyright (c) 2013-2026 Bryce Lanham. Distributed under the
 * terms of the MIT license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <nexrad/feature.h>
#include <nexrad/geo.h>
#include <nexrad/image.h>

int main() {
    /* Sample GeoJSON: A simple triangle */
    const char *json_data = "{"
        "\"type\": \"FeatureCollection\","
        "\"features\": ["
          "{"
            "\"type\": \"Feature\","
            "\"properties\": { \"NAME\": \"Test Triangle\" },"
            "\"geometry\": {"
              "\"type\": \"Polygon\","
              "\"coordinates\": ["
                "[[-98.0, 35.0], [-97.0, 35.0], [-97.5, 36.0], [-98.0, 35.0]]"
              "]"
            "}"
          "}"
        "]"
      "}";

    nexrad_feature_list *features = nexrad_feature_list_from_geojson(json_data);
    if (!features) {
        fprintf(stderr, "Failed to parse GeoJSON\n");
        return 1;
    }

    /* Mock Projection (Equirectangular) */
    nexrad_geo_spheroid *spheroid = nexrad_geo_spheroid_create();
    nexrad_geo_cartesian radar = { 35.0, -97.5 };
    nexrad_geo_projection *proj = nexrad_geo_projection_create_equirect(
        "test_proj.bin", spheroid, &radar, 460, 250, 3600, 0.01
    );

    if (!proj) {
        fprintf(stderr, "Failed to create mock projection\n");
        return 1;
    }

    /* 1. Project as INT16 (Legacy/Reference) */
    printf("Projecting as INT16...\n");
    nexrad_projected_feature_list *list_i16 = nexrad_feature_list_project(features, proj, NEXRAD_POINT_INT16);
    
    /* 2. Project as FLOAT (Cairo/High-Res) */
    printf("Projecting as FLOAT...\n");
    nexrad_projected_feature_list *list_f32 = nexrad_feature_list_project(features, proj, NEXRAD_POINT_FLOAT);

    /* 3. Project as FIXED_16_16 (LVGL/Embedded) */
    printf("Projecting as FIXED_16_16...\n");
    nexrad_projected_feature_list *list_q16 = nexrad_feature_list_project(features, proj, NEXRAD_POINT_FIXED_16_16);

    /* Verify first point of triangle across all precisions */
    if (list_i16 && list_f32 && list_q16) {
        nexrad_render_point p_i16 = list_i16->features[0]->points[0];
        nexrad_render_point p_f32 = list_f32->features[0]->points[0];
        nexrad_render_point p_q16 = list_q16->features[0]->points[0];

        printf("Point 0 Comparison:\n");
        printf("  INT16: %d, %d\n", p_i16.d.i16.x, p_i16.d.i16.y);
        printf("  FLOAT: %.4f, %.4f\n", p_f32.d.f32.x, p_f32.d.f32.y);
        printf("  FIXED: %d, %d (Raw Q16.16)\n", p_q16.d.q16.x, p_q16.d.q16.y);
    }

    /* Cleanup */
    nexrad_projected_feature_list_destroy(list_i16);
    nexrad_projected_feature_list_destroy(list_f32);
    nexrad_projected_feature_list_destroy(list_q16);
    nexrad_feature_list_destroy(features);
    nexrad_geo_projection_close(proj);
    nexrad_geo_spheroid_destroy(spheroid);
    remove("test_proj.bin");

    printf("Multi-precision projection test passed!\n");
    return 0;
}
