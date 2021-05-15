#pragma once

#include <string>
#include <unordered_map>
#include "modo.h"
#include "log4cxx/logger.h"
#include "log4cxx/logger.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace std;
namespace pt = boost::property_tree;

class CConfig
{
public:
  struct comando
  {
    string orden;
    unsigned int num_param;
    vector<vector<string>> parametros;
  };

protected:
  unordered_map<string, comando> comandos;
  pt::ptree tree;

public:
  CConfig();
  ~CConfig();
  void load();
  void put_cupula_max_posiciones(int valor);
  void save();
  bool getComando(string orden,comando& c);

  //General
  int general_simulacion;

  //Websocket
  string websocket_local_ip;
  string websocket_port;

  //gpio
  int gpio_pin_led_reset;
  int gpio_pin_led_ccw;
  int gpio_pin_led_cw;
  int gpio_pin_led_dah;
  int gpio_pin_led_encoder;
  int gpio_pin_key_reset;
  int gpio_pin_key_cw;
  int gpio_pin_key_ccw;
  int gpio_pin_act_cw;
  int gpio_pin_act_ccw;
  int gpio_pin_act_on_movil;
  int gpio_pin_inp_dah;
  int gpio_pin_inp_encoder;

  //pigpio
  string pigpio_server;
  string pigpio_port;

  //puerto serie
  string serial_nombre;

  //Log
  string log_fichero;
  log4cxx::LevelPtr log_nivel;

  //Cupula
  int cupula_max_posiciones;
  int cupula_max_posiciones_simulacion;
  useconds_t cupula_periodo_simulacion;
  int cupula_longitud_onda_simulacion;
  useconds_t cupula_tiempo_entre_lecturas;

protected:
  void loadComandos();

private:
};