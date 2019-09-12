# Small, Better Logger
A small, better logger for C++ (and any project that wishes to use it as .dll) 

#### Observation
From now on, I will refer to this library as **SBL**.

## Setup
All you need to do if you wish to use **SBL** (in a C++ project) is to download the ```SmallBetterLogger.hpp``` file to your project and added as a header file in your code:
````cpp
...
#include "SmallBetterLogger.hpp"
...
````
For other projects, you need to compile it to either a ```.lib``` or a ```.dll``` library and link it in the compiler specific way of your prefered language.

## Usage

All the code which is related to the **SBL** is located in the ```sbl``` namespace. The main class is called ```sbl::Logger``` and there also is an enum, ```STREAM_TYPES```. Both can also be written with lowercase letters (i.e.: ```sbl::logger```, ```sbl::stream_types```).

# WIP
