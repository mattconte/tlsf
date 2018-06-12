#include <wlib/tlsf>

#include <Cosa/Trace.hh>
#include <Cosa/UART.hh>

#include <stdarg.h>
#include <stdio.h>

static char control[512];
static char memory[2048];
static tlsf_t instance;
static pool_t pool;

void tlsf_printf(const char *fmt, ...) {
    static char buffer[256];
    va_list argp;
    va_start(argp, fmt);
    vsprintf(buffer, fmt, argp);
    va_end(argp);
    trace << buffer;
    delay(50);
}

void tlsf_assert(bool expr, const char *msg) {
    if (!expr) {
        trace << msg << endl;
        for (;;) { delay(50); }
    }
}

void setup() {
    uart.begin(19200);
    trace.begin(&uart);

    memset(memory, 0, 2048);
    memset(control, 0, 512);

    trace << "--------------------------------------------------------" << endl;
    trace << "TLSF size: " << tlsf_size() << endl;
    trace << "TLSF max: " << tlsf_block_size_max() << endl;

    trace << "Creating TLSF" << endl;
    delay(50);

    char *addr;

    addr = control;
    if (((ptrdiff_t) addr) % 4 != 0) { addr += 2; }

    instance = tlsf_create(addr);
    if (instance) {
        trace << "TLSF created" << endl;
    } else {
        trace << "Failed to create TLSF" << endl;
        return;
    }

    trace << "Setting up memory pool" << endl;
    delay(50);


    addr = memory;
    if (((ptrdiff_t) addr) % 4 != 0) { addr += 2; }

    pool = tlsf_add_pool(instance, addr, 540);
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
