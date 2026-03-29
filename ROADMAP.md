# libnexrad Roadmap

This document outlines the planned improvements for extending `libnexrad` to support Level II data and enhanced visualization.

## Phase 1: Level II Core Support (Completed)
- [x] Level II detection (AR2V/ARCHIVE2)
- [x] BZIP2 block decompression
- [x] Message Type 31 (Digital Radar Data) structure support
- [x] Record iteration API
- [x] Basic station identification for Level II

## Phase 2: Legacy & Moment Decoding (Completed)
- [x] **Legacy Support (Message Type 1):**
    - Implement structures for Message Type 1 radial data.
    - Add logic to detect and iterate through Type 1 records in older files.
- [x] **Moment Decoding:**
    - Implement helpers to convert raw data (8-bit/16-bit) to physical units.
    - Support reflectivity (dBZ), velocity (m/s), and spectrum width.
    - Support Dual-Pol moments (ZDR, PHI, RHO).

## Phase 3: Spatial Mapping & Projection (Completed)
- [x] **Polar to Geographic Projection:**
    - Use `geodesic.h` to project Level II polar bins to lat/lon.
    - Support elevation-corrected slant range calculations.
- [x] **Integration with `geo.h`:**
    - Ensure Level II data can be used with existing Level III projection logic.

## Phase 4: Visualization & Rendering (Completed)
- [x] **Core Rasterizer (DRY):**
    - Refactor Level III rendering to use a generic geographic rasterizer.
- [x] **Level II Rasterization:**
    - Implement a high-level API to generate projected images from Level II sweeps.
- [x] **Enhanced Examples:**
    - Create a Level II visualizer that renders moments with map overlays.

## Phase 5: Map Data
- [ ] **Vector Map Overlays:**
    - Integrate US State and County boundaries.
    - Integrate major highway datasets.
    - *Research:* Evaluate GeoJSON vs. SVG vs. custom binary format for efficient CLI/rendering performance.

## Phase 6: Data Acquisition
- [ ] **AWS Integration:**
    - Provide tools/scripts to fetch historical NEXRAD data from AWS S3 using `nexradaws`.
