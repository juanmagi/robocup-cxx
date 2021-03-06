#pragma once
#include <string.h>
#include "log4cxx/logger.h"
#include "CConfig.h"
#include "global.h"
#include "CEncoder.h"

class CEncoderReal: public CEncoder
{
private:


public:
    CEncoderReal(CConfig *pParametros,int pi_ID);
    ~CEncoderReal();

    void run() override;
};
