#include <iostream>
#include <string>

#include <boost/asio.hpp>

#define MAXLEN 512 // maximum buffer size

int main()
{
    //
    boost::asio::io_service io;

    try
    {
        // create a serial port object
        boost::asio::serial_port serial(io);

        // open the platform specific device name
        // windows will be COM ports, linux will use /dev/ttyS* or /dev/ttyUSB*, etc
        serial.open("/dev/rfcomm0");
        serial.set_option(boost::asio::serial_port_base::baud_rate(9600));

        for (;;)
        {
            char data[MAXLEN];

            // read bytes from the serial port
            // asio::read will read bytes until the buffer is filled
            /*size_t nread = boost::asio::read(
                serial, boost::asio::buffer(data, 10)
            );
            
            std::string message(data, nread);

            std::cout << "Recieved: ";
            std::cout << message << std::endl;
            */

            // get a string from the user, sentiel is exit
            std::string input;
            std::cout << "Enter Message: ";
            std::cin >> input;

            if (input == "exit") break;

            // write to the port
            // asio::write guarantees that the entire buffer is written to the serial port
            boost::asio::write(serial, boost::asio::buffer(input));

            //char data[MAXLEN];

            // read bytes from the serial port
            // asio::read will read bytes until the buffer is filled
            boost::asio::streambuf b;
            size_t nread = boost::asio::read_until(
//                serial, boost::asio::buffer(data, MAXLEN),'\n'
                serial, b,'\n'
            );

            std::string message(data, nread);

            std::cout << "Recieved: ";
            std::cout << message << std::endl;
        }

        serial.close();
    }
    catch (boost::system::system_error& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}