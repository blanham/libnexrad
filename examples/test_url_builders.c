#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "nexrad/aws.h"

int main() {
    char url[256];
    int res;

    // Test SPC Day 1
    res = nexrad_aws_build_spc_outlook_url(url, sizeof(url), 1);
    assert(res == 0);
    assert(strcmp(url, "https://www.spc.noaa.gov/products/outlook/day1otlk_cat.geojson") == 0);
    printf("SPC Day 1: %s\n", url);

    // Test SPC Day 2
    res = nexrad_aws_build_spc_outlook_url(url, sizeof(url), 2);
    assert(res == 0);
    assert(strcmp(url, "https://www.spc.noaa.gov/products/outlook/day2otlk_cat.geojson") == 0);
    printf("SPC Day 2: %s\n", url);

    // Test SPC Day 3
    res = nexrad_aws_build_spc_outlook_url(url, sizeof(url), 3);
    assert(res == 0);
    assert(strcmp(url, "https://www.spc.noaa.gov/products/outlook/day3otlk_cat.geojson") == 0);
    printf("SPC Day 3: %s\n", url);

    // Test SPC Invalid Day
    res = nexrad_aws_build_spc_outlook_url(url, sizeof(url), 4);
    assert(res == -1);

    // Test NWS Warning
    res = nexrad_aws_build_nws_warning_url(url, sizeof(url), "KATX");
    assert(res == 0);
    assert(strcmp(url, "https://api.weather.gov/alerts/active?radarStation=KATX") == 0);
    printf("NWS Warning: %s\n", url);

    // Test buffer too small
    res = nexrad_aws_build_nws_warning_url(url, 20, "KATX");
    assert(res == -1);

    printf("All URL builder tests passed!\n");
    return 0;
}
