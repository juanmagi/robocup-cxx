#pragma once
#include <string>

void signalHandler( int signum );
int Finalizar(int estado);
int ejecutarTest(int num_test);
std::string enviar(std::string orden);
std::string parametros(std::string respuesta,int posicion);
