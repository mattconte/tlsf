#include <wlib/tlsf>

#include <Cosa/Trace.hh>
#include <Cosa/UART.hh>

#include <stdarg.h>
#include <stdio.h>

static char control[512];
static char memory[2048];
static tlsf_t instance;
static pool_t pool;

static char *string;
static int counter;
static int wrt;

// Message handler
void tlsf_printf(const char *fmt, ...) {
    static char buffer[256];
    va_list argp;
    va_start(argp, fmt);
    vsprintf(buffer, fmt, argp);
    va_end(argp);
    trace << buffer;
    delay(50);
}

// Assert handler
void tlsf_assert(bool expr, const char *msg) {
    if (!expr) {
        trace << msg << endl;
        for (;;) { delay(50); }
    }
}

void setup() {
    uart.begin(19200);
    trace.begin(&uart);

    trace << "--------------------------------------------------------" << endl;
    trace << "TLSF size: " << tlsf_size() << endl;
    trace << "TLSF max: " << tlsf_block_size_max() << endl;

    trace << "Creating TLSF" << endl;
    delay(50);

    instance = tlsf_create(control);
    if (instance) {
        trace << "TLSF created" << endl;
    } else {
        trace << "Failed to create TLSF" << endl;
        return;
    }

    trace << "Setting up memory pool" << endl;
    delay(50);

    pool = tlsf_add_pool(instance, memory, 2048);
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
    tlsf_free(instance, ptr);
}

void loop() {
    wrt = snprintf(nullptr, 0, "Count: %i\n", counter);
    string = static_cast<char *>(tlsf_malloc(instance, wrt + 1));
    snprintf(string, wrt + 1, "Count: %i\n", counter);
    uart.write(string, wrt);
    tlsf_free(instance, string);

    delay(50);
    counter += 16 * 16;
}
