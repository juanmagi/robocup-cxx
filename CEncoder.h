#pragma once
#include <string.h>
#include "pigpiod_if2.h"
#include "log4cxx/logger.h"
#include "CConfig.h"
#include "global.h"

class CEncoder
{
private:

protected:
    CConfig *pParam = nullptr;
    log4cxx::LoggerPtr pLogger = nullptr;
    sentidoMovimiento sentido = sentidoMovimiento::PARADO;
    int piID=-1;
    int t0 = 0;
    int p0 = PI_OFF;
    int rPosicion = 0;
    int rPosiciones = 0;

public:
    CEncoder(CConfig *pParametros,int pi_ID);
    ~CEncoder();

    virtual void run();
    int posicion();
    int posiciones();
    void fin();
    /*
    *Busca el sentido de giro adecuado para ir a la posición dada por el camino más corto
    */
    sentidoMovimiento sentidoMasCorto(int pos);
    /*
    *Mueve la cúpula para contar el número de pins que tiene.
    *Si recalibrar=True, fuerza la operación aunque ya esté calibrada
    */
    void calibrar(bool recalibrar = false);
    virtual int pin(){return 0;};
    virtual bool domeAtHome(){return true;};
    virtual void inicio(sentidoMovimiento sen,int pos,bool vueltaCompleta=false,bool dah=false){return;}
};
