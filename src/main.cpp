#include <tlsf.h>
#include <stdio.h>
#include <string.h>

enum {
    POOL_SIZE = 1024
};

static char s_pool[POOL_SIZE];

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

    printf("control_t size: %i\n", tlsf_size());
    printf("pool overhead: %i\n", tlsf_pool_overhead());
    printf("alloc overhead: %i\n", tlsf_alloc_overhead());

    tlsf_t instance = tlsf_create_with_pool(s_pool, POOL_SIZE);

    auto pData = static_cast<data *>(tlsf_malloc(instance, sizeof(data)));
    pData->ival = -672645;
    pData->fval = 684.23e4;
    strcpy(pData->strval, string);
    print_data(pData);
    tlsf_free(instance, pData);

    tlsf_destroy(instance);
}
