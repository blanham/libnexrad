#include <stdio.h>
#include <string.h>
#include "nexrad/aws.h"

static const char *LEVEL2_BUCKET_URL = "https://unidata-nexrad-level2.s3.amazonaws.com";
static const char *LEVEL3_BUCKET_URL = "https://unidata-nexrad-level3.s3.amazonaws.com";

int nexrad_aws_build_level2_url(char *dest, size_t destlen, const char *icao, int year, int month, int day, const char *filename) {
    if (!dest || !icao || !filename) return -1;

    int res = snprintf(dest, destlen, "%s/%04d/%02d/%02d/%s/%s",
                       LEVEL2_BUCKET_URL, year, month, day, icao, filename);

    if (res < 0 || (size_t)res >= destlen) {
        return -1; // Buffer too small or error
    }

    return 0;
}

int nexrad_aws_build_level3_url(char *dest, size_t destlen, const char *icao, const char *product, int year, int month, int day, const char *filename) {
    if (!dest || !icao || !product || !filename) return -1;

    int res = snprintf(dest, destlen, "%s/%s/%s/%04d/%02d/%02d/%s",
                       LEVEL3_BUCKET_URL, icao, product, year, month, day, filename);

    if (res < 0 || (size_t)res >= destlen) {
        return -1; // Buffer too small or error
    }

    return 0;
}
