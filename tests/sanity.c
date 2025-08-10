#include <stdio.h>
#include "vv_dsp/vv_dsp.h"

int main(void) {
    int ok = 1;
    ok &= (vv_dsp_add_int(2, 3) == 5);
    ok &= (vv_dsp_spectral_dummy() == 42);
    ok &= (vv_dsp_filter_dummy() == 7);
    ok &= (vv_dsp_resample_dummy() == 3);
    ok &= (vv_dsp_envelope_dummy() == 5);
    ok &= (vv_dsp_window_dummy() == 11);
    ok &= (vv_dsp_adapters_dummy() == 1);

    if(!ok) {
        fprintf(stderr, "sanity test failed\n");
        return 1;
    }
    printf("sanity test passed\n");
    return 0;
}
