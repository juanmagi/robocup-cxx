#pragma once
#include <string.h>
#include <SerialPort.h>
#include "log4cxx/logger.h"
#include "CConfig.h"
#include "global.h"

class CCupulaMovil
{
private:
    CConfig *pParam=nullptr;
    SerialPort *p_puerto_serie=nullptr;
    log4cxx::LoggerPtr pLogger=nullptr;

    int conectar();
    std::string comunicar(std::string mensaje);
    std::string parametros(std::string mensaje,unsigned int posicion);

public:
    CCupulaMovil(CConfig *pParametros);
    ~CCupulaMovil();

    triestado estadoLuz(); //Lee el estado de la luz de la cúpula
    void Luz(bool bEncender);    //Enciende/Apaga la luz
    std::string getFirmwareVersion();
    void setLogging(std::string level);
    void latido();
    int calibrate(std::string accion,unsigned long *tiempoAbrir=nullptr,unsigned long *tiempoCerrar=nullptr );
    triestado estadoDomeAtHome();
    void domeAtHome();
    void setEmergencyShutterTimeout(int segundos);
    void moveShutter(std::string accion,unsigned long TimeoutShutter=0);
    void movimiento(std::string *accion, unsigned int *avance, bool *timeout);
    void stopShutter();
    void getRelays(triestado *luz, triestado *cerrar,triestado *abrir);
    void getButtons(triestado *luz, triestado *cerrar,triestado *abrir,triestado *reset);
    void getInputs(triestado *cerrado,triestado *abierto);
    void getStatus(stringmap *estados);

};

