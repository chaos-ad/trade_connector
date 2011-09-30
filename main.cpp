#include <iostream>

#include "transaq_client.h"

int main(int argc, char **argv)
{
    setlocale(LC_CTYPE, "");

    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " host port dll" << std::endl;
        return 1;
    }

    try
    {
        transaq::client client(argv[1], argv[2], argv[3]);
        client.start();
    }
    catch( std::exception & ex )
    {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }
    return 0;
}
