
void putc(char c) {
    // Code to output a single character to your desired output (e.g., UART)
}

void puts(const char *str) {
    while (*str) {
        putc(*str++);
    }
}

void printf(const char *str) {
    while (*str) {
        if (*str == '\n') {
            putc('\r'); // For some terminals, you may need a carriage return
        }
        putc(*str++);
    }
}