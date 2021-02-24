#include <iostream>
#include "log4cxx/logger.h"
#include "log4cxx/xml/domconfigurator.h"
#include "log4cxx/patternlayout.h"
#include "log4cxx/fileappender.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "pigpiod_if2.h"
#include "CConfig.h"

using namespace std;
namespace pt = boost::property_tree;

log4cxx::LoggerPtr logger(log4cxx::Logger::getRootLogger());
CConfig param;
bool m_finalizar=false;
int m_pi_id=0;

void signalHandler( int signum ) {
	if (logger!=nullptr)
		LOG4CXX_DEBUG (logger,"SIGTERM Recibido");
	m_finalizar=true;
}

int main(int /*argc*/, char **/*argv*/){
	//Cargamos los parámteros
	param.load(FICHERO_PARAMETROS,MODO);

	//Inicializo el sistema de log
	std :: setlocale ( LC_ALL , "" );
	std :: locale :: global ( std :: locale ( "" ));

	log4cxx::AppenderPtr defaultAppender = nullptr;
	log4cxx::LayoutPtr   defaultLayout   = nullptr;

	defaultLayout   = new log4cxx::PatternLayout("%p-%d{dd/MMM/yyy-HH:mm:ss}-%m%n");
	defaultAppender = new log4cxx::FileAppender(defaultLayout,param.log_fichero);
	logger->addAppender(defaultAppender);
	logger->setLevel(param.log_nivel);

    m_pi_id=pigpio_start((char*)param.pigpio_server.c_str(),(char*)param.pigpio_port.c_str());
    if (m_pi_id<0){
        std::cout << "TODO MAL!\n";
        return EXIT_FAILURE;
    }

    cout << "HW Versión: " << get_hardware_revision(m_pi_id) << endl;
    cout << "pigpiod Versión: " << get_pigpio_version(m_pi_id) << endl;
    cout << "pigpiod_if2 Versión: " << pigpiod_if_version() << endl;

    pigpio_stop(m_pi_id);

    std::cout << "TODO OK!\n";

    return EXIT_SUCCESS;
}
