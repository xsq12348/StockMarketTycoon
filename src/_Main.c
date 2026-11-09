#include "Project_All.h"
#include <string.h>
#include <time.h>
#include <Windows.h>

void main(int argc, char** argv)
{
	for (int i = 1; i < argc; ++i)
		if (argv[i] && strcmp(argv[i], "--play") == 0)
			g_autoPlay = 1;

	MoonRandom((unsigned int)time(MOON_NULL) ^ (unsigned int)GetTickCount(), 0, 1);

	ShowWindow(GetConsoleWindow(), SW_HIDE);
		MoonProjectInit("Stock Market Tycoon", 0, 40, 1800, 900, 60, _Resource);
	MoonProjectRun(SettingAll, StockLogic, StockDraw);
	MoonProjectOver(SettingOver);
}