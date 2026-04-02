/*
 * Copyright (c) 2016-2026 Bryce Lanham. Distributed under the
 * terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef _NEXRAD_AWS_H
#define _NEXRAD_AWS_H

#include <stddef.h>

/**
 * @file nexrad/aws.h
 * @brief Public AWS S3 and NWS/SPC URL construction utilities.
 * @author Bryce Lanham
 */

/**
 * @brief Constructs a public AWS S3 URL for a NEXRAD Level II file.
 * 
 * Bucket: unidata-nexrad-level2
 * Path: YYYY/MM/DD/ICAO/filename
 * 
 * @param dest Destination buffer.
 * @param destlen Destination buffer length.
 * @param icao Station ICAO code (e.g., KATX).
 * @param year Year.
 * @param month Month.
 * @param day Day.
 * @param filename File name on S3.
 * @return 0 on success, -1 if buffer is too small.
 */
int nexrad_aws_build_level2_url(char *dest, size_t destlen, const char *icao, int year, int month, int day, const char *filename);

/**
 * @brief Constructs a public AWS S3 URL for a NEXRAD Level III file.
 * 
 * Bucket: unidata-nexrad-level3
 * Path: ICAO/PRODUCT/YYYY/MM/DD/filename
 * 
 * @param dest Destination buffer.
 * @param destlen Destination buffer length.
 * @param icao Station ICAO code.
 * @param product Product code (e.g., N0Q).
 * @param year Year.
 * @param month Month.
 * @param day Day.
 * @param filename File name on S3.
 * @return 0 on success, -1 if buffer is too small.
 */
int nexrad_aws_build_level3_url(char *dest, size_t destlen, const char *icao, const char *product, int year, int month, int day, const char *filename);

/**
 * @brief Constructs a URL for an SPC Day 1/2/3 Convective Outlook (GeoJSON).
 * 
 * @param dest Destination buffer.
 * @param destlen Destination buffer length.
 * @param day The outlook day (1, 2, or 3).
 * @return 0 on success, -1 if buffer too small or invalid day.
 */
int nexrad_aws_build_spc_outlook_url(char *dest, size_t destlen, int day);

/**
 * @brief Constructs a URL for active NWS alerts for a specific radar station.
 * 
 * @param dest Destination buffer.
 * @param destlen Destination buffer length.
 * @param icao The station ICAO code (e.g., KATX).
 * @return 0 on success, -1 if buffer too small.
 */
int nexrad_aws_build_nws_warning_url(char *dest, size_t destlen, const char *icao);

#endif /* _NEXRAD_AWS_H */
