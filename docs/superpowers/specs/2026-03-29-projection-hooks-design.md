# Projection Mapping Hooks (Phase 5) Design

## Purpose
Provide foundational hooks to allow external applications (e.g., SDL-based radar viewers) to map arbitrary geographic coordinates (Vector Maps, GPS tracks, Storm Cells) onto the current radar projection coordinate system.

## Architecture

### 1. Single Point Mapping (Approach A)
A simple math hook for mapping one location to a pixel coordinate.

**API**:
```c
int nexrad_geo_projection_latlon_to_pixel(
    nexrad_geo_projection *proj,
    double lat, double lon,
    int16_t *x, int16_t *y
);
```
- **Returns**: `0` if inside bounds, `-1` if outside.
- **Logic**: Performs inverse projection based on the type (Equirectangular or Mercator) and uses `world_offset` and `scale` to determine local image coordinates.

### 2. Batch Point Projection (Approach B)
High-performance batch transformation for large datasets (e.g., city lists).

**API**:
```c
typedef struct _nexrad_geo_screen_point {
    int16_t x, y;
    int8_t  visible;
} nexrad_geo_screen_point;

int nexrad_geo_projection_project_points(
    nexrad_geo_projection *proj,
    nexrad_geo_cartesian *geo_points,
    nexrad_geo_screen_point *screen_points,
    size_t count
);
```

### 3. Line Segment Projection (Culling Helper)
A specialized iterator for drawing continuous lines (boundaries, roads) with built-in culling of invisible segments.

**API**:
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
- **Optimization**: Skips segments where both points are outside the visible projection bounds.

## Implementation Details

- **Module**: All functions will be added to `src/geo.c` and `include/nexrad/geo.h`.
- **DRY**: The core math for `latlon_to_pixel` will be encapsulated in internal helper functions `_equirect_find_xy` and `_mercator_find_xy`.
- **Unit Testing**: A new example `examples/proj_test.c` will be created to verify that re-projecting a mapped pixel returns the original lat/lon (Round-trip verification).

## Backward Compatibility
- These functions are additive and do not break existing `.proj` file versions or Level III/II rasterization APIs.
