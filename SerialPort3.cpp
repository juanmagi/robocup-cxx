#include <iostream>
#include <SerialPort.h>
#include <string.h>

// Create and open the serial port for communication.

int main(){
    SerialPort   my_serial_port( "/dev/rfcomm0" );
    my_serial_port.Open( SerialPort::BAUD_9600 );

    for(int i=0;i<10;i++){
        std::string input;

        input="Homa Mundo número " + std::to_string(i);
        std::cout << "Enviando: " << input <<std::endl;

        my_serial_port.Write( input );

        std::string strLeido=my_serial_port.ReadLine();
        std::cout << "Recibido: " << strLeido <<std::endl;
    }

    my_serial_port.Close();
}