/*
 * Copyright (c) 2016 Dynamic Weather Solutions, Inc. Distributed under the
 * terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <nexrad/feature.h>

nexrad_feature_list *nexrad_feature_list_create() {
    nexrad_feature_list *list = (nexrad_feature_list *)malloc(sizeof(nexrad_feature_list));
    if (!list) return NULL;

    list->features = NULL;
    list->count = 0;
    list->capacity = 0;

    return list;
}

void nexrad_feature_list_destroy(nexrad_feature_list *list) {
    if (!list) return;

    for (size_t i = 0; i < list->count; i++) {
        nexrad_feature_destroy(list->features[i]);
    }

    free(list->features);
    free(list);
}

void nexrad_projected_feature_list_destroy(nexrad_projected_feature_list *list) {
    if (!list) return;

    for (size_t i = 0; i < list->count; i++) {
        if (list->features[i]) {
            free(list->features[i]->points);
            free(list->features[i]);
        }
    }

    free(list->features);
    free(list);
}

int nexrad_feature_list_add(nexrad_feature_list *list, nexrad_feature *feature) {
    if (!list || !feature) return -1;

    if (list->count >= list->capacity) {
        size_t new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        nexrad_feature **new_features = (nexrad_feature **)realloc(list->features, sizeof(nexrad_feature *) * new_capacity);
        if (!new_features) return -1;

        list->features = new_features;
        list->capacity = new_capacity;
    }

    list->features[list->count++] = feature;

    return 0;
}

nexrad_geometry *nexrad_geometry_create(nexrad_geometry_type type, nexrad_geo_cartesian *points, size_t count) {
    nexrad_geometry *geometry = (nexrad_geometry *)malloc(sizeof(nexrad_geometry));
    if (!geometry) return NULL;

    geometry->type = type;
    geometry->count = count;

    if (points && count > 0) {
        geometry->points = (nexrad_geo_cartesian *)malloc(sizeof(nexrad_geo_cartesian) * count);
        if (!geometry->points) {
            free(geometry);
            return NULL;
        }
        memcpy(geometry->points, points, sizeof(nexrad_geo_cartesian) * count);
    } else {
        geometry->points = NULL;
    }

    return geometry;
}

void nexrad_geometry_destroy(nexrad_geometry *geometry) {
    if (!geometry) return;

    free(geometry->points);
    free(geometry);
}

nexrad_feature *nexrad_feature_create(uint32_t id, const char *name, nexrad_geometry *geometry, const char *event_type, uint32_t timestamp) {
    nexrad_feature *feature = (nexrad_feature *)malloc(sizeof(nexrad_feature));
    if (!feature) return NULL;

    feature->id = id;
    feature->timestamp = timestamp;
    feature->geometry = geometry;

    feature->name = name ? strdup(name) : NULL;
    feature->event_type = event_type ? strdup(event_type) : NULL;

    return feature;
}

void nexrad_feature_destroy(nexrad_feature *feature) {
    if (!feature) return;

    free(feature->name);
    free(feature->event_type);
    nexrad_geometry_destroy(feature->geometry);
    free(feature);
}
