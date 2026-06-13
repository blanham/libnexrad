/* Regression for the uncompressed-AR2V0006 iterator (commit 5b59135): the iterator returned 0 records
 * on valid uncompressed Archive II volumes. Conditional on a real sample (the big volumes are not
 * committed): set NEXRAD_L2_SAMPLE to an uncompressed _V06 volume (e.g. the Moore-2013 KTLX volume).
 * Asserts the iterator yields Message-31 records and a clean EOF. */
#include <stdio.h>
#include <stdlib.h>
#include <nexrad/message.h>
#include <nexrad/level2.h>

int main(void) {
    const char *path = getenv("NEXRAD_L2_SAMPLE");
    nexrad_message *msg;
    nexrad_level2_message_header *hdr; void *data; size_t size;
    long m31 = 0; int rc;
    if (path == NULL) { printf("SKIP test_level2 (set NEXRAD_L2_SAMPLE)\n"); return 0; }
    msg = nexrad_message_open(path);
    if (msg == NULL) { fprintf(stderr, "open failed: %s\n", path); return 1; }
    if (nexrad_message_get_level(msg) != NEXRAD_LEVEL_2) { fprintf(stderr, "not level 2\n"); return 1; }
    while ((rc = nexrad_message_next_level2_record(msg, &hdr, &data, &size)) > 0)
        if (hdr->type == 31) ++m31;
    nexrad_message_close(msg);
    if (rc < 0) { fprintf(stderr, "iterator error rc=%d\n", rc); return 1; }
    printf("test_level2: %ld Message-31 records\n", m31);
    if (m31 < 100) { fprintf(stderr, "FAIL: expected many M31 records, got %ld (0-records bug?)\n", m31); return 1; }
    return 0;
}
