#include "CCupulaMovil.h"

using namespace std;

CCupulaMovil::CCupulaMovil(CConfig *pParametros)
{
    pParam=pParametros;
    pLogger=log4cxx::Logger::getRootLogger();
}

CCupulaMovil::~CCupulaMovil()
{
    if (p_puerto_serie != nullptr){
            p_puerto_serie->Close();
    		LOG4CXX_DEBUG (pLogger,"Puerto serie cerrado");
    }
}

int CCupulaMovil::conectar(){
    SerialPort puerto_serie(pParam->serial_nombre);
    p_puerto_serie=&puerto_serie;

    try {
        puerto_serie.Open( SerialPort::BAUD_9600 );
    }
    catch (SerialPort::OpenFailed E) {
		LOG4CXX_FATAL (pLogger,"Error establecido la conexión BLUETOOTH. Asegurar que el device está creado");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

std::string CCupulaMovil::comunicar(std::string mensaje){
    string respuesta;
    p_puerto_serie->Write( mensaje );
    try {
        respuesta=p_puerto_serie->ReadLine(1000U * 60U); //Timeout 1 minuto
    }
    catch (SerialPort::ReadTimeout E) {
        LOG4CXX_FATAL (pLogger,"TIMEOUT esperando datos por bluetooth");
        return "KO=La parte móvil no responde";
    }

    respuesta.erase(respuesta.find("\r\n"),2);
    LOG4CXX_DEBUG (pLogger,mensaje + "-->" + respuesta);
    return respuesta;
}

std::string CCupulaMovil::parametros(std::string mensaje,unsigned int posicion){
    string datos;
    
    if (posicion==1 && mensaje.find("=") != string::npos){
        datos=mensaje.substr(mensaje.find("=")+1);
    } else {
        datos=mensaje;
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

triestado CCupulaMovil::estadoLuz(){
    triestado salida;
    string respuesta=comunicar("getRelays");
    if (respuesta.substr(0,2)=="OK"){
        string p1=parametros(respuesta,1);
        if (p1=="1")
            salida.estado=1;
        else 
            salida.estado=0;
        return salida;
    } else {
        salida.estado=-1;
        salida.mensaje=respuesta;
        return salida;
    }
}

void CCupulaMovil::Luz(bool bEncender){
    string mensaje;
    if (bEncender)
        mensaje="luz=ON";
    else
        mensaje="luz=OFF";
    string respuesta=comunicar(mensaje);
    if (respuesta.substr(0,2)!="OK"){
        LOG4CXX_INFO (pLogger,"Error en la función Luz: " + respuesta);
    }
}

