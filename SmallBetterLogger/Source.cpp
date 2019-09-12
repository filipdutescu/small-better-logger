#include "SmallBetterLogger.hpp"

int main()
{
	sbl::logger l("log.out");

	l.Write("Test");

	std::cin.get();
}