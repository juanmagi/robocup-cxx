#include "pigpiod_if2.h"
#include "CEncoderReal.h"

using namespace std;

CEncoderReal::CEncoderReal(CConfig *pParametros, int pi_ID) : CEncoder(pParametros, pi_ID)
{
    int errCode;
    if (errCode = set_mode(piID, pParam->gpio_pin_act_ccw, PI_OUTPUT) != 0)
    {
        LOG4CXX_FATAL(pLogger, "Error estableciendo las características del pin gpio_pin_act_ccw con código de error: " + to_string(errCode));
        return;
    }
    if (errCode = set_mode(piID, pParam->gpio_pin_act_cw, PI_OUTPUT) != 0)
    {
        LOG4CXX_FATAL(pLogger, "Error estableciendo las características del pin gpio_pin_act_cw con código de error: " + to_string(errCode));
        return;
    }
    if (errCode = set_mode(piID, pParam->gpio_pin_inp_dah, PI_INPUT) != 0)
    {
        LOG4CXX_FATAL(pLogger, "Error estableciendo las características del pin gpio_pin_inp_dah con código de error: " + to_string(errCode));
        return;
    }
    if (errCode = set_mode(piID, pParam->gpio_pin_inp_encoder, PI_INPUT) != 0)
    {
        LOG4CXX_FATAL(pLogger, "Error estableciendo las características del pin gpio_pin_inp_encoder con código de error: " + to_string(errCode));
        return;
    }

    if (errCode = gpio_write(piID, pParam->gpio_pin_act_ccw, PI_HIGH) != 0)
    {
        LOG4CXX_FATAL(pLogger, "Error estableciendo las características del pin gpio_pin_act_ccw con código de error: " + to_string(errCode));
        return;
    }
    if (errCode = gpio_write(piID, pParam->gpio_pin_act_cw, PI_HIGH) != 0)
    {
        LOG4CXX_FATAL(pLogger, "Error estableciendo las características del pin gpio_pin_act_cw con código de error: " + to_string(errCode));
        return;
    }

    if (errCode = set_pull_up_down(piID, pParam->gpio_pin_inp_dah, PI_PUD_UP) != 0)
    {
        LOG4CXX_FATAL(pLogger, "Error estableciendo las características del pin gpio_pin_inp_dah con código de error: " + to_string(errCode));
        return;
    }
    if (errCode = set_pull_up_down(piID, pParam->gpio_pin_inp_encoder, PI_PUD_UP) != 0)
    {
        LOG4CXX_FATAL(pLogger, "Error estableciendo las características del pin gpio_pin_inp_encoder con código de error: " + to_string(errCode));
        return;
    }
}

CEncoderReal::~CEncoderReal()
{
}

void CEncoderReal::run()
{
    int errCode;
    while (true)
    {
        if (sentido == sentidoMovimiento::PARADO)
        {
        }
        else if (sentido == sentidoMovimiento::CW)
        {
            if ((errCode = gpio_write(piID, pParam->gpio_pin_act_cw, PI_LOW)) != 0)
                LOG4CXX_ERROR(pLogger, "Error write gpio_pin_act_cw: " + to_string(errCode));
        }
    }
}
