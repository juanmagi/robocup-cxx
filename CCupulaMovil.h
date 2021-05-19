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

    /*
    Emite un mensaje y devuelve la respuesta recibida. SI en 60s no recibe respuesta, sale con error KO=TIMEOUT=La parte móvil no responde
    param:
        mensaje, el mensaje que se envía
        respuesta, la respuesta recibida
    return:
        El resultado de la comunicación
    */
    int comunicar(std::string mensaje,std::string &respuesta);
    /*
    Devuelve el parámetro posicion que se encuentra en mensaje
    param:
        mensaje, el mensaje donde se encuentra los parámetros
        posicion, la posición que ocupa el parámetro solicitado (1 es la primera)
    return:
        El parámetro solicitado
    */
    std::string parametros(std::string mensaje,unsigned int posicion);

public:
    struct datosCalibrado{
        string estadoCalibrado;
        bool finalizado;
        unsigned long tiempoAbrir;
        unsigned long tiempoCerrar;
    };

    CCupulaMovil(CConfig *pParametros);
    ~CCupulaMovil();

    /*
    Establece la comunicación con el arduino de la parte móvil vía bluetooth
    return:
        EXIT_FAILURE, error en la conexión. El mensaje de error se escribe en el log
        EXIT_SUCCESS, conexión correcta
    */
    triestado conectar();
    /*
    Cancela la comunicación con el arduino de la parte móvil vía bluetooth
    return:
        EXIT_FAILURE, error en la conexión. El mensaje de error se escribe en el log
        EXIT_SUCCESS, desconexión correcta
    */
    int desconectar();
    /*
    Devuelve el estado de la luz de la cúpula
    param:
        salida, el estado de la luz
    return:
        El resultado de la comunicación
    */
    triestado getLuz(triestado &salida);
    /*
    Gestiona la luz de la cúpula
    param:
        bEncender: true la enciende, false la apaga
    return:
        El resultado de la comunicación
    */
    triestado setLuz(bool bEncender);
    /*
    Devuelve la versión del firmware del arduino que gestiona la parte móvil
    param:
        version, variable en la que se devuelve el resultado
    return:
        El resultado de la comunicación
    */
    int getFirmwareVersion(std::string &version);
    /*
    Determina el nivel del logging
    param:
        level: Niveles de log aceptados por la parte móvil
            LOG_LEVEL_SILENT
            LOG_LEVEL_FATAL
            LOG_LEVEL_ERROR
            LOG_LEVEL_WARNING
            LOG_LEVEL_VERBOSE
    return:
        El resultado de la comunicación. También se puede informar error si el level solictado es inexistente
    */
    triestado setLogging(std::string level);
    /*
    Emite un latido para informar a la parte móvil de que la parte fija está viva
    return:
        El resultado de la comunicación
    */
    int latido();
    /*
    calibra el movimiento de la ventana
    param:
        accion
            CALIBRATE: Ordena el inicio del calibrado
            GET: Lee la situación del calibrado
            PUT: Graba la información de calibrado
        dc, estructura para datos de calibrado
    return:
        El resultado de la comunicación.
    */
    triestado calibrate(std::string accion, datosCalibrado &dc);
    /*
    Informa a la parte móvil de que la parte inferior está en DAH o no lo está
    param:
        enDAH, true o false si la arte fija está o no en DAH
    return:
        El resultado de la comunicación
    NOTA: Esta función es absurda dado que si no está en DAH no hay corriente. Se manteniene por compatibilidad con la original
    */
    int domeAtHome(bool enDAH);
    /*
    Pone el timeout de apertura/cierre de la ventana
    param:
        segundos: timeout que se fijará
    return:
        El resultado de la comunicación
    */
    triestado setEmergencyShutterTimeout(int segundos);
    /*
    Ordena la apertura/cierre de la ventana
    param:
        accion, OPEN o CLOSE
        TimeoutShutter, Se define un timeout específico para este movimiento (milisegundos)
    return:
        El resultado de la comunicación
    */
    triestado moveShutter(std::string accion,unsigned long TimeoutShutter=0);
    /*
    Obtiene información de la evolución del movimiento
    param:
        accion, devuelve la acción que se está ejecutando
        avance, devuelve el grado de avance del movimiento
        timeout, devuelve true si se ha producido timeout o false en caso contrario
    return:
        El resultado de la comunicación
    */
    triestado movimiento(std::string &accion, unsigned int &avance, bool &timeout);
    /*
    Para el movimiento de la ventana
    return:
        El resultado de la comunicación
    */
    triestado stopShutter();
    /*
    Obiene el estado de los reles de la parte móvil
    param:
        luz, devuelve el estado del relé de la luz
        cerrar, devuelve el estado del relé de cerrar la ventana
        abrir, devuelve el estado del relé de abrir la ventana
    return:
        El resultado de la comunicación
    */
    triestado getRelays(triestado &luz, triestado &cerrar,triestado &abrir);
    /*
    Obiene el estado de los pulsadores de la parte móvil
    param:
        luz, devuelve el estado del pulsador de la luz
        cerrar, devuelve el estado del pulsador de cerrar la ventana
        abrir, devuelve el estado del pulsador de abrir la ventana
        reset, devuelve el estado del pulsador de reset
    return:
        El resultado de la comunicación
    */
    triestado getButtons(triestado &luz, triestado &cerrar,triestado &abrir,triestado &reset);
    /*
    Obiene el estado de los interruptores de fin de carrera
    param:
        cerrado, devuelve el estado del interruptor de fin de carrera de ventana cerrada
        abierto, devuelve el estado del interruptor de fin de carrera de ventana abierta
    return:
        El resultado de la comunicación
    */
    triestado getInputs(triestado &cerrado,triestado &abierto);
    /*
    Obiene todos los estados de la parte móvil de la cúpula
    param:
        estados, devuelve todos los estado
    return:
        El resultado de la comunicación
    */
    triestado getStatus(stringmap &estados);
    /*
    calibra el movimieto de la ventana de modo síncrono (con bloqueo)
    param:
        dc, estructura para datos de calibrado
    return:
        El resultado de la comunicación.
    */
    triestado calibrateSincrono(datosCalibrado &dc);

};

