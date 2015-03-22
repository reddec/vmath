vmath
=====

Vector math with OpenMP support

Install
=======

Use CMake:

    $# cmake
    $# make

Build requirements:

-   c++ 11 compiler
-   OpenMP
-   CMake
-   Make
-   dpkg

Will be created .deb package with

-   `/usr/local/include/vmath.h` - base operations
-   Parallel binary operation for each element in vector with
    1.  Single value
    2.  Single other vector
    3.  Multiple others vectors (variadic templates)
-   `/usr/local/include/comath.h` - specific functions like Strike-Slip
    function
-   `/usr/local/bin/comath` - command-line utility

Command line help:

    USAGE:

    comath [-t] [-v] [-m] [-x] [-s] [-n] [-c <int>] [-a] [--] [--version]
    [-h]

    Where:

    -t, --total Sum/Min/Max/Avg

    -v, --avg Average of numbers

    -m, --min Min of numbers

    -x, --max Max of numbers

    -s, --sum Sum of numbers

    -n, --normalize Normalize (/max)

    -c <int>, --column <int> Column number

    -a, --alter-djonson Calculate Alter-Djonson

    --, --ignore_rest Ignores the rest of the labeled arguments following
    this flag.

    --version Displays version information and exits.

    -h, --help Displays usage information and exits.
