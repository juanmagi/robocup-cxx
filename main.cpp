//
// Copyright (c) 2021 Juan Manuel Giménez
//

//------------------------------------------------------------------------------
//
// robocup - Procedimiento de inicio - main
//
// Página web de la librerías utizadas e informativas generales
// c++: http://www.cplusplus.com/
// boost: https://www.boost.org/
// boost - property tree:  https://www.boost.org/doc/libs/1_65_1/doc/html/property_tree.html
// boost - beast https://www.boost.org/doc/libs/1_75_0/libs/beast/doc/html/index.html
// pigpio - pigpiod_if2d: http://abyz.me.uk/rpi/pigpio/pdif2.html
// log4cxx - https://logging.apache.org/log4cxx/latest_stable/usage.html
//------------------------------------------------------------------------------

#include <cstdlib>
#include <functional>
#include <atomic>
#include <thread>
#include <unistd.h>
#include <iostream>
#include <string>
#include "log4cxx/logger.h"
#include "log4cxx/xml/domconfigurator.h"
#include "log4cxx/patternlayout.h"
#include "log4cxx/fileappender.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
//#include <boost/asio/buffers_iterator.hpp>
#include "pigpiod_if2.h"
#include "CCupulaFijo.h"
#include "CCupulaMovil.h"
#include "CConfig.h"
#include "main.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

log4cxx::LoggerPtr logger(log4cxx::Logger::getRootLogger());
CConfig param;
bool m_finalizar = false;
int m_pi_id = 0;
atomic_flag afMensajes = ATOMIC_FLAG_INIT;
enum estadoMensaje
{
    VACIO,
    PETICION,
    RESPUESTA
};
struct stMensaje
{
    atomic<estadoMensaje> estado;
    string peticion;
    string respuesta;
};
unordered_map<std::thread::id, stMensaje *> mensajes;
atomic<bool> conexionVivo(true);

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    //Carga de los parámetros
    param.load();

    //Inicialización del sistema de logs
    std ::setlocale(LC_ALL, "");
    std ::locale ::global(std ::locale(""));
    log4cxx::AppenderPtr defaultAppender = nullptr;
    log4cxx::LayoutPtr defaultLayout = nullptr;
    defaultLayout = new log4cxx::PatternLayout("%p-%d{dd/MMM/yyy-HH:mm:ss,SSS}-%m%n");
    defaultAppender = new log4cxx::FileAppender(defaultLayout, param.log_fichero);
    logger->addAppender(defaultAppender);
    logger->setLevel(param.log_nivel);

    //Inicialización del sistema pigpio
    m_pi_id = pigpio_start((char *)param.pigpio_server.c_str(), (char *)param.pigpio_port.c_str());
    if (m_pi_id < 0)
    {
        LOG4CXX_FATAL(logger, "Error establecido la conexión con PIGPIOD. Asegurar que el proceso está lanzado");
        return EXIT_FAILURE;
    }

    //Inicialización de los objetos cúpula
    CCupulaFijo cf = CCupulaFijo(&param, m_pi_id);
    CCupulaMovil cm = CCupulaMovil(&param);

    /*    if (cm.conectar() == EXIT_FAILURE)
    {
        LOG4CXX_FATAL(logger, "no es posible conectar con la parte móvil");
        cf.finalizarThreads(tipoThread::todos);
        return Finalizar(EXIT_FAILURE);
    }
*/
    thread *pConexion = new thread(do_conexion, param.websocket_local_ip, param.websocket_port);

    //Bucle de ejecución
    while (conexionVivo)
    {
        while (afMensajes.test_and_set()) //Espero hasta poder acceder a la cola de mensajes
        {
        }

        //Iteramos por toda la cola atendiendo los mensajes
        if (mensajes.empty())
        {
            afMensajes.clear();
            usleep(1000);
        }
        else
        {
            for (auto &it : mensajes)
            {
                if (it.second->estado == estadoMensaje::PETICION)
                {
                    if (tratamientoMensaje(it.second->peticion, it.second->respuesta, cf, cm, param) == EXIT_SUCCESS)
                    {
                        it.second->estado = estadoMensaje::RESPUESTA;
                    }
                    else
                    {
                        it.second->estado = estadoMensaje::RESPUESTA;
                        afMensajes.clear();
                        LOG4CXX_FATAL(logger, "Error en la función tratamientoMensaje");
                        cf.finalizarThreads(tipoThread::todos);
                        delete pConexion; //No puedo matar el thread porque normalmnte está bloqueado en acceptor
                        return Finalizar(EXIT_FAILURE);
                    }
                }
            }
            afMensajes.clear();
        }
    }
    LOG4CXX_FATAL(logger, "listener websockets se ha muerto"); //No lo rearranco porque es sintoma deque algo va mal
    delete pConexion;
    cf.finalizarThreads(tipoThread::todos);
    return Finalizar(EXIT_FAILURE);
}
//------------------------------------------------------------------------------

// Thread con la gestión de la conexión IP
void do_conexion(string ip, string ip_port)
{
    try
    {
        auto const address = net::ip::make_address(ip);
        auto const port = static_cast<unsigned short>(stoul(ip_port));

        // The io_context is required for all I/O
        net::io_context ioc{1};

        // The acceptor receives incoming connections
        tcp::acceptor acceptor{ioc, {address, port}};
        while (true)
        {
            // This will receive the new connection
            tcp::socket socket{ioc};

            // Block until we get a connection
            LOG4CXX_DEBUG(logger, "Esperando conexión en IP: " + ip + "PORT: " + ip_port);
            try
            {
                acceptor.accept(socket);
            }
            catch (const std::exception &e)
            {
                LOG4CXX_ERROR(logger, "Error en el listener INTERNO de websocket: " + string(e.what()));
                continue;
            }

            // Launch the session, transferring ownership of the socket
            //std::thread(&do_session, std::move(socket), &cf, &cm).detach();
            std::thread(&do_session, std::move(socket)).detach();
        }
    }
    catch (const std::exception &e)
    {
        LOG4CXX_ERROR(logger, "Error en el listener de websocket: " + string(e.what()));
    }
    conexionVivo = false;
}

// Thread con la gestión del socket
//void do_session(tcp::socket socket, CCupulaFijo *pcf, CCupulaMovil *pcm)
void do_session(tcp::socket socket)
{
    log4cxx::LoggerPtr pLoggerLocal = log4cxx::Logger::getRootLogger();
    stringstream ss;
    ss << this_thread::get_id();
    LOG4CXX_DEBUG(pLoggerLocal, "socket creado:" + ss.str());

    try
    {
        // Construct the stream by moving in the socket
        websocket::stream<tcp::socket> ws{std::move(socket)};

        // Set a decorator to change the Server of the handshake
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::response_type &res) {
                res.set(http::field::server,
                        std::string(BOOST_BEAST_VERSION_STRING) +
                            " websocket-server-sync");
            }));

        // Accept the websocket handshake
        ws.accept();

        for (;;)
        {
            // This buffer will hold the incoming message
            beast::flat_buffer buffer;

            // Lee un mensaje.
            // Formato de los mensajes: orden=parametro1:parametro2:...:parametroN
            // Formato de la respuesta: OK o KO=parametro1:parametro2:...:parametroN
            ws.read(buffer);
            string s(boost::asio::buffer_cast<const char *>(buffer.data()), buffer.size());
            LOG4CXX_DEBUG(pLoggerLocal, "Mensaje Recibido: " + s);

            string r;
            stMensaje *pMsj;
            while (afMensajes.test_and_set())
            {
            }
            try
            {
                pMsj = mensajes.at(this_thread::get_id());
            }
            catch (const std::out_of_range &oor)
            {
                pMsj = new stMensaje;
            }
            pMsj->estado = estadoMensaje::PETICION;
            pMsj->peticion = s;
            mensajes[this_thread::get_id()] = pMsj;
            afMensajes.clear();
            while (pMsj->estado != estadoMensaje::RESPUESTA)
            {
                this_thread::yield();
            }
            while (afMensajes.test_and_set())
            {
            }
            r = pMsj->respuesta;
            pMsj->estado == estadoMensaje::VACIO;
            afMensajes.clear();

            // Enviar respuesta
            ws.text(ws.got_text());
            net::const_buffer bufferR(r.c_str(), r.length());
            ws.write(bufferR);
            if (s == "salir" && r == "OK")
            {
                break;
            }
        }
    }
    catch (beast::system_error const &se)
    {
        // This indicates that the session was closed
        if (se.code() != websocket::error::closed)
            LOG4CXX_ERROR(pLoggerLocal, "Error: " + se.code().message());
    }
    catch (std::exception const &e)
    {
        LOG4CXX_ERROR(pLoggerLocal, "Error en el listener de wiebsocket: " + string(e.what()));
    }

    while (afMensajes.test_and_set())
    {
    }
    delete mensajes[this_thread::get_id()];
    mensajes.erase(this_thread::get_id());
    afMensajes.clear();
    LOG4CXX_DEBUG(pLoggerLocal, "socket finalizado:" + ss.str());
}
//------------------------------------------------------------------------------

void signalHandler(int signum)
{
    if (logger != nullptr)
        LOG4CXX_DEBUG(logger, "SIGTERM Recibido");
    m_finalizar = true;
}
//------------------------------------------------------------------------------

int Finalizar(int estado)
{
    pigpio_stop(m_pi_id);
    LOG4CXX_INFO(logger, "TODO OK");
    return estado;
}
//------------------------------------------------------------------------------

int tratamientoMensaje(string s, string &r, CCupulaFijo &cf, CCupulaMovil &cm, CConfig &param)
//int tratamientoMensaje(string s, string &r)
{
    log4cxx::LoggerPtr pLoggerLocal = log4cxx::Logger::getRootLogger();
    string orden;
    string parametros;
    vector<string> vParametros;
    if (s.find("=") == string::npos)
    { //Caracter no encontrado
        orden = s;
    }
    else
    {
        orden = s.substr(0, s.find("="));
        parametros = s.substr(s.find("=") + 1);
        size_t pos = 0;
        vParametros.push_back(parametros); //Si sólo hay un parámetro, ya está cargado
        for (int i = 0; (pos = parametros.find(":")) != string::npos; i++)
        {
            vParametros[i] = parametros.substr(0, pos);
            parametros.erase(0, pos + 1);
            vParametros.push_back(parametros);
        }
    }

    CConfig::comando c;
    if (!param.getComando(orden, c))
    {
        LOG4CXX_ERROR(pLoggerLocal, "Error en el tratamiento del mensaje: " + s + " orden: " + orden + " - La orden no existe");
        r = "KO=la orden no existe";
        return EXIT_SUCCESS;
    }

    //Se verifica que no haya más parámetros de los necesarios
    if (c.num_param < vParametros.size())
    {
        LOG4CXX_ERROR(pLoggerLocal, "Error en el tratamiento del mensaje: " + s + " orden: " + orden + " - Número de parámetros incorrectos");
        r = "KO=Numero de parametros incorrectos";
        return EXIT_SUCCESS;
    }

    //Se verifica que los parámetros informados tengan valores aceptables
    int i, j;
    bool paramOK = false;
    for (i = 0; i < vParametros.size(); i++)
    {
        for (j = 0; j < c.parametros[i].size(); j++)
        {
            try
            {
                if (c.parametros[i][j] == "_int_")
                {
                    int valor = stoi(vParametros[i]);
                }
                else if (c.parametros[i][j] == "_long_")
                {
                    long valor = stol(vParametros[i]);
                }
                else if (c.parametros[i][j] == "_ulong_")
                {
                    unsigned long valor = stoul(vParametros[i]);
                }
                else if (c.parametros[i][j] == "_uint_")
                {
                    unsigned long valor = stoul(vParametros[i]);
                }
            }
            catch (const exception &e)
            {
                break;
            }

            if (c.parametros[i][j] == vParametros[i])
            {
                paramOK = true;
                break;
            }
        }
        if (!paramOK)
        {
            LOG4CXX_ERROR(pLoggerLocal, "Error en el tratamiento del mensaje: " + s + " orden: " + orden + " - Parámetro inconsistente");
            r = "KO=Parametro inconsistente";
            return EXIT_SUCCESS;
        }
        paramOK = false;
    }

    //Se asignan valores por defecto de los parámetros no informados
    for (i = vParametros.size(); i < c.num_param; i++)
    {
        if (c.parametros[i][0][0] == '_') //Si se encuenta,por ejemplo, un "_int_", este debe ser el último y no tiene valor por defecto, por tanto se acaba la sustitución
            break;
        vParametros.push_back(c.parametros[i][0]);
    }

    //Ejecución del comando
    if (orden == "salir")
    {
        r = "OK";
    }
    else if (orden == "getFirmwareVersion")
    {
        string version;
        if (vParametros[0] == "fija")
        {
            r = "OK=";
            r += FIRMWARE_VERSION;
        }
        else
        { //Parte móvil
            if (cm.getFirmwareVersion(version) == EXIT_SUCCESS)
            {
                r = "OK=";
                r += version;
            }
            else
            {
                r = "KO=Error en la ejecución de ";
                r += s;
                r += "Puede ser un error de comunicación con la parte móvil";
            }
        }
    }
    else if (orden == "getDomeAtHome")
    {
        if (cf.estadoDAH())
        {
            r = "OK=";
            r += "si";
        }
        else
        {
            r = "OK=";
            r += "no";
        }
    }
    else if (orden == "setDomeAtHome")
    {
        cf.setPosicion(sentidoMovimiento::CORTA, tipoPosicion::dah, 0);
        r = "OK";
    }
    else if (orden == "getPosicion")
    {
        r = "OK=";
        r += to_string(cf.getAngulo());
    }
    else if (orden == "setPosicion")
    {
        int grados;
        sentidoMovimiento sm;
        tipoPosicion tp;

        if (vParametros.size() == 3)
        {
            grados = stoi(vParametros[2]);
            if (grados < -360 || grados > 360)
            {
                LOG4CXX_ERROR(pLoggerLocal, "Error en el tratamiento del mensaje: " + s + " orden: " + orden + " - Parámetro 3 inconsistente");
                r = "KO=Parametro 3 inconsistente";
                return EXIT_SUCCESS;
            }
        }
        else
        {
            grados = 0;
        }
        if (vParametros[0] == "CW")
            sm = sentidoMovimiento::CW;
        else if (vParametros[0] == "CCW")
            sm = sentidoMovimiento::CCW;
        else if (vParametros[0] == "CORTA")
            sm = sentidoMovimiento::CORTA;

        if (vParametros[1] == "absluta")
            tp = tipoPosicion::absoluta;
        else if (vParametros[1] == "relativa")
            tp = tipoPosicion::relativa;

        cf.setAngulo(sm, tp, grados);
        r = "OK";
    }
    else if (orden == "calibrar")
    {
        bool p1, p2;
        if (vParametros[0] == "si")
            p1 = true;
        else
            p1 = false;
        if (vParametros[1] == "si")
            p2 = true;
        else
            p2 = false;

        cf.calibrate(p1, p2);
        r = "OK";
    }
    else if (orden == "estadoCalibrado")
    {
        string estado = to_string(cf.getEstadoCalibrado());
        r = "OK=";
        r += estado;
    }
    else if (orden == "mover")
    {
        sentidoMovimiento sm;

        if (vParametros[0] == "CW")
            sm = sentidoMovimiento::CW;
        else if (vParametros[0] == "CCW")
            sm = sentidoMovimiento::CCW;

        cf.mover(sm);
        r = "OK";
    }
    else if (orden == "parar")
    {
        cf.parar();
        r = "OK";
    }
    else if (orden == "conectarMovil")
    {
        if (!cf.estadoDAH())
            if (cf.getEstadoCalibrado() == estadosCalibrado::CALIBRADO)
                cf.setPosicion(sentidoMovimiento::CORTA, tipoPosicion::dah, 0);
            else
                cf.DomeAtHome(true);

        int i;
        for (i = 0; !cf.estadoDAH() && i < 120; i++) //Timeout de 2 minutos
            sleep(1);
        if (i >= 120)
        {
            LOG4CXX_ERROR(pLoggerLocal, "Timeout al mover la cúpula a DAH en la acción de conectar con la parte fija");
            r = "KO=Timeout al mover la cúpula a DAH en la acción de conectar con la parte fija";
            return EXIT_SUCCESS;
        }

        for (i = 0; !cf.getOnCupulaMovil() && i < 30; i++) //Timeout de 30 segundos
            sleep(1);
        if (i >= 30)
        {
            LOG4CXX_ERROR(pLoggerLocal, "Timeout al poner en ON la correinte a la parte móvil");
            r = "KO=Timeout al poner en ON la correinte a la parte móvil";
            return EXIT_SUCCESS;
        }

        if (cm.conectar() == EXIT_FAILURE)
        {
            LOG4CXX_ERROR(pLoggerLocal, "No es posible conectar con la parte móvil");
            r = "KO=No es posible conectar con la parte móvil";
            return EXIT_SUCCESS;
        }
        r = "OK";
    }
    else if (orden == "getLuz")
    {
        triestado estado;
        if (cm.estadoLuz(estado) == EXIT_FAILURE)
        {
            LOG4CXX_ERROR(pLoggerLocal, "Error de comunicación con la parte móvil");
            r = "KO=Error de comunicación con la parte móvil";
            return EXIT_SUCCESS;
        }
        string respuesta;
        if (estado.estado == tipoTriestado::ON)
            respuesta = "ON";
        else if (estado.estado == tipoTriestado::OFF)
            respuesta = "OFF";
        else if (estado.estado == tipoTriestado::ERROR)
        {
            respuesta = "ERROR:" + estado.mensaje;
        }
        r = "OK=" + respuesta;
    }
    else
    {
        LOG4CXX_ERROR(pLoggerLocal, "Error en el tratamiento del mensaje: " + s + " orden: " + orden + " - Comando sin tratamiento implementado");
        r = "KO=Comando sin tratamiento implementado";
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}
//------------------------------------------------------------------------------
