#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_exti.h"
#include "touch.h"
#include "lcd.h"
#include "utils.h"

uint32_t pos_x;
uint32_t pos_y;

void EINT1_IRQHandler() {
	EXTI_ClearEXTIFlag(EXTI_EINT1);
	NVIC_DisableIRQ(EINT1_IRQn);
	TFT_fill(White);
	//TODO: COSAS QUE HACER CUANDO TOCO
	TP_read_coordinates(&pos_x,&pos_y);
	Draw_Circle(pos_x, pos_y, 6, YES, Red);
	NVIC_EnableIRQ(EINT1_IRQn);
}


/********************************************************************//**
 * @brief		Realiza las configuraciones de pines para el tactil
 * @return       none
 *********************************************************************/
void config_ts_pinsel()
{
	PINSEL_CFG_Type config_pin;
		/*
		 * P0. -> T_IRQ
		 * P0. .> T_CS
		 */

		config_pin.OpenDrain	=	PINSEL_PINMODE_NORMAL;
		config_pin.Pinmode		=	PINSEL_PINMODE_PULLUP;


		//	T_CS
		config_pin.Portnum		=	PINSEL_PORT_0;
		config_pin.Funcnum		=	PINSEL_FUNC_0;
		config_pin.Pinnum		=	PINSEL_PIN_27;
		GPIO_SetDir(PORT_0,PIN_27,OUTPUT);
		PINSEL_ConfigPin(&config_pin);

		//	T_IRQ
		config_pin.Funcnum		=	PINSEL_FUNC_1;
		config_pin.Portnum		=	PINSEL_PORT_2;
		config_pin.Pinnum		=	PINSEL_PIN_11;
		PINSEL_ConfigPin(&config_pin);
		GPIO_SetDir(PORT_2, PIN_11, INPUT);

		return;

}
/********************************************************************//**
 * @brief		Configura las interrupciones externas del tactil
 * @return       none
 *********************************************************************/
void config_ts_exti() {
	EXTI_InitTypeDef struct_exti;
	struct_exti.EXTI_Line = EXTI_EINT1;
	struct_exti.EXTI_Mode = EXTI_MODE_LEVEL_SENSITIVE;
	struct_exti.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
	EXTI_Config(&struct_exti);
}

/********************************************************************//**
 * @brief		Realiza una lectura de los datos de posicion del
 * 					tactil
 * @return       valor leido por el tactil
 *********************************************************************/
unsigned long TP_read()
{
    unsigned char lb = 0x00;
    unsigned char hb = 0x00;
    unsigned long value = 0x0000;

    send_spi(0);
    send_spi(0);
    value = hb;
    value <<= 8;
    value |= lb;
    value >>= 3;
    value &= 0x0FFF;

    return value;
}

//TODO pasar a uint32_t
/********************************************************************//**
 * @brief		Realiza una serie de lecturas para determinar el punto
 * 				de la pantalla donde se presiono la pantalla tactil
 * @param[in]   x_pos	Puntero a la direccion en x donde escribir valor
 * @param[in]	y_pos	Puntero a la direccion en y donde escribir valor
 * @return       none
 *********************************************************************/
void TP_read_coordinates(uint32_t *x_pos, uint32_t *y_pos)
{
	uint32_t temp = 0x0000;
	uint32_t avg_x = 0x0000;
	uint32_t avg_y = 0x0000;

	uint32_t samples = 0x10;
    
    write_pin(tp_cs_port,tp_cs_pin,LOW);
    while(samples > 0)
    {
        send_spi(CMD_RDY);
        avg_y += TP_read();

        send_spi(CMD_RDX);
        avg_x += TP_read();
        
        samples--;
    };
    write_pin(tp_cs_port,tp_cs_pin,HIGH);
    
    temp = (avg_x >> 4);
    temp = filter_data(temp, 0);
    *x_pos = temp;
    
    temp = (avg_y >> 4);
    temp = filter_data(temp, 1);
    *y_pos = temp;
}

/********************************************************************//**
 * @brief		Funcion privada del tactil que promedia los valores
 * 				leidos en un eje.
 * @return       none
 *********************************************************************/
uint32_t filter_data(uint32_t value, uint32_t axis)
{
	uint32_t temp[3];
    
	uint32_t sum = 0.0;
	uint32_t min_R = 0.0;
	uint32_t max_R = 0.0;
	uint32_t axis_max = 0.0;
    
	uint32_t res = 0x0000;
    
	uint32_t i = 0x00;
    
    switch(axis)
    {
        case 1:
        {
            min_R = RL_min;
            max_R = RL_max;
            axis_max = res_l;
            break;
        }
        default:
        {
            min_R = RW_min;
            max_R = RW_max;
            axis_max = res_w;
            break;
        }
    }
    
    temp[0] = map((((uint32_t)value) - error), min_R, max_R, 0, axis_max);
    temp[1] = map((((uint32_t)value) + error), min_R, max_R, 0, axis_max);
    temp[2] = map(((uint32_t)value), min_R, max_R, 0, axis_max);
    
    for(i = 0; i < 3; i++)
    {
        sum += temp[i];
    }
    
    sum /= 3.0;
    res = constrain(sum, 0.0, axis_max);
    
    return res;
}

