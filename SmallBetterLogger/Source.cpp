#include "SmallBetterLogger.hpp"

int main()
{
	std::string tmp = "Hello {0}";

	// Declarations
	sbl::Logger l;
	sbl::logger logErr(sbl::STREAM_TYPE::STDERR);
	sbl::logger logLog(sbl::STREAM_TYPE::STDLOG);

	// Basic calls
	l.WriteLine(tmp, "World");
	l.Write("I am {0} and {1} years old.{2} {0}", "Michael", 28);
	l.Write("{0}", "\n", "hey");
	
	logErr.WriteLine("logErr - {0}", "stderr");
	logLog.WriteLine("logLog - {0}", "stdlog");

	std::cin.get();
	return 0;
}