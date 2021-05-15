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

int CCupulaMovil::conectar()
{
    if (p_puerto_serie != nullptr)
    {
        LOG4CXX_ERROR(pLogger, "No se puede abrir la conexión porque ya está establecida");
        return EXIT_FAILURE;
    }

    p_puerto_serie = new SerialPort(pParam->serial_nombre);

    try
    {
        p_puerto_serie->Open(SerialPort::BAUD_9600);
    }
    catch (SerialPort::OpenFailed E)
    {
        LOG4CXX_FATAL(pLogger, "Error establecido la conexión BLUETOOTH. Asegurar que el device está creado");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
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

int CCupulaMovil::estadoLuz(triestado &salida)
{
    string respuesta;
    if (comunicar("getRelays", respuesta) == EXIT_SUCCESS)
    {
        if (respuesta.substr(0, 2) == "OK")
        {
            string p1 = parametros(respuesta, 1);
            if (p1 == "1")
                salida.estado = tipoTriestado::ON;
            else
                salida.estado = tipoTriestado::OFF;
            return EXIT_SUCCESS;
        }
        else
        {
            salida.estado = tipoTriestado::ERROR;
            salida.mensaje = respuesta;
            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}

int CCupulaMovil::Luz(bool bEncender)
{
    string mensaje;
    if (bEncender)
        mensaje = "luz=ON";
    else
        mensaje = "luz=OFF";
    string respuesta;
    if (comunicar(mensaje, respuesta) == EXIT_SUCCESS)
    {
        if (respuesta.substr(0, 2) != "OK")
        {
            LOG4CXX_INFO(pLogger, "Error en la función Luz: " + respuesta);
        }
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
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

int CCupulaMovil::setLogging(string level)
{
    string mensaje;
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
        return EXIT_FAILURE;
    }

    string respuesta;
    if (comunicar(mensaje, respuesta) == EXIT_SUCCESS)
    {
        if (respuesta.substr(0, 2) != "OK")
        {
            LOG4CXX_INFO(pLogger, "Error en la función setLogging: " + respuesta);
        }
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
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

int CCupulaMovil::calibrate(string accion, datosCalibrado &dc)
{
    static string accionActual = "";
    string mensaje;
    if (accion == "CALIBRATE")
    {
        if (accionActual != "")
        {
            LOG4CXX_DEBUG(pLogger, "Solicitud de calibrado con un calibrado ya en marcha");
            return EXIT_FAILURE;
        }

        accionActual = accion;
        mensaje = "calibrateShutter=CALIBRATE";
    }
    else if (accion == "GET")
    {
        if (accionActual != "CALIBRATE")
        {
            LOG4CXX_DEBUG(pLogger, "Solicitud de GET sin calibrado en marcha");
            return EXIT_FAILURE;
        }

        accionActual = accion;
        mensaje = "calibrateShutter=GET";
    }
    else if (accion == "PUT")
    {
        if (accionActual != "")
        {
            LOG4CXX_DEBUG(pLogger, "Solicitud de PUT con un calibrado ya en marcha");
            return EXIT_FAILURE;
        }

        accionActual = accion;
        mensaje = "calibrateShutter=PUT:" + to_string(dc.tiempoAbrir) + ":" + to_string(dc.tiempoCerrar);
    }
    else
    {
        LOG4CXX_DEBUG(pLogger, "Parámetro accion incorrecto");
        return EXIT_FAILURE;
    }

    string respuesta;
    if (comunicar(mensaje, respuesta) == EXIT_SUCCESS)
    {
        if (respuesta.substr(0, 2) != "OK")
        {
            LOG4CXX_INFO(pLogger, "Error en la función setLogging: " + respuesta);
        }
        return EXIT_SUCCESS;
    }

    if (accion == "GET")
    {
        dc.estadoCalibrado = parametros(respuesta, 1);
        dc.finalizado = stoi(parametros(respuesta, 2));
        if (dc.estadoCalibrado == "NINGUNO" && parametros(respuesta, 2) == "1")
        {
            dc.tiempoAbrir = stoul(parametros(respuesta, 3));
            dc.tiempoCerrar = stoul(parametros(respuesta, 4));
            accionActual = "";
        }
    }
    return EXIT_SUCCESS;
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

int CCupulaMovil::setEmergencyShutterTimeout(int segundos)
{
    string mensaje;
    mensaje = "setEmergencyShutterTimeout=" + to_string(segundos);
    string respuesta;
    if (comunicar(mensaje, respuesta) == EXIT_SUCCESS)
    {
        if (respuesta.substr(0, 2) != "OK")
        {
            LOG4CXX_INFO(pLogger, "Error en la función setEmergencyShutterTimeout: " + respuesta);
        }
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int CCupulaMovil::moveShutter(string accion, unsigned long TimeoutShutter)
{
    string mensaje;
    if (accion == "OPEN")
        mensaje = "moveShutter=OPEN";
    else if (accion == "CLOSE")
        mensaje = "moveShutter=CLOSE";
    else
    {
        LOG4CXX_DEBUG(pLogger, "Parámetro acción incorrecto");
        return EXIT_FAILURE;
    }

    if (TimeoutShutter > 0)
    {
        mensaje += ":" + to_string(TimeoutShutter);
    }

    string respuesta;
    if (comunicar(mensaje, respuesta) == EXIT_SUCCESS)
    {
        if (respuesta.substr(0, 2) != "OK")
        {
            LOG4CXX_INFO(pLogger, "Error en la función moveShutter: " + respuesta);
        }
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int CCupulaMovil::movimiento(string &accion, unsigned int &avance, bool &timeout)
{
    string mensaje = "getMovimiento";
    string respuesta;
    if (comunicar(mensaje, respuesta) == EXIT_SUCCESS)
    {
        if (respuesta.substr(0, 2) != "OK")
        {
            LOG4CXX_INFO(pLogger, "Error en la función latido: " + respuesta);
        }
        return EXIT_SUCCESS;
    }

    accion = parametros(respuesta, 1);
    avance = stoi(parametros(respuesta, 2));
    timeout = stoi(parametros(respuesta, 3));

    return EXIT_FAILURE;
}

int CCupulaMovil::stopShutter()
{
    string mensaje = "stopShutter";
    string respuesta;
    if (comunicar(mensaje, respuesta) == EXIT_SUCCESS)
    {
        if (respuesta.substr(0, 2) != "OK")
        {
            LOG4CXX_INFO(pLogger, "Error en la función stopShutter: " + respuesta);
        }
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int CCupulaMovil::getRelays(triestado &luz, triestado &cerrar, triestado &abrir)
{
    string mensaje = "getRelays";
    string respuesta;
    if (comunicar(mensaje, respuesta) == EXIT_SUCCESS)
    {
        if (respuesta.substr(0, 2) != "OK")
        {
            LOG4CXX_INFO(pLogger, "Error en la función getRelays: " + respuesta);
            luz.estado = tipoTriestado::ERROR;
            luz.mensaje = respuesta;
            cerrar.estado = tipoTriestado::ERROR;
            cerrar.mensaje = respuesta;
            abrir.estado = tipoTriestado::ERROR;
            abrir.mensaje = respuesta;
        }
        return EXIT_SUCCESS;
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

    return EXIT_FAILURE;
}

int CCupulaMovil::getButtons(triestado &luz, triestado &cerrar, triestado &abrir, triestado &reset)
{
    string mensaje = "getButtons";
    string respuesta;
    if (comunicar(mensaje, respuesta) == EXIT_SUCCESS)
    {
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
        }
        return EXIT_SUCCESS;
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

    return EXIT_FAILURE;
}

int CCupulaMovil::getInputs(triestado &cerrado, triestado &abierto)
{
    string mensaje = "getInputs";
    string respuesta;
    if (comunicar(mensaje, respuesta) == EXIT_SUCCESS)
    {
        if (respuesta.substr(0, 2) != "OK")
        {
            LOG4CXX_INFO(pLogger, "Error en la función getInputs: " + respuesta);
            cerrado.estado = tipoTriestado::ERROR;
            cerrado.mensaje = respuesta;
            abierto.estado = tipoTriestado::ERROR;
            abierto.mensaje = respuesta;
        }
        return EXIT_SUCCESS;
    }

    if (parametros(respuesta, 1) == "1")
        cerrado.estado = tipoTriestado::ON;
    else
        cerrado.estado = tipoTriestado::OFF;
    if (parametros(respuesta, 2) == "1")
        abierto.estado = tipoTriestado::ON;
    else
        abierto.estado = tipoTriestado::OFF;

    return EXIT_FAILURE;
}

int CCupulaMovil::getStatus(stringmap &estados)
{
    string mensaje = "getInputs";
    string respuesta;
    if (comunicar(mensaje, respuesta) == EXIT_SUCCESS)
    {
        if (respuesta.substr(0, 2) != "OK")
        {
            LOG4CXX_INFO(pLogger, "Error en la función getInputs: " + respuesta);
            estados["valido"] = "no";
            estados["mensaje"] = respuesta;
        }
        return EXIT_SUCCESS;
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

    return EXIT_FAILURE;
}

int CCupulaMovil::calibrateSincrono(datosCalibrado &dc)
{
    if (calibrate("CALIBRATE", dc) == EXIT_FAILURE)
        return EXIT_FAILURE;
    while (true)
    {
        int r = calibrate("GET", dc);
        if (r == EXIT_FAILURE)
            return EXIT_FAILURE;
        if (dc.finalizado == true)
            return EXIT_SUCCESS;
        LOG4CXX_DEBUG(pLogger, "Calibrando la ventana: " + dc.estadoCalibrado);
        sleep(5);
    }
}
