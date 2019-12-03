/*
 * app.c
 *
 * Created: 22/10/2019 1:44:19 p. m.
 *  Author: gmont_000
 */ 
#include "..\eer34.h"

// Variables privadas

int timer1;

static enum {
	APP_FSM_INIT = 0,
	APP_FSM_JOINING,
	APP_FSM_JOINFAILED,
	APP_FSM_JOINED,
	APP_FSM_TXWAIT,
	APP_FSM_TXOK,
	APP_FSM_TXERROR,
	APP_FSM_IDLE,
	APP_FSM_IDLE_TX,
	APP_FSM_SLEEP,
} fsm;

/** 
 *	@brief	Callback de status para la aplicacion
 *	@param	sts			status de EER34
 *	@param	loraSts		status del stack LoRaWan
 */
void EER34_statusCallback(EER34_status_t sts, StackRetStatus_t LoraSts)
{
	if (sts == EER34_STATUS_JOIN_SUCCSESS) {
		if (fsm == APP_FSM_JOINING)
			fsm = APP_FSM_JOINED;
	}
	else if (sts == EER34_STATUS_JOIN_ERROR) {
		if (fsm == APP_FSM_JOINING)
			fsm = APP_FSM_JOINFAILED;
	}
	else if (sts == EER34_STATUS_TX_SUCCSESS) {
		if (fsm == APP_FSM_TXWAIT)
			fsm = APP_FSM_TXOK;
	}
	else if (sts == EER34_STATUS_TX_TIMEOUT) {
		if (fsm == APP_FSM_TXWAIT)
		fsm = APP_FSM_TXERROR;
	}
}

/** 
 *	@brief	Callback de recepcion de datos la aplicacion
 *	@param	port		numero de puerto
 *	@param	data		puntero a los datos
 *	@param	len			cantidad de bytes de datos
 */
void EER34_rxDataCallback(int port, uint8_t *data, int len)
{
}

/** 
 *	@brief Callback de entrada en low power
 *
 *  Sirve para que la aplicacion deinicialice y/o apague lo que haga falta
 *  al entrar en el modo de bajo consumo.
 */
void EES34_enterLowPower(void)
{
	// Hay que detener el tick sino no entra en bajo consumo
	EER34_tickStop();
}

/** 
 *	Callback de salida de low power
 *
 *  Sirve para que la aplicacion reponga los recursos al despertar del modo 
 *  de bajo consumo, volviendo a configurar y/o encender los recursos
 *  que deinicializo y/o apago al entrar en el modo de bajo consumo.
 */
void EES34_exitLowPower(void)
{
	// Vuelve a enceder el tick que lo apago al entrar en bajo consumo
	EER34_tickStart(10);
}

/** 
 *	Funcion de inicializacion de la aplicacion
 */
void EES34_appInit(void)
{
	static volatile int res;
	uint8_t devEuix[] = {0xDE, 0xAF, 0xFA, 0xCE, 0xDE, 0xAF, 0x55, 0x20};
	uint8_t appEuix[] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
	uint8_t appKeyx[] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
		0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
		
	printf("\r\n\r\nEESAMR34\r\nInitializing\r\n");

	// Este seteo debe ir primero porque inicia el stack LoRaWan
	res = EER34_setBand(ISM_AU915, 1);

	// Estos seteos pueden ir en cualqueir orden pero siempre
	// despues de setear la banda (sino dan error)
	res = EER34_setDevEui(devEuix);
	res = EER34_setAppEui(appEuix);
	res = EER34_setAppKey(appKeyx);
	res = EER32_setDeviceClass(CLASS_A);
	
	// Arranca tick de 10ms
	EER34_tickStart(10);	// arranca tick de 10ms

	printf("Initialization Done\r\n\r\n");
}

/** 
 *	@brief	Callback del tick de la aplicacion
 */
void EER34_tickCallback(void)
{
	if (timer1)	timer1--;
}

/** 
 *	Task de la aplicacion
 */
void EES34_appTask(void)
{
	unsigned char data[] = {"ABCD"};
	static int count;
		
	switch(fsm) {
	case APP_FSM_JOINFAILED:
		printf("Join failed\r\n\r\n");
	case APP_FSM_INIT:
		printf("Sending join request\r\n");
		EER34_joinOTAA();
		fsm = APP_FSM_JOINING;
		break;
	case APP_FSM_JOINING:
		break;
	case APP_FSM_JOINED:
		count = 1;
		printf("Joined\r\n\r\n");
		if (EER34_tx(EER34_TXMODE_UNCONF, 1, data, 4)) {
			printf("Transmitting ...\r\n");
			fsm = APP_FSM_TXWAIT;
		}
		else {
			printf("Transmit Error\r\n");
			timer1 = 500;
			fsm = APP_FSM_IDLE;
		}
		break;
	case APP_FSM_TXWAIT:
		break;
	case APP_FSM_TXOK:
		printf("Transmit OK\r\n");
		timer1 = 500;
		fsm = APP_FSM_IDLE;
		break;
	case APP_FSM_TXERROR:
		printf("Transmit Timeout\r\n");
		timer1 = 500;
		fsm = APP_FSM_IDLE;
		break;
	case APP_FSM_IDLE:
		if (count % 3 == 0) {
			printf("\r\nGoing to sleep ...\r\n");
			timer1 = 0;
			fsm = APP_FSM_SLEEP;
		}
		else
			fsm = APP_FSM_IDLE_TX;
		break;
	case APP_FSM_IDLE_TX:
		if (!timer1) {
			if (EER34_tx(EER34_TXMODE_CONF, 1, data, 4)) {
				count++;
				printf("Transmitting ...\r\n");
				fsm = APP_FSM_TXWAIT;
			}
			else
				printf("Transmit Error\r\n");
			timer1 = 500;
		}
		break;
	case APP_FSM_SLEEP:
		if (!timer1) {
			if (EER34_sleep(15000)) {
				printf("Slept OK, woken-up!\r\n\r\n");
				fsm = APP_FSM_IDLE_TX;
			}
			else {
				printf("Sleep failed\r\n");
				timer1 = 50;
			}
		}
		break;
	default:
		break;
	}
}

/** 
 *	Callback de procesamiento de la causa de reset
 */
void EES34_appResetCallback(unsigned int rcause)
{
	printf("Last reset cause: ");
	if(rcause & (1 << 6)) {
		printf("System Reset Request\r\n");
	}
	if(rcause & (1 << 5)) {
		printf("Watchdog Reset\r\n");
	}
	if(rcause & (1 << 4)) {
		printf("External Reset\r\n");
	}
	if(rcause & (1 << 2)) {
		printf("Brown Out 33 Detector Reset\r\n");
	}
	if(rcause & (1 << 1)) {
		printf("Brown Out 12 Detector Reset\r\n");
	}
	if(rcause & (1 << 0)) {
		printf("Power-On Reset\r\n");
	}
}
