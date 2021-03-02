#include <iostream>
#include <SerialPort.h>
#include "log4cxx/logger.h"
#include "log4cxx/xml/domconfigurator.h"
#include "log4cxx/patternlayout.h"
#include "log4cxx/fileappender.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "pigpiod_if2.h"
#include "CConfig.h"
#include "test_movil.h"

using namespace std;
namespace pt = boost::property_tree;

log4cxx::LoggerPtr logger(log4cxx::Logger::getRootLogger());
CConfig param;
bool m_finalizar=false;
int m_pi_id=0;
SerialPort *p_puerto_serie=nullptr;

int main(int /*argc*/, char **/*argv*/){
	//Cargamos los parámetros
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
		LOG4CXX_FATAL (logger,"Error establecido la conexión con PIGPIOD. Asegurar que el proceso está lanzado");
        return EXIT_FAILURE;
    }

    SerialPort puerto_serie(param.serial_nombre);
    p_puerto_serie=&puerto_serie;

    try {
        puerto_serie.Open( SerialPort::BAUD_9600 );
    }
    catch (SerialPort::OpenFailed E) {
        pigpio_stop(m_pi_id);
		LOG4CXX_FATAL (logger,"Error establecido la conexión BLUETOOTH. Asegurar que el device está creado");
        return EXIT_FAILURE;
    }

    string orden="latido";
    string respuesta;
    puerto_serie.Write( orden );
    try {
        respuesta=puerto_serie.ReadLine(1000U * 60U); //Timeout 1 minuto
        LOG4CXX_DEBUG (logger,"Recibiendo: " + respuesta);
    }
    catch (SerialPort::ReadTimeout E) {
        LOG4CXX_FATAL (logger,"TIMEOUT esperando datos por bluetooth");
        return Finalizar(EXIT_FAILURE);
    }

    if (respuesta.substr(0,2)=="OK"){
        LOG4CXX_DEBUG (logger,"Conexión Bluetooht OK --> Respuesta al latido: " + respuesta);;
    } else {
        LOG4CXX_FATAL (logger,"Conexión Bluetooht KO esperando la recepción de la orden latido");
        return Finalizar(EXIT_FAILURE);
    }

    //Bucle central
    int num_test=1;
    while (!m_finalizar){
        num_test = ejecutarTest(num_test);
        if (num_test==0){
            return Finalizar(EXIT_SUCCESS);
        }
    }

    return Finalizar(EXIT_SUCCESS);
}

void signalHandler( int signum ) {
	if (logger!=nullptr)
		LOG4CXX_DEBUG (logger,"SIGTERM Recibido");
	m_finalizar=true;
}


int Finalizar(int estado){
    p_puerto_serie->Close();
    pigpio_stop(m_pi_id);
    LOG4CXX_INFO (logger,"TODO OK");
    return estado;            
}

/*Ordenes posibles:
    getFirmwareVersion
    setLogging=LOG_LEVEL
    setEmergencyShutterTimeout=<timeout in secons> --> 0 no hay sistema de emergencia, cualquier otro valor hace que se cierre la puerta si se pierde contacto bluetooth con la parte móvil
    latido --> para detectar la falta de comunicación con la base
    moveShutter=<direction>:<(optional) timeout>:<(optional) release timeout> --> shutter movement; <direction> is 'OPEN' or 'CLOSE', <timeout> and <release timeout> in milliseconds are optional
    getMovimiento -->devuelve MovimientoShutter
    stopShutter
    calibrateShutter=<accion>:<tiempo abrir>:<tiempo cerrar> <accion> es CALIBRATE, GET o PUT
    luz=<accion>, es ON u OFF
    domeAtHome=<accion>, es ON u OFF
    getRelays
    getButtons
    getInputs
    getStatus --> se devuelve respuesta de valores separados por : Pendiente definir que se devuelve
*/

int ejecutarTest(int num_test){
    string orden;
    string rcv;
    static string tiempo_abrir,tiempo_cerrar;
    switch (num_test)
    {
    case 1:
        orden="getFirmwareVersion";
        enviar(orden);
        break;
    case 2:
        orden="setLogging=LOG_LEVEL_SILENT";
        enviar(orden);
        break;
    case 3:
        orden="setLogging=LOG_LEVEL_FATAL";
        enviar(orden);
        break;
    case 4:
        orden="setLogging=LOG_LEVEL_ERROR";
        enviar(orden);
        break;
    case 5:
        orden="setLogging=LOG_LEVEL_WARNING";
        enviar(orden);
        break;
    case 6:
        orden="setLogging=LOG_LEVEL_TRACE";
        enviar(orden);
        break;
    case 7:
        orden="setLogging=LOG_LEVEL_KK";
        enviar(orden);
        break;
    case 8:
        orden="setLogging=LOG_LEVEL_VERBOSE";
        enviar(orden);
        break;
    case 9:
        orden="setEmergencyShutterTimeout=30";
        enviar(orden);
        break;
    case 10:
        orden="latido";
        enviar(orden);
        break;
    case 11:
        orden="calibrateShutter=CALIBRATE";
        enviar(orden);
        break;
    case 12:
        orden="calibrateShutter=GET";
        rcv=enviar(orden);
        if (rcv.substr(0,2)=="OK"){
            if (parametros(rcv,1)=="NINGUNO" && parametros(rcv,2)=="1"){
                tiempo_abrir=parametros(rcv,3);
                tiempo_cerrar=parametros(rcv,4);
            } else {
                return 12;
            }
        }
        break;
    case 13:
        orden="calibrateShutter=PUT:"+tiempo_abrir+":"+tiempo_cerrar;
        enviar(orden);
        break;
    case 14:
        orden="luz=ON";
        enviar(orden);
        break;
    case 15:
        orden="luz=OFF";
        enviar(orden);
        break;
    case 16:
        orden="domeAtHome=ON";
        enviar(orden);
        break;
    case 17:
        orden="domeAtHome=OFF";
        enviar(orden);
        break;
    case 18:
        orden="getRelays";
        enviar(orden);
        break;
    case 19:
        orden="getButtons";
        enviar(orden);
        break;
    case 20:
        orden="getInputs";
        enviar(orden);
        break;
    case 21:
        orden="getStatus";
        enviar(orden);
        break;
    default:
        return 0;
        break;
    }

    num_test+=1;
    return num_test;
}

string enviar(string orden){
    string respuesta;
    p_puerto_serie->Write( orden );
    try {
        respuesta=p_puerto_serie->ReadLine(1000U * 60U); //Timeout 1 minuto
        LOG4CXX_DEBUG (logger,"Recibiendo: " + respuesta);
    }
    catch (SerialPort::ReadTimeout E) {
        LOG4CXX_FATAL (logger,"TIMEOUT esperando datos por bluetooth");
        return "";
    }

    LOG4CXX_DEBUG (logger,orden + "-->" + respuesta);
    return respuesta;
}

string parametros(string respuesta,int posicion){
    string datos;
    if (posicion==1 && respuesta.find("=") != string::npos){
        datos=respuesta.substr(respuesta.find("=")+1);
    } else {
        datos=respuesta;
    }

    vector<string> dato;
    size_t pos = 0;
    dato.push_back(datos);
    for (int i=0;(pos = datos.find(":")) != string::npos;i++) {
        dato[i]=datos.substr(0, pos);
        datos.erase(0, pos + 1);
        dato.push_back(datos);
    }


    return dato[posicion-1];
}