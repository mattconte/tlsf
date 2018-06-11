#include <wlib/tlsf>

#include <Cosa/Trace.hh>
#include <Cosa/UART.hh>

static char pool[1024];
static tlsf_t instance;

void setup() {
    instance = tlsf_create_with_pool(pool, 1024);
}

void loop() {}

