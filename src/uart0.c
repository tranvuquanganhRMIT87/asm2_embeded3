#include "uart0.h"
#include "../mbox/mbox.h"


#define GPIO_PIN_CTS 16
#define GPIO_PIN_RTS 17
#define GPIO_FUNC_ALT3 0x7

#define GPIO_BASE 0x3F200000 // GPIO controller base address

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
	unsigned int r;

	/* Turn off UART0 */
	UART0_CR = 0x0;

	/* NEW: set up UART clock for consistent divisor values
	--> may not work with QEMU, but will work with real board */
	mBuf[0] = 9 * 4;
	mBuf[1] = MBOX_REQUEST;
	mBuf[2] = MBOX_TAG_SETCLKRATE; // set clock rate
	mBuf[3] = 12;				   // Value buffer size in bytes
	mBuf[4] = 0;				   // REQUEST CODE = 0
	mBuf[5] = 2;				   // clock id: UART clock
	mBuf[6] = 4000000;			   // rate: 4Mhz
	mBuf[7] = 0;				   // clear turbo
	mBuf[8] = MBOX_TAG_LAST;
	mbox_call(ADDR(mBuf), MBOX_CH_PROP);

	/* Setup GPIO pins 14 and 15 */

	/* Set GPIO14 and GPIO15 to be pl011 TX/RX which is ALT0	*/
	r = GPFSEL1;
	r &= ~((7 << 12) | (7 << 15));		// clear bits 17-12 (FSEL15, FSEL14)
	r |= (0b100 << 12) | (0b100 << 15); // Set value 0b100 (select ALT0: TXD0/RXD0)
	GPFSEL1 = r;

	/* enable GPIO 14, 15 */

	/* Configure GPIO pins for CTS and RTS */
    // Assuming GPIO_PIN_CTS and GPIO_PIN_RTS are correctly defined elsewhere in your code
    custome_gpio_set_function(GPIO_PIN_CTS, GPIO_FUNC_ALT3); // Configure pin for CTS
    custome_gpio_set_function(GPIO_PIN_RTS, GPIO_FUNC_ALT3); // Configure pin for RTS
#ifdef RPI3	   // RBP3
	GPPUD = 0; // No pull up/down control
	// Toogle clock to flush GPIO setup
	r = 150;
	while (r--)
	{
		asm volatile("nop");
	}								   // waiting 150 cycles
	GPPUDCLK0 = (1 << 14) | (1 << 15); // enable clock for GPIO 14, 15
	r = 150;
	while (r--)
	{
		asm volatile("nop");
	}			   // waiting 150 cycles
	GPPUDCLK0 = 0; // flush GPIO setup

#else // RPI4
	r = GPIO_PUP_PDN_CNTRL_REG0;
	r &= ~((3 << 28) | (3 << 30)); // No resistor is selected for GPIO 14, 15
	GPIO_PUP_PDN_CNTRL_REG0 = r;
#endif

	/* Mask all interrupts. */
	UART0_IMSC = 0;

	/* Clear pending interrupts. */
	UART0_ICR = 0x7FF;

	/* Set integer & fractional part of Baud rate
	Divider = UART_CLOCK/(16 * Baud)
	Default UART_CLOCK = 48MHz (old firmware it was 3MHz);
	Integer part register UART0_IBRD  = integer part of Divider
	Fraction part register UART0_FBRD = (Fractional part * 64) + 0.5 */

	// 115200 baud
	//  UART0_IBRD = 26;
	//  UART0_FBRD = 3;

	// NEW: with UART_CLOCK = 4MHz as set by mailbox:
	// 115200 baud
	UART0_IBRD = 26;
	UART0_FBRD = 3;

	/* Set up the Line Control Register */
	/* Enable FIFO */
	/* Set length to 8 bit */
	/* Defaults for other bit are No parity, 1 stop bit */
	UART0_LCRH = UART0_LCRH_FEN | UART0_LCRH_WLEN_8BIT;

	/* Enable UART0, receive, and transmit */
	UART0_CR = 0x301; // enable Tx, Rx, FIFO
}

/**
 * Send a character
 */
void uart_sendc(char c)
{

	/* Check Flags Register */
	/* And wait until transmitter is not full */
	do
	{
		asm volatile("nop");
	} while (UART0_FR & UART0_FR_TXFF);

	/* Write our data byte out to the data register */
	UART0_DR = c;
}

/**
 * Receive a character
 */
char uart_getc()
{
	char c = 0;

	/* Check Flags Register */
	/* Wait until Receiver is not empty
	 * (at least one byte data in receive fifo)*/
	do
	{
		asm volatile("nop");
	} while (UART0_FR & UART0_FR_RXFE);

	/* read it and return */
	c = (unsigned char)(UART0_DR);

	/* convert carriage return to newline */
	return (c == '\r' ? '\n' : c);
}

/**
 * Display a string
 */
void uart_puts(char *s)
{
	while (*s)
	{
		/* convert newline to carriage return + newline */
		if (*s == '\n')
			uart_sendc('\r');
		uart_sendc(*s++);
	}
}

/**
 * Display a value in hexadecimal format
 */
void uart_hex(unsigned int num)
{
	uart_puts("0x");
	for (int pos = 28; pos >= 0; pos = pos - 4)
	{

		// Get highest 4-bit nibble
		char digit = (num >> pos) & 0xF;

		/* Convert to ASCII code */
		// 0-9 => '0'-'9', 10-15 => 'A'-'F'
		digit += (digit > 9) ? (-10 + 'A') : '0';
		uart_sendc(digit);
	}
}

/*
**
* Display a value in decimal format
*/
void uart_dec(int num)
{
	// A string to store the digit characters
	char str[33] = "";

	// Calculate the number of digits
	int len = 1;
	int temp = num;
	while (temp >= 10)
	{
		len++;
		temp = temp / 10;
	}

	// Store into the string and print out
	for (int i = 0; i < len; i++)
	{
		int digit = num % 10; // get last digit
		num = num / 10;		  // remove last digit from the number
		str[len - (i + 1)] = digit + '0';
	}
	str[len] = '\0';

	uart_puts(str);
}

#include "uart0.h"

// Add a function to set the baud rate
void uart_set_baud_rate(unsigned int baud_rate)
{
	unsigned int uart_clk = 48000000; // Your UART clock frequency
	unsigned int baud_divisor = uart_clk / (16 * baud_rate);
	unsigned int fractional = ((uart_clk * 64) / (16 * baud_rate)) - (baud_divisor * 64);

	// Disable the UART
	UART0_CR = 0;

	// Write to the IBRD and FBRD registers
	UART0_IBRD = baud_divisor;
	UART0_FBRD = fractional;
	uart_puts("Baud rate set to: ");
	uart_dec(baud_rate);
	// uart_puts("IBRD: ");
	// uart_puts(UART0_IBRD);
	// uart_puts("\n");
	// uart_puts("FBRD: ");
	// uart_puts(UART0_FBRD);
	// Enable the UART (assuming TXE and RXE are bits 8 and 9 in the CR register)
	UART0_CR = (1 << 9) | (1 << 8) | (1 << 0);
}

void uart_print_baud_rate()
{
	unsigned int uart_clk = 48000000; // The UART clock frequency you are using
	unsigned int ibrd = UART0_IBRD;	  // Read the integral part of the baud rate divisor
	unsigned int fbrd = UART0_FBRD;	  // Read the fractional part of the baud rate divisor

	// Recalculate the baud rate
	unsigned int baud_rate = uart_clk / (16 * (ibrd + fbrd / 64.0));

	// Print the baud rate
	uart_puts("Baud rate: ");
	uart_dec(baud_rate); // Assuming this function exists to print decimal numbers
	uart_puts("\n");

	// Print the IBRD and FBRD values
	uart_puts("IBRD: ");
	uart_dec(ibrd); // Assuming this function exists to print decimal numbers
	uart_puts("\n");
	uart_puts("FBRD: ");
	uart_dec(fbrd); // Assuming this function exists to print decimal numbers
	uart_puts("\n");
}

void uart_set_data_bits(unsigned int data_bits)
{
	UART0_CR &= ~UART0_CR_UARTEN;

	// Read current LCRH value and mask out word length bits
	unsigned int lcrh = UART0_LCRH & ~(UART0_LCRH_WLEN_8BIT);

	// Set the word length based on the desired number of data bits
	switch (data_bits)
	{
	case 5:
		lcrh |= UART0_LCRH_WLEN_5BIT;
		break;
	case 6:
		lcrh |= UART0_LCRH_WLEN_6BIT;
		break;
	case 7:
		lcrh |= UART0_LCRH_WLEN_7BIT;
		break;
	case 8:
		lcrh |= UART0_LCRH_WLEN_8BIT;
		break;
	default:
		// Handle invalid data bits
		break;
	}

	// Preserve the FIFO enable bit and update LCRH
	lcrh |= UART0_LCRH_FEN;
	UART0_LCRH = lcrh;

	// Re-enable the UART
	UART0_CR |= UART0_CR_UARTEN;
}

void uart_set_stop_bits(unsigned int stop_bits) {
    // Disable the UART
    UART0_CR &= ~UART0_CR_UARTEN;

    // Set the number of stop bits
    if (stop_bits == 2) {
        UART0_LCRH |= UART0_LCRH_STP2;  // Set for 2 stop bits
    } else {
        UART0_LCRH &= ~UART0_LCRH_STP2; // Default to 1 stop bit
    }

    // Re-enable the UART
    UART0_CR |= UART0_CR_UARTEN;
}

void uart_set_parity(char parity) {
    // Disable the UART
    UART0_CR &= ~UART0_CR_UARTEN;

    switch (parity) {
        case 'n':  // No parity
            UART0_LCRH &= ~UART0_LCRH_PEN;
            break;
        case 'e':  // Even parity
            UART0_LCRH |= (UART0_LCRH_PEN | UART0_LCRH_EPS);
            break;
        case 'o':  // Odd parity
            UART0_LCRH |= UART0_LCRH_PEN;
            UART0_LCRH &= ~UART0_LCRH_EPS;
            break;
    }

    // Re-enable the UART
    UART0_CR |= UART0_CR_UARTEN;
}

// Function to enable CTS/RTS
void uart_enable_cts_rts() {
    UART0_CR |= (UART0_CR_RTSEN | UART0_CR_CTSEN);
}

// Function to disable CTS/RTS
void uart_disable_cts_rts() {
    UART0_CR &= ~(UART0_CR_RTSEN | UART0_CR_CTSEN);
}

void custome_gpio_set_function(int pin, int function) {
    volatile unsigned int* gpfsel;
    unsigned int reg, shift, mask, value;

    // Calculate register and bit positions
    reg = pin / 10;
    shift = (pin % 10) * 3;
    mask = ~(0b111 << shift);

    // Point to the correct GPFSEL register based on the pin number
    switch (reg) {
        case 1: gpfsel = GPFSEL1; break;
        default: return; // Return if no valid register is found (safety)
    }

    // Set GPIO function
    value = *gpfsel;           // Read current register value
    value &= mask;             // Clear current settings at the pin position
    value |= (function << shift); // Set new function at the pin position
    *gpfsel = value;           // Write new register value
}

