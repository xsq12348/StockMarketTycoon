#include "Project_All.h"
#include <stdlib.h>

#define STOCK_START_PRICE 100

int g_autoPlay = 0;

static const char* kNames[COMPANY_COUNT] = {
	"MAOTAI", "HUAWEI", "MSFT", "IBM", "MIDEA", "CHANEL",
	"APPLE", "TESLA", "TENCENT", "ALIBABA", "CATL", "NVIDIA"
};

static const int kStartMoney[3] = { 50000, 20000, 5000 };
static const int kVolatility[3] = { 3, 5, 7 };

static const char* kNpcName[NPC_COUNT] = { "CAPITALIST", "SPECULATOR", "SMART GUY", "GAMBLER", "QUANT" };
static const int kNpcMoney[NPC_COUNT] = { 500000, 120000, 60000, 8000, 20000 };
static const int kNpcKind[NPC_COUNT] = { NPC_CAPITALIST, NPC_SPECULATOR, NPC_SMART, NPC_GAMBLER, NPC_QUANT };

static int StartPrice(int index)
{
	return STOCK_START_PRICE + index * 17;
}

void StockGameInit(STOCK_GAME* game, int difficulty)
{
	int i;

	if (!game)
		return;

	if (difficulty < 0 || difficulty > 2)
		difficulty = 1;

	memset(game, 0, sizeof(STOCK_GAME));

	game->difficulty = difficulty;
	game->startMoney = kStartMoney[difficulty];
	game->volatility = kVolatility[difficulty];
	game->money = game->startMoney;
	game->netWorth = game->startMoney;
	game->day = 1;
	game->page = 0;
	game->slot = 0;
	game->newsIndex = -1;
	game->screen = SCREEN_MAIN;
	game->menuIndex = 1;
	game->showBoard = 0;
	game->sentiment = 0.f;

	for (i = 0; i < NPC_COUNT; ++i)
	{
		game->npc[i].name = kNpcName[i];
		game->npc[i].kind = kNpcKind[i];
		game->npc[i].money = kNpcMoney[i];
		game->npc[i].netWorth = kNpcMoney[i];
	}

	for (i = 0; i < COMPANY_COUNT; ++i)
	{
		COMPANY* c = &game->company[i];
		int p = StartPrice(i);
		int j;

		c->name = kNames[i];
		c->shares = 0;
		c->change = 0;

		// 开盘前造 20 根历史 K 线，最后一根收在起始价
		{
			long long hist[20];

			hist[19] = p;
			for (j = 18; j >= 0; --j)
			{
				int chg = MoonRandom(0, 0, 11) - 5;

				hist[j] = hist[j + 1] * (100 - chg) / 100;
				if (hist[j] < 1)
					hist[j] = 1;
			}

			for (j = 0; j < 20; ++j)
			{
				int open = (int)hist[j];
				int close = (j == 19) ? p : (int)hist[j + 1];
				int hi = open > close ? open : close;
				int lo = open < close ? open : close;

				c->bars[j].o = open;
				c->bars[j].h = hi + MoonRandom(0, 0, 4);
				c->bars[j].l = lo - MoonRandom(0, 0, 4) < 1 ? 1 : lo - MoonRandom(0, 0, 4);
				c->bars[j].c = close;
				c->bars[j].vol = 80 + MoonRandom(0, 0, 1200);
			}
		}

		c->price = p;
		c->barCount = 20;
		c->barNext = 20 % HISTORY_MAX;
	}
}
