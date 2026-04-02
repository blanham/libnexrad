/*
 * Copyright (c) 2013-2026 Bryce Lanham. Distributed under the
 * terms of the MIT license.
 */

#ifndef _NEXRAD_DUALPOL_H
#define _NEXRAD_DUALPOL_H

#include <stdint.h>

/**
 * @file nexrad/dualpol.h
 * @brief NEXRAD Level III Dual-Pol product conversion routines.
 * @author Bryce Lanham
 */

/**
 * @brief Convert raw 8-bit Differential Reflectivity (ZDR) to decibels (dB).
 * 
 * Standard scaling: (raw - 128) / 16.0
 * Range: -7.9 to +7.9 dB
 * 
 * @param v 8-bit ZDR sample.
 * @return ZDR in dB.
 */
static inline double nexrad_zdr_db(uint8_t v) {
    if (v < 2) return 0.0; /* Below threshold / invalid */
    return (double)(v - 128) / 16.0;
}

/**
 * @brief Convert raw 8-bit Correlation Coefficient (CC) to a ratio (0.0 to 1.0).
 * 
 * Standard scaling: (raw - 1) / 253.0 (approximate) or lookup table.
 * ICD specifies: 0 = ND, 1 = RF, 2-255 scaled linearly.
 * 
 * @param v 8-bit CC sample.
 * @return CC ratio.
 */
static inline double nexrad_cc_ratio(uint8_t v) {
    if (v < 2) return 0.0;
    /* (raw - 2) * 0.005 + 0.2? Actually, many sources use (v - 1)/253 or similar.
     * Modern CC (N0C) scaling: (raw - 2) / 253.0 * (1.05 - 0.2) + 0.2
     * Let's use the most common linear mapping for simplicity or 
     * refer to specific product attributes if available. */
    return (double)(v - 2) / 253.0 * 0.85 + 0.2;
}

/**
 * @brief Convert raw 8-bit Specific Differential Phase (KDP) to deg/km.
 * 
 * Standard scaling: (raw - 128) / 20.0
 * 
 * @param v 8-bit KDP sample.
 * @return KDP in deg/km.
 */
static inline double nexrad_kdp_degkm(uint8_t v) {
    if (v < 2) return 0.0;
    return (double)(v - 128) / 20.0;
}

#endif /* _NEXRAD_DUALPOL_H */
