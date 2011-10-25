#include <iostream>

#include "transaq_client.h"

int main(int argc, char **argv)
{
    setlocale(LC_CTYPE, "");

    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " host port dll keyfile certfile <logfile>" << std::endl;
        return 1;
    }

    try
    {
        std::string host     = argv[1];
        std::string port     = argv[2];
        std::string libpath  = argv[3];
        std::string keyfile  = argv[4];
        std::string certfile = argv[5];
        std::string logfile  = (argc < 7) ? "" : argv[6];
        transaq::client client(host, port, libpath, keyfile, certfile, logfile);
        client.start();
    }
    catch( std::exception & ex )
    {
        std::cerr << "Exitting with exception: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
