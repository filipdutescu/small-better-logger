#include "SmallBetterLogger.hpp"

int main()
{
	std::string tmp = "Hello {0}";

	sbl::Logger l;

	l.WriteLine(tmp, "World");
	l.Write("I am {0} and {1} years old.{2} {0}", "Filip", 20);
	l.Write("{0}", "\n", "hey");

	std::cerr << "Test\n\n";
	std::clog << "Test\n";

	sbl::logger logErr(sbl::STREAM_TYPE::STDERR);
	logErr.WriteLine("logErr - {0}", "stderr");
	sbl::logger logLog(sbl::STREAM_TYPE::STDLOG);
	logLog.WriteLine("logLog - {0}", "stdlog");

	std::cin.get();
}