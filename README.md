EER34 – API Elemon para stack LoRaWan de SAMR34
Introducción
Este documento es una versión inicial, incompleta, provisoria y preliminar.
El objetivo de esta API es crear un ambiente amigable para desarrollar aplicaciones bajo AtmeStudio para el SAMR34, usando el stack LoRaWan de Microchip, basado en las aplicaciones de demo, pero con una API más sencilla, con prestaciones similares a los comandos del RN2903, y con una estructura simple de la aplicación, similar a la de Arduino.
Funciones de la API
Funciones de inicialización:
void EES34_reset(void);
void EER34_init(void);
void EER34_tickStart(int t);
void EER34_tickStop(void);
int EER34_sleep(uint32_t time);

Funciones de seteos de parametros:
int EER34_setDevEui(uint8_t *devEui);
int EER34_setAppEui(uint8_t *appEui);
int EER34_setAppKey(uint8_t *appKey);
int EER34_setDevAddr(uint32_t *devAddr);
int EER34_setAppSKey(uint8_t *appSKey);
int EER34_setNwkSKey(uint8_t *nwkSKey);
int EER34_setBand(IsmBand_t band, int subBand);

Funciones de operación LoRaWan:
int EER34_joinOTAA(void);
int EER34_joinABP(void);
int EER34_tx(EER34_txMode_t mode, int port, uint8_t *data, int len);

Callbacks a la aplicación:
void EER34_statusCallback(EER34_status_t sts, StackRetStatus_t LoraSts);
void EER34_rxDataCallback(int port, uint8_t *data, int len);
void EER34_tickCallback(void);
void EES34_enterLowPower(void);
void EES34_exitLowPower(void);

Funciones de la aplicación:
void EES34_appInit(void);
void EES34_appTask(void);
Proyecto de Ejemplo
El proyecto de ejemplo hace el join OTAA y transmite cada 5 segundos un paquete fijo con confirmación, imprimiendo mensajes explicando las acciones que va realizando, y cada tres paquetes enviados se pone en modo de bajo consumo por 15 segundos.
Usa las calbacks y el timer de tick.
Está todo armado, compila y funciona bien.
Todos los archivos fuente .c y .h de la aplicación se pueden poner en: ProjectFolder\src\EER34\App\
El archivo principal de la aplicación es app.c, y en el ejemplo es el único archivo fuente de la aplicación. En principio no hay un app.h porque todas las funciones de app.c que son parte de la API están declaradas en eer34.h. 
Las funciones de calback que son opcionales están declaradas weak en eer34.c, de manera que en app.c solo hace falta que poner las que use la aplicación.
Como se Creó el Proyecto de Ejemplo
Partiendo del proyecto APPS_ED_DEMO_SER_NWK_PROVISION1.atsln se creó una carpeta nueva colgada de Src, que se llame EER34. Al crear la carpeta en el proyecto el AtmelStudio crea un subdirectorio, y ahí se pusieron los archivos EER34.c y .h, que hay que agregarlos a esa carpeta del proyecto.
Se agregó el include de eer34.h al comienzo de main.c y se agrego el path correspondiente en el proyecto.
En main.c se reemplazaron las líneas:
    SwTimerCreate(&demoTimerId);
    SwTimerCreate(&lTimerId);

    mote_demo_init();

por:
    EER34_init();
Y un poco antes se reemplazó la línea:
print_reset_causes();
por:
	EES34_reset();
Y se borro en el main la función print_reset_causes();
Se creó otra carpeta de proyecto colgada de EER34 llamada App, para poner los archivos .c y .h de la aplicación. El archivo app.c es el principal, donde este el task de la aplicación, la inicialización, callbacks, etc.
Se eliminaron los archivos enddevice_Demo.c y enddevice_cert.c.
En app.c se crearon las funciones de inicialización task y callbacks necesarias para la aplicación de ejemplo.

