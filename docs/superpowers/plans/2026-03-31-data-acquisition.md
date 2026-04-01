# Data Acquisition & Station Metadata Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Embed a comprehensive NEXRAD station database into the library and provide utility functions for constructing AWS S3 URLs.

**Architecture:**
1.  **Station DB**: A static pre-sorted array of 160 stations in `src/station_data.c` using $O(\log n)$ binary search for lookups.
2.  **AWS Builder**: Stateless string-formatting functions in `src/aws.c` to build standardized NEXRAD S3 URLs.
3.  **Refactor**: Replace legacy 3-station hardcoded table in `util.c` with the new global database.

**Tech Stack:** C99, CMake.

---

### Task 1: Station Database Implementation

**Files:**
- Create: `include/nexrad/station.h`
- Create: `src/station_data.c`
- Create: `src/station.c`
- Modify: `src/CMakeLists.txt`

- [ ] **Step 1: Create `include/nexrad/station.h`**

```c
#ifndef _NEXRAD_STATION_H
#define _NEXRAD_STATION_H

typedef struct _nexrad_station_info {
    char   icao[5];
    char   name[32];
    char   state[3];
    double lat;
    double lon;
    double alt_meters;
} nexrad_station_info;

const nexrad_station_info *nexrad_station_lookup(const char *icao);
const nexrad_station_info *nexrad_station_get_all(int *count);

#endif
```

- [ ] **Step 2: Create `src/station_data.c`**

Generate a file containing the static array of 160 stations, pre-sorted alphabetically by ICAO. Convert altitude from feet to meters (feet * 0.3048).

- [ ] **Step 3: Create `src/station.c`**

Implement `nexrad_station_lookup` using `bsearch()` and `nexrad_station_get_all`.

- [ ] **Step 4: Update `src/CMakeLists.txt`**

Add `station.c` and `station_data.c` to `NEXRAD_SOURCES`.

- [ ] **Step 5: Compile and Commit**

```bash
cmake --build build
git add include/nexrad/station.h src/station.c src/station_data.c src/CMakeLists.txt
git commit -m "feat: implement comprehensive NEXRAD station database"
```

---

### Task 2: AWS URL Builder Implementation

**Files:**
- Create: `include/nexrad/aws.h`
- Create: `src/aws.c`
- Modify: `src/CMakeLists.txt`

- [ ] **Step 1: Create `include/nexrad/aws.h`**

```c
#ifndef _NEXRAD_AWS_H
#define _NEXRAD_AWS_H

#include <stddef.h>

int nexrad_aws_build_level2_url(char *dest, size_t destlen, const char *icao, int y, int m, int d, const char *filename);
int nexrad_aws_build_level3_url(char *dest, size_t destlen, const char *icao, const char *prod, int y, int m, int d, const char *filename);

#endif
```

- [ ] **Step 2: Implement logic in `src/aws.c`**

Implement string formatting using `snprintf` targeting the `unidata-nexrad-level2` and `unidata-nexrad-level3` S3 buckets.

- [ ] **Step 3: Update `src/CMakeLists.txt`**

Add `aws.c` to `NEXRAD_SOURCES`.

- [ ] **Step 4: Compile and Commit**

```bash
cmake --build build
git add include/nexrad/aws.h src/aws.c src/CMakeLists.txt
git commit -m "feat: implement AWS S3 URL builder utility"
```

---

### Task 3: Internal Refactor & Cleanup

**Files:**
- Modify: `src/util.c`
- Modify: `src/util.h`
- Modify: `src/message.c`

- [ ] **Step 1: Remove legacy lookup in `src/util.c`**

Remove `station_table` and the old `nexrad_station_lookup` implementation.

- [ ] **Step 2: Update `src/util.h`**

Remove old declaration of `nexrad_station_lookup`.

- [ ] **Step 3: Update `src/message.c`**

Include `<nexrad/station.h>` and update `nexrad_message_read_station_location` to use the new `nexrad_station_lookup`.

- [ ] **Step 4: Verify and Commit**

```bash
cmake --build build
git commit -a -m "refactor: replace legacy station table with global database"
```

---

### Task 4: Verification Test

**Files:**
- Create: `examples/station_test.c`
- Modify: `examples/CMakeLists.txt`

- [ ] **Step 1: Create `examples/station_test.c`**

Implement a test that:
1. Looks up `KATX` and verifies coords match `48.19472, -122.49583`.
2. Generates an AWS URL for a specific date and prints it.

- [ ] **Step 2: Update `examples/CMakeLists.txt`**

Add `station_test` to the `EXAMPLES` list.

- [ ] **Step 3: Run and Verify**

```bash
cmake --build build
./build/examples/station_test
```

- [ ] **Step 4: Commit**

```bash
git add examples/station_test.c examples/CMakeLists.txt
git commit -m "test: add station database and URL builder verification test"
```
