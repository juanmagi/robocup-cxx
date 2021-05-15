#pragma once
#include <string>
#include <unordered_map>
#include "log4cxx/logger.h"
#include <boost/property_tree/ptree.hpp>

//Versión
#define FIRMWARE_VERSION "250421"

typedef std::unordered_map<std::string, std::string> stringmap;
enum sentidoMovimiento
{
    PARADO,
    CW,
    CCW,
    CORTA
};

enum tipoPosicion
{
    absoluta,
    relativa,
    dah,
    manual,
    vuelta
};

enum tipoThread
{
    todos = 96,
    stop = 97,
    encoder = 98,
    encoderSimulator = 99
};

enum tipoTriestado {OFF=0,ON=1,ERROR=-1};
struct triestado
{
    tipoTriestado estado; //0=OFF, 1=ON, -1=ERROR
    string mensaje;
};

enum estadosCalibrado {
    NO_CALIBRADO_FISICO,
    NO_CALIBRADO_LOGICO,
    MOVIENDO_A_DAH_PLUS,
    EN_DAH_PLUS,
    MOVIENDO_A_DAH,
    EN_DAH,
    CALIBRADO
};

using namespace std;
namespace pt = boost::property_tree;
