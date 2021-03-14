#include <unistd.h>
#include <iostream>
#include "CCupulaFijo.h"

using namespace std;

sentidoMovimiento CCupulaFijo::sentido = sentidoMovimiento::PARADO;
bool CCupulaFijo::mapaPines[100] = {false};
//bool CCupulaFijo::mapaPines[100];
int CCupulaFijo::posicionAbsoluta = -1;
int CCupulaFijo::posicionRelativa = 0;
int CCupulaFijo::posicionAbsolutaFinal = -1;
int CCupulaFijo::posicionRelativaFinal = 0;

CCupulaFijo::CCupulaFijo(CConfig *pParametros, int pi_ID)
{
    pParam = pParametros;
    piID = pi_ID;
    pLogger = log4cxx::Logger::getRootLogger();
    //    for (int i=0;i<100;i++)
    //        mapaPines[i]=false;

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
    else
    { //Si es simulación
        mapaPines[pParam->gpio_pin_act_ccw] = true;
        mapaPines[pParam->gpio_pin_act_cw] = true;
        mapaPines[pParam->gpio_pin_inp_dah] = true;
        mapaPines[pParam->gpio_pin_inp_encoder] = true;
        pEncoderSimulator = new thread(runEncoderSimulator, pParametros);
    }
    pEncoder = new thread(runEncoder, pParametros, pi_ID);
}

CCupulaFijo::~CCupulaFijo()
{
    finalizarThreads(tipoThread::todos);
}

void CCupulaFijo::runEncoder(CConfig *pParametros, int pi_ID)
{
    int ccw = pParametros->gpio_pin_act_ccw;
    int cw = pParametros->gpio_pin_act_cw;
    int dah = pParametros->gpio_pin_inp_dah;
    int pinEncoder = pParametros->gpio_pin_inp_encoder;
    int valor, valorAnterior;
    bool darPosicionAbsoluta = true;
    int tiempoEntreLecturas = 10000;
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
            valorAnterior = valor;
            usleep(tiempoEntreLecturas);
        }
    }
    else //NO es simulación, es real
    {
        valorAnterior = gpio_read(pi_ID, pinEncoder);
        while (mapaPines[99] == false)
        {
            valor = gpio_read(pi_ID, pinEncoder);
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
            valorAnterior = valor;
            usleep(tiempoEntreLecturas);
        }
    }
}

void CCupulaFijo::runEncoderSimulator(CConfig *pParametros)
{
    log4cxx::LoggerPtr pLoggerLocal = log4cxx::Logger::getRootLogger();
    bool imprMsjCW = true;
    bool imprMsjCCW = true;

    int ccw = pParametros->gpio_pin_act_ccw;
    int cw = pParametros->gpio_pin_act_cw;
    int pinEncoder = pParametros->gpio_pin_inp_encoder;

    useconds_t periodo = pParametros->cupula_periodo_simulacion;              //Periodo en microsegundos
    unsigned int longitudOnda = pParametros->cupula_longitud_onda_simulacion; //Undidades de distacia
    useconds_t tiempoEntreMuestras = periodo / longitudOnda;                  //tiempo entre una muestra y la siguiente
    int posicion = 0;

    while (mapaPines[tipoThread::encoderSimulator] == false)
    {
        if (posicion >= 0 && posicion < 5)
            mapaPines[pinEncoder] = true;
        else
            mapaPines[pinEncoder] = false; //Se devuelve false en el 1 de la onda
        if (mapaPines[cw] == true && mapaPines[ccw] == false)
        { //Movimientos CCW
            if (imprMsjCCW)
            {
                LOG4CXX_DEBUG(pLoggerLocal, "Iniciando movimiento simulado CCW");
                imprMsjCCW = false;
                imprMsjCW = true;
            }
            posicion += 1;
            if (posicion == longitudOnda)
                posicion = 0;
        }
        else if (mapaPines[cw] == false && mapaPines[ccw] == true)
        { //Movimiento CW
            if (imprMsjCW)
            {
                LOG4CXX_DEBUG(pLoggerLocal, "Iniciando movimiento simulado CW");
                imprMsjCW = false;
                imprMsjCCW = true;
            }
            posicion -= 1;
            if (posicion == -1)
                posicion = longitudOnda - 1;
        }
        else if (mapaPines[cw] == true && mapaPines[ccw] == true)
        { //Parar movimiento
            if (imprMsjCW == false || imprMsjCCW == false)
            {
                LOG4CXX_DEBUG(pLoggerLocal, "Parando movimiento simulado CW");
                imprMsjCW = true;
                imprMsjCCW = true;
            }
        }
        else if (mapaPines[cw] == false && mapaPines[ccw] == false)
        { //Opción incompatible: No se pueden activar los dos movimientos a la vez
            LOG4CXX_DEBUG(pLoggerLocal, "Error simulador: Los dos movimiento activados a la vez");
        }
        usleep(tiempoEntreMuestras);
    }
}

void CCupulaFijo::runStop(CConfig *pParametros, int pi_ID, tipoPosicion tp, int posRelIni)
{
    int tiempoEntreLecturas = 10000;

    switch (tp)
    {
    case tipoPosicion::absoluta:
        if (sentido == sentidoMovimiento::CW)
        {
            if (posicionAbsolutaFinal > posicionAbsoluta)
            {
                while (posicionAbsoluta < posicionAbsolutaFinal)
                {
                    if (mapaPines[tipoThread::stop] == true)
                        break;
                    usleep(tiempoEntreLecturas);
                }
                pararMovimiento(pParametros, pi_ID);
            }
            else
            {
                while (posicionAbsoluta > posicionAbsolutaFinal)
                {
                    if (mapaPines[tipoThread::stop] == true)
                        break;
                    usleep(tiempoEntreLecturas);
                }
                while (posicionAbsoluta < posicionAbsolutaFinal)
                {
                    if (mapaPines[tipoThread::stop] == true)
                        break;
                    usleep(tiempoEntreLecturas);
                }
                pararMovimiento(pParametros, pi_ID);
            }
        }
        else
        { //sentido CCW
            if (posicionAbsolutaFinal < posicionAbsoluta)
            {
                while (posicionAbsoluta > posicionAbsolutaFinal)
                {
                    if (mapaPines[tipoThread::stop] == true)
                        break;
                    usleep(tiempoEntreLecturas);
                }
                pararMovimiento(pParametros, pi_ID);
            }
            else
            {
                while (posicionAbsoluta < posicionAbsolutaFinal)
                {
                    if (mapaPines[tipoThread::stop] == true)
                        break;
                    usleep(tiempoEntreLecturas);
                }
                while (posicionAbsoluta > posicionAbsolutaFinal)
                {
                    if (mapaPines[tipoThread::stop] == true)
                        break;
                    usleep(tiempoEntreLecturas);
                }
                pararMovimiento(pParametros, pi_ID);
            }
        }
        break;
    case tipoPosicion::relativa:
        if (sentido == sentidoMovimiento::CW)
        {
            while (posicionRelativa < posicionRelativaFinal)
            {
                if (mapaPines[tipoThread::stop] == true)
                    break;
                usleep(tiempoEntreLecturas);
            }
            pararMovimiento(pParametros, pi_ID);
        }
        else
        { //sentido CCW
            while (posicionRelativa > posicionRelativaFinal)
            {
                if (mapaPines[tipoThread::stop] == true)
                    break;
                usleep(tiempoEntreLecturas);
            }
            pararMovimiento(pParametros, pi_ID);
        }
        break;
    case tipoPosicion::dah:
        if (sentido == sentidoMovimiento::CW)
        {
            int posicionFinal = posRelIni + pParametros->cupula_max_posiciones;
            while (posicionRelativa < posicionFinal)
            {
                if (pParametros->general_simulacion == 1)
                {
                    if (mapaPines[pParametros->gpio_pin_inp_dah] == false)
                        break;
                }
                else
                {
                    if (gpio_read(pi_ID, pParametros->gpio_pin_inp_dah) == PI_LOW)
                        break;
                }
                if (mapaPines[tipoThread::stop] == true)
                    break;
                usleep(tiempoEntreLecturas);
            }
            pararMovimiento(pParametros, pi_ID);
        }
        else
        { //sentido CCW
            int posicionFinal = posRelIni - pParametros->cupula_max_posiciones;
            while (posicionRelativa > posicionFinal)
            {
                if (gpio_read(pi_ID, pParametros->gpio_pin_inp_dah) == PI_LOW)
                    break;
                if (mapaPines[tipoThread::stop] == true)
                    break;
                usleep(tiempoEntreLecturas);
            }
            pararMovimiento(pParametros, pi_ID);
        }
        break;
    case tipoPosicion::vuelta:
        if (sentido == sentidoMovimiento::CW)
        {
            int posicionFinal = posRelIni + pParametros->cupula_max_posiciones;
            while (posicionRelativa < posicionFinal)
            {
                if (mapaPines[tipoThread::stop] == true)
                    break;
                usleep(tiempoEntreLecturas);
            }
            pararMovimiento(pParametros, pi_ID);
        }
        else
        { //sentido CCW
            int posicionFinal = posRelIni - pParametros->cupula_max_posiciones;
            while (posicionRelativa > posicionFinal)
            {
                if (mapaPines[tipoThread::stop] == true)
                    break;
                usleep(tiempoEntreLecturas);
            }
            pararMovimiento(pParametros, pi_ID);
        }
        break;
    default:
        break;
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

void CCupulaFijo::mover(sentidoMovimiento sm)
{
    LOG4CXX_DEBUG(pLogger, "Mover fijo: " + to_string(sentido));
    if (sm == sentidoMovimiento::CORTA)
    {
        LOG4CXX_ERROR(pLogger, "La función mover no acepta el sentidoMovimiento::CORTA");
        return;
    }

    if (sentido != sentidoMovimiento::PARADO)
    {
        finalizarThreads(tipoThread::stop); //También para cualquier movimiento
        sentido = sentidoMovimiento::PARADO;
    }
    if (pParam->general_simulacion == 1)
    {
        switch (sm)
        {
        case sentidoMovimiento::CCW:
            mapaPines[pParam->gpio_pin_act_cw] = true;
            mapaPines[pParam->gpio_pin_act_ccw] = false;
            break;
        case sentidoMovimiento::CW:
            mapaPines[pParam->gpio_pin_act_ccw] = true;
            mapaPines[pParam->gpio_pin_act_cw] = false;
            break;
        case sentidoMovimiento::PARADO:
            mapaPines[pParam->gpio_pin_act_cw] = true;
            mapaPines[pParam->gpio_pin_act_ccw] = true;
            break;
        default:
            LOG4CXX_ERROR(pLogger, "Sentido movimiento indefinido");
            break;
        }
    }
    else
    {
        int errCode;
        switch (sm)
        {
        case sentidoMovimiento::CW:
            if ((errCode = gpio_write(piID, pParam->gpio_pin_act_cw, PI_HIGH)) != 0)
                LOG4CXX_ERROR(pLogger, "Error gpio_write: " + to_string(errCode));
            if ((errCode = gpio_write(piID, pParam->gpio_pin_act_ccw, PI_LOW)) != 0)
                LOG4CXX_ERROR(pLogger, "Error gpio_write: " + to_string(errCode));
            break;
        case sentidoMovimiento::CCW:
            if ((errCode = gpio_write(piID, pParam->gpio_pin_act_ccw, PI_HIGH)) != 0)
                LOG4CXX_ERROR(pLogger, "Error gpio_write: " + to_string(errCode));
            if ((errCode = gpio_write(piID, pParam->gpio_pin_act_cw, PI_LOW)) != 0)
                LOG4CXX_ERROR(pLogger, "Error gpio_write: " + to_string(errCode));
            break;
        case sentidoMovimiento::PARADO:
            if ((errCode = gpio_write(piID, pParam->gpio_pin_act_cw, PI_HIGH)) != 0)
                LOG4CXX_ERROR(pLogger, "Error gpio_write: " + to_string(errCode));
            if ((errCode = gpio_write(piID, pParam->gpio_pin_act_ccw, PI_HIGH)) != 0)
                LOG4CXX_ERROR(pLogger, "Error gpio_write: " + to_string(errCode));
            break;
        default:
            LOG4CXX_ERROR(pLogger, "Sentido movimiento indefinido");
            break;
        }
    }
    sentido = sm;
}

void CCupulaFijo::finalizarThreads(tipoThread valor)
{
    switch (valor)
    {
    case tipoThread::stop:
        mapaPines[tipoThread::stop] = true;
        if (pStop != nullptr)
        {
            pStop->join();
            delete pStop;
            pStop = nullptr;
        }
        break;
    case tipoThread::encoder:
        mapaPines[tipoThread::encoder] = true;
        if (pEncoder != nullptr)
        {
            pEncoder->join();
            delete pEncoder;
            pEncoder = nullptr;
        }
        break;
    case tipoThread::encoderSimulator:
        mapaPines[tipoThread::encoderSimulator] = true;
        if (pEncoderSimulator != nullptr)
        {
            pEncoderSimulator->join();
            delete pEncoderSimulator;
            pEncoderSimulator = nullptr;
        }
        break;
    default:
        mapaPines[tipoThread::stop] = true;
        mapaPines[tipoThread::encoder] = true;
        mapaPines[tipoThread::encoderSimulator] = true;
        if (pStop != nullptr)
        {
            pStop->join();
            delete pStop;
            pStop = nullptr;
        }
        if (pEncoder != nullptr)
        {
            pEncoder->join();
            delete pEncoder;
            pEncoder = nullptr;
        }
        if (pEncoderSimulator != nullptr)
        {
            pEncoderSimulator->join();
            delete pEncoderSimulator;
            pEncoderSimulator = nullptr;
        }
        break;
    }
}

bool CCupulaFijo::estadoDAH(CConfig *pParametros, int pi_ID)
{
    log4cxx::LoggerPtr pLoggerLocal = log4cxx::Logger::getRootLogger();

    if (pParametros->general_simulacion == 1)
    {                                                 //Simulación
        if (posicionAbsoluta==-1) //Cúpula no calibrada o sin origen establecido. Hay que trabajar con posición relativa
        {
            int max_posiciones = pParametros->cupula_max_posiciones_simulacion;
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
        else //Cúpula calibrada y origen establecido
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

sentidoMovimiento CCupulaFijo::sentidoMasCorto(int pos)
{
    int posActual = getPosicion();
    if (pos > posActual)
        return sentidoMovimiento::CW;
    else
        return sentidoMovimiento::CCW;
}

void CCupulaFijo::setPosicion(sentidoMovimiento sm, tipoPosicion tp, int posicion)
{ //TODO implementar el tp manual, dah y vuelta
    if (sm == sentidoMovimiento::PARADO)
    {
        LOG4CXX_ERROR(pLogger, "sentidoMovimiento::PARADO no es un valor de parámetro aceptable en setPosicion");
        return;
    }
    if (sentido != sentidoMovimiento::PARADO)
    {
        LOG4CXX_ERROR(pLogger, "Hay un movimiento en marcha. No se puede hacer setPosicion hasta que no finalice");
        return;
    }
    if (posicionAbsoluta == -1 && (tp == tipoPosicion::absoluta || sentido == sentidoMovimiento::CORTA || tp == tipoPosicion::vuelta))
    { //Cúpula no calibrada y tipo posición absoluta, no es posible
        LOG4CXX_ERROR(pLogger, "La cúpula no está calibrada. El movimiento sólo puede ser relativo");
        return;
    }
    if ((tp == tipoPosicion::manual || tp == tipoPosicion::vuelta) && sm == sentidoMovimiento::CORTA)
    {
        LOG4CXX_ERROR(pLogger, "Si tipoPosicion es manual o vuelta, el sentido debe ser CW o CCW");
        return;
    }

    sentidoMovimiento smFinal;
    switch (tp)
    {
    case tipoPosicion::relativa:
        posicionRelativaFinal += posicion;
        if (posicion > 0)
            smFinal = sentidoMovimiento::CW;
        else
            smFinal = sentidoMovimiento::CCW;
        break;
    case tipoPosicion::absoluta:
        posicionAbsolutaFinal = posicion;
        if (sm == sentidoMovimiento::CORTA)
            smFinal = sentidoMasCorto(posicion);
        else
            smFinal = sm;
        break;
    case tipoPosicion::dah:
        posicionAbsolutaFinal = 0;
        if (sm == sentidoMovimiento::CORTA)
            smFinal = sentidoMasCorto(posicion);
        else
            smFinal = sm;
        break;
    case tipoPosicion::manual:
        posicionAbsolutaFinal = posicionAbsoluta;
        smFinal = sm;
        break;
    case tipoPosicion::vuelta:
        posicionAbsolutaFinal = posicionAbsoluta;
        smFinal = sm;
        break;
    default:
        break;
    }
    int posRelIni = posicionRelativa;
    mover(smFinal);
    if (tp != tipoPosicion::manual)
        pStop = new thread(runStop, pParam, piID, tp, posRelIni);
}

void CCupulaFijo::DomeAtHome(bool valor)
{
    if (valor)
    { //Vamos al DAH
        if (estadoDAH())
            return;
        if (posicionAbsoluta == -1)
            setPosicion(sentidoMovimiento::CW, tipoPosicion::dah, 0);
        else
            setPosicion(sentidoMovimiento::CORTA, tipoPosicion::dah, 0);
    }
    else
    { //Vamos al DAH + 20 CW
        if (estadoDAH())
        {
            setPosicion(sentidoMovimiento::CW, tipoPosicion::absoluta, 20);
            return;
        }
        if (posicionAbsoluta == -1)
            setPosicion(sentidoMovimiento::CW, tipoPosicion::dah, 0);
        else
            setPosicion(sentidoMovimiento::CORTA, tipoPosicion::dah, 0);
        setPosicion(sentidoMovimiento::CW, tipoPosicion::absoluta, 10);
    }
}

void CCupulaFijo::pararMovimiento(CConfig *pParametros, int pi_ID)
{
    int errCode;
    log4cxx::LoggerPtr pLoggerLocal = log4cxx::Logger::getRootLogger();

    if (pParametros->general_simulacion == 1)
    {
        mapaPines[pParametros->gpio_pin_act_cw] = true;
        mapaPines[pParametros->gpio_pin_act_ccw] = true;
    }
    else
    {
        if ((errCode = gpio_write(pi_ID, pParametros->gpio_pin_act_cw, PI_HIGH)) != 0)
            LOG4CXX_ERROR(pLoggerLocal, "Error gpio_write: " + to_string(errCode));
        if ((errCode = gpio_write(pi_ID, pParametros->gpio_pin_act_ccw, PI_HIGH)) != 0)
            LOG4CXX_ERROR(pLoggerLocal, "Error gpio_write: " + to_string(errCode));
    }

    sentido = sentidoMovimiento::PARADO;
}

void CCupulaFijo::calibrate(bool recalibrar)
{
    if (pParam->cupula_max_posiciones>-1 && !recalibrar) //Ya está calibrado y no se pide recalibrar, salir
        return;

    int errCode;
    //Nos movemos en sentido CW hasta que encontremos el DAH y pasamos 10 unidades
    DomeAtHome(true);
    //Giramos en sentido CCW
    setPosicion(sentidoMovimiento::CCW, tipoPosicion::manual, 0);
    //Esperamos hasta que se detecte DAH y entonces de define la posición 0
    while (!estadoDAH(pParam, piID))
    {
    }
    int posicionInicial = posicionRelativa;
    //Esperamos hasta que se salga de DAH
    while (estadoDAH(pParam, piID))
    {
        LOG4CXX_DEBUG(pLogger, "Pin dah: " + to_string(mapaPines[pParam->gpio_pin_inp_dah]) + " Pin encoder " + to_string(mapaPines[pParam->gpio_pin_inp_encoder]) + " Posición: " + to_string(posicionRelativa));
    }
    //Esperamos hasta el nuevo DAH
    while (!estadoDAH(pParam, piID))
    {
        LOG4CXX_DEBUG(pLogger, "Pin dah: " + to_string(mapaPines[pParam->gpio_pin_inp_dah]) + " Pin encoder " + to_string(mapaPines[pParam->gpio_pin_inp_encoder]) + " Posición: " + to_string(posicionRelativa));
    }
    int posicionFinal = posicionRelativa;
    parar();
    sleep(5); //Espero segundos hasta que se pare completamente la cúpula
    int inercia = abs(posicionRelativa - posicionFinal);
    posicionRelativa = -inercia;
    pParam->put_cupula_max_posiciones(abs(posicionFinal - posicionInicial));
    if (inercia > 0)
        posicionAbsoluta = pParam->cupula_max_posiciones - inercia;
    else
        posicionAbsoluta = 0;
}