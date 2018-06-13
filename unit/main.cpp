#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <wlib/tlsf>

static char s_instance[512];
static char s_pool[2048];

struct data {
    int ival;
    double fval;
    char strval[32];
};

struct int128 {
    int64_t first;
    int64_t second;
};

static void print_data(data *pData) {
    printf("ival: %i\nfval: %f\nstrval: %s\n",
        pData->ival, pData->fval, pData->strval);
}

int main(int argc, char *argv[]) {
    static char string[] = "Hello World";

    printf("control_t size: %u\n", static_cast<unsigned int>(tlsf_size()));
    printf("pool overhead: %u\n", static_cast<unsigned int>(tlsf_pool_overhead()));
    printf("alloc overhead: %u\n", static_cast<unsigned int>(tlsf_alloc_overhead()));

    tlsf_t instance = tlsf_create(s_instance);
    pool_t pool = tlsf_add_pool(instance, s_pool, 2048);

    auto pData = static_cast<data *>(tlsf_malloc(instance, sizeof(data)));
    pData->ival = -672645;
    pData->fval = 684.23e4;
    strcpy(pData->strval, string);
    print_data(pData);
    tlsf_free(instance, pData);

    void *data512 = tlsf_malloc(instance, 512);
    if (!data512) { return -1; }
    tlsf_free(instance, data512);

    static int128 *data128[32] = {nullptr};
    for (auto &i : data128) {
        i = static_cast<int128 *>(tlsf_malloc(instance, sizeof(int128)));
        if (!i) { return -1; }
    }
    for (auto &i : data128) {
        tlsf_free(instance, i);
    }

    tlsf_destroy(instance);
}
