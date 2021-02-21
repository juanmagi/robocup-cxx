#pragma once

#include <string>
//using namespace std;

class CPruebaPython{
    public:
        CPruebaPython();
        ~CPruebaPython()=default;
        int getDato();
        void setDato(int dato);
        std::string getSDato();
        void setSDato(const std::string strDato);


    protected:
        int datoAlcuadrado();

    private:
        int m_iDato=0;
        std::string m_strDato;

};