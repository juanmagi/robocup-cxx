#include <unistd.h>
#include <iostream>
#include "CCupulaFijo.h"
#include "CEncoder.h"

using namespace std;

bool CCupulaFijo::mapaPines[100] = {false};
int CCupulaFijo::posicionAbsoluta = -1;
int CCupulaFijo::posicionRelativa = 0;

CCupulaFijo::CCupulaFijo(CConfig *pParametros, int pi_ID)
{
    pParam = pParametros;
    piID = pi_ID;
    pLogger = log4cxx::Logger::getRootLogger();

    //TODO Hay que leer las posiciones máximas en el disco. Ahi se guarda cuando se calibra la parte fija. Si no se ha calibrado nunca, tendra -1

    if (pParam->general_simulacion == 0)
    {
        int errCode;
        if (errCode = set_mode(piID, pParam->gpio_pin_act_ccw, PI_OUTPUT) != 0)
        {
            LOG4CXX_FATAL(pLogger, "Error estableciendo las características del pin gpio_pin_act_ccw con código de error: " + to_string(errCode));
            return;
        }
        if (errCode = set_mode(piID, pParam->gpio_pin_act_cw, PI_OUTPUT) != 0)
        {
            LOG4CXX_FATAL(pLogger, "Error estableciendo las características del pin gpio_pin_act_cw con código de error: " + to_string(errCode));
            return;
        }
        if (errCode = set_mode(piID, pParam->gpio_pin_inp_dah, PI_INPUT) != 0)
        {
            LOG4CXX_FATAL(pLogger, "Error estableciendo las características del pin gpio_pin_inp_dah con código de error: " + to_string(errCode));
            return;
        }
        if (errCode = set_mode(piID, pParam->gpio_pin_inp_encoder, PI_INPUT) != 0)
        {
            LOG4CXX_FATAL(pLogger, "Error estableciendo las características del pin gpio_pin_inp_encoder con código de error: " + to_string(errCode));
            return;
        }

        if (errCode = gpio_write(piID, pParam->gpio_pin_act_ccw, PI_HIGH) != 0)
        {
            LOG4CXX_FATAL(pLogger, "Error estableciendo las características del pin gpio_pin_act_ccw con código de error: " + to_string(errCode));
            return;
        }
        if (errCode = gpio_write(piID, pParam->gpio_pin_act_cw, PI_HIGH) != 0)
        {
            LOG4CXX_FATAL(pLogger, "Error estableciendo las características del pin gpio_pin_act_cw con código de error: " + to_string(errCode));
            return;
        }

        if (errCode = set_pull_up_down(piID, pParam->gpio_pin_inp_dah, PI_PUD_UP) != 0)
        {
            LOG4CXX_FATAL(pLogger, "Error estableciendo las características del pin gpio_pin_inp_dah con código de error: " + to_string(errCode));
            return;
        }
        if (errCode = set_pull_up_down(piID, pParam->gpio_pin_inp_encoder, PI_PUD_UP) != 0)
        {
            LOG4CXX_FATAL(pLogger, "Error estableciendo las características del pin gpio_pin_inp_encoder con código de error: " + to_string(errCode));
            return;
        }
    }
    else //Si es simulación
        pEncoderSimulator = new thread(runEncoderSimulator, pParametros);

    pEncoder = new thread(runEncoder, pParametros, pi_ID);
}

CCupulaFijo::~CCupulaFijo()
{
    if (pEncoder != nullptr)
        delete pEncoder;
    if (pEncoderSimulator != nullptr)
        delete pEncoderSimulator;
}

void CCupulaFijo::runEncoder(CConfig *pParametros, int pi_ID)
{
    int ccw = pParametros->gpio_pin_act_ccw;
    int cw = pParametros->gpio_pin_act_cw;
    int dah = pParametros->gpio_pin_inp_dah;
    int pinEncoder = pParametros->gpio_pin_inp_encoder;
    int valor, valorAnterior;
    bool darPosicionAbsoluta = true;
    int tiempoEntreLecturas=10000;
    log4cxx::LoggerPtr pLoggerLocal = log4cxx::Logger::getRootLogger();

    if (pParametros->cupula_max_posiciones < 0)
    {
        darPosicionAbsoluta = false;
        LOG4CXX_INFO(pLoggerLocal, "Cúpula no calibrada. Sólo se obtienen posiciones relativas, considerando la posición actual como posición 0");
    }
    else if (!estadoDAH(pParametros, pi_ID))
    { //Cúpula no está en la posición DAH
        darPosicionAbsoluta = false;
        LOG4CXX_INFO(pLoggerLocal, "Cúpula no en DAH. Sólo se obtienen posiciones relativas, considerando la posición actual como posición 0");
    }

    if (pParametros->general_simulacion == 1) //Es simulación
    {
        valorAnterior = mapaPines[pinEncoder];
        while (mapaPines[99] == false)
        {
            valor = mapaPines[pinEncoder];
            if (mapaPines[cw] == false && mapaPines[ccw] == true)
            { //movimiento CW
                if (valor < valorAnterior)
                {
                    posicionRelativa += 1;
                    if (darPosicionAbsoluta)
                        operarPosicionAbsoluta(pParametros, +1);
                }
            }
            else if (mapaPines[cw] == true && mapaPines[ccw] == false)
            { //movimiento CCW
                if (valor > valorAnterior)
                {
                    posicionRelativa -= 1;
                    if (darPosicionAbsoluta)
                        operarPosicionAbsoluta(pParametros, -1);
                }
            }
            usleep(tiempoEntreLecturas);
        }
    }
    else //NO es simulación es real
    {
        valorAnterior = gpio_read(pi_ID,pinEncoder);
        while (mapaPines[99] == false)
        {
            valor = gpio_read(pi_ID,pinEncoder);
            if (mapaPines[cw] == false && mapaPines[ccw] == true)
            { //movimiento CW
                if (valor < valorAnterior)
                {
                    posicionRelativa += 1;
                    if (darPosicionAbsoluta)
                        operarPosicionAbsoluta(pParametros, +1);
                }
            }
            else if (mapaPines[cw] == true && mapaPines[ccw] == false)
            { //movimiento CCW
                if (valor > valorAnterior)
                {
                    posicionRelativa -= 1;
                    if (darPosicionAbsoluta)
                        operarPosicionAbsoluta(pParametros, -1);
                }
            }
            //Leo DAH para actualizar el mapaPines
            estadoDAH(pParametros, pi_ID);
            usleep(tiempoEntreLecturas);
        }
    }
}

void CCupulaFijo::runEncoderSimulator(CConfig *pParametros)
{
    int ccw = pParametros->gpio_pin_act_ccw;
    int cw = pParametros->gpio_pin_act_cw;
    int pinEncoder = pParametros->gpio_pin_inp_encoder;

    useconds_t periodo = pParametros->cupula_periodo_simulacion;              //Periodo en microsegundos
    unsigned int longitudOnda = pParametros->cupula_longitud_onda_simulacion; //Undidades de distacia
    useconds_t tiempoEntreMuestras = periodo / longitudOnda;                  //tiempo entre una muestra y la siguiente
    int posicion = 0;

    while (mapaPines[99] == false)
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

int CCupulaFijo::getPosicion()
{
    LOG4CXX_DEBUG(pLogger, "valor del encoder: " + to_string(mapaPines[pParam->gpio_pin_inp_encoder]));
    return 0;
}

void CCupulaFijo::mover(sentidoMovimiento sentido)
{
    LOG4CXX_DEBUG(pLogger, "Cambio de sentido: " + to_string(sentido));
    if (pParam->general_simulacion == 1)
    {
        switch (sentido)
        {
        case sentidoMovimiento::CCW:
            mapaPines[pParam->gpio_pin_act_cw] = true;
            mapaPines[pParam->gpio_pin_act_ccw] = false;
            break;
        case sentidoMovimiento::CW:
            mapaPines[pParam->gpio_pin_act_cw] = false;
            mapaPines[pParam->gpio_pin_act_ccw] = true;
            break;
        case sentidoMovimiento::PARADO:
            mapaPines[pParam->gpio_pin_act_cw] = false;
            mapaPines[pParam->gpio_pin_act_ccw] = false;
            break;
        case sentidoMovimiento::CORTA: //TODO
            break;
        default:
            break;
        }
    }
    else
    {
        int errCode;
        switch (sentido)
        {
        case sentidoMovimiento::CW:
            if ((errCode = gpio_write(piID, pParam->gpio_pin_act_cw, PI_HIGH)) != 0)
                LOG4CXX_ERROR(pLogger, "Error gpio_write: " + to_string(errCode));
            if ((errCode = gpio_write(piID, pParam->gpio_pin_act_ccw, PI_LOW)) != 0)
                LOG4CXX_ERROR(pLogger, "Error gpio_write: " + to_string(errCode));
            break;
        case sentidoMovimiento::CCW:
            if ((errCode = gpio_write(piID, pParam->gpio_pin_act_cw, PI_LOW)) != 0)
                LOG4CXX_ERROR(pLogger, "Error gpio_write: " + to_string(errCode));
            if ((errCode = gpio_write(piID, pParam->gpio_pin_act_ccw, PI_HIGH)) != 0)
                LOG4CXX_ERROR(pLogger, "Error gpio_write: " + to_string(errCode));
            break;
        case sentidoMovimiento::PARADO:
            if ((errCode = gpio_write(piID, pParam->gpio_pin_act_cw, PI_LOW)) != 0)
                LOG4CXX_ERROR(pLogger, "Error gpio_write: " + to_string(errCode));
            if ((errCode = gpio_write(piID, pParam->gpio_pin_act_ccw, PI_LOW)) != 0)
                LOG4CXX_ERROR(pLogger, "Error gpio_write: " + to_string(errCode));
            break;
        case sentidoMovimiento::CORTA: //TODO
            break;
        default:
            break;
        }
    }
}

void CCupulaFijo::finalizarThreads()
{
    mapaPines[99] = true;
    if (pEncoder != nullptr)
        pEncoder->join();
    if (pEncoderSimulator != nullptr)
        pEncoderSimulator->join();
}

bool CCupulaFijo::estadoDAH(CConfig *pParametros, int pi_ID)
{
    log4cxx::LoggerPtr pLoggerLocal = log4cxx::Logger::getRootLogger();

    if (pParametros->general_simulacion == 1)
    {                                                 //Simulación
        if (pParametros->cupula_max_posiciones == -1) //Cúpula no calibrada
        {
            LOG4CXX_INFO(pLoggerLocal, "Cúpula no calibrada. Se usa cupula_max_posiciones_simulacion para determinar DAH");
            int max_posiciones = pParametros->cupula_max_posiciones_simulacion;
            if (posicionRelativa == 0)
            {
                mapaPines[pParametros->gpio_pin_inp_dah] = false;
                return true;
            }
            if (posicionRelativa < pParametros->cupula_max_posiciones_simulacion)
            {
                mapaPines[pParametros->gpio_pin_inp_dah] = true;
                return false;
            }
            if (posicionRelativa % max_posiciones == 0)
            {
                mapaPines[pParametros->gpio_pin_inp_dah] = false;
                return true;
            }
            else
            {
                mapaPines[pParametros->gpio_pin_inp_dah] = true;
                return false;
            }
        }
        else //Cúpula calibrada
        {
            if (posicionAbsoluta == 0)
            {
                mapaPines[pParametros->gpio_pin_inp_dah] = false;
                return true;
            }
            else
            {
                mapaPines[pParametros->gpio_pin_inp_dah] = true;
                return false;
            }
        }
    }
    else //Real
    {
        if (gpio_read(pi_ID, pParametros->gpio_pin_inp_dah) == PI_LOW)
        {
            mapaPines[pParametros->gpio_pin_inp_dah] = false;
            return true;
        }
        else
        {
            mapaPines[pParametros->gpio_pin_inp_dah] = true;
            return false;
        }
    }
}

void CCupulaFijo::operarPosicionAbsoluta(CConfig *pParametros, int valor)
{
    int posAbs = posicionAbsoluta;
    if (posicionAbsoluta == -1)
        return;
    posAbs = posAbs + valor;
    if (posAbs == pParametros->cupula_max_posiciones)
        posAbs = 0;
    else if (posAbs < 0)
        posAbs = pParametros->cupula_max_posiciones - 1;
    posicionAbsoluta = posAbs;
}