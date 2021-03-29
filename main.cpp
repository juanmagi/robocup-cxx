#include <unistd.h>
#include <iostream>
#include <string>
#include "log4cxx/logger.h"
#include "log4cxx/xml/domconfigurator.h"
#include "log4cxx/patternlayout.h"
#include "log4cxx/fileappender.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "pigpiod_if2.h"
#include "CCupulaFijo.h"
#include "CCupulaMovil.h"
#include "CConfig.h"
#include "main.h"

/*
Página web de la librerías utizadas e informativas generales
c++: http://www.cplusplus.com/
boost: https://www.boost.org/
boost - property tree:  https://www.boost.org/doc/libs/1_65_1/doc/html/property_tree.html
pigpio - pigpiod_if2d: http://abyz.me.uk/rpi/pigpio/pdif2.html
log4cxx - https://logging.apache.org/log4cxx/latest_stable/usage.html
*/

log4cxx::LoggerPtr logger(log4cxx::Logger::getRootLogger());
CConfig param;
bool m_finalizar = false;
int m_pi_id = 0;

int main(int /*argc*/, char ** /*argv*/)
{
	//Cargamos los parámetros
	param.load();

	//Inicializo el sistema de log
	std ::setlocale(LC_ALL, "");
	std ::locale ::global(std ::locale(""));

	log4cxx::AppenderPtr defaultAppender = nullptr;
	log4cxx::LayoutPtr defaultLayout = nullptr;

	defaultLayout = new log4cxx::PatternLayout("%p-%d{dd/MMM/yyy-HH:mm:ss,SSS}-%m%n");
	defaultAppender = new log4cxx::FileAppender(defaultLayout, param.log_fichero);
	logger->addAppender(defaultAppender);
	logger->setLevel(param.log_nivel);

	m_pi_id = pigpio_start((char *)param.pigpio_server.c_str(), (char *)param.pigpio_port.c_str());
	if (m_pi_id < 0)
	{
		LOG4CXX_FATAL(logger, "Error establecido la conexión con PIGPIOD. Asegurar que el proceso está lanzado");
		return EXIT_FAILURE;
	}

	CCupulaFijo cf = CCupulaFijo(&param, m_pi_id);
	CCupulaMovil cm = CCupulaMovil(&param);
	cf.calibrate();
	LOG4CXX_DEBUG(logger, "Número de pines: " + to_string(param.cupula_max_posiciones));
	LOG4CXX_DEBUG(logger, "A posición 10 absoluta");
	cf.setPosicion(sentidoMovimiento::CORTA, tipoPosicion::absoluta, 10);
	while (CCupulaFijo::sentido != sentidoMovimiento::PARADO)
	{
		LOG4CXX_DEBUG(logger, "Posición: " + to_string(cf.getPosicion()));
		usleep(10000);
	}
	LOG4CXX_DEBUG(logger, "A posición +10 relativa (20 absoluta)");
	cf.setPosicion(sentidoMovimiento::CW, tipoPosicion::relativa, 10);
	while (CCupulaFijo::sentido != sentidoMovimiento::PARADO)
	{
		LOG4CXX_DEBUG(logger, "Posición: " + to_string(cf.getPosicion()));
		usleep(10000);
	}
	LOG4CXX_DEBUG(logger, "A posición -30 relativa (50 absoluta)");
	cf.setPosicion(sentidoMovimiento::CW, tipoPosicion::relativa, -30);
	while (CCupulaFijo::sentido != sentidoMovimiento::PARADO)
	{
		LOG4CXX_DEBUG(logger, "Posición: " + to_string(cf.getPosicion()));
		usleep(10000);
	}
	LOG4CXX_DEBUG(logger, "A posición DAH");
	cf.DomeAtHome(true);
	while (CCupulaFijo::sentido != sentidoMovimiento::PARADO)
	{
		LOG4CXX_DEBUG(logger, "Posición: " + to_string(cf.getPosicion())+ " OnMovil: "+to_string(cf.mapaPines[param.gpio_pin_act_on_movil]));
		usleep(10000);
	}
	LOG4CXX_DEBUG(logger, "A ángulo 180");
	cf.setAngulo(sentidoMovimiento::CW, tipoPosicion::absoluta, 180);
	while (CCupulaFijo::sentido != sentidoMovimiento::PARADO)
	{
		LOG4CXX_DEBUG(logger, "Posición: " + to_string(cf.getPosicion())+ " OnMovil: "+to_string(cf.mapaPines[param.gpio_pin_act_on_movil]));
		usleep(10000);
	}
	cf.finalizarThreads(tipoThread::todos);
	Finalizar(EXIT_SUCCESS);
}

void signalHandler(int signum)
{
	if (logger != nullptr)
		LOG4CXX_DEBUG(logger, "SIGTERM Recibido");
	m_finalizar = true;
}

int Finalizar(int estado)
{
	pigpio_stop(m_pi_id);
	LOG4CXX_INFO(logger, "TODO OK");
	return estado;
}