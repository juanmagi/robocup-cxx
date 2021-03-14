#pragma once
#include <string>
#include <thread>
#include "log4cxx/logger.h"
#include "pigpiod_if2.h"
#include "CConfig.h"
#include "global.h"

class CCupulaFijo
{
private:
    //Variables convencionales
    CConfig *pParam = nullptr;
    log4cxx::LoggerPtr pLogger = nullptr;
    int piID = -1;
    std::thread *pEncoder = nullptr;
    std::thread *pEncoderSimulator = nullptr;
    std::thread *pStop = nullptr;

    //Funciones convencionales
    void onCupulaMovil(bool on);
    sentidoMovimiento sentidoMasCorto(int pos);


public:
    CCupulaFijo(CConfig *pParametros, int pi_ID);
    ~CCupulaFijo();

    //Funciones convencionales

    /*  true: Mueve la cupula a la posicion DAH en la direccion CW
    *   false: Desplaza la cupula 20 posiciones en el sentido CW de la posicion DAH
    */
    void DomeAtHome(bool valor);
    /*  Versión no static de la función que indica si la cúula está o no está en DAH. 
    *   Return:
    *   true: la cúpula está         if (mapaPines[pParametros->gpio_pin_act_cw]==false && mapaPines[pParametros->gpio_pin_act_cw]==false)
en DAH
    *   false: la cúpula no está en DAH
    */
    bool estadoDAH() { return estadoDAH(pParam, piID); }
    int getPosicion();
    void setPosicion(sentidoMovimiento sm, tipoPosicion tp,int posicion);
    void calibrate(bool recalibrar = false);
    void mover(sentidoMovimiento sm);
    /*  Para el movimiento de la cúpula
    */
    void parar(){mover(sentidoMovimiento::PARADO);}
    void finalizarThreads(tipoThread valor);

    //Funciones static
    /*  Versión static de la función que indica si la cúula está o no está en DAH. Se puede llamar desde dentro de los threads.
    *   Return:
    *   true: la cúpula está en DAH
    *   false: la cúpula no está en DAH
    */
    static void pararMovimiento(CConfig *pParametros, int pi_ID);
    static bool estadoDAH(CConfig *pParametros, int pi_ID);
    static void operarPosicionAbsoluta(CConfig *pParametros, int valor);
    /*  Código del Thread que controla los pines de encoder y dah
    */
    static void runEncoder(CConfig *pParametros, int pi_ID);
    /*  Código del Thread que simula un encoder
    */
    static void runEncoderSimulator(CConfig *pParametros);
    /*  Código del Thread que simula un encoder
    */
    static void runStop(CConfig *pParametros, int pi_ID,tipoPosicion tp,int posRelIni);

    //Variables convencionales
    
    //Variables static
    static sentidoMovimiento sentido;
    static bool mapaPines[100];
    static int posicionAbsoluta, posicionRelativa;
    static int posicionAbsolutaFinal, posicionRelativaFinal;
};
