#include <iostream>
#include "CConfig.h"
CConfig::CConfig()
{
}

CConfig::~CConfig()
{
}

void CConfig::load()
{
    const string filename = FICHERO_PARAMETROS;
    const string modo = MODO;

    // Parse the XML into the property tree.
    try
    {
        pt::read_xml(filename, tree);
    }
    catch (const exception &e)
    {
        cerr << "No se ha podido abrir el fichero de parámetros: " << filename << endl;
        exit(EXIT_FAILURE);
    }

    string s_log_nivel;
    // Use the throwing version of get to find the debug filename.
    // If the path cannot be resolved, an exception is thrown.
    try
    {
        general_simulacion = tree.get<int>("comun.general.simulacion");

        websocket_local_ip = tree.get<string>("comun.websocket.local_ip");
        websocket_port = tree.get<string>("comun.websocket.port");

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

        cupula_max_posiciones = tree.get<int>("comun.cupula.max_posiciones");
        cupula_max_posiciones_simulacion = tree.get<int>("comun.cupula.max_posiciones_simulacion");
        cupula_periodo_simulacion = tree.get<useconds_t>("comun.cupula.periodo_simulacion");
        cupula_longitud_onda_simulacion = tree.get<int>("comun.cupula.longitud_onda_simulacion");
        cupula_tiempo_entre_lecturas = tree.get<useconds_t>("comun.cupula.tiempo_entre_lecturas");
    }
    catch (const exception &e)
    {
        cerr << "En el fichero de parámetros falta un parámetro obligatorio: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }

    string s;
    double d;
    int i;
    useconds_t u;
    i = tree.get<int>(modo + ".general.simulacion", -1);
    if (i != -1)
        general_simulacion = i;

    s = tree.get<string>(modo + ".websocket.local_ip", "");
    if (s != "")
        websocket_local_ip = s;
    s = tree.get<string>(modo + ".websocket.port", "");
    if (s != "")
        websocket_port = s;

    i = tree.get<int>(modo + ".gpio.pin_led_reset", -1);
    if (i != -1)
        gpio_pin_led_reset = i;
    i = tree.get<int>(modo + ".gpio.pin_led_ccw", -1);
    if (i != -1)
        gpio_pin_led_ccw = i;
    i = tree.get<int>(modo + ".gpio.pin_led_cw", -1);
    if (i != -1)
        gpio_pin_led_cw = i;
    i = tree.get<int>(modo + ".gpio.pin_led_dah", -1);
    if (i != -1)
        gpio_pin_led_dah = i;
    i = tree.get<int>(modo + ".gpio.pin_led_encoder", -1);
    if (i != -1)
        gpio_pin_led_encoder = i;
    i = tree.get<int>(modo + ".gpio.pin_key_reset", -1);
    if (i != -1)
        gpio_pin_key_reset = i;
    i = tree.get<int>(modo + ".gpio.pin_key_cw", -1);
    if (i != -1)
        gpio_pin_key_cw = i;
    i = tree.get<int>(modo + ".gpio.pin_key_ccw", -1);
    if (i != -1)
        gpio_pin_key_ccw = i;
    i = tree.get<int>(modo + ".gpio.pin_act_cw", -1);
    if (i != -1)
        gpio_pin_act_cw = i;
    i = tree.get<int>(modo + ".gpio.pin_act_ccw", -1);
    if (i != -1)
        gpio_pin_act_ccw = i;
    i = tree.get<int>(modo + ".gpio.pin_act_on_movil", -1);
    if (i != -1)
        gpio_pin_act_on_movil = i;
    i = tree.get<int>(modo + ".gpio.pin_inp_dah", -1);
    if (i != -1)
        gpio_pin_inp_dah = i;
    i = tree.get<int>(modo + ".gpio.pin_inp_encoder", -1);
    if (i != -1)
        gpio_pin_inp_encoder = i;

    s = tree.get<string>(modo + ".pigpio.server", "");
    if (s != "")
        pigpio_server = s;
    s = tree.get<string>(modo + ".pigpio.port", "");
    if (s != "")
        pigpio_port = s;

    s = tree.get<string>(modo + ".serial.nombre", "");
    if (s != "")
        serial_nombre = s;

    s = tree.get<string>(modo + ".log.fichero", "");
    if (s != "")
        log_fichero = s;
    s = tree.get<string>(modo + ".log.nivel", "");
    if (s != "")
        s_log_nivel = s;

    i = tree.get<int>(modo + ".cupula.max_posiciones", -2);
    if (i != -2)
        cupula_max_posiciones = i;
    i = tree.get<int>(modo + ".cupula.max_posiciones_simulacion", -1);
    if (i != -1)
        cupula_max_posiciones_simulacion = i;
    u = tree.get<useconds_t>(modo + ".cupula.periodo_simulacion", 0);
    if (u != 0)
        cupula_periodo_simulacion = u;
    i = tree.get<int>(modo + ".cupula.longitud_onda_simulacion", -1);
    if (i != -1)
        cupula_longitud_onda_simulacion = i;
    u = tree.get<useconds_t>(modo + ".cupula.tiempo_entre_lecturas", 0);
    if (u != 0)
        cupula_tiempo_entre_lecturas = u;

    if (s_log_nivel == "OFF")
        log_nivel = log4cxx::Level::getOff();
    else if (s_log_nivel == "FATAL")
        log_nivel = log4cxx::Level::getFatal();
    else if (s_log_nivel == "ERROR")
        log_nivel = log4cxx::Level::getError();
    else if (s_log_nivel == "WARN")
        log_nivel = log4cxx::Level::getWarn();
    else if (s_log_nivel == "INFO")
        log_nivel = log4cxx::Level::getInfo();
    else if (s_log_nivel == "DEBUG")
        log_nivel = log4cxx::Level::getDebug();
    else if (s_log_nivel == "TRACE")
        log_nivel = log4cxx::Level::getTrace();
    else if (s_log_nivel == "ALL")
        log_nivel = log4cxx::Level::getAll();
    else
        log_nivel = log4cxx::Level::getInfo();

    loadComandos();
}

void CConfig::put_cupula_max_posiciones(int valor)
{
    if (MODO == "desarrollo")
        tree.put("desarrollo.cupula.max_posiciones", valor);
    else
        tree.put("produccion.cupula.max_posiciones", valor);

    cupula_max_posiciones = valor;
    save();
}

void CConfig::save()
{
    const string filename = FICHERO_PARAMETROS;
    pt::write_xml(filename, tree);
}

void CConfig::loadComandos()
{
    comando c;
    c.orden = "salir";
    c.num_param = 0;
    vector<string> vs;
    vector<vector<string>> vvs;
    c.parametros = vvs;
    c.parametros.push_back(vs);
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "getFirmwareVersion";
    c.num_param = 1;
    c.parametros[0].push_back("fija");
    c.parametros[0].push_back("fija");
    c.parametros[0].push_back("movil");
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "getDomeAtHome";
    c.num_param = 0;
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "setDomeAtHome";
    c.num_param = 0;
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "getPosicion";
    c.num_param = 0;
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "setPosicion";
    c.num_param = 3;
    c.parametros[0].push_back("CORTA"); //Sentido del movimiento
    c.parametros[0].push_back("CORTA");
    c.parametros[0].push_back("CW");
    c.parametros[0].push_back("CCW");
    c.parametros[1].push_back("absoluta");  //Tipo de posición
    c.parametros[1].push_back("absoluta");
    c.parametros[1].push_back("relativa");
    c.parametros[2].push_back("_int_"); //Grados posición final (absoluta) o +/-grados de movimiento (relativa)
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "calibrar";
    c.num_param = 2;
    c.parametros[0].push_back("no"); //Forzar recalibrado
    c.parametros[0].push_back("no");
    c.parametros[0].push_back("si");
    c.parametros[1].push_back("si"); //Mover a DAH
    c.parametros[1].push_back("si");
    c.parametros[1].push_back("no");
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "estadoCalibrado";
    c.num_param = 0;
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "mover";
    c.num_param = 1;
    c.parametros[0].push_back("CW");
    c.parametros[0].push_back("CW");
    c.parametros[0].push_back("CCW");
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "parar";
    c.num_param = 0;
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "conectarMovil";
    c.num_param = 0;
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "getLuz";
    c.num_param = 0;
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "setLuz";
    c.num_param = 1;
    c.parametros[0].push_back("OFF"); //ON enciende la luz, OFF la apaga
    c.parametros[0].push_back("ON");
    c.parametros[0].push_back("OFF");
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "setLog";
    c.num_param = 2;
    c.parametros[0].push_back("fija"); //Fija o Movil
    c.parametros[0].push_back("fija");
    c.parametros[0].push_back("movil");
    c.parametros[1].push_back("INFO"); //Nivel de log
    c.parametros[1].push_back("OFF");
    c.parametros[1].push_back("FATAL");
    c.parametros[1].push_back("ERROR");
    c.parametros[1].push_back("WARN");
    c.parametros[1].push_back("INFO");
    c.parametros[1].push_back("DEBUG");
    c.parametros[1].push_back("TRACE");
    c.parametros[1].push_back("ALL");
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "calibrarMovil";
    c.num_param = 3;
    c.parametros[0].push_back("CALIBRATE"); //acción: Calibrate para calibrar, Get para obtener datos de calibrado, Put para enviar adtos de calibrados anteriores
    c.parametros[0].push_back("CALIBRATE");
    c.parametros[0].push_back("GET");
    c.parametros[0].push_back("PUT");
    c.parametros[1].push_back("_ulong_"); //tiempo abrir (sólo si PUT)
    c.parametros[2].push_back("_ulong_"); //tiempo cerrar (sólo si PUT)
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "setEmergencyShutterTimeout";
    c.num_param = 1;
    c.parametros[0].push_back("_uint_"); //tiempo en segundos máximo para abrir/cerrar la ventana
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "moverShutter";
    c.num_param = 2;
    c.parametros[0].push_back("CLOSE"); //Acción para la ventana
    c.parametros[0].push_back("OPEN");
    c.parametros[0].push_back("CLOSE");
    c.parametros[1].push_back("_ulong_"); //tiempo en milisegndos máximo para abrir/cerrar la ventana exclusivo para este movimiento. OPCIONAL
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "getEstadoMovimientoShutter";
    c.num_param = 0;
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "pararShutter";
    c.num_param = 0;
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "paraShutter";
    c.num_param = 0;
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "getRelays";
    c.num_param = 0;
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "getButtons";
    c.num_param = 0;
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "getInputs";
    c.num_param = 0;
    comandos[c.orden] = c;

    for (auto it = c.parametros.begin(); it != c.parametros.end(); ++it)
        it->clear();
    c.parametros.clear();
    c.parametros.push_back(vs);
    c.orden = "getStatus";
    c.num_param = 0;
    comandos[c.orden] = c;
}

bool CConfig::getComando(string orden, comando &c)
{
    try
    {
        c = comandos.at(orden);
        return true;
    }
    catch (const std::out_of_range &e)
    {
        c = comandos["no_existe"];
        return false;
    }
}
