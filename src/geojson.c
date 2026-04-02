/*
 * Copyright (c) 2024 Dynamic Weather Solutions, Inc. Distributed under the
 * terms of the MIT license.
 */

#include <stdlib.h>
#include <string.h>
#include <nexrad/feature.h>

#define JSON_IMPLEMENTATION
#include <json.h>

static nexrad_geometry *parse_geometry(struct json_value_s *geometry_value) {
    struct json_object_s *geometry_obj = json_value_as_object(geometry_value);
    if (!geometry_obj) return NULL;

    struct json_string_s *type_str = NULL;
    struct json_value_s *coordinates_value = NULL;

    for (struct json_object_element_s *el = geometry_obj->start; el; el = el->next) {
        if (strcmp(el->name->string, "type") == 0) {
            type_str = json_value_as_string(el->value);
        } else if (strcmp(el->name->string, "coordinates") == 0) {
            coordinates_value = el->value;
        }
    }

    if (!type_str || !coordinates_value) return NULL;

    nexrad_geometry_type type;
    nexrad_geo_cartesian *points = NULL;
    size_t count = 0;

    if (strcmp(type_str->string, "Point") == 0) {
        type = NEXRAD_GEOMETRY_POINT;
        struct json_array_s *coords = json_value_as_array(coordinates_value);
        if (coords && coords->length >= 2) {
            count = 1;
            points = (nexrad_geo_cartesian *)malloc(sizeof(nexrad_geo_cartesian));
            struct json_array_element_s *el = coords->start;
            points[0].lon = atof(json_value_as_number(el->value)->number);
            el = el->next;
            points[0].lat = atof(json_value_as_number(el->value)->number);
        }
    } else if (strcmp(type_str->string, "LineString") == 0) {
        type = NEXRAD_GEOMETRY_LINESTRING;
        struct json_array_s *coords = json_value_as_array(coordinates_value);
        if (coords) {
            count = coords->length;
            points = (nexrad_geo_cartesian *)malloc(sizeof(nexrad_geo_cartesian) * count);
            size_t i = 0;
            for (struct json_array_element_s *el = coords->start; el; el = el->next, i++) {
                struct json_array_s *point_coords = json_value_as_array(el->value);
                if (point_coords && point_coords->length >= 2) {
                    struct json_array_element_s *pel = point_coords->start;
                    points[i].lon = atof(json_value_as_number(pel->value)->number);
                    pel = pel->next;
                    points[i].lat = atof(json_value_as_number(pel->value)->number);
                }
            }
        }
    } else if (strcmp(type_str->string, "Polygon") == 0) {
        type = NEXRAD_GEOMETRY_POLYGON;
        struct json_array_s *rings = json_value_as_array(coordinates_value);
        if (rings && rings->length > 0) {
            struct json_array_s *ring = json_value_as_array(rings->start->value);
            if (ring) {
                count = ring->length;
                points = (nexrad_geo_cartesian *)malloc(sizeof(nexrad_geo_cartesian) * count);
                size_t i = 0;
                for (struct json_array_element_s *el = ring->start; el; el = el->next, i++) {
                    struct json_array_s *point_coords = json_value_as_array(el->value);
                    if (point_coords && point_coords->length >= 2) {
                        struct json_array_element_s *pel = point_coords->start;
                        points[i].lon = atof(json_value_as_number(pel->value)->number);
                        pel = pel->next;
                        points[i].lat = atof(json_value_as_number(pel->value)->number);
                    }
                }
            }
        }
    } else {
        return NULL;
    }

    if (count == 0 || !points) {
        free(points);
        return NULL;
    }

    nexrad_geometry *geom = nexrad_geometry_create(type, points, count);
    free(points);
    return geom;
}

nexrad_feature_list *nexrad_feature_list_from_geojson(const char *json_data) {
    if (!json_data) return NULL;

    struct json_value_s *root = json_parse(json_data, strlen(json_data));
    if (!root) return NULL;

    struct json_object_s *root_obj = json_value_as_object(root);
    if (!root_obj) {
        free(root);
        return NULL;
    }

    struct json_array_s *features_array = NULL;
    for (struct json_object_element_s *el = root_obj->start; el; el = el->next) {
        if (strcmp(el->name->string, "features") == 0) {
            features_array = json_value_as_array(el->value);
            break;
        }
    }

    if (!features_array) {
        free(root);
        return NULL;
    }

    nexrad_feature_list *list = nexrad_feature_list_create();
    if (!list) {
        free(root);
        return NULL;
    }

    for (struct json_array_element_s *el = features_array->start; el; el = el->next) {
        struct json_object_s *feature_obj = json_value_as_object(el->value);
        if (!feature_obj) continue;

        uint32_t id = 0;
        char *name = NULL;
        char *event_type = NULL;
        nexrad_geometry *geometry = NULL;

        for (struct json_object_element_s *fel = feature_obj->start; fel; fel = fel->next) {
            if (strcmp(fel->name->string, "id") == 0) {
                struct json_number_s *id_num = json_value_as_number(fel->value);
                if (id_num) id = (uint32_t)atoi(id_num->number);
            } else if (strcmp(fel->name->string, "geometry") == 0) {
                geometry = parse_geometry(fel->value);
            } else if (strcmp(fel->name->string, "properties") == 0) {
                struct json_object_s *props_obj = json_value_as_object(fel->value);
                if (props_obj) {
                    for (struct json_object_element_s *pel = props_obj->start; pel; pel = pel->next) {
                        struct json_string_s *val_str = json_value_as_string(pel->value);
                        if (!val_str) continue;

                        if (strcmp(pel->name->string, "NAME") == 0 || 
                            strcmp(pel->name->string, "LABEL") == 0) {
                            if (!name) name = strndup(val_str->string, val_str->string_size);
                        } else if (strcmp(pel->name->string, "event") == 0) {
                            if (!event_type) event_type = strndup(val_str->string, val_str->string_size);
                        }
                    }
                }
            }
        }

        if (geometry) {
            nexrad_feature *feature = nexrad_feature_create(id, name, geometry, event_type, 0);
            if (feature) {
                nexrad_feature_list_add(list, feature);
            } else {
                nexrad_geometry_destroy(geometry);
            }
        }
        
        free(name);
        free(event_type);
    }

    free(root);
    return list;
}
