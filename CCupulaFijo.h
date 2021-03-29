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
    int getPosicion(int angulo=-1);
    int getAngulo(int posicion=-1);
    void setPosicion(sentidoMovimiento sm, tipoPosicion tp,int posicion);
    void setAngulo(sentidoMovimiento sm, tipoPosicion tp,int angulo);
    void calibrate(bool recalibrar = false, bool moverAdah=true);
    void mover(sentidoMovimiento sm);
    /*  Para el movimiento de la cúpula
    */
    void parar(){mover(sentidoMovimiento::PARADO);}
    void finalizarThreads(tipoThread valor);

    //Funciones static

    /*  Para el movimiento de la parte fija de la cúpula
    *   Param:
    *       CConfig *pParametros: Puntero al objeto de configuración
    *       int pi_ID: ID del pigpio
    *   Return: 
    *       Ninguno
    */
    static void pararMovimiento(CConfig *pParametros, int pi_ID);
    /*  Versión static de la función que indica si la cúpula está o no está en DAH. Se puede llamar desde dentro de los threads.
    *   Return:
    *   true: la cúpula está en DAH
    *   false: la cúpula no está en DAH
    */
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
    /*  Activa el rele que da correinte a la parte móvil
    */
    static void onCupulaMovil(CConfig *pParametros, int pi_ID,bool on);

    //Variables convencionales
    
    //Variables static
    static sentidoMovimiento sentido;
    static bool mapaPines[100];
    static int posicionAbsoluta, posicionRelativa;
    static int posicionAbsolutaFinal, posicionRelativaFinal;
};
