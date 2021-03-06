#include <unistd.h>
#include "CCupulaFijo.h"
#include "CEncoder.h"

using namespace std;

bool CCupulaFijo::mapaPines[100] = {false};
CCupulaFijo::CCupulaFijo(CConfig *pParametros, int pi_ID)
{
    pParam = pParametros;
    piID = pi_ID;
    pLogger = log4cxx::Logger::getRootLogger();
    thread th1(runEncoder, pParametros, pi_ID);
    thread th2(runEncoderSimulator, pParametros);
    int kk = 0;
}

CCupulaFijo::~CCupulaFijo()
{
}

void CCupulaFijo::runEncoder(CConfig *pParametros, int pi_ID)
{
}

void CCupulaFijo::runEncoderSimulator(CConfig *pParametros)
{
    int ccw = pParametros->gpio_pin_act_ccw;
    int cw = pParametros->gpio_pin_act_cw;
    int pinEncoder = pParametros->gpio_pin_inp_encoder;
    useconds_t periodo = 1000000;          //Periodo en microsegundos
    unsigned int longitudOnda = 10;        //Undidades de distacia
    useconds_t tiempoEntreMuestras = periodo / longitudOnda; //tiempo entre una muestra y la siguiente
    int posicion = 0;

    while (true)
    {
        if (posicion >= 0 && posicion < 5)
            mapaPines[pinEncoder] = true;
        else
            mapaPines[pinEncoder] = false; //Se devuelve false en el 1 de la onda
        if (mapaPines[cw] == true && mapaPines[ccw] == false)
        { //Movimientos CCW
            posicion += 1;
            if (posicion == longitudOnda)
                posicion = 0;
        }
        else if (mapaPines[cw] == false && mapaPines[ccw] == true) //Movimiento CW
        {
            posicion -= 1;
            if (posicion == -1)
                posicion = longitudOnda - 1;
        }
        usleep(tiempoEntreMuestras);
    }
}

void CCupulaFijo::onCupulaMovil(bool on)
{
}
