#pragma once

#include <string>
#include "log4cxx/logger.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace std;
namespace pt = boost::property_tree;

class CConfig
{
  public:
    //CConfig();
    //~CConfig() = default;
    void load(const string &filename,const string &modo);

    //General
    int general_simulacion;

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


    //Log
    string log_fichero;
    log4cxx::LevelPtr log_nivel;

  protected:

  private:
};