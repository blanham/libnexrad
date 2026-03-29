# Level II Rasterization & Projection System DRY Refactor

## Purpose
Extend `libnexrad` to support rasterizing Level II moment data (Reflectivity, Velocity, etc.) into projected images, while refactoring the existing Level III projection logic to eliminate code duplication and support arbitrary azimuth resolutions.

## Architecture

### 1. Projection System Upgrade (Core)
The projection system will be updated to handle variable horizontal resolutions (azimuth counts), moving away from the hardcoded 0.1° (3600 rays) standard.

- **Header Changes (`include/nexrad/geo.h`)**:
    - Update `NEXRAD_GEO_PROJECTION_VERSION` to `0x02`.
    - Add `uint16_t azimuth_count` to `nexrad_geo_projection_header`.
- **API Changes**:
    - `nexrad_geo_projection_create_equirect` and `_create_mercator` will now accept an `azimuth_count` parameter.
- **Backward Compatibility**:
    - When opening a Version 1 projection file, the system will default to `azimuth_count = 3600`.

### 2. Generic Rasterization Logic (DRY Refactor)
A new internal/exported function in `src/geo.c` will handle the heavy lifting of mapping polar grids to Cartesian pixel buffers.

```c
nexrad_image *nexrad_geo_project_polar_grid(
    nexrad_geo_projection *proj, 
    uint8_t *grid, 
    uint16_t rays, 
    uint16_t bins, 
    nexrad_color_table *table
);
```

### 3. Level II Rasterization API
A high-level API will be added to `level2.c` to facilitate easy rendering of specific sweeps and moments.

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

**Data Flow**:
1. Allocate a flat 8-bit polar grid of size `rays * bins`.
2. Iterate through the Level II message, filtering by `sweep_num`.
3. Decode physical values for the specified `moment_name`.
4. Map `float` values to 8-bit color indices using `(val * scale) + offset`.
5. Pass the grid to `nexrad_geo_project_polar_grid`.

## Implementation Details

- **Scaling**: Standard reflectivity (dBZ) will use the provided `scale` and `offset` to map to the 0-255 range.
- **Transparency**: The output image is an RGBA buffer. Areas with no data or colors with alpha=0 will remain transparent, allowing for easy map compositing.
- **Refactoring**: `src/radial.c` (Level III) will be updated to call the new generic projection function.

## Verification Plan
- **Unit Tests**: Update `level2_info` example to generate a sample PNG.
- **Visual Check**: Verify generated PNGs have correct geometry and transparent backgrounds.
- **Compatibility**: Ensure Level III rasterization still works with legacy Version 1 `.proj` files.
