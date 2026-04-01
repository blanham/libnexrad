# Projection Mapping Hooks Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement inverse projection hooks to allow external C/SDL applications to map geographic coordinates (lat/lon) onto radar projection pixels.

**Architecture:**
1.  **Inverse Math**: Add internal helper functions for reverse Equirectangular and Mercator projection.
2.  **Point Mapping**: Implement single-point and batch-point projection APIs.
3.  **Line Culling**: Implement a callback-based line segment projector that culled invisible segments.
4.  **Verification**: Add an integration test to verify the "Round Trip" accuracy (LatLon -> Pixel -> LatLon).

**Tech Stack:** C, POSIX, libmath.

---

### Task 1: Foundation & Single Point Mapping

**Files:**
- Modify: `include/nexrad/geo.h`
- Modify: `src/geo.c`

- [ ] **Step 1: Update `geo.h` with new types and single-point API**

Add to `include/nexrad/geo.h`:
```c
typedef struct _nexrad_geo_screen_point {
    int16_t x, y;
    int8_t  visible;
} nexrad_geo_screen_point;

int nexrad_geo_projection_latlon_to_pixel(
    nexrad_geo_projection *proj,
    double lat, double lon,
    int16_t *x, int16_t *y
);
```

- [ ] **Step 2: Implement inverse math helpers in `src/geo.c`**

Add internal static functions to `src/geo.c`:
```c
static void _equirect_find_xy(double lat, double lon, uint32_t world_width, uint32_t world_height, int *x, int *y) {
    *x = (int)round((world_width * (lon + 180.0)) / 360.0);
    *y = (int)round(world_height - (world_height * (lat + 90.0) / 180.0));
}

static void _mercator_find_xy(double lat, double lon, uint32_t world_size, int *x, int *y) {
    static const double rad = M_PI / 180.0;
    int cy = world_size / 2;
    *x = (int)round((double)world_size * ((lon + 180.0) / 360.0));
    double sinl = sin(lat * rad);
    double yrad = log((1.0 + sinl) / (1.0 - sinl)) / 2.0;
    *y = cy - (int)round(world_size * (yrad / (2.0 * M_PI)));
}
```

- [ ] **Step 3: Implement `nexrad_geo_projection_latlon_to_pixel`**

In `src/geo.c`:
```c
int nexrad_geo_projection_latlon_to_pixel(nexrad_geo_projection *proj, double lat, double lon, int16_t *x, int16_t *y) {
    if (!proj) return -1;
    uint16_t type = be16toh(proj->header->type);
    uint32_t world_w = be32toh(proj->header->world_width);
    uint32_t world_h = be32toh(proj->header->world_height);
    int wx, wy;

    if (type == NEXRAD_GEO_PROJECTION_EQUIRECT) _equirect_find_xy(lat, lon, world_w, world_h, &wx, &wy);
    else if (type == NEXRAD_GEO_PROJECTION_MERCATOR) _mercator_find_xy(lat, lon, world_w, &wx, &wy);
    else return -1;

    *x = (int16_t)(wx - be32toh(proj->header->world_offset_x));
    *y = (int16_t)(wy - be32toh(proj->header->world_offset_y));

    if (*x < 0 || *x >= be16toh(proj->header->width) || *y < 0 || *y >= be16toh(proj->header->height)) return -1;
    return 0;
}
```

- [ ] **Step 4: Compile and Commit**

Run: `make -C src`
```bash
git add include/nexrad/geo.h src/geo.c
git commit -m "feat: implement nexrad_geo_projection_latlon_to_pixel hook"
```

---

### Task 2: Implement Batch Point Projection

**Files:**
- Modify: `include/nexrad/geo.h`
- Modify: `src/geo.c`

- [ ] **Step 1: Declare `nexrad_geo_projection_project_points`**

Add to `include/nexrad/geo.h`:
```c
int nexrad_geo_projection_project_points(
    nexrad_geo_projection *proj,
    nexrad_geo_cartesian *geo_points,
    nexrad_geo_screen_point *screen_points,
    size_t count
);
```

- [ ] **Step 2: Implement batch logic in `src/geo.c`**

```c
int nexrad_geo_projection_project_points(nexrad_geo_projection *proj, nexrad_geo_cartesian *geo_points, nexrad_geo_screen_point *screen_points, size_t count) {
    if (!proj || !geo_points || !screen_points) return -1;
    for (size_t i = 0; i < count; i++) {
        int16_t x, y;
        int res = nexrad_geo_projection_latlon_to_pixel(proj, geo_points[i].lat, geo_points[i].lon, &x, &y);
        screen_points[i].x = x;
        screen_points[i].y = y;
        screen_points[i].visible = (res == 0);
    }
    return 0;
}
```

- [ ] **Step 3: Compile and Commit**

Run: `make -C src`
```bash
git add include/nexrad/geo.h src/geo.c
git commit -m "feat: implement nexrad_geo_projection_project_points batch hook"
```

---

### Task 3: Implement Line Segment Culling Helper

**Files:**
- Modify: `include/nexrad/geo.h`
- Modify: `src/geo.c`

- [ ] **Step 1: Declare callback and line projection API**

Add to `include/nexrad/geo.h`:
```c
typedef void (*nexrad_geo_line_cb)(int16_t x1, int16_t y1, int16_t x2, int16_t y2, void *user_data);

int nexrad_geo_projection_project_lines(
    nexrad_geo_projection *proj,
    nexrad_geo_cartesian *geo_points,
    size_t count,
    nexrad_geo_line_cb callback,
    void *user_data
);
```

- [ ] **Step 2: Implement line iterator with culling in `src/geo.c`**

```c
int nexrad_geo_projection_project_lines(nexrad_geo_projection *proj, nexrad_geo_cartesian *geo_points, size_t count, nexrad_geo_line_cb callback, void *user_data) {
    if (!proj || !geo_points || !callback || count < 2) return -1;
    
    int16_t px, py;
    int p_vis = (nexrad_geo_projection_latlon_to_pixel(proj, geo_points[0].lat, geo_points[0].lon, &px, &py) == 0);

    for (size_t i = 1; i < count; i++) {
        int16_t cx, cy;
        int c_vis = (nexrad_geo_projection_latlon_to_pixel(proj, geo_points[i].lat, geo_points[i].lon, &cx, &cy) == 0);
        
        if (p_vis || c_vis) {
            callback(px, py, cx, cy, user_data);
        }
        px = cx; py = cy; p_vis = c_vis;
    }
    return 0;
}
```

- [ ] **Step 3: Compile and Commit**

Run: `make -C src`
```bash
git add include/nexrad/geo.h src/geo.c
git commit -m "feat: implement nexrad_geo_projection_project_lines with culling"
```

---

### Task 4: Round-Trip Verification Test

**Files:**
- Create: `examples/proj_test.c`
- Modify: `examples/Makefile`

- [ ] **Step 1: Create `examples/proj_test.c`**

Implementation should:
1. Open a sample `.proj` file.
2. Pick a random pixel `(x, y)`.
3. Map `Pixel -> LatLon` using `nexrad_geo_projection_find_cartesian_point`.
4. Map `LatLon -> Pixel` using `nexrad_geo_projection_latlon_to_pixel`.
5. Assert that the resulting pixel matches the original pixel.

- [ ] **Step 2: Update `examples/Makefile`**

Add `proj_test` to the build list.

- [ ] **Step 3: Run and Verify**

Run: `make -C examples && ./examples/proj_test samples/some.proj`
Expected: "Round trip successful"

- [ ] **Step 4: Commit**

```bash
git add examples/proj_test.c examples/Makefile
git commit -m "test: add round-trip projection verification"
```
