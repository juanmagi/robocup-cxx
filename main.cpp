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

log4cxx::LoggerPtr logger(log4cxx::Logger::getRootLogger());
CConfig param;
bool m_finalizar=false;
int m_pi_id=0;

int main(int /*argc*/, char **/*argv*/){
	//Cargamos los parámetros
	param.load();

	//Inicializo el sistema de log
	std :: setlocale ( LC_ALL , "" );
	std :: locale :: global ( std :: locale ( "" ));

	log4cxx::AppenderPtr defaultAppender = nullptr;
	log4cxx::LayoutPtr   defaultLayout   = nullptr;

	defaultLayout   = new log4cxx::PatternLayout("%p-%d{dd/MMM/yyy-HH:mm:ss,SSS}-%m%n");
	defaultAppender = new log4cxx::FileAppender(defaultLayout,param.log_fichero);
	logger->addAppender(defaultAppender);
	logger->setLevel(param.log_nivel);

    m_pi_id=pigpio_start((char*)param.pigpio_server.c_str(),(char*)param.pigpio_port.c_str());
    if (m_pi_id<0){
		LOG4CXX_FATAL (logger,"Error establecido la conexión con PIGPIOD. Asegurar que el proceso está lanzado");
        return EXIT_FAILURE;
    }

    CCupulaFijo cf=CCupulaFijo(&param,m_pi_id);
    CCupulaMovil cm=CCupulaMovil(&param);
	cf.mover(sentidoMovimiento::CW);
	for (int i=0;i<1000;i++){
		cf.getPosicion();
		usleep(10000);
	}
	cf.mover(sentidoMovimiento::CCW);
	for (int i=0;i<1000;i++){
		cf.getPosicion();
		usleep(10000);
	}
	cf.mover(sentidoMovimiento::PARADO);
	for (int i=0;i<1000;i++){
		cf.getPosicion();
		usleep(10000);
	}
	cf.finalizarThreads();

	Finalizar(EXIT_SUCCESS);
}


void signalHandler( int signum ) {
	if (logger!=nullptr)
		LOG4CXX_DEBUG (logger,"SIGTERM Recibido");
	m_finalizar=true;
}


int Finalizar(int estado){
    pigpio_stop(m_pi_id);
    LOG4CXX_INFO (logger,"TODO OK");
    return estado;            
}