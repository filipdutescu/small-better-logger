# Small, Better Logger
A small, better logger for C++ (and any project that wishes to use it as .dll) 

#### Observation
From now on, I will refer to this library as **SBLogger**.

## Setup
All you need to do if you wish to use **SBLogger** (in a C++ project) is to download the ```SmallBetterLogger.hpp``` file to your project and added as a header file in your code:
````cpp
...
#include "SmallBetterLogger.hpp"
...
````
For other projects, you need to compile it to either a ```.lib``` or a ```.dll``` library and link it in the compiler specific way of your prefered language.

## Usage

All the code which is related to the **SBL** is located in the ```sblogger``` namespace. The loggers are of 2 types: ```sblogger::Logger``` (which writes to the STD streams), ```sblogger::FileLogger``` (which writes to a file). There is also an enum, ```STREAM_TYPES```. All can also be written with lowercase letters (i.e.: ```sblogger::logger```, ```sblogger::stream_types```).

# WIP
