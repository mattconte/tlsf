#include <wlib/tlsf>

#include <Cosa/Trace.hh>
#include <Cosa/UART.hh>

static char control[500];
static char memory[1500];
static tlsf_t instance;
static pool_t pool;

void setup() {
    uart.begin(9600);
    trace.begin(&uart);

    trace << "Creating TLSF" << endl;
    instance = tlsf_create(control);
    if (instance) {
        trace << "TLSF created" << endl;
    } else {
        trace << "Failed to create TLSF" << endl;
        return;
    }

    trace << "Setting up memory pool" << endl;
    pool = tlsf_add_pool(instance, memory, 1500);
    if (pool) {
        trace << "Memory pool established" << endl;
    } else {
        trace << "Failed to establish memory pool" << endl;
        return;
    }

    char *ptr = (char *) tlsf_malloc(instance, 64);
    if (ptr) {
        trace << "Memory allocation successful" << endl;
    } else {
        trace << "Failed to allocate memory" << endl;
        return;
    }
}

void loop() {
    delay(100);
}
