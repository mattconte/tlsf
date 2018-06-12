#include <stdio.h>
#include <string.h>

#include <wlib/tlsf>

static char s_instance[512];
static char s_pool[2048];

struct data {
    int ival;
    double fval;
    char strval[32];
};

static void print_data(data *pData) {
    printf("ival: %i\nfval: %f\nstrval: %s\n",
        pData->ival, pData->fval, pData->strval);
}

int main(int argc, char *argv[]) {
    static char string[] = "Hello World";

    printf("control_t size: %lu\n", tlsf_size());
    printf("pool overhead: %lu\n", tlsf_pool_overhead());
    printf("alloc overhead: %lu\n", tlsf_alloc_overhead());

    tlsf_t instance = tlsf_create(s_instance);
    pool_t pool = tlsf_add_pool(instance, s_pool, 540);

    auto pData = static_cast<data *>(tlsf_malloc(instance, sizeof(data)));
    pData->ival = -672645;
    pData->fval = 684.23e4;
    strcpy(pData->strval, string);
    print_data(pData);
    tlsf_free(instance, pData);

    tlsf_destroy(instance);
}
