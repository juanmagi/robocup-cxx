#include "CCupulaMovil.h"

using namespace std;

CCupulaMovil::CCupulaMovil(CConfig *pParametros)
{
    pParam = pParametros;
    pLogger = log4cxx::Logger::getRootLogger();
}

CCupulaMovil::~CCupulaMovil()
{
    desconectar();
}

triestado CCupulaMovil::conectar()
{
    triestado codret;

    if (p_puerto_serie != nullptr)
    {
        LOG4CXX_DEBUG(pLogger, "No se puede abrir la conexión porque ya está establecida");
        codret.estado = tipoTriestado::ERROR;
        codret.mensaje = "KO=No se puede abrir la conexión porque ya está establecida";
        return codret;
    }

    p_puerto_serie = new SerialPort(pParam->serial_nombre);

    try
    {
        p_puerto_serie->Open(SerialPort::BAUD_9600);
    }
    catch (SerialPort::OpenFailed E)
    {
        LOG4CXX_FATAL(pLogger, "Error establecido la conexión BLUETOOTH. Asegurar que el device está creado");
        codret.estado = tipoTriestado::ERROR;
        codret.mensaje = "KO=No se puede abrir la conexión porque ya está establecida";
        return codret;
    }

    codret.estado = tipoTriestado::ON;
    codret.mensaje = "";
    return codret;
}

int CCupulaMovil::desconectar()
{
    if (p_puerto_serie == nullptr)
    {
        LOG4CXX_ERROR(pLogger, "No se puede cerrar la conexión porque no está establecida");
        return EXIT_FAILURE;
    }

    try
    {
        p_puerto_serie->Close();
    }
    catch (SerialPort::OpenFailed E)
    {
        p_puerto_serie = nullptr;
        LOG4CXX_FATAL(pLogger, "Error cerrando la conexión BLUETOOTH.");
        return EXIT_FAILURE;
    }

    LOG4CXX_DEBUG(pLogger, "Puerto serie cerrado");
    delete p_puerto_serie;
    p_puerto_serie = nullptr;
    return EXIT_SUCCESS;
}

int CCupulaMovil::comunicar(std::string mensaje, string &respuesta)
{
    p_puerto_serie->Write(mensaje);
    try
    {
        respuesta = p_puerto_serie->ReadLine(1000U * 60U); //Timeout 1 minuto
    }
    catch (SerialPort::ReadTimeout E)
    {
        LOG4CXX_FATAL(pLogger, "TIMEOUT esperando datos por bluetooth");
        respuesta = "KO=TIMEOUT:La parte móvil no responde";
        return EXIT_FAILURE;
    }
    respuesta.erase(respuesta.find("\r\n"), 2);
    LOG4CXX_DEBUG(pLogger, mensaje + "-->" + respuesta);
    return EXIT_SUCCESS;
}

std::string CCupulaMovil::parametros(std::string mensaje, unsigned int posicion)
{
    string datos;

    if (posicion == 1 && mensaje.find("=") != string::npos)
    {
        datos = mensaje.substr(mensaje.find("=") + 1);
    }
    else
    {
        datos = mensaje;
    }

    vector<string> dato;
    size_t pos = 0;
    dato.push_back(datos);
    for (int i = 0; (pos = datos.find(":")) != string::npos; i++)
    {
        dato[i] = datos.substr(0, pos);
        datos.erase(0, pos + 1);
        dato.push_back(datos);
    }

    return dato[posicion - 1];
}

triestado CCupulaMovil::getLuz(triestado &salida)
{
    triestado cerrar, abrir;
    triestado codret;

    codret = getRelays(salida, cerrar, abrir);
    if (codret.estado == tipoTriestado::ERROR)
    {
        LOG4CXX_INFO(pLogger, "Error en la función getLuz: " + codret.mensaje);
        codret.estado = tipoTriestado::ERROR;
        codret.mensaje = codret.mensaje;
        return codret;
    }

    codret.estado = tipoTriestado::ON;
    codret.mensaje = "";
    return codret;
}

triestado CCupulaMovil::setLuz(bool bEncender)
{
    string mensaje;
    triestado codret;
    if (bEncender)
        mensaje = "luz=ON";
    else
        mensaje = "luz=OFF";

    string respuesta;
    comunicar(mensaje, respuesta);
    if (respuesta.substr(0, 2) != "OK")
    {
        LOG4CXX_INFO(pLogger, "Error en la función setLuz: " + respuesta);
        codret.estado = tipoTriestado::ERROR;
        codret.mensaje = respuesta;
        return codret;
    }
    codret.estado = tipoTriestado::ON;
    codret.mensaje = "";
    return codret;
}

int CCupulaMovil::getFirmwareVersion(string &version)
{
    string mensaje = "getFirmwareVersion";
    if (comunicar(mensaje, version) == EXIT_SUCCESS)
    {
        if (version.substr(0, 2) != "OK")
        {
            LOG4CXX_INFO(pLogger, "Error en la función getFirmwareVersion: " + version);
        }
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

triestado CCupulaMovil::setLogging(string level)
{
    string mensaje;
    triestado codret;

    if (level == "LOG_LEVEL_SILENT")
        mensaje = "setLogging=LOG_LEVEL_SILENT";
    else if (level == "LOG_LEVEL_FATAL")
        mensaje = "setLogging=LOG_LEVEL_FATAL";
    else if (level == "LOG_LEVEL_ERROR")
        mensaje = "setLogging=LOG_LEVEL_ERROR";
    else if (level == "LOG_LEVEL_WARNING")
        mensaje = "setLogging=LOG_LEVEL_WARNING";
    else if (level == "LOG_LEVEL_TRACE")
        mensaje = "setLogging=LOG_LEVEL_TRACE";
    else if (level == "LOG_LEVEL_VERBOSE")
        mensaje = "setLogging=LOG_LEVEL_VERBOSE";
    else
    {
        LOG4CXX_DEBUG(pLogger, "Parámetro level incorrecto");
        codret.estado = tipoTriestado::ERROR;
        codret.mensaje = "KO=Parámetro level incorrecto";
        return codret;
    }

    string respuesta;
    comunicar(mensaje, respuesta);
    if (respuesta.substr(0, 2) != "OK")
    {
        LOG4CXX_INFO(pLogger, "Error en la función setEmergencyShutterTimeout: " + respuesta);
        codret.estado = tipoTriestado::ERROR;
        codret.mensaje = respuesta;
        return codret;
    }
    codret.estado = tipoTriestado::ON;
    codret.mensaje = "";
    return codret;
}

int CCupulaMovil::latido()
{
    string mensaje = "latido";
    string respuesta;
    if (comunicar(mensaje, respuesta) == EXIT_SUCCESS)
    {
        if (respuesta.substr(0, 2) != "OK")
        {
            LOG4CXX_INFO(pLogger, "Error en la función latido: " + respuesta);
        }
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

triestado CCupulaMovil::calibrate(string accion, datosCalibrado &dc)
{
    string mensaje;
    triestado codret;
    if (accion == "CALIBRATE")
    {
        mensaje = "calibrateShutter=CALIBRATE";
    }
    else if (accion == "GET")
    {
        mensaje = "calibrateShutter=GET";
    }
    else if (accion == "PUT")
    {
        mensaje = "calibrateShutter=PUT:" + to_string(dc.tiempoAbrir) + ":" + to_string(dc.tiempoCerrar);
    }
    else
    {
        LOG4CXX_DEBUG(pLogger, "Parámetro acción incorrecto");
        codret.estado = tipoTriestado::ERROR;
        codret.mensaje = "KO=Parámetro acción incorrecto";
        return codret;
    }

    string respuesta;
    comunicar(mensaje, respuesta);
    if (respuesta.substr(0, 2) != "OK")
    {
        LOG4CXX_INFO(pLogger, "Error en la función calibrate: " + respuesta);
        codret.estado = tipoTriestado::ERROR;
        codret.mensaje = respuesta;
        return codret;
    }

    if (accion == "GET")
    {
        dc.estadoCalibrado = parametros(respuesta, 1);
        dc.finalizado = stoi(parametros(respuesta, 2));
        if (dc.estadoCalibrado == "NINGUNO" && parametros(respuesta, 2) == "1")
        {
            dc.tiempoAbrir = stoul(parametros(respuesta, 3));
            dc.tiempoCerrar = stoul(parametros(respuesta, 4));
        }
    }
    codret.estado = tipoTriestado::ON;
    codret.mensaje = "";
    return codret;
}

int CCupulaMovil::domeAtHome(bool enDAH)
{
    string mensaje;
    if (enDAH)
        mensaje = "domeAtHome=ON";
    else
        mensaje = "domeAtHome=OFF";
    string respuesta;
    if (comunicar(mensaje, respuesta) == EXIT_SUCCESS)
    {
        if (respuesta.substr(0, 2) != "OK")
        {
            LOG4CXX_INFO(pLogger, "Error en la función domeAtHome: " + respuesta);
        }
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

triestado CCupulaMovil::setEmergencyShutterTimeout(int segundos)
{
    string mensaje;
    triestado codret;

    mensaje = "setEmergencyShutterTimeout=" + to_string(segundos);
    string respuesta;
    comunicar(mensaje, respuesta);
    if (respuesta.substr(0, 2) != "OK")
    {
        LOG4CXX_INFO(pLogger, "Error en la función setEmergencyShutterTimeout: " + respuesta);
        codret.estado = tipoTriestado::ERROR;
        codret.mensaje = respuesta;
        return codret;
    }
    codret.estado = tipoTriestado::ON;
    codret.mensaje = "";
    return codret;
}

triestado CCupulaMovil::moveShutter(string accion, unsigned long TimeoutShutter)
{
    string mensaje;
    triestado codret;

    if (accion == "OPEN")
        mensaje = "moveShutter=OPEN";
    else if (accion == "CLOSE")
        mensaje = "moveShutter=CLOSE";
    else
    {
        LOG4CXX_DEBUG(pLogger, "Parámetro acción incorrecto");
        codret.estado = tipoTriestado::ERROR;
        codret.mensaje = "KO=Parámetro acción incorrecto";
        return codret;
    }

    if (TimeoutShutter > 0)
    {
        mensaje += ":" + to_string(TimeoutShutter);
    }

    string respuesta;
    comunicar(mensaje, respuesta);
    if (respuesta.substr(0, 2) != "OK")
    {
        LOG4CXX_INFO(pLogger, "Error en la función moveShutter: " + respuesta);
        codret.estado = tipoTriestado::ERROR;
        codret.mensaje = respuesta;
        return codret;
    }
    codret.estado = tipoTriestado::ON;
    codret.mensaje = "";
    return codret;
}

triestado CCupulaMovil::movimiento(string &accion, unsigned int &avance, bool &timeout)
{
    string mensaje = "getMovimiento";
    triestado codret;
    string respuesta;

    comunicar(mensaje, respuesta);
    if (respuesta.substr(0, 2) != "OK")
    {
        LOG4CXX_INFO(pLogger, "Error en la función movimiento: " + respuesta);
        codret.estado = tipoTriestado::ERROR;
        codret.mensaje = respuesta;
        return codret;
    }

    accion = parametros(respuesta, 1);
    avance = stoi(parametros(respuesta, 2));
    timeout = stoi(parametros(respuesta, 3));
    codret.estado = tipoTriestado::ON;
    codret.mensaje = "";
    return codret;
}

triestado CCupulaMovil::stopShutter()
{
    string mensaje = "stopShutter";
    triestado codret;
    string respuesta;
    comunicar(mensaje, respuesta);
    if (respuesta.substr(0, 2) != "OK")
    {
        LOG4CXX_INFO(pLogger, "Error en la función stopShutter: " + respuesta);
        codret.estado = tipoTriestado::ERROR;
        codret.mensaje = respuesta;
        return codret;
    }
    codret.estado = tipoTriestado::ON;
    codret.mensaje = "";
    return codret;
}

triestado CCupulaMovil::getRelays(triestado &luz, triestado &cerrar, triestado &abrir)
{
    string mensaje = "getRelays";
    triestado codret;
    string respuesta;
    comunicar(mensaje, respuesta);
    if (respuesta.substr(0, 2) != "OK")
    {
        LOG4CXX_INFO(pLogger, "Error en la función getRelays: " + respuesta);
        luz.estado = tipoTriestado::ERROR;
        luz.mensaje = respuesta;
        cerrar.estado = tipoTriestado::ERROR;
        cerrar.mensaje = respuesta;
        abrir.estado = tipoTriestado::ERROR;
        abrir.mensaje = respuesta;
        codret.estado = tipoTriestado::ERROR;
        codret.mensaje = respuesta;
        return codret;
    }

    if (parametros(respuesta, 1) == "1")
        luz.estado = tipoTriestado::ON;
    else
        luz.estado = tipoTriestado::OFF;
    if (parametros(respuesta, 2) == "1")
        cerrar.estado = tipoTriestado::ON;
    else
        cerrar.estado = tipoTriestado::OFF;
    if (parametros(respuesta, 3) == "1")
        abrir.estado = tipoTriestado::ON;
    else
        abrir.estado = tipoTriestado::OFF;

    codret.estado = tipoTriestado::ON;
    codret.mensaje = "";
    return codret;
}

triestado CCupulaMovil::getButtons(triestado &luz, triestado &cerrar, triestado &abrir, triestado &reset)
{
    string mensaje = "getButtons";
    triestado codret;
    string respuesta;
    comunicar(mensaje, respuesta);
    if (respuesta.substr(0, 2) != "OK")
    {
        LOG4CXX_INFO(pLogger, "Error en la función getButtons: " + respuesta);
        luz.estado = tipoTriestado::ERROR;
        luz.mensaje = respuesta;
        cerrar.estado = tipoTriestado::ERROR;
        cerrar.mensaje = respuesta;
        abrir.estado = tipoTriestado::ERROR;
        abrir.mensaje = respuesta;
        reset.estado = tipoTriestado::ERROR;
        reset.mensaje = respuesta;
        codret.estado = tipoTriestado::ERROR;
        codret.mensaje = respuesta;
        return codret;
    }
    if (parametros(respuesta, 1) == "1")
        luz.estado = tipoTriestado::ON;
    else
        luz.estado = tipoTriestado::OFF;
    if (parametros(respuesta, 2) == "1")
        cerrar.estado = tipoTriestado::ON;
    else
        cerrar.estado = tipoTriestado::OFF;
    if (parametros(respuesta, 3) == "1")
        abrir.estado = tipoTriestado::ON;
    else
        abrir.estado = tipoTriestado::OFF;
    if (parametros(respuesta, 4) == "1")
        reset.estado = tipoTriestado::ON;
    else
        reset.estado = tipoTriestado::OFF;

    codret.estado = tipoTriestado::ON;
    codret.mensaje = "";
    return codret;
}

triestado CCupulaMovil::getInputs(triestado &cerrado, triestado &abierto)
{
    string mensaje = "getInputs";
    triestado codret;
    string respuesta;
    comunicar(mensaje, respuesta);
    if (respuesta.substr(0, 2) != "OK")
    {
        LOG4CXX_INFO(pLogger, "Error en la función getInputs: " + respuesta);
        cerrado.estado = tipoTriestado::ERROR;
        cerrado.mensaje = respuesta;
        abierto.estado = tipoTriestado::ERROR;
        abierto.mensaje = respuesta;
        codret.estado = tipoTriestado::ERROR;
        codret.mensaje = respuesta;
        return codret;
    }

    if (parametros(respuesta, 1) == "1")
        cerrado.estado = tipoTriestado::ON;
    else
        cerrado.estado = tipoTriestado::OFF;
    if (parametros(respuesta, 2) == "1")
        abierto.estado = tipoTriestado::ON;
    else
        abierto.estado = tipoTriestado::OFF;

    codret.estado = tipoTriestado::ON;
    codret.mensaje = "";
    return codret;
}

triestado CCupulaMovil::getStatus(stringmap &estados)
{
    string mensaje = "getInputs";
    triestado codret;
    string respuesta;
    comunicar(mensaje, respuesta);
    if (respuesta.substr(0, 2) != "OK")
    {
        LOG4CXX_INFO(pLogger, "Error en la función getInputs: " + respuesta);
        estados["valido"] = "no";
        estados["mensaje"] = respuesta;
        return codret;
    }

    estados["valido"] = "si";
    estados["r_luz"] = parametros(respuesta, 1);
    estados["r_cerrar"] = parametros(respuesta, 2);
    estados["r_abrir"] = parametros(respuesta, 3);
    estados["k_luz"] = parametros(respuesta, 4);
    estados["k_cerrar"] = parametros(respuesta, 5);
    estados["k_abrir"] = parametros(respuesta, 6);
    estados["k_reset"] = parametros(respuesta, 7);
    estados["i_abierto"] = parametros(respuesta, 8);
    estados["i_cerrado"] = parametros(respuesta, 9);
    estados["m_accion"] = parametros(respuesta, 10);
    estados["m_gradoAvance"] = parametros(respuesta, 11);
    estados["m_timeout"] = parametros(respuesta, 12);
    estados["c_estado"] = parametros(respuesta, 13);
    estados["m_tiempoAbrir"] = parametros(respuesta, 14);
    estados["m_tiempoCerrar"] = parametros(respuesta, 15);

    codret.estado = tipoTriestado::ON;
    codret.mensaje = "";
    return codret;
}

triestado CCupulaMovil::calibrateSincrono(datosCalibrado &dc)
{
    triestado codret = calibrate("CALIBRATE", dc);
    if (codret.estado == tipoTriestado::ERROR)
        return codret;
    while (true)
    {
        codret = calibrate("GET", dc);
        if (codret.estado == tipoTriestado::ERROR)
            return codret;
        if (dc.finalizado == true)
        {
            codret.estado = tipoTriestado::ON;
            codret.mensaje = "";
            return codret;
        }
        LOG4CXX_DEBUG(pLogger, "Calibrando la ventana: " + dc.estadoCalibrado);
        sleep(5);
    }
}
