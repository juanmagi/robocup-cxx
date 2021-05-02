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
    /*
    Determina cual es el sentido de movimiento que hace el camino más corto hasta la posición indicada
    Param
        posicion Posición de destino
    Return
        sentido de movimiento del camino más corto
    */
    sentidoMovimiento sentidoMasCorto(int pos);

public:
    CCupulaFijo(CConfig *pParametros, int pi_ID);
    ~CCupulaFijo();

    //Funciones convencionales

    /*  
    *   Mueve la cúpula al DAH o cerca de ese punto
    *   true: Mueve la cupula a la posicion DAH en la direccion CW
    *   false: Desplaza la cupula 20 posiciones en el sentido CW de la posicion DAH
    */
    void DomeAtHome(bool valor);
    /*  Versión no static de la función que indica si la cúula está o no está en DAH. 
    *   Return:
    *   true: la cúpula está en DAH
    *   false: la cúpula no está en DAH
    */
    bool estadoDAH() { return estadoDAH(pParam, piID); }
    /*
    Devuelve la posición de la cúpula
    Param
        angulo > -1 Devuelve el valor de la posición informada en el ángulo en unidades
        angulo = -1 Devueve la posición de la cúpula en unidades
    Return
        Posición en unidades según el valor del ángulo informado en la entrada
    */
    int getPosicion(int angulo = -1);
    /*
    Devuelve la posición de la cúpula
    Param
        posicion > -1 Devuelve el valor de la posición informada en grados
        posicion = -1 Devueve la posición de la cúpula en grados
    Return
        Posición en grados según el valor de la posición informada en la entrada
    */
    int getAngulo(int posicion = -1);
    /*
    Mueve la cúpula a la posición solicitada
    Param
        sm sentido del movimiento
        tp tipo de posición
        posicion según el tipo de posicicionamiento, posición a la que mover la cúpula en unidades
    Return
        Ninguna
    */
    void setPosicion(sentidoMovimiento sm, tipoPosicion tp, int posicion);
    /*
    Mueve la cúpula a la posición solicitada
    Param
        sm sentido del movimiento
        tp tipo de posición
        posicion según el tipo de posicicionamiento, posición a la que mover la cúpula en grados
    Return
        Ninguna
    */
    void setAngulo(sentidoMovimiento sm, tipoPosicion tp, int angulo);
    /*
    Calibra la cúpula. Es básico para poder hacer movimientos absolutos y detectar posiciones absolutas
    Param
        recalibrar=true, recalibra la cúpula aunque ya se haya recalibrado previamente
        recalibrar=false, Si la cúpula ya está calibrada, no calibra
        moverAdah=true, Mueve la cúpula a la posición DAH
    Return
        Ninguna
    */
    void calibrate(bool recalibrar = false, bool moverAdah = true);
    /*
    Inicia el movimiento de la cúpula en el sentido sm pero sin punto final de manera que si no se da instrucción de parar girará indefinidamente
    Param
        sm sentido del movimiento
    Return
        Ninguna
    */
    void mover(sentidoMovimiento sm);
    /*  Para el movimiento de la cúpula
    */
    void parar() { mover(sentidoMovimiento::PARADO); }
    /*
    Finaliza Threads
    Param
        valor Thread a finalizar 
    Return
        Ninguna
    */
    void finalizarThreads(tipoThread valor);

    //Funciones static

    /*  Para el movimiento de la parte fija de la cúpula
       Param:
           CConfig *pParametros: Puntero al objeto de configuración
           int pi_ID: ID del pigpio
       Return: 
           Ninguno
    */
    static void pararMovimiento(CConfig *pParametros, int pi_ID);
    /* Versión static de la función que indica si la cúpula está o no está en DAH. Se puede llamar desde dentro de los threads.
       Param:
           CConfig *pParametros: Puntero al objeto de configuración
           int pi_ID: ID del pigpio
       Return:
       true: la cúpula está en DAH
       false: la cúpula no está en DAH
    */
    static bool estadoDAH(CConfig *pParametros, int pi_ID);
    /* Suma a la posición absoluta el valor informado en valor. Este parámetro puede ser negativo
       Param:
           CConfig *pParametros: Puntero al objeto de configuración
           valor la posición relativa a suma a la posición absoluta. Puede ser negativo
       Return:
            Ninguno. El valor totalizado se suma en la variable global de la posición absoluta
    */
    static void operarPosicionAbsoluta(CConfig *pParametros, int valor);
    /* Ejecuta el thread que gestiona el encoder y el DAH
       Param:
           CConfig *pParametros: Puntero al objeto de configuración
           int pi_ID: ID del pigpio
       Return:
            Ninguno
    */
    static void runEncoder(CConfig *pParametros, int pi_ID);
    /* Ejecuta el thread que simula el movimiento de la cúpula
       Param:
           CConfig *pParametros: Puntero al objeto de configuración
       Return:
            Ninguno
    */
    static void runEncoderSimulator(CConfig *pParametros);
    /* Ejecuta el thread que gestiona la parada de la cúpula
       Param:
           CConfig *pParametros: Puntero al objeto de configuración
           int pi_ID: ID del pigpio
           tp tipo de posición con la que se ha solictdo el inicio del movimiento
           posRelIni posición relativa de la cúpula al inicio del movimiento
       Return:
            Ninguno
    */
    static void runStop(CConfig *pParametros, int pi_ID, tipoPosicion tp, int posRelIni);
    /*  Activa el rele que da corriente a la parte móvil
       Param:
           CConfig *pParametros: Puntero al objeto de configuración
           int pi_ID: ID del pigpio
       Return:
            Ninguno
    */
    static void onCupulaMovil(CConfig *pParametros, int pi_ID, bool on);

    //Variables convencionales

    //Variables static
    static sentidoMovimiento sentido;
    static bool mapaPines[100];
    static int posicionAbsoluta, posicionRelativa;
    static int posicionAbsolutaFinal, posicionRelativaFinal;
};
