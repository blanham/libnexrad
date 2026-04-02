# Spec: Meteorological Features API (SPC Outlooks, NWS Warnings, & Indicators)

**Date:** 2026-04-01
**Status:** Draft
**Topic:** Integration of SPC outlooks, NWS warning polygons, and NEXRAD-derived indicators (TVS/Meso) into `libnexrad`.

## 1. Objective
Extend `libnexrad` to support the acquisition, parsing, and projection of non-radial meteorological data. This data should interoperate with existing radar projection and imaging logic, allowing users to overlay warnings, outlooks, and storm indicators onto radar products.

## 2. Architecture

### 2.1 Core Data Structures (`include/nexrad/feature.h`)
We will use a "Generic Feature API" to represent all non-radial data.

```c
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
    // Metadata (severity, product type, etc.)
    char *event_type; 
    uint32_t timestamp;
} nexrad_feature;

typedef struct {
    nexrad_feature **features;
    size_t count;
} nexrad_feature_list;
```

### 2.2 Data Acquisition & Ingestion
- **URL Builders (`aws.h`)**:
    - `nexrad_aws_build_spc_outlook_url()`: For Day 1/2/3 Convective Outlooks.
    - `nexrad_aws_build_nws_warning_url()`: For active SVR/TOR/FFW GeoJSON.
- **GeoJSON Parsing (`geojson.c`)**:
    - Uses `sheredom/json.h` to parse standard GeoJSON `FeatureCollection` objects into `nexrad_feature_list`.
- **Packet Conversion (`packet.c`)**:
    - `nexrad_packet_to_feature()`: Converts Level 3 packets (Product 61 TVS, Product 19 Hail) from radar-relative coordinates to geographic Lat/Lon features.

### 2.3 Projection & Rendering
- **Feature Projection (`geo.h`)**:
    - `nexrad_feature_list_project()`: Transforms a `nexrad_feature_list` (Lat/Lon) into a `nexrad_projected_feature_list` (Pixel X/Y) for a specific `nexrad_geo_projection`.
- **Drawing (`image.h`)**:
    - `nexrad_image_draw_features()`: Rasterizes lines and polygons directly onto a `nexrad_image` object.

## 3. Interoperability
- All meteorological features are decoupled from radar stations until the projection step.
- Features from different sources (NWS API, Level 3 Packets) can be combined into a single `nexrad_feature_list` and rendered together.

## 4. Dependencies
- **json.h**: Single-header JSON parser by sheredom.
- **zlib/bzip2**: Existing dependencies for packet decompression.

## 5. Success Criteria
- [ ] Successfully parse an SPC Day 1 Outlook GeoJSON file.
- [ ] Successfully convert a Level 3 TVS indicator (Product 61) into a geographic point.
- [ ] Render a "TOR" warning polygon over a radar image from `KATX` with correct geographic alignment.
