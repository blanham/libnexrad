# Meteorological Features API Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Integrate SPC outlooks, NWS warnings, and NEXRAD storm indicators into `libnexrad` using a generic geographic feature API.

**Architecture:** A decoupled "Generic Feature API" that stores data in Lat/Lon coordinates and projects them onto radar images on-demand. Uses `json.h` for GeoJSON parsing.

**Tech Stack:** C, `json.h` (sheredom), `libnexrad` existing geo/radar logic.

---

### Task 1: Core Data Structures and Lifecycle

**Files:**
- Create: `include/nexrad/feature.h`
- Create: `src/feature.c`
- Modify: `src/CMakeLists.txt`

- [ ] **Step 1: Define core structures in `feature.h`**

```c
#ifndef _NEXRAD_FEATURE_H
#define _NEXRAD_FEATURE_H

#include <nexrad/geo.h>

typedef enum {
    NEXRAD_GEOMETRY_POINT,
    NEXRAD_GEOMETRY_LINESTRING,
    NEXRAD_GEOMETRY_POLYGON
} nexrad_geometry_type;

typedef struct {
    nexrad_geometry_type type;
    nexrad_geo_cartesian *points;
    size_t count;
} nexrad_geometry;

typedef struct {
    char *id;
    char *name;
    nexrad_geometry *geometry;
    char *event_type;
    uint32_t timestamp;
} nexrad_feature;

typedef struct {
    nexrad_feature **features;
    size_t count;
} nexrad_feature_list;

nexrad_feature_list *nexrad_feature_list_create();
void nexrad_feature_list_destroy(nexrad_feature_list *list);
int nexrad_feature_list_add(nexrad_feature_list *list, nexrad_feature *feature);

#endif
```

- [ ] **Step 2: Implement lifecycle in `src/feature.c`**

```c
#include <stdlib.h>
#include <string.h>
#include <nexrad/feature.h>

nexrad_feature_list *nexrad_feature_list_create() {
    nexrad_feature_list *list = malloc(sizeof(nexrad_feature_list));
    list->features = NULL;
    list->count = 0;
    return list;
}

void nexrad_feature_list_destroy(nexrad_feature_list *list) {
    if (!list) return;
    for (size_t i = 0; i < list->count; i++) {
        nexrad_feature *f = list->features[i];
        if (f->id) free(f->id);
        if (f->name) free(f->name);
        if (f->event_type) free(f->event_type);
        if (f->geometry) {
            if (f->geometry->points) free(f->geometry->points);
            free(f->geometry);
        }
        free(f);
    }
    if (list->features) free(list->features);
    free(list);
}

int nexrad_feature_list_add(nexrad_feature_list *list, nexrad_feature *feature) {
    list->features = realloc(list->features, sizeof(nexrad_feature *) * (list->count + 1));
    list->features[list->count++] = feature;
    return 0;
}
```

- [ ] **Step 3: Update `src/CMakeLists.txt` and commit**

```cmake
# Add feature.c to the library sources
set(LIBNEXRAD_SOURCES
    ...
    src/feature.c
    ...
)
```

### Task 2: Add `json.h` Dependency

**Files:**
- Create: `include/json.h`

- [ ] **Step 1: Download/Add `sheredom/json.h`**
- [ ] **Step 2: Verify it compiles with a simple test**
- [ ] **Step 3: Commit**

### Task 3: GeoJSON Parsing

**Files:**
- Create: `src/geojson.c`
- Modify: `include/nexrad/feature.h`
- Modify: `src/CMakeLists.txt`

- [ ] **Step 1: Add GeoJSON parser declaration to `feature.h`**
```c
nexrad_feature_list *nexrad_feature_list_from_geojson(const char *json_data);
```
- [ ] **Step 2: Implement parser in `src/geojson.c` using `json.h`**
- [ ] **Step 3: Update `src/CMakeLists.txt` to include `src/geojson.c`**
- [ ] **Step 4: Write a test case using a sample SPC Outlook JSON**
- [ ] **Step 5: Commit**

### Task 4: URL Builders for SPC and NWS

**Files:**
- Modify: `include/nexrad/aws.h`
- Modify: `src/aws.c`

- [ ] **Step 1: Add `nexrad_aws_build_spc_outlook_url` and `nexrad_aws_build_nws_warning_url` to `aws.h`**
- [ ] **Step 2: Implement builders in `src/aws.c`**
- [ ] **Step 3: Verify URL generation in a test**
- [ ] **Step 4: Commit**

### Task 5: Level 3 Packet to Feature Conversion

**Files:**
- Modify: `include/nexrad/packet.h`
- Modify: `src/packet.c`

- [ ] **Step 1: Add `nexrad_packet_to_feature` declaration to `packet.h`**
- [ ] **Step 2: Implement conversion for Product 61 (TVS) and Product 19 (Hail) in `src/packet.c`**
- [ ] **Step 3: Write a test using a sample Level 3 product**
- [ ] **Step 4: Commit**

### Task 6: Feature Projection

**Files:**
- Modify: `include/nexrad/geo.h`
- Modify: `src/geo.c`

- [ ] **Step 1: Define `nexrad_projected_feature_list` in `geo.h`**
- [ ] **Step 2: Implement `nexrad_feature_list_project` in `src/geo.c` using `nexrad_geo_projection_latlon_to_pixel`**
- [ ] **Step 3: Verify projection with a known Lat/Lon and projection object**
- [ ] **Step 4: Commit**

### Task 7: Rendering Features onto Images

**Files:**
- Modify: `include/nexrad/image.h`
- Modify: `src/image.c`

- [ ] **Step 1: Implement `nexrad_image_draw_features` in `src/image.c`**
- [ ] **Step 2: Support drawing points (as markers) and polygons (as outlines)**
- [ ] **Step 3: Write an example script that renders a radar image with a warning overlay**
- [ ] **Step 4: Commit**

### Task 8: Verification and Documentation

- [ ] **Step 1: Run all tests**
- [ ] **Step 2: Update `README` and `ROADMAP.md`**
- [ ] **Step 3: Final commit**
