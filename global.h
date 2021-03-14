#pragma once
#include <string>
#include <unordered_map>
#include "log4cxx/logger.h"
#include <boost/property_tree/ptree.hpp>

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

struct triestado
{
    int estado; //0=OFF, 1=ON, -1=ERROR
    string mensaje;
};

using namespace std;
namespace pt = boost::property_tree;
