#include <string>
#include "CPruebaPython.h"

using namespace std;

CPruebaPython::CPruebaPython(){
    
}
int CPruebaPython::getDato(){
    return m_iDato;
}

void CPruebaPython::setDato(int dato){
    m_iDato=dato;
}

string CPruebaPython::getSDato(){
    return m_strDato;
}

void CPruebaPython::setSDato(const string strDato){
    m_strDato=strDato;
}

int CPruebaPython::datoAlcuadrado(){
    return m_iDato*m_iDato;
}

