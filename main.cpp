#include <iostream>

#include "transaq_client.h"

int main(int argc, char **argv)
{
    setlocale(LC_CTYPE, "");

    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " host port dll <logfile>" << std::endl;
        return 1;
    }

    try
    {
        std::string host    = argv[1];
        std::string port    = argv[2];
        std::string libpath = argv[3];
        std::string logfile = (argc < 5) ? "" : argv[4];
        transaq::client client(host, port, libpath, logfile);
        client.start();
    }
    catch( std::exception & ex )
    {
        std::cerr << "Exitting with exception: " << ex.what() << std::endl;
    }
    return 0;
}
