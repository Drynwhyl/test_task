#include <iostream>
#include "../crashparser/crashparser.hpp"

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        try
        {
            crashparser::parse_file(argv[1], std::cout);
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

    return 0;
}