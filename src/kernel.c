// -----------------------------------main.c -------------------------------------

#include "uart0.h"
#include "printf.h"

// include ultilities
#include "../ulti/resetString.c"
#include "../ulti/compare2String.c"
#include "../ulti/removeString.c"
#include "../ulti/containString.c"
#include "../ulti/copyString.c"
#include "../ulti/returnStringLen.c"
#include "../ulti/splitString.c"

#include "../mbox/mbox.h"
#include "../mbox/mbox.c"

#define INITIAL_TEXT "MyOS>"
#define MAX_CMD_LEN 100

void uart_send_hex(unsigned int value) {
    const char hex_digits[] = "0123456789ABCDEF";
    char buffer[9]; // 8 digits plus string terminator
    buffer[8] = '\0'; // null-terminate the string

    for (int i = 7; i >= 0; i--) {
        buffer[i] = hex_digits[value % 16];
        value /= 16;
    }

    uart_puts("Board Revision: 0x");
    uart_puts(buffer);
    uart_puts("\n");
}


#define MBOX_TAG_GET_BOARD_MAC_ADDRESS 0x00010003 // The tag for getting the MAC address

void showInfo() {
    // Buffer size and request code for both queries
    mBuf[0] = 7 * 4; // Buffer size for board revision
    mBuf[1] = MBOX_REQUEST; 
    
    // Get Board Revision
    mBuf[2] = 0x00010002; // Tag for 'Get Board Revision'
    mBuf[3] = 4;          // Response length
    mBuf[4] = 0;          // Request length
    mBuf[5] = 0;          // Space for the board revision to be filled in
    mBuf[6] = MBOX_TAG_LAST;

    // Call the mailbox property interface
    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP)) {
        uart_puts("Board Revision: 0x");
        uart_hex(mBuf[5]);
        uart_puts("\n");
    } else {
        uart_puts("Unable to query board revision!\n");
    }

    // Reset buffer for MAC address request
    mBuf[0] = 8 * 4; // Adjust the buffer size if needed
    mBuf[1] = MBOX_REQUEST;
    mBuf[2] = 0x00010003; // Tag for 'Get MAC Address'
    mBuf[3] = 6;          // Response length for MAC
    mBuf[4] = 0;          // Request length is 0
    mBuf[5] = 0;          // Space for the MAC address to be filled in (upper 4 bytes)
    mBuf[6] = 0;          // Space for the MAC address to be filled in (lower 2 bytes)
    mBuf[7] = MBOX_TAG_LAST;

    // Call the mailbox property interface
    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP)) {
        uart_puts("MAC Address: ");
        for (int i = 0; i < 6; i++) {
            uart_hex(((unsigned char*)&mBuf[5])[i]); // MAC address starts at mBuf[5]
			// uart_hex(((unsigned char*)mBuf)[20+i]);
            if (i < 5) uart_puts(":");
        }
        uart_puts("\n");
    } else {
        uart_puts("Unable to query MAC address!\n");
    }
}



void helpCmdList()
{
	uart_puts("Cmd list:\n");
	uart_puts("help: show brief information of all command\n");
	uart_puts("help <command_name> : show detail information of <command_name>\n");
	uart_puts("clear: clear the screen\n");
	uart_puts("setcolor : set the text color\n");
	uart_puts("-t <text color> : set the text color\n -b <background color> : set the background color\n");
	uart_puts("showinfo: show board revision and Mac address\n");
	uart_puts("configBR <baudrate>: confige custome baudrate\n");
	uart_puts("setdatabits <databits>: set the number of data bits\n");
	uart_puts("setstopbits <stopbits>: set the number of stop bits\n");
	uart_puts("setparity <parity>: set the parity\n");
	uart_puts("handshaking : enable and disable handshaking\n");
}

int detailedHelpCmd(char *cmd)
{
	if (compare2String(cmd, "help") == 0)
	{
		uart_puts("help: Show brief information of all commands\n");
		uart_puts("help <command_name> : show detail information of <command_name>\n");
	}
	else if (compare2String(cmd, "clear") == 0)
	{
		uart_puts("clear: Use ANSI sequences to clear the screen and set the cursor to top left corner.\n");
	}
	else if (compare2String(cmd, "setcolor") == 0)
	{
		uart_puts("setcolor : set the text color\n");
		uart_puts("-t <text color> : set the text color\n -b <background color> : set the background color\n");
	}
	else if (compare2String(cmd, "showinfo") == 0)
	{
		uart_puts("showinfo: show board revision and Mac address\n");
	}
	else if (compare2String(cmd, "configBR") == 0)
	{
		uart_puts("configBR <baudrate>: confige custome baudrate\n");
	}
	else if (compare2String(cmd,"setdatabits") == 0)
	{
		uart_puts("setdatabits <databits>: set the number of data bits\n");
	}
	else if (compare2String(cmd, "setstopbits") == 0)
	{
		uart_puts("setstopbits <stopbits>: set the number of stop bits\n");
	}
	else if (compare2String(cmd,"setparity") == 0){
		uart_puts("setparity <parity>: set the parity\n");
	}
	else if (compare2String(cmd,"handshaking") == 0){
		uart_puts("handshaking : enable and disable handshaking\n");
	}
	else
	{
		uart_puts("Command not found! Please type 'help' to see the list of commands\n");
		return -1;
	}
	return 1;
}

void eraseScreenReverse(char *string)
{
	// Find the end of the string
	while (*string != '\0')
	{
		uart_sendc('\b');
		uart_sendc(' ');
		uart_sendc('\b');
		string++;
	}
}

// function to set the color
int setColor(char *color, int place)
{
	// check if the target is text color or background color
	if (place == 1)
	{ // set color for text
		if (compare2String(color, "black") == 0)
		{
			uart_puts("\033[30m");
		}
		else if (compare2String(color, "red") == 0)
		{
			uart_puts("\033[31m");
		}
		else if (compare2String(color, "green") == 0)
		{
			uart_puts("\033[32m");
		}
		else if (compare2String(color, "yellow") == 0)
		{
			uart_puts("\033[33m");
		}
		else if (compare2String(color, "blue") == 0)
		{
			uart_puts("\033[34m");
		}
		else if (compare2String(color, "magenta") == 0)
		{
			uart_puts("\033[35m");
		}
		else if (compare2String(color, "cyan") == 0)
		{
			uart_puts("\033[36m");
		}
		else if (compare2String(color, "white") == 0)
		{
			uart_puts("\033[37m");
		}
		else
		{
			uart_puts("Color not found!\n");
			uart_puts(color);
			uart_puts("Please type 'help setcolor' to see the list of colors\n");
		}
	}
	else if (place == 0)
	{ // set color for background
		if (compare2String(color, "black") == 0)
		{
			uart_puts("set background color: black\n");
			uart_puts("\033[40m");
		}
		else if (compare2String(color, "red") == 0)
		{
			uart_puts("set background color: red\n");
			uart_puts("\033[41m");
		}
		else if (compare2String(color, "green") == 0)
		{
			uart_puts("set background color: green\n");
			uart_puts("\033[42m");
		}
		else if (compare2String(color, "yellow") == 0)
		{
			uart_puts("set background color: yellow\n");
			uart_puts("\033[43m");
		}
		else if (compare2String(color, "blue") == 0)
		{
			uart_puts("set background color: blue\n");
			uart_puts("\033[44m");
		}
		else if (compare2String(color, "magenta") == 0)
		{
			uart_puts("set background color: magenta\n");
			uart_puts("\033[45m");
		}
		else if (compare2String(color, "cyan") == 0)
		{
			uart_puts("set background color: cyan\n");
			uart_puts("\033[46m");
		}
		else if (compare2String(color, "white") == 0)
		{
			uart_puts("set background color: white\n");
			uart_puts("\033[47m");
		}
		else
		{
			uart_puts("Color not found!\n");
			uart_puts(color);
			uart_puts("Please type 'help setcolor' to see the list of colors\n");
		}
	}
	return -1;
}

void cli()
{
	static char cmd[MAX_CMD_LEN];
	static int cmdIndex = 0;

	static char tmp_cmd[4][MAX_CMD_LEN];
	static int test;

	static char cmd_history[10][MAX_CMD_LEN];
	// the latest index of the history array
	static int cmd_history_index;
	// the current index of the history array
	static int current_his_pos = 0;

	static char split_cmd[10][MAX_CMD_LEN];

	char c = uart_getc();
	if ((cmdIndex == 0 && c == '\b') || c == '_' || c == '+' || c == '\t')
	{
	}
	else
	{
		uart_sendc(c);
	}

	if (c != '\n' && c != '\b' && c != '_' && c != '+' && c != '\t')
	{
		resetString(&tmp_cmd[0][0]);
		resetString(&tmp_cmd[1][0]);
		resetString(&tmp_cmd[2][0]);
		resetString(&tmp_cmd[3][0]);

		int tmp_cmdIndex = 0;

		cmd[cmdIndex] = c;
		cmdIndex++;

		if (containString(cmd, "help") == 1)
		{
			copyString("help", &tmp_cmd[tmp_cmdIndex][0]);
			tmp_cmdIndex++;
		}
		if (containString(cmd, "clear") == 1)
		{
			copyString("clear", &tmp_cmd[tmp_cmdIndex][0]);
			tmp_cmdIndex++;
		}
		if (containString(cmd, "setcolor") == 1)
		{
			copyString("setcolor", &tmp_cmd[tmp_cmdIndex][0]);
			tmp_cmdIndex++;
		}
		if (containString(cmd, "showinfo") == 1)
		{
			copyString("showinfo", &tmp_cmd[tmp_cmdIndex][0]);
			tmp_cmdIndex++;
		}
		if (containString(cmd, "configBR") == 1)
		{
			copyString("configBR", &tmp_cmd[tmp_cmdIndex][0]);
			tmp_cmdIndex++;
		}
		if (containString(cmd, "check") == 1)
		{
			copyString("check", &tmp_cmd[tmp_cmdIndex][0]);
			tmp_cmdIndex++;
		}
		if (containString(cmd, "setdatabits") == 1)
		{
			copyString("setdatabits", &tmp_cmd[tmp_cmdIndex][0]);
			tmp_cmdIndex++;
		}
		if (containString(cmd, "setstopbits") == 1)
		{
			copyString("setstopbits", &tmp_cmd[tmp_cmdIndex][0]);
			tmp_cmdIndex++;
		}
		if (containString(cmd, "configParity") == 1)
		{
			copyString("configParity", &tmp_cmd[tmp_cmdIndex][0]);
			tmp_cmdIndex++;
		}
		if (containString(cmd, "handshaking") == 1)
		{
			copyString("handshaking", &tmp_cmd[tmp_cmdIndex][0]);
			tmp_cmdIndex++;
		}
		test = 0;
	}
	else if (c == '\b')
	{
		// if the user press the backspace button
		if (cmdIndex > 0)
		{
			cmd[cmdIndex - 1] = '\0';
			cmdIndex--;
			uart_sendc(' ');
			uart_sendc('\b');
		}
		else
		{
			cmd[cmdIndex] = '\0';
			removeString(&tmp_cmd[0][0]);
			removeString(&tmp_cmd[1][0]);
			removeString(&tmp_cmd[2][0]);
			removeString(&tmp_cmd[3][0]);
		}
	}
	else if (c == '+')
	{
		eraseScreenReverse(cmd);
		resetString(cmd);


		// Attempt to find the next non-empty command in the history.
		int attempts = 0; // To avoid infinite loops in case all entries are empty
		do
		{
			if (current_his_pos >= 9)
			{						 // Check if it is at the start of the array
				current_his_pos = 0; // Wrap around to the last index
			}
			else
			{
				current_his_pos++; // Decrement to go backwards in history
			}
			attempts++;
		} while (cmd_history[current_his_pos][0] == '\0' && attempts < 10);

		// If all positions are checked and all are empty, this will naturally stop after 'history_size' iterations
		// The current_his_pos will point to the first command after the last non-empty command (or stay the same if all are empty)
		// If the history is full, the oldest command will be overwritten
		copyString(cmd_history[current_his_pos], cmd);
		uart_puts(cmd);
		cmdIndex = returnStringLen(cmd);
	}
	else if (c == '_')
	{
		eraseScreenReverse(cmd);
		resetString(cmd);

		int attempts = 0; // To avoid infinite loops in case all entries are empty
		do
		{
			if (current_his_pos < 0)
			{						 // Check if it is at the start of the array
				current_his_pos = 9; // Wrap around to the last index
			}
			else
			{
				current_his_pos--; // Decrement to go backwards in history
			}
			attempts++;
		} while (cmd_history[current_his_pos][0] == '\0' && attempts < 10);

		// If all positions are checked and all are empty, this will naturally stop after 'history_size' iterations
		// The current_his_pos will point to the first command after the last non-empty command (or stay the same if all are empty)
		// If the history is full, the oldest command will be overwritten
		copyString(cmd_history[current_his_pos], cmd);
		uart_puts(cmd);
		cmdIndex = returnStringLen(cmd);
	}
	else if (c == '\t')
	{
		if (tmp_cmd[0][0] != '\0')
		{
			eraseScreenReverse(&cmd[0]);
			resetString(&cmd[0]);

			// copy the matching command into the input buffer
			copyString(&(*(tmp_cmd + test)), &cmd[0]);
			// print to the terminal
			uart_puts(cmd);
			cmdIndex = returnStringLen(cmd);

			// if the user continue to press tab button, move to next matching command
			if (tmp_cmd[test + 1][0] != '\0')
			{
				test++;
			}
			else
			{
				test = 0;
			}
		}
	}
	else if (c == '\n'){
		int executed = 0;

		cmd[cmdIndex] = '\0';
		splitString(cmd, split_cmd);
		
		if (compare2String(&split_cmd[0][0], "help") == 0){
			if (compare2String(&split_cmd[1][0], "") == 0){
				helpCmdList();
				executed++;
			}
			else {
				if (detailedHelpCmd(&split_cmd[1][0]) == 1){
					executed++;
				};
			}
		} else if (compare2String(&split_cmd[0][0],"showinfo") == 0){
			showInfo();
			executed++;
		} else if (compare2String(&split_cmd[0][0],"clear") == 0) {
			uart_puts("\033[2J\033[1;1H"); // consider
			executed++;
		} else if (compare2String(&split_cmd[0][0],"setcolor") == 0){
			if (compare2String(&split_cmd[1][0],"-t") == 0){
				if (compare2String(&split_cmd[3][0],"-b") == 0){
					setColor(&split_cmd[2][0],1);
					setColor(&split_cmd[4][0],0);
					executed++;
				}
				else {
					setColor(&split_cmd[2][0],1);
					uart_puts("set color oke");
					executed++;
				}
			}
			else if (compare2String(&split_cmd[1][0],"-b") == 0){
				if (compare2String(&split_cmd[3][0],"-t") == 0){
					setColor(&split_cmd[2][0],0);
					setColor(&split_cmd[4][0],1);
					executed++;
				}
				else {
					setColor(&split_cmd[2][0],0);
					executed++;
				}
			}
		}
		else if (compare2String(&split_cmd[0][0],"configBR") == 0){
			if (compare2String(&split_cmd[1][0],"9600") == 0){
				uart_set_baud_rate(9600);
			} else if (compare2String(&split_cmd[1][0],"19200") == 0){
				uart_set_baud_rate(19200);
			} else if (compare2String(&split_cmd[1][0],"38400") == 0){
				uart_set_baud_rate(38400);
			} else if (compare2String(&split_cmd[1][0],"57600") == 0){
				uart_set_baud_rate(57600);
			} else if (compare2String(&split_cmd[1][0],"115200") == 0){
				uart_set_baud_rate(115200);
			} else {
				uart_puts("Baudrate not found! Please type 'help' to see the list of commands\n");
			}
		}
		else if (compare2String(&split_cmd[0][0],"check") == 0){
			uart_puts("oke");
				 uart_print_baud_rate();
		}
		else if (compare2String(&split_cmd[0][0],"setdatabits") == 0){
			if (compare2String(&split_cmd[1][0],"5") == 0){
				uart_puts("set databits 5");
				uart_set_data_bits(5);
			}
			else if (compare2String(&split_cmd[1][0],"6") == 0){
				uart_puts("set databits 6");
				uart_set_data_bits(6);
			}
			else if (compare2String(&split_cmd[1][0],"7") == 0){
				uart_puts("set databits 7");
				uart_set_data_bits(7);
			}
			else if (compare2String(&split_cmd[1][0],"8") == 0){
				uart_puts("set databits 8");
				uart_set_data_bits(8);
			}
			else {
				uart_puts("Databits not found! Please type 'help' to see the list of commands\n");
			}
		}
		else if (compare2String(&split_cmd[0][0],"setstopbits") == 0){
			if (compare2String(&split_cmd[1][0],"1") == 0){
				uart_puts("set stopbits 1");
				uart_set_stop_bits(1);
			}
			else if (compare2String(&split_cmd[1][0],"2") == 0){
				uart_puts("set stopbits 2");
				uart_set_stop_bits(2);
			}
			else {
				uart_puts("Stopbits not found! Please type 'help' to see the list of commands\n");
			}
		}
		else if (compare2String(&split_cmd[0][0],"configParity") == 0){
			if (compare2String(&split_cmd[1][0],"even") == 0){
				uart_puts("set parity even");
				uart_set_parity('e');
			}
			else if (compare2String(&split_cmd[1][0],"odd") == 0){
				uart_puts("set parity odd");
				uart_set_parity('o');
			}
			else if (compare2String(&split_cmd[1][0],"none") == 0){
				uart_puts("set parity none");
				uart_set_parity('n');
			}
			else {
				uart_puts("Parity not found! Please type 'help' to see the list of commands\n");
			}
		} else if (compare2String(&split_cmd[0][0],"handshaking") == 0){
			if (compare2String(&split_cmd[1][0],"enable") == 0){
				uart_puts("enable handshaking");
				uart_enable_cts_rts();
			}
			else if (compare2String(&split_cmd[1][0],"disable") == 0){
				uart_puts("disable handshaking");
				uart_disable_cts_rts();
			}
			else {
				uart_puts("Handshaking not found! Please type 'help' to see the list of commands\n");
			}
		}
		else{
				uart_puts("Command not found! Please type 'help' to see the list of commands\n");
				uart_puts(&split_cmd[0][0]);
			}

		if (executed != 0){
				copyString(cmd,&cmd_history[cmd_history_index]);
				cmd_history_index++;

				//if the number of history exceed 10
				if(cmd_history_index >= 10){
					cmd_history_index = 0;
				}
				current_his_pos = cmd_history_index;
		}
		
		//Return to command line
		cmdIndex = 0;

		//reset all the buffer except history, avoiding error in the next command
		resetString(cmd);
		resetString(&split_cmd[0][0]);
		resetString(&split_cmd[1][0]);
		resetString(&split_cmd[2][0]);
		resetString(&split_cmd[3][0]);
		resetString(&split_cmd[4][0]);
		resetString(&tmp_cmd[0][0]);
		resetString(&tmp_cmd[1][0]);
		resetString(&tmp_cmd[2][0]);
		resetString(&tmp_cmd[3][0]);

		uart_puts("\n");
		uart_puts(INITIAL_TEXT);
	}
}

void main()
{
	// intitialize UART
	uart_init();

	uart_puts("######## ######## ######## ########     #######  ##         #######    #####   \n");
	uart_puts("##       ##       ##          ##       ##     ## ##    ##  ##     ##  ##   ##  \n");
	uart_puts("##       ##       ##          ##              ## ##    ##  ##     ## ##     ## \n");
	uart_puts("######   ######   ######      ##        #######  ##    ##   ######## ##     ## \n");
	uart_puts("##       ##       ##          ##       ##        #########        ## ##     ## \n");
	uart_puts("##       ##       ##          ##       ##              ##  ##     ##  ##   ##  \n");
	uart_puts("######## ######## ########    ##       #########       ##   #######    #####   \n\n");

	uart_puts("         ########     ###    ########  ########     #######   ######           \n");
	uart_puts("         ##     ##   ## ##   ##     ## ##          ##     ## ##    ##          \n");
	uart_puts("         ##     ##  ##   ##  ##     ## ##          ##     ## ##                \n");
	uart_puts("         ########  ######### ########  ######      ##     ##  ######           \n");
	uart_puts("         ##     ## ##     ## ##   ##   ##          ##     ##       ##          \n");
	uart_puts("         ##     ## ##     ## ##    ##  ##          ##     ## ##    ##          \n");
	uart_puts("         ########  ##     ## ##     ## ########     #######   ######           \n");

	uart_puts("                Developed by Tran Vu Quang Anh - s3916566                  \n");

	// printf("%s",INITIAL_TEXT);
	// say hello
	uart_puts(INITIAL_TEXT);

	// echo everything back
	while (1)
	{
		cli();
	}
}
