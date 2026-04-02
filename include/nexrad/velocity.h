/*
 * Copyright (c) 2013-2026 Bryce Lanham. Distributed under the
 * terms of the MIT license.
 */

#ifndef _NEXRAD_VELOCITY_H
#define _NEXRAD_VELOCITY_H

#include <stdint.h>

/**
 * @file nexrad/velocity.h
 * @brief NEXRAD Level III Velocity product conversion routines.
 * @author Bryce Lanham
 */

/**
 * @brief Determine if a velocity sample is valid.
 * @param v 8-bit velocity sample.
 * @return 1 if valid, 0 if invalid (e.g., range folded or below threshold).
 */
static inline int nexrad_velocity_valid(uint8_t v) {
    return (v > 1);
}

/**
 * @brief Determine if a velocity sample is range-folded.
 * @param v 8-bit velocity sample.
 * @return 1 if range-folded, 0 otherwise.
 */
static inline int nexrad_velocity_folded(uint8_t v) {
    return (v == 1);
}

/**
 * @brief Convert raw 8-bit velocity to meters per second.
 * 
 * Based on standard Level III scaling: (raw - 129) / 2.0
 * 
 * @param v 8-bit velocity sample.
 * @return Velocity in m/s.
 */
static inline double nexrad_velocity_ms(uint8_t v) {
    if (!nexrad_velocity_valid(v)) return 0.0;
    return (double)(v - 129) / 2.0;
}

/**
 * @brief Convert raw 8-bit velocity to knots.
 * @param v 8-bit velocity sample.
 * @return Velocity in knots.
 */
static inline double nexrad_velocity_kts(uint8_t v) {
    return nexrad_velocity_ms(v) * 1.94384;
}

#endif /* _NEXRAD_VELOCITY_H */
