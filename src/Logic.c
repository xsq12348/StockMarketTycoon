#include "Project_All.h"
#include "nn_weights.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define STOCK_DAYS 365

static MOON_TIMELOAD* logicTps = MOON_NULL;
static STOCK_GAME* game = MOON_NULL;
static unsigned char seeded = 0;


/* 主键盘回车 257，小键盘回车 335（GLFW_KEY_KP_ENTER），两个都要认 */
#define KEY_KP_ENTER ((MOON_KEY_TYPE)335)

static int keyHit(int k);

static int keyEnter(void)
{
	return (keyHit(MOON_KEY_RETURN) || keyHit(KEY_KP_ENTER)) ? 1 : 0;
}

/* 引擎自带的随机数：种子由调用方保存，跨机器结果稳定 */

/* 市值系数：盘子越大越抗压（单位：百万级的相对规模） */
static const int kSize[COMPANY_COUNT] = { 3, 2, 4, 5, 2, 2, 6, 2, 4, 6, 2, 8 };

/* 按成交金额给次日施加压力：每 100 万元推动 1 个点，再除以市值系数 */
static void addPressure(int i, int qty, int sign)
{
	long long value = (long long)qty * game->company[i].price;
	long long push = value * 5 / (1000000LL * kSize[i]);

	if (push > 6) push = 6;
	if (push < 1 && value >= 200000) push = 1;

	game->pressure[i] += (int)(sign * push);
}


static int keyHit(int k)
{
	if (k < 0 || k >= KEY_LATCH_MAX)
		return 0;
	if (g_keyLatch[k])
	{
		g_keyLatch[k] = 0;
		return 1;
	}
	return 0;
}

static int rnd(int min, int max)
{
	if (max <= min)
		return min;
	return MoonRandom(0, min, max);
}

static int slopeOf(COMPANY* c, int n)
{
	int i;
	float sx = 0, sy = 0, sxx = 0, sxy = 0;
	int total = 0;

	if (n < 2)
		n = 2;
	if (n > c->barCount)
		n = c->barCount;
	if (n < 2)
		return 0;

	for (i = 0; i < n; ++i)
	{
		int idx = (c->barNext - n + i + HISTORY_MAX) % HISTORY_MAX;
		float y = (float)c->bars[idx].c;

		sx += (float)i;
		sy += y;
		sxx += (float)(i * i);
		sxy += (float)i * y;
		total++;
	}

	if (total < 2)
		return 0;

	return (int)((total * sxy - sx * sy) / (total * sxx - sx * sx));
}

static int avgPrice(COMPANY* c, int n)
{
	int i, sum = 0;

	if (n < 1)
		return c->price;
	if (n > c->barCount)
		n = c->barCount;

	for (i = 0; i < n; ++i)
	{
		int idx = (c->barNext - n + i + HISTORY_MAX) % HISTORY_MAX;
		sum += c->bars[idx].c;
	}

	return sum / n;
}

static void pushBar(COMPANY* c, BAR bar)
{
	c->bars[c->barNext] = bar;
	c->barNext = (c->barNext + 1) % HISTORY_MAX;
	if (c->barCount < HISTORY_MAX)
		c->barCount++;
}

static void advanceCompany(COMPANY* c, int change)
{
	int open = c->price;
	int close = (int)((long long)open * (100 + change) / 100);
	int high, low;
	BAR bar;

	if (close < 0)
		close = 0;

	high = open > close ? open : close;
	low = open < close ? open : close;
	high += rnd(0, 4);
	low -= rnd(0, 4);
	if (low < 1)
		low = 1;

	bar.o = open;
	bar.h = high;
	bar.l = low;
	bar.c = close;
	bar.vol = rnd(80, 1200);

	pushBar(c, bar);

	c->price = close;
	c->change = change;
}


/* 神经网络预测次日涨跌幅（与 train_sim.py 的特征顺序一致） */
static void histCloses(COMPANY* c, int* out)
{
	int j, n = c->barCount < 20 ? c->barCount : 20;
	int pad = 20 - n;

	for (j = 0; j < pad; ++j)
		out[j] = c->price;
	for (j = 0; j < n; ++j)
	{
		int idx = (c->barNext - n + j + HISTORY_MAX) % HISTORY_MAX;
		out[pad + j] = c->bars[idx].c;
	}
}

/* 神经网络预测次日超额收益（12 个特征，和 train_nn_dll.py 顺序一致） */
static float nnPredict(int i)
{
	COMPANY* c = &game->company[i];
	int h[20];
	int j, k;
	float x[12], hid[32];
	float px = c->price > 0 ? (float)c->price : 1.f;
	int base = 100 + i * 17;
	int ma5sum = 0, ma20sum = 0, hi = 0;
	float vol5 = 0.f;

	histCloses(c, h);

	for (j = 16; j < 20; ++j) ma5sum += h[j];
	for (j = 0; j < 20; ++j) { ma20sum += h[j]; if (h[j] > hi) hi = h[j]; }

	for (j = 1; j <= 5; ++j)
	{
		float r = 0.f;

		if (h[20 - j] > 0 && h[19 - j] > 0)
			r = (float)(h[20 - j] - h[19 - j]) / (float)h[19 - j];
		else
			r = (float)(h[20 - j]) / (float)(h[19 - j] > 0 ? h[19 - j] : 1) - 1.f;
		vol5 += r * r;
	}
	vol5 = sqrtf(vol5 / 5.f);

	x[0] = px / (float)base;
	x[1] = (float)c->change / 20.f;
	x[2] = h[18] > 0 ? px / (float)h[18] - 1.f : 0.f;
	x[3] = h[16] > 0 ? px / (float)h[16] - 1.f : 0.f;
	x[4] = h[14] > 0 ? px / (float)h[14] - 1.f : 0.f;
	x[5] = h[9] > 0 ? px / (float)h[9] - 1.f : 0.f;
	x[6] = ma5sum > 0 ? px / ((float)ma5sum / 5.f) - 1.f : 0.f;
	x[7] = ma20sum > 0 ? px / ((float)ma20sum / 20.f) - 1.f : 0.f;
	x[8] = vol5;
	x[9] = game->sentiment;
	x[10] = (i == game->newsIndex) ? (float)game->newsPercent / 25.f : 0.f;
	x[11] = hi > 0 ? px / (float)hi - 1.f : 0.f;

	for (j = 0; j < 12; ++j)
		x[j] = (x[j] - NN_MU[j]) / (NN_SD[j] != 0.f ? NN_SD[j] : 1.f);

	for (k = 0; k < 32; ++k)
	{
		float s = NN_B1[k];

		for (j = 0; j < 12; ++j)
			s += x[j] * NN_W1[j * 32 + k];
		hid[k] = tanhf(s);
	}

	{
		float y = NN_B2[0];

		for (k = 0; k < 32; ++k)
			y += hid[k] * NN_W2[k];
		return y;
	}
}

static void npcAct()
{
	int i, k;

	for (k = 0; k < NPC_COUNT; ++k)
	{
		NPC* npc = &game->npc[k];

		switch (npc->kind)
		{
		case NPC_CAPITALIST:
		{
			/* 有钱但不傻：跌到均线下方才买，涨过头就减一半，手里始终留现金 */
			int i2;

			if (game->day % 3 == 0)
			{
				int best = -1;
				int bestDev = 0;

				for (i2 = 0; i2 < COMPANY_COUNT; ++i2)
				{
					int avg = avgPrice(&game->company[i2], 20);
					int dev;

					if (avg <= 0 || game->company[i2].price <= (100 + i2 * 17) / 3)
						continue;
					if (game->company[i2].shares <= 0)
						continue;

					dev = (game->company[i2].price - avg) * 100 / avg;
					if (dev < -5 && dev < bestDev)
					{
						bestDev = dev;
						best = i2;
					}
				}

				if (best >= 0)
				{
					int amount = npc->money / game->company[best].price / 10;

					if (amount > 0)
					{
						npc->money -= amount * game->company[best].price;
						npc->holdings[best] += amount;
						game->pressure[best] += amount / 200;
					}
				}
			}

			/* 涨过均线 15% 就兑现一半 */
			if (game->day % 5 == 0)
			{
				for (i2 = 0; i2 < COMPANY_COUNT; ++i2)
				{
					int avg = avgPrice(&game->company[i2], 20);

					if (npc->holdings[i2] > 0 && avg > 0 &&
						game->company[i2].price * 100 > avg * 115)
					{
						int amount = npc->holdings[i2] / 2;

						if (amount < 1) amount = 1;
						npc->holdings[i2] -= amount;
						npc->money += amount * game->company[i2].price;
						game->pressure[i2] -= amount / 200;
					}
				}
			}
			break;
		}

		case NPC_SPECULATOR:
			// 追涨杀跌，同时读当天的新闻
			if (game->newsIndex >= 0)
			{
				int ni = game->newsIndex;

				if (game->newsPercent > 0 && npc->money > game->company[ni].price * 100)
				{
					int amount = npc->money / game->company[ni].price / 2;

					npc->money -= amount * game->company[ni].price;
					npc->holdings[ni] += amount;
					game->pressure[ni] += amount / 200;
				}
				else if (game->newsPercent < 0 && npc->holdings[ni] > 0)
				{
					npc->money += npc->holdings[ni] * game->company[ni].price;
					game->pressure[ni] -= npc->holdings[ni] / 200;
					npc->holdings[ni] = 0;
				}
			}

			for (i = 0; i < COMPANY_COUNT; ++i)
			{
				int s = slopeOf(&game->company[i], 10);

				if (s > 1 && npc->money > game->company[i].price * 50)
				{
					int amount = npc->money / game->company[i].price / 3;

					npc->money -= amount * game->company[i].price;
					npc->holdings[i] += amount;
					game->pressure[i] += amount / 200;
				}
				else if (s < -1 && npc->holdings[i] > 0)
				{
					npc->money += npc->holdings[i] * game->company[i].price;
					game->pressure[i] -= npc->holdings[i] / 200;
					npc->holdings[i] = 0;
				}
			}
			break;

		case NPC_SMART:
			// 低买高卖：低于 20 日均价买，赚 3% 就卖
			for (i = 0; i < COMPANY_COUNT; ++i)
			{
				int avg = avgPrice(&game->company[i], 20);

				if (game->company[i].price * 100 < avg * 98 && npc->money > game->company[i].price * 20)
				{
					int amount = npc->money / game->company[i].price / 20;

					if (amount < 1) amount = 1;
					npc->money -= amount * game->company[i].price;
					npc->holdings[i] += amount;
					game->pressure[i] += amount / 400;
				}
				else if (npc->holdings[i] > 0 && game->company[i].price * 100 > avg * 103)
				{
					int amount = npc->holdings[i] / 2;

					if (amount < 1) amount = 1;
					npc->holdings[i] -= amount;
					npc->money += amount * game->company[i].price;
					game->pressure[i] -= amount / 400;
				}
			}
			break;

		case NPC_QUANT:
		{
			/* 微操：把预测换算成目标仓位，每天只向目标挪一部分 */
			float pred[COMPANY_COUNT];
			float wsum = 0.f;
			long long total = npc->money;
			float target[COMPANY_COUNT];
			int moved = 0;

			for (i = 0; i < COMPANY_COUNT; ++i)
			{
				pred[i] = nnPredict(i);
				if (pred[i] < 0.f) pred[i] = 0.f;

				/* 避开退市区 */
				if (game->company[i].price <= (100 + i * 17) * 2 / 5)
					pred[i] = 0.f;

				wsum += pred[i];
				total += (long long)npc->holdings[i] * game->company[i].price;
			}

			for (i = 0; i < COMPANY_COUNT; ++i)
				target[i] = (wsum > 0.f) ? (pred[i] / wsum) : 0.f;

			for (i = 0; i < COMPANY_COUNT; ++i)
			{
				/* 目标市值只占八成，留两成现金 */
				long long want = (long long)(target[i] * 0.8f * (float)total);
				long long have = (long long)npc->holdings[i] * game->company[i].price;
				long long diff = want - have;
				int price = game->company[i].price;

				if (price <= 0 || diff == 0)
					continue;

				diff = diff / 3;			/* 每天只调整三分之一，慢慢挪 */
				if (diff > npc->money) diff = npc->money;
				if (diff == 0)
					continue;

				if (diff > 0)
				{
					int qty = (int)(diff / price);

					if (qty > 0)
					{
						npc->money -= (long long)qty * price;
						npc->holdings[i] += qty;
						game->pressure[i] += qty / 200;
						moved = 1;
					}
				}
				else
				{
					int qty = (int)((-diff) / price);

					if (qty > npc->holdings[i]) qty = npc->holdings[i];
					if (qty > 0)
					{
						npc->holdings[i] -= qty;
						npc->money += (long long)qty * price;
						game->pressure[i] -= qty / 200;
						moved = 1;
					}
				}
			}

			(void)moved;
			break;
		}

		default: // NPC_GAMBLER
			if (rnd(0, 100) < 45)
			{
				int idx = rnd(0, COMPANY_COUNT);
				int amount = npc->money / game->company[idx].price;

				if (amount > 0)
				{
					npc->money -= amount * game->company[idx].price;
					npc->holdings[idx] += amount;
					addPressure(idx, amount, 1);
				}
			}
			else
			{
				for (i = 0; i < COMPANY_COUNT; ++i)
					if (npc->holdings[i] > 0)
					{
						npc->money += npc->holdings[i] * game->company[i].price;
						addPressure(i, npc->holdings[i], -1);
						npc->holdings[i] = 0;
					}
			}
			break;
		}
	}

	for (k = 0; k < NPC_COUNT; ++k)
	{
		long long total;

		game->npc[k].money -= game->npc[k].money / 1000;	/* 0.1%/天 管理费 */

		total = game->npc[k].money;

		for (i = 0; i < COMPANY_COUNT; ++i)
			total += (long long)game->npc[k].holdings[i] * game->company[i].price;

		if (game->npc[k].money > 2000000000) game->npc[k].money = 2000000000;
		if (game->npc[k].money < 0) game->npc[k].money = 0;
		game->npc[k].netWorth = total > 2147483647LL ? 2147483647 : (total < 0 ? 0 : (int)total);
	}

	/* QUANT 当基金：每 30 天拿出净值的 2% 分给其它四个 NPC */
	if (game->day % 30 == 0)
	{
		long long fee = (long long)game->npc[NPC_QUANT].netWorth * 2 / 100;
		long long share;

		if (fee > game->npc[NPC_QUANT].money)
			fee = game->npc[NPC_QUANT].money;

		share = fee / (NPC_COUNT - 1);
		if (share > 0)
		{
			int q;

			game->npc[NPC_QUANT].money -= fee;
			for (q = 0; q < NPC_COUNT; ++q)
				if (q != NPC_QUANT)
					game->npc[q].money += share;

			game->npc[NPC_QUANT].netWorth -= (int)fee;
		}
	}
}

static void writeAiState()
{
	FILE* fp = fopen("ai_state.txt", "w");

	if (!fp)
		return;

	fprintf(fp, "day %d\ncash %d\nnet %d\ndifficulty %d\n", game->day, game->money, game->netWorth, game->difficulty);

	fprintf(fp, "mood %.3f\n", game->sentiment);

	if (game->newsIndex >= 0 && game->newsText)
		fprintf(fp, "news %d %d %s\n", game->newsIndex, game->newsPercent, game->newsText);
	for (int i = 0; i < COMPANY_COUNT; ++i)
	{
		COMPANY* c = &game->company[i];
		int n = c->barCount < 20 ? c->barCount : 20;
		int start = (c->barNext - n + HISTORY_MAX) % HISTORY_MAX;

		fprintf(fp, "stock %d %s %d %d %d\n", i, c->name, c->price, c->change, c->shares);
		fprintf(fp, "hist %d", i);
		for (int j = 0; j < n; ++j)
			fprintf(fp, " %d", c->bars[(start + j) % HISTORY_MAX].c);
		fprintf(fp, "\n");
	}

	for (int i = 0; i < NPC_COUNT; ++i)
		fprintf(fp, "npc %d %s %d\n", i, game->npc[i].name, game->npc[i].netWorth);

	fclose(fp);
}


/* ───────── 新闻库 ───────── */
typedef struct
{
	const char* text;
	int lo, hi;		/* 幅度范围（百分比，正数） */
} NEWS_ITEM;

static const NEWS_ITEM kGood[] = {
	{ "AI CHIP ORDERS HIT RECORD HIGH", 8, 20 },
	{ "DATA CENTER CAPACITY DOUBLES", 8, 18 },
	{ "CENTRAL BANK CUTS RATES", 6, 14 },
	{ "GOVERNMENT UNVEILS SUBSIDY PLAN", 7, 16 },
	{ "CHIP EXPORT RULES RELAXED", 8, 20 },
	{ "MAJOR CONTRACT WINS BID", 8, 22 },
	{ "QUANTUM CHIP BREAKTHROUGH", 10, 25 },
	{ "STRONG EARNINGS BEAT FORECASTS", 9, 24 },
	{ "BUYBACK PROGRAM ANNOUNCED", 6, 15 },
	{ "ANALYSTS RAISE TARGET PRICE", 5, 12 }
};

static const NEWS_ITEM kBad[] = {
	{ "EXPORT BAN EXPANDED", 8, 20 },
	{ "REGULATOR FINES INDUSTRY GIANT", 6, 15 },
	{ "SENATE OPENS ANTITRUST PROBE", 6, 16 },
	{ "EARNINGS MISS EXPECTATIONS", 9, 24 },
	{ "CEO RESIGNS AMID SCANDAL", 8, 20 },
	{ "DATA CENTER FIRE DISRUPTS SERVICE", 10, 22 },
	{ "SUPPLY CHAIN STRIKE ESCALATES", 6, 14 },
	{ "PRODUCT RECALL ANNOUNCED", 8, 20 },
	{ "ACCOUNTING FRAUD ALLEGED", 12, 28 },
	{ "RIVAL CUTS PRICES, MARGINS SLUMP", 6, 14 }
};

static const NEWS_ITEM kMacro[] = {
	{ "GLOBAL RECESSION FEARS SPREAD", 5, 12 },
	{ "INFLATION HITS TWO-DECADE HIGH", 5, 12 },
	{ "CURRENCY CRASH SPARKS PANIC", 6, 14 },
	{ "WAR ESCALATION ROCKS MARKETS", 6, 15 },
	{ "TARIFF WAR INTENSIFIES", 5, 12 },
	{ "CENTRAL BANK RAISES RATES", 5, 12 },
	{ "SOVEREIGN FUND DOUBLES STAKE", 4, 10 },
	{ "IMF WARNS OF SLOWDOWN", 4, 10 },
	{ "OIL PRICE SPIKES ON SUPPLY CUT", 5, 12 },
	{ "BOND MARKET SELLOFF DEEPENS", 5, 12 }
};

static const NEWS_ITEM kBlack[] = {
	{ "HACKERS BREACH CLOUD PROVIDER", 20, 45 },
	{ "EARTHQUAKE HITS MAIN FACTORY", 25, 50 },
	{ "WHISTLEBLOWER LEAKS SECRETS", 15, 35 },
	{ "SHORT SELLER PUBLISHES REPORT", 20, 40 },
	{ "GOVERNMENT SEIZES ASSETS", 25, 50 },
	{ "SUPPLIER BANKRUPTCY FILED", 15, 30 },
	{ "PRODUCT FOUND UNSAFE, BANNED", 20, 40 },
	{ "FOUNDER ARRESTED OVERSEAS", 25, 45 },
	{ "PATENT LOSS IN COURT", 10, 25 },
	{ "MASS LAYOFFS ANNOUNCED", 10, 22 }
};

/* 每家公司自己的特色新闻（第一条偏利好，第二条偏利空） */
static const NEWS_ITEM kOwn[COMPANY_COUNT][2] = {
	{ { "MAOTAI RAISES BOTTLE PRICE", 6, 15 }, { "FAKE LIQUOR SCANDAL HITS BRAND", 6, 16 } },
	{ { "HUAWEI WINS 5G CONTRACT ABROAD", 8, 20 }, { "NEW SANCTIONS TARGET HUAWEI", 10, 24 } },
	{ { "MSFT CLOUD REVENUE JUMPS", 6, 16 }, { "MSFT OUTAGE AFFECTS MILLIONS", 6, 18 } },
	{ { "IBM LANDS GOVERNMENT DEAL", 5, 14 }, { "IBM CUTS JOBS IN SERVICES UNIT", 5, 14 } },
	{ { "MIDEA HOME APPLIANCE SALES SURGE", 6, 15 }, { "MIDEA RECALLS FAULTY UNITS", 6, 15 } },
	{ { "CHANEL RAISES PRICES WORLDWIDE", 5, 12 }, { "LUXURY DEMAND SLOWS IN ASIA", 5, 12 } },
	{ { "APPLE IPHONE PREORDERS STRONG", 6, 16 }, { "APPLE FACES ANTITRUST FINE", 6, 16 } },
	{ { "TESLA DELIVERIES BEAT TARGET", 10, 22 }, { "TESLA AUTOPILOT PROBE OPENS", 12, 25 } },
	{ { "TENCENT GAME REVENUE RECORD", 6, 16 }, { "REGULATOR PAUSES GAME APPROVALS", 8, 18 } },
	{ { "ALIBABA SINGLES DAY RECORD", 6, 16 }, { "ALIBABA FACES DELISTING RISK", 8, 20 } },
	{ { "CATL SIGNS BATTERY SUPPLY DEAL", 8, 18 }, { "CATL SUBSIDY CUT HITS MARGINS", 8, 18 } },
	{ { "NVIDIA AI CHIP ORDERS EXPLODE", 10, 25 }, { "CHIP EXPORT CURBS HIT NVIDIA", 12, 28 } }
};

static int newsLen(const NEWS_ITEM* a) { return 0; }

static void pickNews(const NEWS_ITEM* arr, int n, int* pct)
{
	int i = rnd(0, n);

	*pct = rnd(arr[i].lo, arr[i].hi + 1);
	game->newsText = arr[i].text;
}

static void generateNews()
{
	int roll;

	game->newsIndex = -1;
	game->newsPercent = 0;
	game->newsText = MOON_NULL;

	if (rnd(0, 100) >= 60)		/* 60% 的日子有新闻 */
		return;

	roll = rnd(0, 100);

	if (roll < 30)
	{
		/* 公司自己的特色新闻 */
		int idx = rnd(0, COMPANY_COUNT);
		int pct;

		if (rnd(0, 2) == 0)
		{
			pct = rnd(kOwn[idx][0].lo, kOwn[idx][0].hi + 1);
			game->newsText = kOwn[idx][0].text;
		}
		else
		{
			pct = -rnd(kOwn[idx][1].lo, kOwn[idx][1].hi + 1);
			game->newsText = kOwn[idx][1].text;
		}

		game->newsIndex = idx;
		game->newsPercent = pct;
	}
	else if (roll < 60)
	{
		int pct;

		if (rnd(0, 2) == 0)
		{
			pickNews(kGood, (int)(sizeof(kGood) / sizeof(kGood[0])), &pct);
			game->newsPercent = pct;
		}
		else
		{
			pickNews(kBad, (int)(sizeof(kBad) / sizeof(kBad[0])), &pct);
			game->newsPercent = -pct;
		}
		game->newsIndex = rnd(0, COMPANY_COUNT);
	}
	else if (roll < 80)
	{
		/* 宏观：影响全市场，指数级情绪波动 */
		int pct;

		pickNews(kMacro, (int)(sizeof(kMacro) / sizeof(kMacro[0])), &pct);
		game->newsIndex = -1;
		game->newsPercent = (rnd(0, 2) == 0) ? pct : -pct;
	}
	else
	{
		/* 黑天鹅：单只、幅度大 */
		int pct;

		pickNews(kBlack, (int)(sizeof(kBlack) / sizeof(kBlack[0])), &pct);
		game->newsIndex = rnd(0, COMPANY_COUNT);
		game->newsPercent = -pct;	/* 黑天鹅默认利空 */
	}
}

/* 每只股票的性格：波动倍率(%)、自身漂移(%)、新闻敏感度(%) */
static const int kVolPct[COMPANY_COUNT] = { 70, 150, 120, 90, 80, 70, 120, 180, 130, 120, 160, 200 };
static const int kBiasPct[COMPANY_COUNT] = { 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 2 };
static const int kNewsSens[COMPANY_COUNT] = { 90, 140, 120, 100, 90, 80, 120, 150, 130, 120, 140, 160 };
/* 新闻好坏改变市场情绪：情绪再决定每天的漂移方向与幅度 */
static void updateSentiment()
{
	if (game->newsIndex >= 0 || game->newsText)
	{
		float v = (float)game->newsPercent / 25.0f;

		game->sentiment = game->sentiment * 0.85f + v * 0.15f;
		if (game->sentiment > 1.f) game->sentiment = 1.f;
		if (game->sentiment < -1.f) game->sentiment = -1.f;
	}
	else
	{
		game->sentiment *= 0.98f;
	}
}

/* 跌破 0：持仓清零不返还，价格重置到基准价，重新上市 */
static void collapseCompany(int i)
{
	COMPANY* c = &game->company[i];
	int base = 100 + i * 17;
	int k;

	c->shares = 0;
	c->price = base;
	c->change = 0;
	c->barCount = 1;
	c->barNext = 1;
	c->bars[0].o = base;
	c->bars[0].h = base;
	c->bars[0].l = base;
	c->bars[0].c = base;
	c->bars[0].vol = 100;

	for (k = 0; k < NPC_COUNT; ++k)
		game->npc[k].holdings[i] = 0;

	game->newsIndex = i;
	game->newsPercent = 0;
	game->newsText = "COMPANY COLLAPSED - DELISTED AND RESET";
}
static void recalcNetWorth();

static void advanceDay()
{
	int i;

	generateNews();
	updateSentiment();

	for (i = 0; i < COMPANY_COUNT; ++i)
	{
		int v = game->volatility * kVolPct[i] / 100;
		int drift = (int)(game->sentiment * 2.f);	/* 情绪 -1..+1 -> 漂移 -2%..+2% */
		int change = rnd(-v, v + 1) + drift + kBiasPct[i];

		if (v < 1) v = 1;

		/* 一半的日子 +1%，让长期收益接近零和，而不是单边下跌 */
		if (rnd(0, 20) == 0)
			change += 1;

		/* 动量：延续昨天方向的 30% */
		{
			int prev = 0;
			int newest = (game->company[i].barNext - 1 + HISTORY_MAX) % HISTORY_MAX;
			int older = (game->company[i].barNext - 2 + HISTORY_MAX) % HISTORY_MAX;

			if (game->company[i].bars[older].c > 0)
				prev = (game->company[i].bars[newest].c - game->company[i].bars[older].c) * 100
					/ game->company[i].bars[older].c;
			change += prev * 3 / 100;
		}

		/* 均值回归：偏离 20 日均线越多，回拉越强 */
		{
			int sum = 0, j, n = game->company[i].barCount < 20 ? game->company[i].barCount : 20;

			for (j = 0; j < n; ++j)
			{
				int idx = (game->company[i].barNext - n + j + HISTORY_MAX) % HISTORY_MAX;
				sum += game->company[i].bars[idx].c;
			}
			if (n > 0 && sum > 0)
				change -= ((game->company[i].price * 100 / (sum / n)) - 100) * 20 / 100;
		}

		if (game->newsIndex >= 0)
		{
			if (i == game->newsIndex)
				change += game->newsPercent * kNewsSens[i] / 100;
		}
		else if (game->newsText)
		{
			/* 宏观新闻：所有股票同向 */
			change += game->newsPercent * kNewsSens[i] / 200;
		}

		change += game->pressure[i];
		game->pressure[i] = 0;

		if (change < -100) change = -100;
		if (change > 30) change = 30;

		if ((long long)game->company[i].price * (100 + change) / 100 < 1)
		{
			collapseCompany(i);
			continue;
		}

		advanceCompany(&game->company[i], change);
	}

	npcAct();

	/* 记一条当日净值曲线 */
	{
		int rec = game->day - 1;
		int k;

		if (rec >= 0 && rec < 366)
		{
			recalcNetWorth();
			game->equity[0][rec] = game->netWorth;
			for (k = 0; k < NPC_COUNT; ++k)
				game->equity[1 + k][rec] = game->npc[k].netWorth;
		}
	}

	game->day++;


	if (game->day > STOCK_DAYS)
	{
		game->gameOver = 1;
		game->menuIndex = 0;
	}

	writeAiState();
}

static void recalcNetWorth()
{
	long long total = game->money;
	int i;

	for (i = 0; i < COMPANY_COUNT; ++i)
		total += (long long)game->company[i].shares * game->company[i].price;

	game->netWorth = total > 2147483647LL ? 2147483647 : (int)total;
}

static COMPANY* selectedCompany()
{
	return &game->company[game->page * PAGE_SIZE + game->slot];
}


/* 交易日志：day 动作 名称 价格 数量 现金 */
static void logTrade(const char* action, COMPANY* c, int qty)
{
	FILE* fp = fopen("trades.log", "a");

	if (!fp)
		return;

	fprintf(fp, "day %d  %-4s  %-8s  price %4d  qty %5d  cash %7d\n",
		game->day, action, c->name, c->price, qty, game->money);
	fclose(fp);
}

static void buyShares(int amount)
{
	COMPANY* c = selectedCompany();
	long long cost;

	if (amount <= 0)
		amount = game->money / c->price;

	if (amount <= 0)
		return;

	cost = (long long)amount * c->price * 1003 / 1000;
	if (cost > game->money)
	{
		/* 含手续费后不能超过现金 */
		amount = (int)((long long)game->money * 1000 / ((long long)c->price * 1003));
		cost = (long long)amount * c->price * 1003 / 1000;
	}

	if (amount <= 0)
		return;

	game->money -= (int)cost;
	c->shares += amount;
	logTrade("BUY", c, amount);

	/* 大额买入对下一交易日形成上涨压力 */
	{
		int idx = (int)(c - game->company);
		addPressure(idx, amount, 1);
	}
}

static void sellShares(int amount)
{
	COMPANY* c = selectedCompany();

	if (amount <= 0 || amount > c->shares)
		amount = c->shares;

	if (amount <= 0)
		return;

	c->shares -= amount;
	game->money += c->price * amount * 997 / 1000;
	logTrade("SELL", c, amount);

	/* 大额卖出对下一交易日形成下跌压力 */
	{
		int idx = (int)(c - game->company);
		addPressure(idx, amount, -1);
	}
}

/* 外部接口：ai_cmd.txt 里每行一条命令，处理完改名 ai_cmd.done.txt */
static void readAiCommands()
{
	FILE* fp = fopen("ai_cmd.txt", "r");
	char line[128];

	if (!fp)
		return;

	while (fgets(line, sizeof(line), fp))
	{
		int a = 0, b = 0;

		if (sscanf(line, "buy %d %d", &a, &b) == 2 && a >= 0 && a < COMPANY_COUNT)
		{
			game->page = a / PAGE_SIZE;
			game->slot = a % PAGE_SIZE;
			buyShares(b);
		}
		else if (sscanf(line, "sell %d %d", &a, &b) == 2 && a >= 0 && a < COMPANY_COUNT)
		{
			game->page = a / PAGE_SIZE;
			game->slot = a % PAGE_SIZE;
			sellShares(b);
		}
		else if (strncmp(line, "all", 3) == 0)
		{
			buyShares(-1);
		}
		else if (strncmp(line, "dump", 4) == 0)
		{
			sellShares(-1);
		}
		else if (strncmp(line, "board", 5) == 0)
		{
			game->showBoard = !game->showBoard;
		}
		else if (strncmp(line, "next", 4) == 0)
		{
			advanceDay();
		}
	}

	fclose(fp);
	remove("ai_cmd.done.txt");
	rename("ai_cmd.txt", "ai_cmd.done.txt");
	recalcNetWorth();
	writeAiState();
}

static void mainMenuInput()
{
	if (keyHit(MOON_KEY_UP))
		game->menuIndex = (game->menuIndex + MAIN_ITEM_COUNT - 1) % MAIN_ITEM_COUNT;

	if (keyHit(MOON_KEY_DOWN))
		game->menuIndex = (game->menuIndex + 1) % MAIN_ITEM_COUNT;

	if (keyHit(MOON_KEY_1)) game->menuIndex = 0;
	if (keyHit(MOON_KEY_2)) game->menuIndex = 1;
	if (keyHit(MOON_KEY_3)) game->menuIndex = 2;

	if (keyHit(MOON_KEY_H))
	{
		game->helpFrom = SCREEN_MAIN;
		game->screen = SCREEN_HELP;
		return;
	}

	if (keyHit(MOON_KEY_ESCAPE))
	{
		MoonDead();
		return;
	}

	if (keyEnter() || keyHit(MOON_KEY_SPACE))
	{
		if (game->menuIndex >= 3)
		{
			MoonDead();
			return;
		}

		StockGameInit(game, game->menuIndex);
		game->difficulty = game->menuIndex;
		game->screen = SCREEN_GAME;
		writeAiState();
	}
}

static void pauseInput()
{
	if (keyHit(MOON_KEY_UP))
		game->menuIndex = (game->menuIndex + PAUSE_ITEM_COUNT - 1) % PAUSE_ITEM_COUNT;

	if (keyHit(MOON_KEY_DOWN))
		game->menuIndex = (game->menuIndex + 1) % PAUSE_ITEM_COUNT;

	if (keyHit(MOON_KEY_ESCAPE))
	{
		game->screen = SCREEN_GAME;
		return;
	}

	if (keyEnter())
	{
		switch (game->menuIndex)
		{
		case 0:
			game->screen = SCREEN_GAME;
			break;
		case 1:
		{
			int difficulty = game->difficulty;

			StockGameInit(game, difficulty);
			game->screen = SCREEN_GAME;
			writeAiState();
			break;
		}
		case 2:
			StockGameInit(game, game->difficulty);
			game->screen = SCREEN_MAIN;
			break;
		case 3:
			game->helpFrom = SCREEN_PAUSE;
			game->screen = SCREEN_HELP;
			break;
		default:
			MoonDead();
			break;
		}
	}
}

static void helpInput()
{
	if (keyHit(MOON_KEY_ESCAPE) || keyEnter() || keyHit(MOON_KEY_SPACE))
	{
		game->screen = game->helpFrom;
		game->menuIndex = 0;
	}
}

static void gameInput()
{
	readAiCommands();

	if (game->gameOver)
	{
		/* 上下、左右都能选（左右更符合横向排列的菜单） */
		if (keyHit(MOON_KEY_UP) || keyHit(MOON_KEY_LEFT))
			game->menuIndex = (game->menuIndex + 2) % 3;
		if (keyHit(MOON_KEY_DOWN) || keyHit(MOON_KEY_RIGHT))
			game->menuIndex = (game->menuIndex + 1) % 3;

		if (keyEnter())
		{
			if (game->menuIndex == 0)
			{
				int difficulty = game->difficulty;

				StockGameInit(game, difficulty);
				game->screen = SCREEN_GAME;
				writeAiState();
			}
			else if (game->menuIndex == 1)
			{
				StockGameInit(game, game->difficulty);
				game->screen = SCREEN_MAIN;
			}
			else
			{
				MoonDead();
			}
			return;
		}

		if (keyHit(MOON_KEY_ESCAPE))
		{
			MoonDead();
			return;
		}

		return;
	}

	{
		if (keyHit(MOON_KEY_ESCAPE))
		{
			game->screen = SCREEN_PAUSE;
			game->menuIndex = 0;
			return;
		}

		if (keyHit(MOON_KEY_TAB))
			game->showBoard = !game->showBoard;

		if (keyHit(MOON_KEY_Q))
			game->page = (game->page + PAGE_COUNT - 1) % PAGE_COUNT;
		if (keyHit(MOON_KEY_E))
			game->page = (game->page + 1) % PAGE_COUNT;

		/* 左右先在行内移动，到了边缘才换页 */
		if (keyHit(MOON_KEY_LEFT))
		{
			if (game->slot % 2 == 1)
				game->slot--;
			else if (game->page > 0)
			{
				game->page--;
				game->slot++;
			}
		}

		if (keyHit(MOON_KEY_RIGHT))
		{
			if (game->slot % 2 == 0)
				game->slot++;
			else if (game->page < PAGE_COUNT - 1)
			{
				game->page++;
				game->slot--;
			}
		}
		if (keyHit(MOON_KEY_UP) && game->slot >= 2)
			game->slot -= 2;
		if (keyHit(MOON_KEY_DOWN) && game->slot < 2)
			game->slot += 2;

		if (keyHit(MOON_KEY_1)) buyShares(1);
		if (keyHit(MOON_KEY_2)) sellShares(1);
		if (keyHit(MOON_KEY_3)) buyShares(50);
		if (keyHit(MOON_KEY_4)) sellShares(50);
		if (keyHit(MOON_KEY_5)) buyShares(-1);
		if (keyHit(MOON_KEY_6)) sellShares(-1);

		/* 回车 = 进入下一个交易日 */
		if (keyEnter())
			advanceDay();
	}
}

extern MOON_PROJECTMODULE(StockLogicLoad)
{
	MOON_TIMELOAD* foundTps = MOON_NULL;
	STOCK_GAME* foundGame = MOON_NULL;

	MoonHashFindEntity("LogicTps", MOON_TIMELOAD, tps_2);
	foundTps = tps_2;

	MoonHashFindEntity("StockGame", STOCK_GAME, game_2);
	foundGame = game_2;

	logicTps = foundTps;
	game = foundGame;


	return 1;
}

extern MOON_PROJECTMODULE(StockLogic)
{
	if (!game)
		return 1;

	if (!MoonTimeLoad(logicTps, MOON_TRUE))
	{
		MoonSleep(1);
		return 1;
	}

	switch (game->screen)
	{
	case SCREEN_MAIN:	mainMenuInput(); break;
	case SCREEN_GAME:	gameInput(); break;
	case SCREEN_PAUSE:	pauseInput(); break;
	case SCREEN_HELP:	helpInput(); break;
	}

	static int lastWrittenDay = -1;

	if (game->day != lastWrittenDay)
	{
		writeAiState();
		lastWrittenDay = game->day;
	}

	recalcNetWorth();
	return 1;
}
