// A COMPLETER

/*
Service permettant de chornométrer jusqu'à 59mn 59s 99 1/100
Utilise un timer au choix (TIMER1 à TIMER4).
Utilise la lib MyTimers.h /.c
*/


#include "Chrono.h"
#include "MyTimer.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_tim.h"
#include "stm32f1xx_ll_usart.h"

#define separator 0x3a
#define retour 0x0a
#define usart USART2

// variable privée de type Time qui mémorise la durée mesurée
static Time Chrono_Time; // rem : static rend la visibilité de la variable Chrono_Time limitée à ce fichier 
// variable privée qui mémorise pour le module le timer utilisé par le module
static int lock = 0;
static int cpt = 0;
static TIM_TypeDef * Chrono_Timer=TIM1; // init par défaut au cas où l'utilisateur ne lance pas Chrono_Conf avant toute autre fct.
// déclaration callback appelé toute les 10ms
void Chrono_Task_10ms(void);
void Chrono_Background(void);
void usart_conf(USART_TypeDef * ux);
void Chrono_Conf_io(GPIO_TypeDef * gpio);
/**
	* @brief  Configure le chronomètre. 
  * @note   A lancer avant toute autre fonction.
	* @param  Timer : indique le timer à utiliser par le chronomètre, TIM1, TIM2, TIM3 ou TIM4
  * @retval None
  */
void Chrono_Conf(TIM_TypeDef * Timer)
{
	// Reset Time
	Chrono_Time.Hund=0;
	Chrono_Time.Sec=0;
	Chrono_Time.Min=0;
	
	// Fixation du Timer
	Chrono_Timer=Timer;

	// Réglage Timer pour un débordement à 10ms
	MyTimer_Conf(Chrono_Timer,999, 719);
	
	// Réglage interruption du Timer avec callback : Chrono_Task_10ms()
	MyTimer_IT_Conf(Chrono_Timer, Chrono_Task_10ms,3);
	
	Chrono_Conf_io(GPIOC);
	
	usart_conf(usart);
	// Validation IT
	MyTimer_IT_Enable(Chrono_Timer);
	
	
}

void Chrono_Background(void) {
	
	uint8_t val_pc8;
	uint8_t val_pc13;
	
	val_pc8 = LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_8);
	val_pc13 = !LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_13);
	
	if (val_pc13) { // si bouton user activé, on reset les timer
			Chrono_Reset();
	}
	
	if (!val_pc8) { 
		val_pc8 = LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_8);
		cpt=0;
		if (lock == 0) {
			lock = 1; // on bloque le chrono
			while( cpt < 300 ) {} // on attend en utilisant l'horloge interne pour avoir un "delai"
		}
		else {
			lock = 0; // on unlock le chrono
			while( cpt < 300 ) {}
		}
		
	}
	
	
	
}

void Chrono_Conf_io(GPIO_TypeDef * gpio) {

		/*
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);
		// ------- setup led pc10
		LL_GPIO_InitTypeDef gpioStruct1;
		gpioStruct1.Pin = LL_GPIO_PIN_10; 
		gpioStruct1.Mode = LL_GPIO_MODE_OUTPUT;
		gpioStruct1.Speed = LL_GPIO_MODE_OUTPUT_10MHz;
	  gpioStruct1.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		LL_GPIO_Init(gpio, &gpioStruct1);
		LL_GPIO_ResetOutputPin(gpio, LL_GPIO_PIN_10); // eteint led
		//LL_GPIO_SetOutputPin(gpio, LL_GPIO_PIN_10); // allume led
	
		// set up bouton start/stop pc8
		LL_GPIO_InitTypeDef gpioStruct2;
		gpioStruct2.Pin = LL_GPIO_PIN_8; 
		gpioStruct2.Mode = LL_GPIO_MODE_FLOATING;
		gpioStruct2.Speed = LL_GPIO_MODE_OUTPUT_10MHz;
		gpioStruct2.Pull = LL_GPIO_PULL_UP;
		LL_GPIO_Init(gpio, &gpioStruct2);
		*/
		
		// set up bouton reset timer pc13
	
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);
		LL_GPIO_InitTypeDef gpioStruct3;
		gpioStruct3.Pin = LL_GPIO_PIN_13; 
		gpioStruct3.Mode = LL_GPIO_MODE_FLOATING;
		gpioStruct3.Pull = LL_GPIO_PULL_UP;
		gpioStruct3.Speed = LL_GPIO_MODE_OUTPUT_10MHz;
		LL_GPIO_Init(gpio, &gpioStruct3);
}


/**
	* @brief  Démarre le chronomètre. 
  * @note   si la durée dépasse 59mn 59sec 99 Hund, elle est remise à zéro et repart
	* @param  Aucun
  * @retval Aucun
  */
void Chrono_Start(void)
{
	MyTimer_Start(Chrono_Timer);
}


/**
	* @brief  Arrête le chronomètre. 
  * @note   
	* @param  Aucun
  * @retval Aucun
  */
void Chrono_Stop(void)
{
	MyTimer_Stop(Chrono_Timer);
}


/**
	* @brief  Remet le chronomètre à 0 
  * @note   
	* @param  Aucun
  * @retval Aucun
  */
void Chrono_Reset(void)
{
	// Reset Time
	Chrono_Time.Hund=0;
	Chrono_Time.Sec=0;
	Chrono_Time.Min=0;
}


/**
	* @brief  Renvoie l'adresse de la variable Time privée gérée dans le module Chrono.c
  * @note   
	* @param  Aucun
  * @retval adresse de la variable Time
  */
Time * Chrono_Read(void)
{
	return &Chrono_Time;
}

void usart_conf(USART_TypeDef * ux) {
		LL_USART_Disable(ux);
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
	
		LL_GPIO_InitTypeDef gp;
		
		gp.Pin = LL_GPIO_PIN_1;
		gp.Mode = LL_GPIO_MODE_ALTERNATE;
		gp.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		gp.Speed = LL_GPIO_SPEED_FREQ_HIGH;
		gp.Pull = LL_GPIO_PULL_UP;
	
		LL_GPIO_Init(GPIOA, &gp); // pin TX 1
	
		gp.Pin = LL_GPIO_PIN_2;
		gp.Mode = LL_GPIO_MODE_ALTERNATE;
		gp.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		gp.Speed = LL_GPIO_SPEED_FREQ_HIGH;
		gp.Pull = LL_GPIO_PULL_UP;
	
		LL_GPIO_Init(GPIOA, &gp); // pin RX = 2
	
		LL_USART_InitTypeDef us;
		us.BaudRate = 9600;
		us.HardwareFlowControl =	LL_USART_HWCONTROL_NONE;
		us.TransferDirection = LL_USART_DIRECTION_TX_RX;
		us.Parity = LL_USART_PARITY_NONE;
		us.StopBits = LL_USART_STOPBITS_1;
		us.DataWidth = LL_USART_DATAWIDTH_8B;
		LL_USART_Init(ux, &us);
		
		LL_USART_EnableIT_RXNE(ux);
		LL_USART_Enable(ux);

}


void send2b(int toSend) {
	int dec;
	for (int i=0; i <= 10 ; i++) {
		for (int j=0; j < 10 ; j++) {
				dec = i+j; 
				if (dec == toSend) {
					 LL_USART_TransmitData8(usart,i+0x30);
					 while(LL_USART_IsActiveFlag_TC(usart) == 0) {} // on att la transmission du bit 1
					 LL_USART_TransmitData8(usart,j+0x30);
					 while(LL_USART_IsActiveFlag_TC(usart) == 0) {} // on att la transmission du bit 2
					 i=11;
					 j=10;
				}
		}			
	}
}

void send1b(int toSend) {
	LL_USART_TransmitData8(usart,toSend);
	while(LL_USART_IsActiveFlag_TC(usart) == 0) {} // on att la transmission du bit 1
}

/**
	* @brief  incrémente la variable privée Chron_Time modulo 60mn 
  * @note   
	* @param  Aucun
  * @retval Aucun
  */
void Chrono_Task_10ms(void)
{ 
	cpt++;
	
	send2b(Chrono_Time.Min);
	send1b(separator);
	send2b(Chrono_Time.Sec);
	send1b(separator);
	send2b(Chrono_Time.Hund);
	send1b(retour);
	
	if (Chrono_Time.Sec % 2 == 0) {
		LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_10);
	}
	else {
		LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_10);
	}
	
	if (lock == 0) {
		Chrono_Time.Hund++;
		if (Chrono_Time.Hund==100)
		{
			Chrono_Time.Sec++;
			Chrono_Time.Hund=0;
		}
		if (Chrono_Time.Sec==60)
		{
			Chrono_Time.Min++;
			Chrono_Time.Sec=0;
		}
		if (Chrono_Time.Min==60)
		{
			Chrono_Time.Hund=0;
		}
	}
	
}


