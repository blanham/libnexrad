#include <stdio.h>
#include <stdlib.h>
#include <nexrad/feature.h>

int main() {
    const char *json_data = "{"
        "\"type\": \"FeatureCollection\","
        "\"features\": ["
          "{"
            "\"type\": \"Feature\","
            "\"properties\": { \"NAME\": \"Slight Risk\", \"LABEL\": \"SLGT\" },"
            "\"geometry\": {"
              "\"type\": \"Polygon\","
              "\"coordinates\": ["
                "[[-98.0, 35.0], [-97.0, 35.0], [-97.0, 36.0], [-98.0, 36.0], [-98.0, 35.0]]"
              "]"
            "}"
          "}"
        "]"
      "}";

    nexrad_feature_list *list = nexrad_feature_list_from_geojson(json_data);
    if (!list) {
        fprintf(stderr, "Failed to parse GeoJSON\n");
        return 1;
    }

    printf("Parsed %zu features\n", list->count);

    for (size_t i = 0; i < list->count; i++) {
        nexrad_feature *f = list->features[i];
        printf("Feature %zu: ID=%u, Name=%s, Event=%s\n", i, f->id, f->name ? f->name : "N/A", f->event_type ? f->event_type : "N/A");
        if (f->geometry) {
            printf("  Geometry: Type=%d, Points=%zu\n", f->geometry->type, f->geometry->count);
            for (size_t j = 0; j < f->geometry->count; j++) {
                printf("    Point %zu: Lat=%f, Lon=%f\n", j, f->geometry->points[j].lat, f->geometry->points[j].lon);
            }
        }
    }

    nexrad_feature_list_destroy(list);

    return 0;
}
