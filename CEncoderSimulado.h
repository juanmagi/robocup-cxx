#pragma once
#include <string.h>
#include "log4cxx/logger.h"
#include "CConfig.h"
#include "global.h"
#include "CEncoder.h"

class CEncoderSimulado: public CEncoder
{
private:

public:
    CEncoderSimulado(CConfig *pParametros);
    ~CEncoderSimulado();

};
