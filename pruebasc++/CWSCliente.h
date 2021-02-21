#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>

class CWSCliente
{
private:
    boost::asio::io_context ioc;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws{ioc};

public:
    CWSCliente(/* args */);
    ~CWSCliente();
    int conectar(std::string host,std::string port);
    std::string enviar(std::string text);
    int desconectar();
};
