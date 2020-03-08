# Small, Better Logger
A small, better logger for C++ 

## Getting Started
This section will provide the information needed to include **SBLogger** in your projects, either as source code or as a library.
> ***Note:*** *From now on, I will refer to this library as ***SBLogger***.*

### Prerequisites
This library was developed using **C++17** and thus you will have the best experience using a **C++17 (or later)** compiler. For compilers pre **C++17** (**C++11** or **C++14**) or post **C++17** (as of this writing, **C++20**), please refer to [Setting Up](README.md#Setting-Up).

### Including as Source Code
All you need to do if you wish to use **SBLogger** (in a C++ project) is to clone/fork the repo or download the [`SmallBetterLogger.hpp`](SmallBetterLogger/SmallBetterLogger.hpp) file to your project and add it as a header file in your code:
````cpp
...
#include "SmallBetterLogger.hpp"
...
````

***

### Setting Up

#### Using Older C++ Standards
The **SBLogger** library makes use of the predefined [```__cplusplus```](https://en.cppreference.com/w/cpp/preprocessor/replace#Predefined_macros) macro, which provides the compiler version, to auto-detect which code needs to change in order for it to be used in the project it is included in.

If you are using a pre **C++17** (**C++11** or **C++14**) compiler which does not support/properly implement the [```__cplusplus```](https://en.cppreference.com/w/cpp/preprocessor/replace#Predefined_macros) macro (such as **MSVC**), you should define the macro ```SBLOGGER_LEGACY```, in order to use this library. If you are using a pre **C++20** compiler that lacks support for the aforementioned [```__cplusplus```](https://en.cppreference.com/w/cpp/preprocessor/replace#Predefined_macros), you should define the ```SBLOGGER_OLD_DATES``` macro, in order to make use of this library.

Please define them either before including **SBLogger** or in the first line of the [`SmallBetterLogger.hpp`](SmallBetterLogger/SmallBetterLogger.hpp) file, as shown bellow.

````cpp
#define SBLOGGER_LEGACY     // Pre C++17 Compiler
#define SBLOGGER_OLD_DATES  // Pre C++20 Compiler
````
> ***Note:*** *For **MSVC** you can fix this problem, by setting up the **`/Zc:__cplusplus`** compiler option. If you do not know how to do this, please refer to the following [Microsoft guide](https://docs.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=vs-2019#to-set-this-compiler-option-in-visual-studio) for setting it up.*

***

#### Cross-Platform Info
In order for **SBLogger** to work properly outside of **MS Windows**, you should define (at the beginning of the file preferably) the following macros:
  * ```#define SBLOGGER_NIX``` - for **Unix/Linux** and **Mac OS X+**
  * ```#define SBLOGGER_OS9``` - for **Mac OS 9 and lower**

> ***Note:*** *There is no need to define any macros for ***Windows***, as that is the default for this library.*

#### Default Log Level
During compile time, it is possible to set a default log level for all of the loggers, by defining the **```SBLOGGER_LOG_LEVEL```** and giving it a value ranging between 0 and 6 (the meaning will be explained a bit further in this section). **By default**, the loggers are all given the level of **Trace**. All loggers contain the static method ```void sblogger::SetLoggingLevel(const sblogger::LOG_LEVELS& level)```, which can be set to update the logging level at runtime for all loggers (more about ```LOG_LEVELS``` further in this section).

To set the desired log level at compile time, either uncomment the line in the [`SmallBetterLogger.hpp`](SmallBetterLogger/SmallBetterLogger.hpp) file which defines the previously mentioned macro as ```SBLOGGER_LEVEL_TRACE```, or defined your own before including **SBLogger**:

````cpp
#define SBLOGGER_LOG_LEVEL SBLOGGER_LEVEL_TRACE // Example found in "SmallBetterLogger.hpp"
````
Or using a number rather than another macro:
````cpp
#define SBLOGGER_LOG_LEVEL 0    // Equivalent to the example above
````

The values that ```SBLOGGER_LOG_LEVEL``` macro can take values from 0 to 6, each of them having the following meanings, as defined in the [`SmallBetterLogger.hpp`](SmallBetterLogger/SmallBetterLogger.hpp) file:

````cpp
// Log Levels macros to be used with "SBLOGGER_LOG_LEVEL" macro for defining a default level
#define SBLOGGER_LEVEL_TRACE     0
#define SBLOGGER_LEVEL_DEBUG     1
#define SBLOGGER_LEVEL_INFO      2
#define SBLOGGER_LEVEL_WARN      3
#define SBLOGGER_LEVEL_ERROR     4
#define SBLOGGER_LEVEL_CRITICAL  5
#define SBLOGGER_LEVEL_OFF       6
````
> ***Note:*** *Should any other value be provided - concretely, any value lower than 0 or higher than 6, ***SBLogger*** will assume a default level of **```SBLOGGER_LEVEL_TRACE```**.*

To change the level of all logs (**for all loggers**, concretely: ```sblogger::StreamLogger```, ```sblogger::FileLogger``` or ```sblogger::DailyLogger```) during runtime, one can use the ```void sblogger::SetLoggingLevel(const sblogger::LOG_LEVELS& level)``` static method, accessible from all of the loggers. It takes as its parameter a value of the ```sblogger::LOG_LEVEL``` enum, which can be one of the following:

````cpp
// Log level enum. Contains all possible log levels, such as TRACE, ERROR, FATAL etc.
enum class LOG_LEVELS
{ 
  TRACE /*= 0*/, DEBUG /*= 1*/, INFO /*= 2*/, WARN /*= 3*/, ERROR /*= 4*/, CRITICAL /*= 5*/, OFF /*= 6*/ 
};
````

As you can see, they mirror the previously defined log level macros. With this, one can now set the logging level during runtime, as follows:

````cpp
...
sblogger::Logger::SetLoggingLevel(sblogger::LOG_LEVELS::WARN);  // Example of setting the level to WARN at runtime
...
````

The current logging level can also be found out at runtime, by using the ```const sblogger::LOG_LEVELS sblogger::GetLoggingLevel()``` static method, also found in all loggers:

````cpp
...
auto currentLevel = sblogger::Logger::GetLoggingLevel();  // Example of getting the current level
...
````

## Usage
All the code which is related to the **SBL** is located in the ```sblogger``` namespace. The loggers are of 3 types: 
  * **```sblogger::StreamLogger```** (which writes to the standard streams)
  * **```sblogger::FileLogger```** (which writes to a file) - which also has a specialized derivate,
    * **```sblogger::DailyLogger```** (which writes to a file that changes daily at the specified time) 

There is also an enum, ```sblogger::STREAM_TYPES``` which is useful when logging with ```sblogger::StreamLogger```, in order to specify STDOUT, STDERR or STDLOG. The library also defines its own custom errors, which help identify issues a user might run into:
  * ```sblogger::SBLoggerException``` - the default base error from **SBLogger**
  * ```sblogger::NullOrEmptyPathException``` - thrown when the given file path is null or empty
  * ```sblogger::NullOrWhitespaceNameException``` - thrown when the given file name is null or whitespace
  * ```sblogger::InvalidFilePathException``` - thrown when the specified file could not be opened
  * ```sblogger::TimeRangeException``` - thrown when a time related value is out of bounds (e.g.: hours not in [0, 23])

> ***Note:*** *All those previously mentioned can also be written with lowercase letters (i.e.: ```sblogger::stream_logger```, ```sblogger::stream_types```).*

### Logging messages
### Logger Methods
All loggers have the following general methods for printing and formatting messages (inherited from ```sblogger::Logger```):
  * ```void Write(...)``` - write the message ```const std::string& message``` after replacing all placeholders with the respective parameter value (ex.: ```"{0}"``` will be changed to the value of the first parameter after the string)
  * ```void WriteLine([...])``` - same as ```Write(...)```, but appends the newline character (system dependent, define system macros for proper support, check the [Cross-Platform Info](README.md#Cross-Platform-Info))
  * ```void Trace/Debug/Info/Warn/Error/Critical(...)``` - same as the ```Write(...)``` method previously mentioned, with the addition that output from the call of one of those methods will only appear if the log level (at the time of the call) is at most the same as the method's (i.e. a message written with ```Info(...)``` will only appear if the log level is set to **Info**)
  * ```int Indent()```/```int Dedent()``` - increase/decrease indent by 1
  * ```void Flush()``` - flushes the stream

> ***Note:*** *```sblogger::Write``` and ```sblogger::WriteLine``` methods use a logging level of **Trace**.*

> ***Note:*** *In order to set the logging level, you can do it either at compile or at run time. More information concerning them can be found either in the [Default Log Level](README.md#Default-Log-Level) section or in the [Wiki](https://github.com/filipdutescu/small-better-logger/wiki) (WIP).*

**```sblogger::StreamLogger```** contains an additional method:
  * ```void SetStreamType(STREAM_TYPE streamType)``` - change the current stream type to a different ```STREAM_TYPE```

**```sblogger::FileLogger```** also contains an additional method:
  * ```void ClearLogs()``` - removes all content from the log file

***

### Logger Predefined Macros
For all the aforementioned methods of logging there are also compile-time ways of outputting messages to streams. There are predefined macros for each method, with the exception of ```sblogger::Indent```, ```sblogger::Dedent``` and ```sblogger::Flush```.
  * ```SBLOGGER_WRITE(x, ...)``` - write the message ```const std::string& message``` after replacing all placeholders with the respective parameter value (ex.: ```"{0}"``` will be changed to the value of the first parameter after the string)
  * ```SBLOGGER_WRITELINE(x, ...)``` - same as ```Write(...)```, but appends the newline character (system dependent, define system macros for proper support, check the [Cross-Platform Info](README.md#Cross-Platform-Info))
  * ```void SBLOGGER_TRACE/SBLOGGER_DEBUG/SBLOGGER_INFO/SBLOGGER_WARN/SBLOGGER_ERROR/SBLOGGER_CRITICAL(x, ...)``` - same as the ```SBLOGGER_WRITE(x, ...)``` method previously mentioned, with the addition that output from the call of one of those methods will only appear if the log level (at the time of the call) is at most the same as the method's (i.e. a message written with ```SBLOGGER_INFO(x, ...)``` will only appear if the log level is set to **Info**)

> ***Note:*** *The ```x``` found in the macro parameters denotes the message that would be passed in the method calls, shown previously. In the case of the predefined macros it is mandatory to have it.*

> ***Note:*** *The predefined macros also expose placeholders for the ***file***, ***line*** and ***function*** information. Please check the [Wiki](https://github.com/filipdutescu/small-better-logger/wiki) *(WIP)* for more info related to placeholders.*

Another important aspect concerning this way of logging is that **those macros are available based on whether or not the ```SBLOGGER_LOG_LEVEL``` macro is defined** (which is defined and initialized by default with the ```SBLOGGER_LEVEL_TRACE``` value).

> ***Note:*** *For more information regarding available methods and macros, please refer to the [Wiki](https://github.com/filipdutescu/small-better-logger/wiki) *(WIP)*.*

***

### Placeholders
**SBLogger** provides a good amount of placeholders at your disposal, in order to allow easier creation and formatting of logs. They vary from date related placeholders (such as for the current date or time) to colour placeholders (which allow you to add colours for ```sblogger::StreamLogger``` generated output). Below you can find a short summary of the most relevant of those placeholders, with more detail to be found in the [Wiki](https://github.com/filipdutescu/small-better-logger/wiki) *(WIP)*:

| Placeholder     | Meaning                                                                      | Other                                                                                    |
|-----------------|------------------------------------------------------------------------------|------------------------------------------------------------------------------------------|
| **{i}**         | Replace with the value of the **i**th parameter from the method call         | If parameter **i** does not exist, the placeholder won't be replaced                     |
| **%[^]tr**      | Replace with the string ```"Trace"```                                        | Using ```^``` (```%^tr```) will use the string ```"TRACE"```                             |
| **%[^]dbg**     | Replace with the string ```"Debug"```                                        | Using ```^``` (```%^dbg```) will use the string ```"DEBUG"```                            |
| **%[^]inf**     | Replace with the string ```"Info"```                                         | Using ```^``` (```%^inf```) will use the string ```"INFO"```                             |
| **%[^]wn**      | Replace with the string ```"Warn"```                                         | Using ```^``` (```%^wn```) will use the string ```"WARN"```                              |
| **%[^]er**      | Replace with the string ```"Error"```                                        | Using ```^``` (```%^er```) will use the string ```"ERROR"```                             |
| **%[^]crt**     | Replace with the string ```"Critical"```                                     | Using ```^``` (```%^crt```) will use the string ```"CRITICAL"```                         |
| **%[^]lvl**     | Replace with the aforementioned string that corresponds to the current level | Using ```^``` (```%^lvl```) will use the uppercase string                                |
| **%src**        | Replace with the name of the current file                                    | Only works using the [predefined macros for logging](README.md#Logger-Predefined-Macros) |
| **%fsrc**       | Replace with the complete path and name of the current file                  | Only works using the [predefined macros for logging](README.md#Logger-Predefined-Macros) |
| **%ln**         | Replace with the current line of the file that the call is made in           | Only works using the [predefined macros for logging](README.md#Logger-Predefined-Macros) |
| **%fnc**        | Replace with the name of the current function (no return type params. etc.)  | Only works using the [predefined macros for logging](README.md#Logger-Predefined-Macros) |
| **Time & Date** | Same as those from [```strftime```](https://en.cppreference.com/w/cpp/chrono/c/strftime) or [```std::chrono::format```](https://en.cppreference.com/w/cpp/chrono/format) | N/A |

> ***Note:*** *You can find more about the available placeholders (those being only part of all of the placeholders) by going to the [Wiki](https://github.com/filipdutescu/small-better-logger/wiki) *(WIP)*.*

***

### Usage Examples
The quickest way to use **SBLogger** is to simply create an instance of it. Then just use the methods available for outputting your logs:
````cpp
...
#include "SmallBetterLogger.hpp"
...
int main()
{
  sblogger::StreamLogger logger;
  ...
  logger.WriteLine("Hello");
  logger.Write("{0}!", "World");
  ...
}
````
Output:
````console
Hello
World!
````

***

Or if you wanted to write to a file:
````cpp
...
#include "SmallBetterLogger.hpp"
...
int main()
{
  sblogger::FileLogger logger("mylog.log");
  ...
  logger.WriteLine("Hello");
  logger.Write("{0}!", "World");
  ...
}
````
Output:
````console
#In "mylog.log"
Hello
World!
````

***

If you wanted to stylize your logs a bit, a basic way to do this is to use a [format](http://github.com/filipdutescu/small-better-logger/wiki) *(WIP)* when instantiating a logger and/or use ```sblogger::Logger::Indent```/```sblogger::Logger::Dedent```, as follows:
````cpp
...
#include "SmallBetterLogger.hpp"
...
int main()
{
  sblogger::StreamLogger logger("[MyLogFormat]");
  ...
  logger.Indent();
  logger.WriteLine("Hello World,");
  logger.Dedent();
  logger.Write("This is my logger!");
  logger.WriteLine();
  ...
  logger.SetFormat("[%Y-%m-%d %H-%M-%S][%^er]");
  logger.Error("This is an error with a custom format.\n");
  ...
}
````
Output:
````console
    [MyLogFormat] Hello World,
[MyLogFormat] This is my logger!
[2019-11-07 13:37:00][ERROR] This is an error with a custom format.

````

> ***Note:*** *You can find more about the supported formats in the [Wiki](https://github.com/filipdutescu/small-better-logger/wiki) (WIP) and basic usage examples in the [`Source.cpp`](SmallBetterLogger/Source.cpp) file.*

## License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) or [`SmallBetterLogger.hpp`](SmallBetterLogger/SmallBetterLogger.hpp) files for details.

## Acknowledgments
  * [@eugencutic](http://github.com/eugencutic) - Special thanks for code reviews and suggestions
