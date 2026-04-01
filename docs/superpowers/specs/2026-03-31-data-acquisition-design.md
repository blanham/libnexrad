# Data Acquisition & Station Metadata (Phase 6) Design

## Purpose
Enhance `libnexrad` with a comprehensive, built-in database of all WSR-88D radar stations and provide utility functions to construct AWS S3 URLs for automated data acquisition.

## Architecture

### 1. Station Database (Metadata)
A complete database of all 160 operational NEXRAD stations will be embedded into the library to provide accurate coordinates and altitudes for geographic mapping.

**API (`include/nexrad/station.h`)**:
```c
typedef struct _nexrad_station_info {
    char icao[5];
    char name[32];
    char state[3];
    double lat;
    double lon;
    double alt_meters;
} nexrad_station_info;

const nexrad_station_info *nexrad_station_lookup(const char *icao);
const nexrad_station_info *nexrad_station_get_all(int *count);
```
- **Storage**: A static, pre-sorted C array in `src/station_data.c`.
- **Search**: $O(\log n)$ binary search by ICAO identifier.

### 2. AWS URL Builder (Acquisition)
Stateless utility functions to generate public S3 URLs for NEXRAD data, allowing external applications to fetch data using any HTTP client.

**API (`include/nexrad/aws.h`)**:
```c
int nexrad_aws_build_level2_url(char *dest, size_t destlen, const char *icao, int y, int m, int d, const char *file);
int nexrad_aws_build_level3_url(char *dest, size_t destlen, const char *icao, const char *prod, int y, int m, int d, const char *file);
```
- **Buckets**: Targets `unidata-nexrad-level2` and `unidata-nexrad-level3`.
- **Logic**: Handles date formatting and directory structure required by the Unidata/AWS archives.

## Implementation Details

- **Module**: New files `src/station.c`, `src/station_data.c`, and `src/aws.c`.
- **DRY Refactor**: The existing `nexrad_station_lookup` in `util.c` (currently containing only 3 stations) will be removed in favor of this comprehensive database.
- **Conversion**: Station altitudes will be stored in **meters**, converted from the source feet (1 ft = 0.3048 m).

## Verification Plan
- **Unit Tests**: Create `examples/station_test.c` to verify lookup speed and coordinate accuracy.
- **Round-Trip**: Verify that `nexrad_message_read_station_location` for Level II files matches the database entries exactly.
- **URL Verification**: Print constructed URLs and verify they resolve to valid data files via `curl`.
