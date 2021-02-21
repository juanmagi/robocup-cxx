#include <string>
#include <iostream>

using namespace std;
#include "CPruebaPython.h"
#include "CPyWP.h"

int main(int /*argc*/, char **/*argv*/)
{
    CPruebaPython pp;
    string strDato="Hola mundo";
    int i=13;
    pp.setSDato(strDato);
    pp.setDato(i);
    cout << "Frase: " << pp.getSDato() << " número: " << pp.getDato() << endl;

    CPyWP cliente;
    if (cliente.conectar("localhost","8080")==EXIT_SUCCESS){
        cout << "Conexión establecida" <<endl;
        string respuesta=cliente.enviar("Hola mundo");
        cout << "Respuesta1: " << respuesta << endl;
        respuesta=cliente.enviar("Hola mundo 2");
        cout << "Respuesta2: " << respuesta << endl;
        cliente.desconectar();
    } else
        cout << "Error al establecer la Conexión" << endl;
    
    
}
