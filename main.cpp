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
CCupulaFijo cf = CCupulaFijo(&param, m_pi_id);;
CCupulaMovil cm = CCupulaMovil(&param);
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
    //cf = &CCupulaFijo(&param, m_pi_id);
    //cm = &CCupulaMovil(&param);
    cf.set_pi_id(m_pi_id);


    if (cm.conectar() == EXIT_FAILURE)
    {
        LOG4CXX_ERROR(logger, "no es posible conectar con la parte móvil");
        cf.finalizarThreads(tipoThread::todos);
        return Finalizar(EXIT_FAILURE);
    }

    string respuesta;
    cm.getFirmwareVersion(respuesta);

    //A la espera de conexión
    try
    {
        auto const address = net::ip::make_address(param.websocket_local_ip);
        auto const port = static_cast<unsigned short>(stoul(param.websocket_port));

        // The io_context is required for all I/O
        net::io_context ioc{1};

        // The acceptor receives incoming connections
        tcp::acceptor acceptor{ioc, {address, port}};
        while (true)
        {
            // This will receive the new connection
            tcp::socket socket{ioc};

            // Block until we get a connection
            LOG4CXX_DEBUG(logger, "Esperando conexión en IP: " + param.websocket_local_ip + "PORT: " + param.websocket_port);
            acceptor.accept(socket);

            // Launch the session, transferring ownership of the socket
            std::thread(&do_session, std::move(socket), &cf, &cm).detach();
        }
    }
    catch (const std::exception &e)
    {
        LOG4CXX_ERROR(logger, "Error en el listener de websocket: " + string(e.what()));
        cf.finalizarThreads(tipoThread::todos);
        return Finalizar(EXIT_FAILURE);
    }
}
//------------------------------------------------------------------------------

// Thread con la gestión del socket
void do_session(tcp::socket socket, CCupulaFijo *pcf, CCupulaMovil *pcm)
{
    log4cxx::LoggerPtr pLoggerLocal = log4cxx::Logger::getRootLogger();

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
            tratamientoMensaje(s, r, pcf, pcm);

            // Enviar respuesta
            ws.text(ws.got_text());
            net::const_buffer bufferR(r.c_str(),r.length());
            ws.write(bufferR);
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

int tratamientoMensaje(string s, string &r, CCupulaFijo *pcf, CCupulaMovil *pcm)
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

    if (orden == "getFirmwareVersion")
    {
        string parte;
        if (vParametros.size() == 0)
            parte = "fija";
        else if (vParametros.size() == 1)
        {
            if (vParametros[0] == "fija")
                parte = "fija";
            else
                parte = "movil";
        }
        else
        {
            LOG4CXX_ERROR(pLoggerLocal, "Error en el tratamiento del mensaje: " + s + " orden: " + orden + " - Número de parámetros incorrectos");
            r = "KO=Numero de parametros incorrectos";
            return EXIT_SUCCESS;
        }

        string version;
        if (parte == "fija")
        {
            r = "OK=";
            r += FIRMWARE_VERSION;
        }
        else
        {
            if (pcm->getFirmwareVersion(version) == EXIT_SUCCESS)
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
        return EXIT_SUCCESS;
    }
    else
    {
        LOG4CXX_ERROR(pLoggerLocal, "Error en el tratamiento del mensaje: " + s + " orden: " + orden + " - La orden no existe");
        r = "KO=la orden no existe";
        return EXIT_SUCCESS;
    }
}
//------------------------------------------------------------------------------
