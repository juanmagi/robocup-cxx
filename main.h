#pragma once
#include <boost/asio/ip/tcp.hpp>
#include "global.h"

void do_conexion(std::string ip,std::string ip_port);
void do_session(boost::asio::ip::tcp::socket socket);
void signalHandler( int signum );
int Finalizar(int estado);
//------------------------------------------------------------------------------
//
// tratamientoMensaje: Gestión del mensake recibido
// param:
//  s: Mensaje de entrada
//  r: Mensaje de salida (respuesta)
//  cf: Objeto parte fija de la cúpula
//  cm: Objeto parte móvil de la cúpula
// 
//------------------------------------------------------------------------------
int tratamientoMensaje(std::string s, std::string &r, CCupulaFijo &cf, CCupulaMovil &cm);
