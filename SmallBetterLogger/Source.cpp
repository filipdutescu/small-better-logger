#include <iostream>
#include <string>

#define SBLOGGER_LOG_LEVEL SBLOGGER_LEVEL_TRACE
#include "SmallBetterLogger.hpp"

int main()
{
	// Declarations (Logger)
	sblogger::StreamLogger l;																		// Simplest way to make a logger
	sblogger::stream_logger logErr(sblogger::STREAM_TYPE::STDERR, "[%F %T][%^er]");					// Set stream type and custom format
	sblogger::logger* logLog = new sblogger::StreamLogger(sblogger::STREAM_TYPE::STDLOG, "[Log]");  // Making use of the abstract class

	// Declarations (FileLogger)
	sblogger::FileLogger fileLogger("example.log", "[File Log]");
	//sblogger::FileLogger fileLogger2("   .txt", "[File Log]");	// Will throw error since filename is empty

	// Basic calls (Logger)
	l.WriteLine("This is a normal log to STDOUT.");
	l.WriteLine();

	l.Indent();											// Add indent to stylize logs
	l.WriteLine(std::string("Hello, {0}!"), "World");	// Message to be writen can also be a variable of type std::string
	l.Dedent();											// Remove indent at any time from the logger

	l.Write("I am {0} and {1} years old.{2} {0}", "Michael", 28);	// If you give more placeholders than parameters, they are just writen as is
	l.Write("{0}", "\n", "hey");									// If you give more parameters, they are just ignored

	logErr.WriteLine("stderr");
	logLog->WriteLine("stdlog");

	// Basic calls (FileLogger)
	fileLogger.ClearLogs();						// This will clear the log file utilised by the FileLogger
	fileLogger.WriteLine("This is a test.");	// All methods previously shown can also be used with files
	fileLogger.Indent();
	fileLogger.Write("Hello World!");
	
	// ======================================= TODELETE =======================================
	sblogger::StreamLogger testingLogger("${yellow}[%F %T] $^{yellow}%msg%{yellow} [%^lvl]${reset}");

	testingLogger.WriteLine("Test");
	testingLogger.WriteLine("%2.3Test, test");
	testingLogger.WriteLine("%.5^er");
	testingLogger.WriteLine("t%{reset}%{red}e%{green}s%{blue}t%{magenta}i%{yellow}n%{cyan}g colors%{reset}");
	SBLOGGER_TRACE(testingLogger, "%ln");
	// ===================================== END TODELETE =====================================

	delete logLog;
	std::cin.get();

	return 0;
}