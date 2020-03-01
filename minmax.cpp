/**
 * Improved min() and max() that can handle references, rvalue-references, and mixed integral types.
 * Unit tests.
 * @maintainer xavier dot roche at algolia.com
 */

#include "minmax.h"
#include <cassert>
#include <cstdlib>
#include <iostream>

using namespace enhancedminmax;

int main(int argc, char** argv)
{
    if (argc != 4) {
        return EXIT_FAILURE;
    }
    int a = atoi(argv[1]);
    int b = atoi(argv[2]);
    int c = atoi(argv[3]);

    std::cout << "a==" << a << ", b ==" << b << ", c == " << c << std::endl;
    for (std::size_t i = 0; i < 10; i++) {
        std::cout << "value==" << ++min(a, b, c) << " - a==" << a << ", b ==" << b << ", c == " << c << std::endl;
    }

    return EXIT_SUCCESS;
}
