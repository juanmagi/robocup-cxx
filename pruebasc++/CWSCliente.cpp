#include <cstdlib>
#include <iostream>

#include "CWSCliente.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


CWSCliente::CWSCliente(/* args */)
{
}

CWSCliente::~CWSCliente()
{
}

int CWSCliente::conectar(std::string host,std::string port){
    try
    {
        // These objects perform our I/O
        tcp::resolver resolver{ioc};

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        auto ep = net::connect(ws.next_layer(), results);

        // Update the host_ string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4
        host += ':' + std::to_string(ep.port());

        std::cout << "host: " << host << std::endl;

        // Set a decorator to change the User-Agent of the handshake
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-coro");
            }));

        // Perform the websocket handshake
        std::cout << "set option" << std::endl;
        ws.handshake(host, "/");
        std::cout << "Apretón de manos" << std::endl;
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;


}

std::string CWSCliente::enviar(std::string text){
    try
    {
        // Send the message
        ws.write(net::buffer(std::string(text)));

        // This buffer will hold the incoming message
        beast::flat_buffer buffer;

        // Read a message into our buffer
        ws.read(buffer);

        std::string respuesta=beast::buffers_to_string(buffer.data());

        return respuesta;
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return "ERROR";
    }
}

int CWSCliente::desconectar(){
    try
    {
        // Close the WebSocket connection
        ws.close(websocket::close_code::normal);
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
