#include "SmallBetterLogger.hpp"

int main()
{
	std::string tmp = "Hello {0}";

	// Declarations (Logger)
	sblogger::Logger l;
	sblogger::logger logErr(sblogger::STREAM_TYPE::STDERR);
	sblogger::logger logLog(sblogger::STREAM_TYPE::STDLOG);


	// Basic calls (Logger)
	l.WriteLine(tmp, "World");
	l.Write("I am {0} and {1} years old.{2} {0}", "Michael", 28);
	l.Write("{0}", "\n", "hey");

	logErr.WriteLine("logErr - {0}", "stderr");
	logLog.WriteLine("logLog - {0}", "stdlog");

	{
		// Declarations (FileLogger)
		sblogger::FileLogger fileLogger("example.log");

		// Basic calls (FileLogger)
		fileLogger.WriteLine("This is a test.");
		fileLogger.Write("Hello World!");
	}

	std::cin.get();
	return 0;
}