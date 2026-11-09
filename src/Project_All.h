#pragma once

#include "Moon.h"
#include "MoonCore.h"

#pragma warning(disable:4996)

#define COMPANY_COUNT 12
#define PAGE_SIZE 4
#define PAGE_COUNT (COMPANY_COUNT / PAGE_SIZE)
#define HISTORY_MAX 64

#define SCREEN_MAIN		0
#define SCREEN_GAME		1
#define SCREEN_PAUSE	2
#define SCREEN_HELP		3

#define MAIN_ITEM_COUNT		4	// 简单/普通/困难/退出
#define PAUSE_ITEM_COUNT	5	// 继续/重新开始/主界面/帮助/退出

#define NPC_COUNT 5
#define NPC_CAPITALIST	0
#define NPC_SPECULATOR	1
#define NPC_SMART		2
#define NPC_GAMBLER		3
#define NPC_QUANT		4

typedef struct
{
	int o, h, l, c;
	int vol;
} BAR;

typedef struct
{
	const char* name;
	int price;
	int shares;
	int change;
	BAR bars[HISTORY_MAX];
	int barCount;
	int barNext;
} COMPANY;

typedef struct
{
	const char* name;
	int kind;
	int money;
	int holdings[COMPANY_COUNT];
	int netWorth;
} NPC;

typedef struct
{
	COMPANY company[COMPANY_COUNT];
	int money;
	int day;
	int page;
	int slot;
	int newsIndex;
	int newsPercent;
	const char* newsText;
	int netWorth;
	int gameOver;

	int screen;
	int menuIndex;
	int difficulty;
	int helpFrom;
	int startMoney;
	int volatility;

	NPC npc[NPC_COUNT];
	int pressure[COMPANY_COUNT];
	float sentiment;			/* -1 极空 .. +1 极多 */	// NPC 交易对明天价格的压力
	int showBoard;
	int equity[1 + NPC_COUNT][366];	/* 0=玩家，1..=NPC，逐日净值 */					// TAB 显示排行榜
} STOCK_GAME;

extern int g_autoPlay;

/* 按键锁存：绘制线程（与主循环同频）用 Win32 状态置位，逻辑线程消费后清零 */
#define KEY_LATCH_MAX 512
extern volatile unsigned char g_keyLatch[KEY_LATCH_MAX];



void StockGameInit(STOCK_GAME* game, int difficulty);

extern MOON_PROJECTSETTING(_Resource);
extern MOON_PROJECTSETTING(SettingAll);
extern MOON_PROJECTSETTING(SettingOver);

extern MOON_PROJECTMODULE(StockLogicLoad);
extern MOON_PROJECTMODULE(StockLogic);

extern MOON_PROJECTMODULE(StockDrawLoad);
extern MOON_PROJECTMODULE(StockDraw);
