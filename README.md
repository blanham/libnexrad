# libnexrad

A high-performance C library for decoding, projecting, and visualizing NEXRAD Doppler radar data.

`libnexrad` provides a comprehensive toolkit for working with both **Level II** (base data) and **Level III** (derived products) NEXRAD files. It includes support for modern dual-pol moments, geographic projections, and integration with meteorological alert feeds.

## Features

- **Level II Support**: Decode Message Type 1 and Type 31 records with support for BZIP2 decompression.
- **Level III Support**: Full parsing of NIDS product files and symbology blocks.
- **Geographic Projections**: Built-in Equirectangular and Web Mercator projections with slant-range correction.
- **Meteorological Features**: API for parsing NWS Warnings and SPC Outlooks (GeoJSON) and storm indicators (TVS/Hail).
- **Visualization**: Fast rasterizer for creating map-aligned radar images (outputs to PNG).
- **AWS Integration**: Stateless URL builders for fetching historical data from public S3 buckets.
- **Station Database**: Embedded database of all 160 NEXRAD station locations and altitudes.

## Installation

### Dependencies

- **CMake** (3.15+)
- **ZLIB**
- **BZip2**
- **Math Library** (m)

### Building

```bash
mkdir build
cd build
cmake ..
make
```

## Quick Start

### Building an AWS URL

```c
char url[256];
nexrad_aws_build_level2_url(url, sizeof(url), "KATX", 2024, 4, 1, "KATX20240401_120000_V06");
```

### Parsing a Warning Polygon

```c
nexrad_feature_list *list = nexrad_feature_list_from_geojson(json_string);
// Project onto an existing radar projection
nexrad_projected_feature_list *projected = nexrad_feature_list_project(list, proj);
// Draw onto an image
nexrad_image_draw_features(img, projected, red_color);
```

## Documentation

Full documentation is generated via Doxygen. Run `doxygen Doxyfile` in the root directory to generate HTML documentation in `doc/html`.

## License

This project is licensed under the MIT License. See [LICENSE.md](LICENSE.md) for details.

---
Copyright (c) 2024-2026 Bryce Lanham

Based upon MIT licensed code:
Copyright (c) 2016 Dynamic Weather Solutions, Inc.

Original source code and documentaiton written by:

    Alexandra Hrefna Hilmisdóttir <xan@xan.email>

