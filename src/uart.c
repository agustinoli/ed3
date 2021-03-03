#include "LPC17xx.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_uart.h"
#include "utils.h"

#include "uart.h"



uint8_t buffer[SIZE];
uint8_t flag;

uint8_t rojo[] = "R";
uint8_t verde[] = "V";
uint8_t azul[] = "A";
uint8_t negro[] = "N";
uint8_t blanco[] = "B";

/********************************************************************//**
 * @brief		Configura los pines necesarios para controlar uart
 * @return       none
 *********************************************************************/
void config_uart_pinsel() {
	PINSEL_CFG_Type pin_config;

	// pin 0.11 RXD2
	pin_config.Portnum = PINSEL_PORT_0;
	pin_config.Pinnum = PINSEL_PIN_11;
	pin_config.Pinmode = PINSEL_PINMODE_PULLUP;
	pin_config.Funcnum = PINSEL_FUNC_1;
	pin_config.OpenDrain = PINSEL_PINMODE_TRISTATE;
	PINSEL_ConfigPin(&pin_config);
}


/********************************************************************//**
 * @brief		Configura las estructuras para controlar uart
 * @return       none
 *********************************************************************/
void config_uart() {
	UART_CFG_Type struct_uart;
	UART_FIFO_CFG_Type struct_fifo;

	struct_uart.Baud_rate = 9600;
	struct_uart.Databits = UART_DATABIT_5;

	UART_ConfigStructInit(&struct_uart);
	UART_Init(LPC_UART2, &struct_uart);
	UART_FIFOConfigStructInit(&struct_fifo);
	UART_FIFOConfig(LPC_UART2, &struct_fifo);
}

/********************************************************************//**
 * @brief		Inicializa uart
 * @return       none
 *********************************************************************/
void start_uart() {
	GPIO_ClearValue(PORT_0, PIN_22);
	UART_TxCmd(LPC_UART2, ENABLE);
}

/********************************************************************//**
 * @brief		Desinicializa uart
 * @return       none
 *********************************************************************/
void stop_uart() {
	GPIO_SetValue(PORT_0, PIN_22);
	UART_TxCmd(LPC_UART2, DISABLE);
}


/********************************************************************//**
 * @brief		Espera un comando por uart
 * @return       El valor recibido por uart como entero de 8 bits
 *********************************************************************/
uint8_t recibir_cmd(){
	UART_Receive(LPC_UART2, buffer, sizeof(buffer), BLOCKING);		// entrada por pc
	return buffer[0];
}
