# Small, Better Logger
A small, better logger for C++ (and any project that wishes to use it as .dll) 
> ***Note:*** *From now on, I will refer to this library as ***SBLogger***.*

## Getting Started
This section will provide the information needed to include **SBLogger** in your projects, either as source code or as a library (```.lib``` or ```.dll```).

### Including as Source Code
All you need to do if you wish to use **SBLogger** (in a C++ project) is to clone/fork the repo or download the ```SmallBetterLogger.hpp``` file to your project and added as a header file in your code:
````cpp
...
#include "SmallBetterLogger.hpp"
...
````

### Including as Library (WIP)
For other projects, you need to compile it to either a ```.lib``` or a ```.dll``` library and link it in the compiler specific way of your prefered language.

## Usage
All the code which is related to the **SBL** is located in the ```sblogger``` namespace. The loggers are of 2 types: 
  * ```sblogger::Logger``` (which writes to the standard streams)
  * ```sblogger::FileLogger``` (which writes to a file) 

There is also an enum, ```STREAM_TYPES``` which is useful when logging with ```sblogger::Logger```, in order to specify STDOUT, STDERR or STDLOG. 

> ***Note:*** *All those previously mentioned can also be written with lowercase letters (i.e.: ```sblogger::logger```, ```sblogger::stream_types```).*

Both loggers have the following methods for printing messages:
  * ```void Write(const std::string& message, const T& ...t)``` - write the message ```const std::string& message``` after replacing all placehodlers with the respective parameter value (ex.: ```"{0}"``` will be changed to the value of the first parameter after the string)
  * ```void WriteLine(const std::string& message, const T& ...t)``` - same as ```Write(...)```, but appends the newline character (system dependent, define system macros for proper support, check the [Cross-Platform Info](README.md#Cross-Platform-Info))
  * ```void Flush()``` - flushes the stream
```sblogger::FileLogger``` also contains an additional method:
  * ```void ClearLogs()``` - removes all content from the log file

For more information regarding usage, please refer to the [Wiki](Work in progress) *(WIP)*.
> ***Note:*** *You can find basic usage examples in the `[Source.cpp](SmallBetterLogger/Source.cpp)` file.*

## Cross-Platform Info
In order for **SBLogger** to work properly outside of **MS Windows**, you should define (at the begining of the file preferably) the following macros:
  * ```#define SBLOGGER_UNIX``` - for ***nix** systems and **Mac OS X+**
  * ```#define SBLOGGER_OS9``` - for **Mac OS 9** and lower
> ***Note:*** *There is no reason to define any macros for windows, as that is the default for this library.*

## License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) or [SmallBetterLogger.hpp](SmallBetterLogger/SmallBetterLogger.hpp) files for details.

## Acknowledgments
  * [@eugencutic](http://github.com/eugencutic) - Special thanks for code reviews and advice
