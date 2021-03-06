#pragma once
#include <string>
#include <unordered_map>
#include "log4cxx/logger.h"
#include <boost/property_tree/ptree.hpp>

typedef std::unordered_map<std::string,std::string> stringmap;
enum sentidoMovimiento {PARADO,CW,CCW,CORTA};

struct triestado {
    int estado; //0=OFF, 1=ON, -1=ERROR
    string mensaje;
};

using namespace std;
namespace pt = boost::property_tree;
