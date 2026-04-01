#ifndef _NEXRAD_AWS_H
#define _NEXRAD_AWS_H

#include <stddef.h>

/**
 * @brief Constructs a public AWS S3 URL for a NEXRAD Level II file.
 * 
 * Bucket: unidata-nexrad-level2
 * Path: YYYY/MM/DD/ICAO/filename
 * 
 * @return 0 on success, -1 if buffer is too small.
 */
int nexrad_aws_build_level2_url(char *dest, size_t destlen, const char *icao, int year, int month, int day, const char *filename);

/**
 * @brief Constructs a public AWS S3 URL for a NEXRAD Level III file.
 * 
 * Bucket: unidata-nexrad-level3
 * Path: ICAO/PRODUCT/YYYY/MM/DD/filename
 * 
 * @return 0 on success, -1 if buffer is too small.
 */
int nexrad_aws_build_level3_url(char *dest, size_t destlen, const char *icao, const char *product, int year, int month, int day, const char *filename);

#endif /* _NEXRAD_AWS_H */
