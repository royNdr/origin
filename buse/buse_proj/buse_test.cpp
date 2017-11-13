/*
 *
 *
 *
 *
 */

#include <iostream>
#include "buse.hpp"

using namespace ilrd;

void testBuse();

int main()
{
	testBuse();

	return 0;
}

void testBuse()
{
	std::cout << " ------- START TESTING --------- " << std::endl;

	Buse b("/dev/nbd0", "fs_file");
	b.Start();

	std::cout << " -------- END TESTING ---------- " << std::endl;
}
