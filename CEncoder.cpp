#include <cmath>
#include "CEncoder.h"

using namespace std;

CEncoder::CEncoder(CConfig *pParametros,int pi_ID)
{
    pParam = pParametros;
    piID=pi_ID;
    pLogger = log4cxx::Logger::getRootLogger();
}

CEncoder::~CEncoder()
{
}

void CEncoder::run()
{
    int p1 = pin();
    if (p1 < p0 && sentido == sentidoMovimiento::CW)
        rPosicion += 1;
    if (p1 > p0 && sentido == sentidoMovimiento::CCW)
        rPosicion -= 1;
    p0 = p1;
}

int CEncoder::posicion()
{
    int nuevaPosicion = rPosicion;
    if (nuevaPosicion < 0)
        nuevaPosicion += rPosiciones;
    if (nuevaPosicion >= rPosiciones)
        nuevaPosicion -= floor(nuevaPosicion / rPosiciones) * rPosiciones;

    return nuevaPosicion;
}

int CEncoder::posiciones()
{
    return rPosiciones;
}

void CEncoder::fin()
{
    sentido = sentidoMovimiento::PARADO;
}

sentidoMovimiento CEncoder::sentidoMasCorto(int pos)
{
    int posActual = posicion();
    if (pos > posActual)
        return sentidoMovimiento::CW;
    else
        return sentidoMovimiento::CCW;
}

void CEncoder::calibrar(bool recalibrar){
    if (rPosiciones>0 && !recalibrar)
        return;

    if (!domeAtHome()){
        rPosicion=0;
        rPosiciones=10000;
        inicio(sentidoMovimiento::CW,0,false,true);
        rPosicion=0;
        inicio(sentidoMovimiento::CW,0,true,true);
    }

}