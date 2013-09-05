#include <stdlib.h>

#include <nexrad/symbology.h>

nexrad_chunk *nexrad_symbology_block_open(nexrad_symbology_block *block) {
    return nexrad_chunk_open(block, NEXRAD_CHUNK_SYMBOLOGY_BLOCK);
}

nexrad_chunk *nexrad_symbology_block_read_layer(nexrad_chunk *block) {
    return nexrad_chunk_read_block_layer(block, NEXRAD_CHUNK_SYMBOLOGY_LAYER);
}

void nexrad_symbology_layer_next_packet(nexrad_chunk *layer, size_t size) {
    nexrad_chunk_next(layer, size);
}

nexrad_packet *nexrad_symbology_layer_peek_packet(nexrad_chunk *layer, size_t *size) {
    return nexrad_chunk_peek(layer, size, NULL, NULL);
}

nexrad_packet *nexrad_symbology_layer_read_packet(nexrad_chunk *layer, size_t *size) {
    return nexrad_chunk_read(layer, size, NULL, NULL);
}

void nexrad_symbology_layer_close(nexrad_chunk *layer) {
    nexrad_chunk_close(layer);
}

void nexrad_symbology_block_close(nexrad_chunk *block) {
    nexrad_chunk_close(block);
}