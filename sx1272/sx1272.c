#include "sx1272.h"

#define RSSI_OFFSET (-139)
#define PIN_OUT 24
#define LED_PIN_ID (QM_PIN_ID_24)
#define PIN_MUX_FN (QM_PMUX_FN_0)

uint8_t currentMode = 0x81;

uint16_t good_pkt = 0,
		 bad_pkt = 0;
float per = 0;
uint8_t rand_seed;
uint16_t const_time = 0;
uint16_t seed_init = 0;
uint32_t sync_delay = 0;

void Spi_Init(void);
void writeBuffer(const uint8_t value);
void setLoRaMode();

int16_t SX1272ReadPacketRssi( void );
int8_t SX1272ReadSNR( void );

typedef enum {
	RX_DONE = 0x00,
	TX_DONE = 0x40,
} rxtx_done_t;


#ifdef EU868
#define DEFAULT_CH					(5)
const uint32_t lora_ch[8] = { 868100000,
							  868300000,
							  868500000,
							  867100000,
							  867300000,
							  867500000,
							  867700000,
							  867900000};
#endif

#ifdef US915
#define DEFAULT_CH					(7)
const uint32_t lora_ch[13] = { 903080000,
							  905240000,
							  907400000,
							  909560000,
							  911720000,
							  913880000,
							  916040000,
							  918200000,
							  920360000,
							  922520000,
							  924680000,
							  926840000,
							  915000000};
#endif

/*void SX1272_GetRandSeed(uint8_t * seed)
{
	setMode(SX1272_MODE_RX_CONTINUOS);
    for(int j=0; j<8; j++) {
        uint8_t b; // wait for two non-identical subsequent least-significant bits
        while( (b = readRegister(REG_RSSI_WIDEBAND) & 0x01) == (readRegister(REG_RSSI_WIDEBAND) & 0x01) );
        seed = (seed << 1) | b;
    }
	QM_PRINTF("Seed: %d", seed);
	setMode(SX1272_MODE_SLEEP);
}*/

void Spi_Init(void)
{
	qm_spi_config_t cfg;
	qm_gpio_port_config_t gpio_cfg;

	/* Set the GPIO pin muxing. */
	qm_pmux_select(QM_PIN_ID_16, QM_PMUX_FN_2); /* SCK */
	qm_pmux_select(QM_PIN_ID_17, QM_PMUX_FN_2); /* MOSI */
	qm_pmux_select(QM_PIN_ID_18, QM_PMUX_FN_2); /* MISO */
	qm_pmux_select(LED_PIN_ID, PIN_MUX_FN); // LED
	qm_pmux_select(2, PIN_MUX_FN); // RELAY
	gpio_cfg.direction |= BIT(CSN_SX1272);
	gpio_cfg.direction |= BIT(PIN_OUT);
	gpio_cfg.direction |= BIT(2);
	gpio_cfg.int_en = 0x0;
	gpio_cfg.int_type = 0x0;
	gpio_cfg.int_polarity = 0x0;
	gpio_cfg.int_debounce = 0x0;
	gpio_cfg.int_bothedge = 0x0;
	gpio_cfg.callback = NULL;
	qm_gpio_set_config(QM_GPIO_0, &gpio_cfg);

	//QM_PUTS("Synchronous TXRX started\r\n");

	/*  Initialise SPI configuration */
	cfg.frame_size = QM_SPI_FRAME_SIZE_8_BIT;
	cfg.transfer_mode = QM_SPI_TMOD_TX_RX;
	cfg.bus_mode = QM_SPI_BMODE;
	cfg.clk_divider = SPI_CLOCK_1MHZ_DIV;
	qm_spi_set_config(QM_SPI_MST_0, &cfg);
	qm_spi_slave_select(QM_SPI_MST_0, QM_SPI_SS_0);
	sx1272_csn_disable();
}

void Radio_Init(void)
{
	// initialize the pins

	/* Pin muxing. */
	qm_pmux_select(PIN_DIO0, QM_PMUX_FN_0);
	qm_pmux_input_en(PIN_DIO0, true);

	qm_pmux_select(PIN_DIO5, QM_PMUX_FN_0);
	qm_pmux_input_en(PIN_DIO5, true);
	Spi_Init();
	RTC_WaitTicks(100);
	// LoRa mode
	uint8_t data;
	data = readRegister(0x42);
	QM_PRINTF("SX1272 status: 0x%X\r\n", data);
	if (data!=0x22) {qm_gpio_set_pin(QM_GPIO_0, PIN_OUT);}
	setLoRaMode();

	// Turn on implicit header mode and set payload length
	writeRegister(REG_MODEM_CONFIG1,IMPLICIT_MODE);
	writeRegister(REG_MODEM_CONFIG2, 0xA4); //SF10
	writeRegister(REG_PAYLOAD_LENGTH,PAYLOAD_LENGTH);

	SX1272SetChannel(DEFAULT_CH);
	writeRegister(REG_LR_SYNCWORD, DEFAULT_SYNCWORD);

	writeRegister(REG_LR_PARAMP, PA_RAMP);

	// Change the DIO mapping to 01 so we can listen for TxDone on the interrupt
	writeRegister(REG_DIO_MAPPING_1,0x40);
	writeRegister(REG_DIO_MAPPING_2,0x00);

	// Go to standby mode
	setMode(SX1272_MODE_STANDBY);
	//SX1272_GetRandSeed(&rand_seed);
	setMode(SX1272_MODE_RX_CONTINUOS);
	for(int j=0; j<8; j++) {
	    uint8_t b; // wait for two non-identical subsequent least-significant bits
	    while( (b = readRegister(REG_RSSI_WIDEBAND) & 0x01) == (readRegister(REG_RSSI_WIDEBAND) & 0x01) );
	    rand_seed = (rand_seed << 1) | b;
	}
	setMode(SX1272_MODE_SLEEP);
	//QM_PRINTF("Seed: %d\r\n", rand_seed);
	srand(rand_seed);
	seed_init = rand() % 13107 + 6553;
}

/////////////////////////////////////
//    Method:   Receive FROM BUFFER
//////////////////////////////////////
uint8_t receiveMessage(uint8_t * message, uint8_t ret)
{
	uint8_t rx_Done = readRegister(REG_IRQ_FLAGS);

	if ((rx_Done & 0x40) == 0x40)
	{
		// clear the rxDone flag
		writeRegister(REG_IRQ_FLAGS, 0x40);

		int x = readRegister(REG_IRQ_FLAGS); // if any of these are set then the inbound message failed

		// check for payload crc issues (0x20 is the bit we are looking for
		if((x & 0x20) == 0x20)
		{
			bad_pkt++;
			QM_PRINTF("[CRC ERR!] RSSI: %d | SNR: %d | FERR: %ld\r\n", SX1272ReadPacketRssi(), SX1272ReadSNR(),
						SX1272ReadFreqError());
			if (ret == 1){
				//clk_sys_udelay(1000000UL);
				uint8_t buf[200];
				sprintf((char*)buf, "RSSI: %d | SNR: %d", SX1272ReadPacketRssi(), SX1272ReadSNR());
				sendData(buf, sizeof(buf));
			}
			// reset the crc flags
			writeRegister(REG_IRQ_FLAGS, 0x20);
		}
		else{
			good_pkt++;
			uint8_t currentAddr = readRegister(REG_FIFO_RX_CURRENT_ADDR);
			uint8_t receivedCount = readRegister(REG_RX_NB_BYTES);
			writeRegister(REG_FIFO_ADDR_PTR, currentAddr);
			// now loop over the fifo getting the data
			for(int i = 0; i < receivedCount; i++)
			{
				message[i] = readRegister(REG_FIFO);
			}
			if ((good_pkt!=0) && (bad_pkt!=0))
			{
				per=(100/good_pkt)*bad_pkt;
			}
			int d1 = per;
			float f2 = per - d1;
			int d2 = f2 * 100;

			if (ret == 1){
				QM_PRINTF("[%s] | BYTES: %u | "
						  ANSI_COLOR_GREEN  "RSSI: %d"   ANSI_COLOR_RESET " | "
						  ANSI_COLOR_YELLOW "SNR: %d " ANSI_COLOR_RESET
						  "| FERR: %d | PER: %d.%d%%\r\n", message, receivedCount,
						  SX1272ReadPacketRssi(), SX1272ReadSNR(), SX1272ReadFreqError(), d1, d2);
				uint8_t buf[200];
				sprintf((char*)buf, "RSSI: %d | SNR: %d", SX1272ReadPacketRssi(), SX1272ReadSNR());
				sendData(buf, sizeof(buf));
			}

			if (ret == 2){
				QM_PRINTF("%s,%d,%d\r\n", message, SX1272ReadPacketRssi(), SX1272ReadSNR());
			}
			//led_blink(3, DELAY100);
		}
		return 0;
	} else return 1;
}

/////////////////////////////////////
//    Method:   Send TO BUFFER
//////////////////////////////////////
void sendData(uint8_t buffer[], uint8_t size)
{
	qm_gpio_state_t state;

	//QM_PUTS("Sending: ");
	//QM_PRINTF("%s \r\n", buffer);
	//while(!SX1272IsChannelFree(-120)){}

	//setMode(SX1272_MODE_STANDBY);
	//uint16_t seed = rand() % 13107 + 6553;
	//setMode(SX1272_MODE_STANDBY);
	//QM_PRINTF("Seed Wait: %d\r\n", seed);
	RTC_WaitTicks(seed_init);
	uint32_t start = QM_RTC[QM_RTC_0].rtc_ccvr;
	if (SX1272_CadDetector())
	{
		//clk_sys_udelay(250000UL);
		//rtc_wait(7209);
		QM_PRINTF("Cad detected\r\n");
		qm_gpio_set_pin(QM_GPIO_0, PIN_OUT);
		while(!SX1272IsChannelFree(-120)){}
		qm_gpio_clear_pin(QM_GPIO_0, PIN_OUT);
		//rtc_wait(seed);
		//const_time = seed;
		const_time = QM_RTC[QM_RTC_0].rtc_ccvr-start;
	} else {
		//if (!SX1272IsChannelFree(-120))
		RTC_WaitTicks(const_time);
	}
	setMode(SX1272_MODE_STANDBY);
	writeRegister(REG_PAYLOAD_LENGTH,size);
	writeRegister(REG_DIO_MAPPING_1,TX_DONE);
	writeRegister(REG_FIFO_TX_BASE_AD, 0x00);  // Update the address ptr to the current tx base address
	writeRegister(REG_FIFO_ADDR_PTR, 0x00);

	sx1272_csn_enable();
	// tell SPI which address you want to write to
	writeBuffer(REG_FIFO | 0x80);
	// loop over the payload and put it on the buffer
	for (uint8_t i = 0; i < size; i++){
		writeBuffer(buffer[i]);
		//qm_uart_write(QM_UART_0, buffer[i]);
	}
	sx1272_csn_disable();
  // go into transmit mode

	setMode(SX1272_MODE_TX);

  // once TxDone has flipped, everything has been sent
	while(qm_gpio_read_pin(QM_GPIO_0, PIN_DIO0, &state)){
	}

	//QM_PUTS("Done Sending \r\n");

	// clear the flags 0x08 is the TxDone flag
	writeRegister(REG_IRQ_FLAGS, 0x08);
	setMode(SX1272_MODE_SLEEP);
}

/////////////////////////////////////
//    Method:   Setup to receive continuously
//////////////////////////////////////
void startReceiving()
{
  // Turn on implicit header mode and set payload length
  writeRegister(REG_MODEM_CONFIG1,IMPLICIT_MODE);
  //writeRegister(REG_PAYLOAD_LENGTH,PAYLOAD_LENGTH);
  writeRegister(REG_HOP_PERIOD,0xFF);
  writeRegister(REG_FIFO_ADDR_PTR, readRegister(REG_FIFO_RX_BASE_AD));

  // Setup Receive Continous Mode
  setMode(SX1272_MODE_RX_CONTINUOS);
}

/////////////////////////////////////
//    Method:   Read Register
//////////////////////////////////////

uint8_t readRegister(uint8_t addr)
{
	qm_spi_transfer_t polled_xfer_desc;
	qm_spi_status_t status;

	uint8_t rx_buf[2];
	uint8_t spi_buf[2];

	spi_buf[0] = addr & 0x7F;
	spi_buf[1] = 0x00;

	polled_xfer_desc.tx = spi_buf;
	polled_xfer_desc.rx = rx_buf;
	polled_xfer_desc.tx_len = 2;
	polled_xfer_desc.rx_len = 2;
	sx1272_csn_enable();
	qm_spi_transfer(QM_SPI_MST_0, &polled_xfer_desc, &status);
	sx1272_csn_disable();

    return rx_buf[1];
}

/////////////////////////////////////
//    Method:   Write Register
//////////////////////////////////////

void writeRegister(uint8_t addr, uint8_t value)
{
	qm_spi_transfer_t polled_xfer_desc;
	qm_spi_status_t status;

	uint8_t spi_buf[2];
	uint8_t rx_buf[2];

	spi_buf[0] = addr | 0x80;
	spi_buf[1] = value;

	polled_xfer_desc.tx = spi_buf;
	polled_xfer_desc.rx = rx_buf;
	polled_xfer_desc.tx_len = 2;
	polled_xfer_desc.rx_len = 2;

	sx1272_csn_enable();
	qm_spi_transfer(QM_SPI_MST_0, &polled_xfer_desc, &status);
	sx1272_csn_disable();
}

void writeBuffer(const uint8_t value)
{
	qm_spi_transfer_t polled_xfer_desc;
	qm_spi_status_t status;

	uint8_t spi_buf;
	uint8_t rx_buf;

	spi_buf = value;

	polled_xfer_desc.tx = &spi_buf;
	polled_xfer_desc.rx = &rx_buf;
	polled_xfer_desc.tx_len = 1;
	polled_xfer_desc.rx_len = 1;

	//sx1272_csn_enable();
	qm_spi_transfer(QM_SPI_MST_0, &polled_xfer_desc, &status);
	//sx1272_csn_disable();
}

/////////////////////////////////////
//    Method:   Read ALL Registers
//////////////////////////////////////
void readAllRegs(void)
{
	for (uint8_t regAddr = 1; regAddr <= 0x46; regAddr++)
	{
		QM_PRINTF("REG ADDR: 0x%x", regAddr);
		QM_PRINTF(" - ");
		QM_PRINTF("VALUE: 0x%x\r\n", readRegister(regAddr));
	}
}

bool SX1272_CadDetector(void)
{
	setMode(SX1272_MODE_SLEEP);
	//clk_sys_udelay(240);
	RTC_WaitTicks(1);
	setMode(SX1272_MODE_CAD);
	RTC_WaitTicks(1);
	while((readRegister(REG_IRQ_FLAGS) & 0x04)!= 0x04){}
	writeRegister(REG_IRQ_FLAGS, 0x04);
	if ((readRegister(REG_IRQ_FLAGS) & 0x01) == 0x01)
	{
		writeRegister(REG_IRQ_FLAGS, 0x01);
		setMode(SX1272_MODE_SLEEP);
		RTC_WaitTicks(1);
		return true;
	}
	else
	{
		writeRegister(REG_IRQ_FLAGS, 0x01);
		setMode(SX1272_MODE_SLEEP);
		RTC_WaitTicks(1);
		return false;
	}

}

/////////////////////////////////////
//    Method:   Change the mode
//////////////////////////////////////
void setMode(uint8_t newMode)
{
	qm_gpio_state_t state;

  if(newMode == currentMode)
    return;

  switch (newMode)
  {
    case SX1272_MODE_RX_CONTINUOS:
      //QM_PRINTF("Changing to Receive Continous Mode\r\n");
      //writeRegister(REG_PA_CONFIG, PA_OFF_BOOST);  // TURN PA OFF FOR RECIEVE??
      writeRegister(REG_LNA, LNA_MAX_GAIN);  // MAX GAIN FOR RECIEVE
      writeRegister(REG_OPMODE, newMode);
      currentMode = newMode;
      break;
    case SX1272_MODE_TX:
      //QM_PRINTF("Changing to Transmit Mode\r\n");
      //writeRegister(REG_LNA, LNA_OFF_GAIN);  // TURN LNA OFF FOR TRANSMITT
      writeRegister(REG_PA_CONFIG, 0xFF);    // TURN PA TO MAX POWER RFIO
      writeRegister(REG_OPMODE, newMode);
      currentMode = newMode;

      break;
    case SX1272_MODE_SLEEP:
      //QM_PRINTF("Changing to Sleep Mode\r\n");
      writeRegister(REG_OPMODE, newMode);
      currentMode = newMode;
      break;
    case SX1272_MODE_STANDBY:
      //QM_PRINTF("Changing to Standby Mode\r\n");
      writeRegister(REG_OPMODE, newMode);
      currentMode = newMode;
      break;

    case SX1272_MODE_CAD:
       //QM_PRINTF("Changing to Standby Mode\r\n");
       writeRegister(REG_OPMODE, newMode);
       currentMode = newMode;
       break;
    default: return;
  }

  if(newMode != SX1272_MODE_SLEEP){
    while(qm_gpio_read_pin(QM_GPIO_0, PIN_DIO5, &state))
    {
      //QM_PUTS("z\r");
    }
  }

 //QM_PRINTF(" Mode Change Done\r\n");
}

/////////////////////////////////////
//    Method:   Enable LoRa mode
//////////////////////////////////////
void setLoRaMode()
{
  //QM_PRINTF("Setting LoRa Mode\r\n");
  setMode(SX1272_MODE_SLEEP);
  writeRegister(REG_OPMODE,0x80);

  //QM_PRINTF("LoRa Mode Set\r\n");
}

int8_t SX1272ReadSNR( void )
{
	uint8_t snr_value =  readRegister(REG_LR_PKTSNRVALUE);
	if(snr_value & 0x80 ) // The SNR sign bit is 1
	{
	    // Invert and divide by 4
		snr_value = ( ( ~snr_value + 1 ) & 0xFF ) >> 2;
		snr_value = -snr_value;
	}
	else
	{
		// Divide by 4
		snr_value = ( snr_value & 0xFF ) >> 2;
	}

    return snr_value;
}

int16_t SX1272ReadPacketRssi( void )
{
    int16_t rssi = 0;
    int8_t snr = SX1272ReadSNR();
    rssi = (int16_t)readRegister(REG_LR_PKTRSSIVALUE);
    if (snr<0)
    {
    	rssi = RSSI_OFFSET + rssi + ( rssi >> 4 ) + snr;
    }
    else
    {
    	rssi = RSSI_OFFSET + rssi + ( rssi >> 4 );
    }
    return rssi;
}

void SX1272SetChannel(uint8_t channel)
{
	uint32_t freq;
    freq = ( uint32_t )( ( double )lora_ch[channel] / ( double )FREQ_STEP );
    writeRegister( REG_LR_FRFMSB, ( uint8_t )( ( freq >> 16 ) & 0xFF ) );
    writeRegister( REG_LR_FRFMID, ( uint8_t )( ( freq >> 8 ) & 0xFF ) );
    writeRegister( REG_LR_FRFLSB, ( uint8_t )( freq & 0xFF ) );
}

uint32_t SX1272ReadFreqError(void)
{
	uint8_t freqerr_buf[3];

	freqerr_buf[0] = readRegister(REG_LR_FEIMSB);
	freqerr_buf[1] = readRegister(REG_LR_FEIMID);
	freqerr_buf[2] = readRegister(REG_LR_FEILSB);

	//QM_PRINTF("REG_LR_FEIMSB: %d | REG_LR_FEIMID: %d | REG_LR_FEILSB %d\r\n", freqerr_buf[0], freqerr_buf[1], freqerr_buf[2]);

	uint32_t freqerr = (freqerr_buf[0] << 16) | (freqerr_buf[1] << 8) | (freqerr_buf[2]);
	//QM_PRINTF("freqerr reg: %d\r\n", freqerr);
	freqerr = (freqerr * 16777216)/32000000;
	return freqerr;
}

int16_t SX1272ReadRssi()
{
    int16_t rssi = 0;
    setMode(SX1272_MODE_RX_CONTINUOS);
    rssi = RSSI_OFFSET + readRegister( REG_LR_RSSIVALUE );
    setMode(SX1272_MODE_SLEEP);
    return rssi;
}

bool SX1272IsChannelFree( int16_t rssiThresh )
{
    int16_t rssi = 0;
    setMode(SX1272_MODE_RX_CONTINUOS);
    RTC_WaitTicks(64);
    rssi = SX1272ReadRssi();
    QM_PRINTF("CAD RSSI: %d\r\n", rssi);
    setMode(SX1272_MODE_SLEEP);
    if( rssi > rssiThresh )
    {
        return false;
    }
    return true;
}


