
#include <iostream>
#include "CConfig.h"

void CConfig::load(const string &filename,const string &modo)
{
    // Create empty property tree object
    pt::ptree tree;

    // Parse the XML into the property tree.
	try {
    	pt::read_xml(filename, tree);
	} catch (const exception &e) {
		cerr << "No se ha podido abrir el fichero de parámetros: "<< filename << endl;
		exit(EXIT_FAILURE);
	}

	string s_log_nivel;
    // Use the throwing version of get to find the debug filename.
    // If the path cannot be resolved, an exception is thrown.
	try {
        general_simulacion = tree.get<int>("comun.general.simulacion");

        gpio_pin_led_reset = tree.get<int>("comun.gpio.pin_led_reset");
        gpio_pin_led_ccw = tree.get<int>("comun.gpio.pin_led_ccw");
        gpio_pin_led_cw = tree.get<int>("comun.gpio.pin_led_cw");
        gpio_pin_led_dah = tree.get<int>("comun.gpio.pin_led_dah");
        gpio_pin_led_encoder = tree.get<int>("comun.gpio.pin_led_encoder");
        gpio_pin_key_reset = tree.get<int>("comun.gpio.pin_key_reset");
        gpio_pin_key_cw = tree.get<int>("comun.gpio.pin_key_cw");
        gpio_pin_key_ccw = tree.get<int>("comun.gpio.pin_key_ccw");
        gpio_pin_act_cw = tree.get<int>("comun.gpio.pin_act_cw");
        gpio_pin_act_ccw = tree.get<int>("comun.gpio.pin_act_ccw");
        gpio_pin_act_on_movil = tree.get<int>("comun.gpio.pin_act_on_movil");
        gpio_pin_inp_dah = tree.get<int>("comun.gpio.pin_inp_dah");
        gpio_pin_inp_encoder = tree.get<int>("comun.gpio.pin_inp_encoder");
        
        pigpio_server = tree.get<string>("comun.pigpio.server");
        pigpio_port = tree.get<string>("comun.pigpio.port");
        
        serial_nombre = tree.get<string>("comun.serial.nombre");
        
        log_fichero = tree.get<string>("comun.log.fichero");
        s_log_nivel = tree.get<string>("comun.log.nivel", "INFO");
	} catch (const exception &e) {
		cerr << "En el fichero de parámetros falta un parámetro obligatorio: "<< e.what() << endl;
		exit(EXIT_FAILURE);
	}

    string s;double d;int i;
    i = tree.get<int>(modo + ".general.simulacion",-1);
    if (i!=-1) general_simulacion=i;

    i = tree.get<int>(modo + ".gpio.pin_led_reset",-1);
    if (i!=-1) gpio_pin_led_reset=i;
    i = tree.get<int>(modo + ".gpio.pin_led_ccw",-1);
    if (i!=-1) gpio_pin_led_ccw=i;
    i = tree.get<int>(modo + ".gpio.pin_led_cw",-1);
    if (i!=-1) gpio_pin_led_cw=i;
    i = tree.get<int>(modo + ".gpio.pin_led_dah",-1);
    if (i!=-1) gpio_pin_led_dah=i;
    i =  tree.get<int>(modo + ".gpio.pin_led_encoder",-1);
    if (i!=-1) gpio_pin_led_encoder=i;
    i = tree.get<int>(modo + ".gpio.pin_key_reset",-1);
    if (i!=-1) gpio_pin_key_reset=i;
    i = tree.get<int>(modo + ".gpio.pin_key_cw",-1);
    if (i!=-1) gpio_pin_key_cw=i;
    i = tree.get<int>(modo + ".gpio.pin_key_ccw",-1);
    if (i!=-1) gpio_pin_key_ccw=i;
    i = tree.get<int>(modo + ".gpio.pin_act_cw",-1);
    if (i!=-1) gpio_pin_act_cw=i;
    i = tree.get<int>(modo + ".gpio.pin_act_ccw",-1);
    if (i!=-1) gpio_pin_act_ccw=i;
    i = tree.get<int>(modo + ".gpio.pin_act_on_movil",-1);
    if (i!=-1) gpio_pin_act_on_movil=i;
    i = tree.get<int>(modo + ".gpio.pin_inp_dah",-1);
    if (i!=-1) gpio_pin_inp_dah=i;
    i = tree.get<int>(modo + ".gpio.pin_inp_encoder",-1);
    if (i!=-1) gpio_pin_inp_encoder=i;

    s = tree.get<string>(modo + ".pigpio.server","");
    if (s!="") pigpio_server=s;
    s = tree.get<string>(modo + ".pigpio.port","");
    if (s!="") pigpio_port=s;

    s = tree.get<string>(modo + ".serial.nombre","");
    if (s!="") serial_nombre=s;

    s = tree.get<string>(modo + ".log.fichero", "");
    if (s!="") log_fichero=s;
    s = tree.get<string>(modo + ".log.nivel", "");
    if (s!="") s_log_nivel=s;
    
	if (s_log_nivel=="OFF")	log_nivel=log4cxx::Level::getOff();
	else if (s_log_nivel=="FATAL") log_nivel=log4cxx::Level::getFatal();
	else if (s_log_nivel=="ERROR") log_nivel=log4cxx::Level::getError();
	else if (s_log_nivel=="WARN") log_nivel=log4cxx::Level::getWarn();
	else if (s_log_nivel=="INFO") log_nivel=log4cxx::Level::getInfo();
	else if (s_log_nivel=="DEBUG") log_nivel=log4cxx::Level::getDebug();
	else if (s_log_nivel=="TRACE") log_nivel=log4cxx::Level::getTrace();
	else if (s_log_nivel=="ALL") log_nivel=log4cxx::Level::getAll();
	else log_nivel=log4cxx::Level::getInfo();

}
