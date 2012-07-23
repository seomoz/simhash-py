#include <iostream>

#include "hash.hpp"

int main(int argc, char* argv[]) {
    /* If we are provided with command line arguments, we should assume that 
     * each of them is a file to be read */
    if (argc > 1) {
        std::cout << "Going to read some files" << std::endl;
    } else {
        /* We should read from stdin and then print out the hash */
        std::cout << "Going to read from stdin" << std::endl;
    }

    return 0;
}
