/*
 * Copyright (c) 2025 Dynamic Weather Solutions, Inc. Distributed under the
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

#include <stdlib.h>
#include <string.h>
#include "util.h"

#include <nexrad/level2.h>

nexrad_level2_data_header *nexrad_level2_get_data_header(void *data, size_t size) {
    if (data == NULL || size < sizeof(nexrad_level2_data_header)) {
        return NULL;
    }

    return (nexrad_level2_data_header *)data;
}

void *nexrad_level2_get_block(nexrad_level2_data_header *header, const char *name) {
    if (header == NULL || name == NULL) {
        return NULL;
    }

    uint16_t count = be16toh(header->data_block_count);
    for (int i = 0; i < count && i < 9; i++) {
        uint32_t offset = be32toh(header->data_block_pointers[i]);
        if (offset == 0) continue;

        nexrad_level2_data_block *block = (nexrad_level2_data_block *)((char *)header + offset);
        if (strncmp(block->type, name, 3) == 0) {
            return (void *)block;
        }
    }

    return NULL;
}
