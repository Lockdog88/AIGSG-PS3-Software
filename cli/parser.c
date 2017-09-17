#include "parser.h"
#include "qm_uart.h"
#include "string.h"
#include "stdlib.h"
#include "qm_common.h"
#include "sx1272.h"

const uint32_t channels[8] = { 868100000,
							   868300000,
							   868500000,
							   867100000,
							   867300000,
							   867500000,
							   867700000,
							   867900000};

//available  commands
char * keyworld [] = {_CMD_HELP, _CMD_CLEAR, _CMD_INFO};
// 'set/clear' command argements
//char * set_clear_key [] = {_SCMD_PB, _SCMD_PD};
// array for comletion
char * compl_world [_NUM_OF_CMD + 1];

void print(const char* str)
{
	qm_uart_status_t uart_status __attribute__((unused)) = 0;
	unsigned int i;
	for(i = 0; str[i]; i++)
	{
		//while((uart_status & QM_UART_TX_BUSY)==0) qm_uart_get_status(QM_UART_0, &uart_status);
		qm_uart_write(QM_UART_0,str[i]);
	}
}

char get_char (void)
{
	uint8_t data;
	qm_uart_status_t uart_status __attribute__((unused)) = 0;
	while ((uart_status & QM_UART_RX_BUSY)==0)qm_uart_get_status(QM_UART_0, &uart_status);
	//data = USART_ReceiveData(USART1);
	qm_uart_read(QM_UART_0, &data, &uart_status);
	return data;
}

int execute (int argc, const char * const * argv)
{
	int i = 0;
	// just iterate through argv word and compare it with your commands
	while (i < argc) {
		if (strcmp (argv[i], _CMD_INFO) == 0) {
			print ("Parking sensor with LoRa radio");
			print (MICRORL_LIB_VER);
			print (" Version");
			print (VER);
			print("\n\r");
		}
		else if (strcmp (argv[i], _CMD_SETCH) == 0) {
				if ((++i) < argc) { // if value preset
							SX1272SetChannel(atoi(argv[i]));
							QM_PRINTF("New Channel#: %u, Frequency: %u\r\n", atoi(argv[i]), channels[atoi(argv[i])]);
						}
		} else {
			print ("command: '");
			print ((char*)argv[i]);
			print ("' Not found.\n\r");
		}
		i++;
	}
	return 0;
}

char ** complet (int argc, const char * const * argv)
{
	int j = 0;

	compl_world [0] = NULL;

	// if there is token in cmdline
	if (argc == 1) {
		// get last entered token
		char * bit = (char*)argv [argc-1];
		// iterate through our available token and match it
		int i;
		for (i = 0; i < _NUM_OF_CMD; i++) {
			// if token is matched (text is part of our token starting from 0 char)
			if (strstr(keyworld [i], bit) == keyworld [i]) {
				// add it to completion set
				compl_world [j++] = keyworld [i];
			}
		}
	}

	// note! last ptr in array always must be NULL!!!
	compl_world [j] = NULL;
	// return set of variants
	return compl_world;
}

void sigint (void)
{
	print ("^C catched!\n\r");
}
