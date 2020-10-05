// A COMPLETER

/*
Service permettant de chornom�trer jusqu'� 59mn 59s 99 1/100
Utilise un timer au choix (TIMER1 � TIMER4).
Utilise la lib MyTimers.h /.c
*/


#include "Chrono.h"
#include "MyTimer.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_tim.h"

// variable priv�e de type Time qui m�morise la dur�e mesur�e
static Time Chrono_Time; // rem : static rend la visibilit� de la variable Chrono_Time limit�e � ce fichier 
// variable priv�e qui m�morise pour le module le timer utilis� par le module
static int lock = 0;
static int cpt = 0;
static TIM_TypeDef * Chrono_Timer=TIM1; // init par d�faut au cas o� l'utilisateur ne lance pas Chrono_Conf avant toute autre fct.
// d�claration callback appel� toute les 10ms
void Chrono_Task_10ms(void);
void Chrono_Background(void);
void Chrono_Conf_io(GPIO_TypeDef * gpio);
/**
	* @brief  Configure le chronom�tre. 
  * @note   A lancer avant toute autre fonction.
	* @param  Timer : indique le timer � utiliser par le chronom�tre, TIM1, TIM2, TIM3 ou TIM4
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

	// R�glage Timer pour un d�bordement � 10ms
	MyTimer_Conf(Chrono_Timer,999, 719);
	
	// R�glage interruption du Timer avec callback : Chrono_Task_10ms()
	MyTimer_IT_Conf(Chrono_Timer, Chrono_Task_10ms,3);
	
	Chrono_Conf_io(GPIOC);
	// Validation IT
	MyTimer_IT_Enable(Chrono_Timer);
	
	
}

void Chrono_Background(void) {
	
	uint8_t val_pc8;
	uint8_t val_pc13;
	
	val_pc8 = LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_8);
	val_pc13 = !LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_13);
	
	if (val_pc13) { // si bouton user activ�, on reset les timer
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
		
		// set up bouton reset timer pc13
	
		LL_GPIO_InitTypeDef gpioStruct3;
		gpioStruct3.Pin = LL_GPIO_PIN_13; 
		gpioStruct3.Mode = LL_GPIO_MODE_FLOATING;
		gpioStruct3.Pull = LL_GPIO_PULL_UP;
		gpioStruct3.Speed = LL_GPIO_MODE_OUTPUT_10MHz;
		LL_GPIO_Init(gpio, &gpioStruct3);
}


/**
	* @brief  D�marre le chronom�tre. 
  * @note   si la dur�e d�passe 59mn 59sec 99 Hund, elle est remise � z�ro et repart
	* @param  Aucun
  * @retval Aucun
  */
void Chrono_Start(void)
{
	MyTimer_Start(Chrono_Timer);
}


/**
	* @brief  Arr�te le chronom�tre. 
  * @note   
	* @param  Aucun
  * @retval Aucun
  */
void Chrono_Stop(void)
{
	MyTimer_Stop(Chrono_Timer);
}


/**
	* @brief  Remet le chronom�tre � 0 
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
	* @brief  Renvoie l'adresse de la variable Time priv�e g�r�e dans le module Chrono.c
  * @note   
	* @param  Aucun
  * @retval adresse de la variable Time
  */
Time * Chrono_Read(void)
{
	return &Chrono_Time;
}




/**
	* @brief  incr�mente la variable priv�e Chron_Time modulo 60mn 
  * @note   
	* @param  Aucun
  * @retval Aucun
  */
void Chrono_Task_10ms(void)
{ 
	cpt++;
	
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


