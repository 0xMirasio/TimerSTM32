
/*
 indispensable pour pouvoir adresser les registres des périphériques.
 Rem : OBLIGATION d'utiliser les définitions utiles contenues dans ce fichier (ex : TIM_CR1_CEN, RCC_APB1ENR_TIM2EN ...)
 pour une meilleure lisibilité du code.

 Pour les masques, utiliser également les définitions proposée
 Rappel : pour mettre à 1  , reg = reg | Mask (ou Mask est le représente le ou les bits à positionner à 1)
				  pour mettre à 0  , reg = reg&~ Mask (ou Mask est le représente le ou les bits à positionner à 0)
 
*/ 
#include "stm32f103xb.h" 

static void (*func)(void); // variable statique contenant l'adresse de callback_10ms

void MyTimer_Conf(TIM_TypeDef * Timer,int Arr, int Psc) {
	Timer->ARR = Arr; // conf du autoreload
	Timer->PSC = Psc; // conf du prescaler
	
}

void MyTimer_Start(TIM_TypeDef * Timer) {
	Timer->CR1 = Timer->CR1 | (1 << 0); // lancement du timer
}

void MyTimer_Stop(TIM_TypeDef * Timer) {
	Timer->CR1 = Timer->CR1 & (0 << 0); // fin du timer
}


void MyTimer_IT_Conf(TIM_TypeDef * Timer, void (*callback) (void),int Prio) {
	//configuration de TIM2 pour appeler callback tout les X ms avec une certaine priorité
	int X = 28; // numéro interuptions (0-81)
	NVIC->ISER[0] = NVIC->ISER[0] | (1 << X); // activation de l'interuptions
	//configuration de la priorité
	NVIC->IP[X] = NVIC->IP[X] | (Prio << 4);
	
	func = callback;
}

void MyTimer_IT_Enable(TIM_TypeDef * Timer) {
	Timer->DIER |= TIM_DIER_UIE; // autorise les interruptions sur TIM2
	
}

void MyTimer_IT_Disable(TIM_TypeDef * Timer) {
	Timer->DIER &= ~TIM_DIER_UIE; // interdit les interruptions sur TIM2
	
}

void TIM2_IRQHandler(void) { 
	// do nothing, test
	TIM2->SR &= ~TIM_SR_UIF; // validation du timer
	func();
}
