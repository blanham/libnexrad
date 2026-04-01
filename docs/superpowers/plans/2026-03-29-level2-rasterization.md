# Level II Rasterization & Projection System Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Extend `libnexrad` to support rasterizing Level II moment data into projected images while refactoring the projection system to support arbitrary azimuth counts and sharing logic across radar levels.

**Architecture:**
1.  **Core Upgrade**: Modify `nexrad_geo_projection` to store `azimuth_count` and bump version to `0x02`.
2.  **DRY Refactor**: Extract the projection-lookup loop from `src/radial.c` into a generic `nexrad_geo_project_polar_grid` in `src/geo.c`.
3.  **Level II Feature**: Implement `nexrad_level2_create_projected_image` which builds a polar grid and calls the generic rasterizer.

**Tech Stack:** C, POSIX (mmap, stat), BZIP2, libz, libmath.

---

### Task 1: Update Projection Header & API

**Files:**
- Modify: `include/nexrad/geo.h`
- Modify: `src/geo.c`

- [ ] **Step 1: Update header structure and version**

Modify `include/nexrad/geo.h`:
```c
#define NEXRAD_GEO_PROJECTION_VERSION 0x02

typedef struct _nexrad_geo_projection_header {
    char     magic[4];
    uint16_t version;
    uint16_t type;
    uint16_t width;
    uint16_t height;
    uint32_t world_width;
    uint32_t world_height;
    uint32_t world_offset_x;
    uint32_t world_offset_y;
    uint16_t rangebins;
    uint16_t rangebin_meters;
    uint16_t azimuth_count; // New field replaces 'angle' or adds to it
    uint16_t angle;         // Keep for alignment/backcompat
    // ... rest unchanged ...
```

- [ ] **Step 2: Update creation API signatures**

Modify `include/nexrad/geo.h`:
```c
nexrad_geo_projection *nexrad_geo_projection_create_equirect(
    const char *path,
    nexrad_geo_spheroid *spheroid,
    nexrad_geo_cartesian *radar,
    uint16_t rangebins,
    uint16_t rangebin_meters,
    uint16_t azimuth_count, // Added
    double scale
);

nexrad_geo_projection *nexrad_geo_projection_create_mercator(
    const char *path,
    nexrad_geo_spheroid *spheroid,
    nexrad_geo_cartesian *radar,
    uint16_t rangebins,
    uint16_t rangebin_meters,
    uint16_t azimuth_count, // Added
    int zoom
);
```

- [ ] **Step 3: Update `src/geo.c` implementation to use `azimuth_count`**

In `nexrad_geo_projection_create_mercator` and `equirect`:
```c
    proj->header->version         = htobe16(NEXRAD_GEO_PROJECTION_VERSION);
    proj->header->azimuth_count   = htobe16(azimuth_count);
    // ... in the loop ...
    azimuth = (int)round(polar.azimuth * (azimuth_count / 360.0));
    while (azimuth >= azimuth_count) azimuth -= azimuth_count;
    while (azimuth < 0) azimuth += azimuth_count;
```

- [ ] **Step 4: Implement backward compatibility in `_projection_open`**

In `src/geo.c`:
```c
    if (be16toh(proj->header->version) == 0x01) {
        proj->header->azimuth_count = htobe16(3600); // Default for v1
    }
```

- [ ] **Step 5: Compile and Commit**

Run: `make -C src`
```bash
git add include/nexrad/geo.h src/geo.c
git commit -m "feat: upgrade projection system to version 0.2 with arbitrary azimuth support"
```

---

### Task 2: Implement Generic Rasterizer (DRY)

**Files:**
- Modify: `include/nexrad/geo.h`
- Modify: `src/geo.c`

- [ ] **Step 1: Declare `nexrad_geo_project_polar_grid`**

Add to `include/nexrad/geo.h`:
```c
nexrad_image *nexrad_geo_project_polar_grid(
    nexrad_geo_projection *proj,
    uint8_t *grid,
    uint16_t rays,
    uint16_t bins,
    nexrad_color_table *table
);
```

- [ ] **Step 2: Implement logic in `src/geo.c`**

```c
nexrad_image *nexrad_geo_project_polar_grid(nexrad_geo_projection *proj, uint8_t *grid, uint16_t rays, uint16_t bins, nexrad_color_table *table) {
    nexrad_image *image;
    nexrad_color *entries;
    uint16_t x, y, width, height;

    if (!proj || !grid || !table) return NULL;

    entries = nexrad_color_table_get_entries(table, NULL);
    nexrad_geo_projection_read_dimensions(proj, &width, &height);
    image = nexrad_image_create(width, height);

    for (y=0; y<height; y++) {
        for (x=0; x<width; x++) {
            nexrad_geo_projection_point *point = &proj->points[y*width+x];
            int azimuth = (int)be16toh(point->azimuth);
            int range   = (int)be16toh(point->range);

            if (azimuth < rays && range < bins) {
                uint8_t value = grid[azimuth * bins + range];
                nexrad_color color = entries[value];
                if (color.a) nexrad_image_draw_pixel(image, color, x, y);
            }
        }
    }
    return image;
}
```

- [ ] **Step 3: Refactor Level III to use it**

Modify `src/radial.c`, `nexrad_radial_create_projected_image`:
```c
    // ... after unpacking ...
    image = nexrad_geo_project_polar_grid(proj, (uint8_t *)(buffer + 1), buffer->rays, buffer->bins, table);
    free(buffer);
    return image;
```

- [ ] **Step 4: Compile and Commit**

Run: `make -C src`
```bash
git add include/nexrad/geo.h src/geo.c src/radial.c
git commit -m "refactor: extract generic projection rasterizer to geo.c"
```

---

### Task 3: Implement Level II Rasterization API

**Files:**
- Modify: `include/nexrad/level2.h`
- Modify: `src/level2.c`

- [ ] **Step 1: Declare API**

Add to `include/nexrad/level2.h`:
```c
nexrad_image *nexrad_level2_create_projected_image(
    nexrad_message *message,
    const char *moment_name,
    int sweep_num,
    nexrad_color_table *table,
    nexrad_geo_projection *proj,
    float scale,
    float offset
);
```

- [ ] **Step 2: Implement moment-to-grid extraction**

In `src/level2.c`:
```c
nexrad_image *nexrad_level2_create_projected_image(...) {
    uint16_t rays = be16toh(proj->header->azimuth_count);
    uint16_t bins = be16toh(proj->header->rangebins);
    uint8_t *grid = calloc(rays, bins);
    
    // Iterate radials, find sweep, decode moment to grid
    // ... implementation ...
    
    nexrad_image *img = nexrad_geo_project_polar_grid(proj, grid, rays, bins, table);
    free(grid);
    return img;
}
```

- [ ] **Step 3: Compile and Commit**

Run: `make -C src`
```bash
git add include/nexrad/level2.h src/level2.c
git commit -m "feat: implement Level 2 projected image creation API"
```

---

### Task 4: Integration Test (Example Update)

**Files:**
- Modify: `examples/level2_info.c`

- [ ] **Step 1: Update example to generate a PNG**

Add logic to create a projection, load a color table, and call `nexrad_level2_create_projected_image`.

- [ ] **Step 2: Run and Verify**

Run: `make -C examples && ./examples/level2_info samples/Level2_KATX_20130717_1950.ar2v`
Verify: `output.png` exists and contains radar data.

- [ ] **Step 3: Commit**

```bash
git add examples/level2_info.c
git commit -m "test: add rasterization demo to level2_info example"
```
