#include "Project_All.h"

extern MOON_PROJECTSETTING(SettingAll)
{
	MOON_TIMELOAD* logicTps = MOON_NULL;

	MoonHashFindEntity("LogicTps", MOON_TIMELOAD, tps);
	logicTps = tps;

		MoonTimeLoadInit(logicTps, 1000 / 120);

	MoonSetMouse(MOON_CURSOR_MODE_HIDDEN);
	StockDrawLoad();
	StockLogicLoad();
}

extern MOON_PROJECTSETTING(SettingOver)
{
}
