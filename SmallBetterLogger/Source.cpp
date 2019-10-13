#include "SmallBetterLogger.hpp"

int main()
{
	std::string tmp = "Hello {0}";

	// Declarations (Logger)
	sblogger::StreamLogger l;
	sblogger::stream_logger logErr(sblogger::STREAM_TYPE::STDERR, "[Error]");
	// Making use of the abstract class
	sblogger::logger* logLog = new sblogger::StreamLogger(sblogger::STREAM_TYPE::STDLOG, "[Log]");


	// Basic calls (Logger)
	l.WriteLine(tmp, "World");
	l.Write("I am {0} and {1} years old.{2} {0}", "Michael", 28);
	l.Write("{0}", "\n", "hey");

	logErr.WriteLine("logErr - {0}", "stderr");
	logLog->WriteLine("logLog - {0}", "stdlog");

	{
		// Declarations (FileLogger)
		sblogger::FileLogger fileLogger("example.log", "[File Log]");

		// Basic calls (FileLogger)
		fileLogger.ClearLogs();
		fileLogger.WriteLine("This is a test.");
		fileLogger.Write("Hello World!");
	}

	delete logLog;
	std::cin.get();
	return 0;
}