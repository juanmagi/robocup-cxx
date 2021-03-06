#pragma once
#include <string>
#include <thread>
#include "log4cxx/logger.h"
#include "CConfig.h"
#include "global.h"

class CCupulaFijo
{
private:
    CConfig *pParam=nullptr;
    log4cxx::LoggerPtr pLogger=nullptr;
    int piID=-1;
    std::thread *pEncoder=nullptr;
    enum tipoPosicion {absoluta,relativa};

    void onCupulaMovil(bool on);


public:
    CCupulaFijo(CConfig *pParametros,int pi_ID);
    ~CCupulaFijo();

    static bool mapaPines[100];

    triestado estadoDomeAtHome();
    /* true: Mueve la cupula a la posicion DAH en la direccion CW
       false: Desplaza la cupula 20 posiciones en el sentido CW de la posicion DAH
    */
    void DomeAtHome(bool valor);
    int getPosicion();
    void setPosicion(int posicion, tipoPosicion tp, sentidoMovimiento dm);
    void calibrate(bool recalibrar=false);
    void mover(sentidoMovimiento sentido);
    void parar();
    static void runEncoder(CConfig* pParametros,int pi_ID);
    static void runEncoderSimulator(CConfig* pParametros);

};
