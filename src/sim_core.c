/* sim_core.c —— 独立的行情推演后端（编译成 sim.dll，供 Python ctypes 调用）
 *
 * 规则与 Logic.c 保持一致：价格基准、漂移、新闻、情绪、退市、NPC 压力。
 * 不依赖 MoonEngine，纯 C99。
 */
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define N 12
#define NPC_COUNT 5
#define HIST 20

typedef struct
{
	int price[N], change[N], shares[N];
	int hist[N][HIST];
	long long npcMoney[NPC_COUNT];
	int npcHold[NPC_COUNT][N];
	int pressure[N];
	int mood100;			/* 情绪 *100，-100..100 */
	int newsIdx, newsPct;
	int day;
	int v;
	long long cash;
	unsigned int rng;
} SIM;

static unsigned int nextRand(SIM* s)
{
	s->rng ^= s->rng << 13;
	s->rng ^= s->rng >> 17;
	s->rng ^= s->rng << 5;
	return s->rng;
}

static int rndRange(SIM* s, int a, int b)	/* [a, b) */
{
	if (b <= a) return a;
	return a + (int)(nextRand(s) % (unsigned)(b - a));
}

/* 每只股票的性格：波动倍率(%)、自身漂移(%)、新闻敏感度(%) */
static const int kVolPct[N] = { 70, 150, 120, 90, 80, 70, 120, 180, 130, 120, 160, 200 };
static const int kBiasPct[N] = { 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 2 };
static const int kNewsSens[N] = { 90, 140, 120, 100, 90, 80, 120, 150, 130, 120, 140, 160 };

static int basePrice(int i) { return 100 + i * 17; }

static int slopeOf(SIM* s, int i)
{
	int j, n = 10;
	float sx = 0, sy = 0, sxx = 0, sxy = 0;
	int start = HIST - n;

	for (j = 0; j < n; ++j)
	{
		sx += (float)j;
		sy += (float)s->hist[i][start + j];
		sxx += (float)(j * j);
		sxy += (float)j * s->hist[i][start + j];
	}

	{
		float den = n * sxx - sx * sx;

		if (den == 0.f) return 0;
		return (int)((n * sxy - sx * sy) / den);
	}
}

static void npcAct(SIM* s)
{
	int k, i;

	for (k = 0; k < NPC_COUNT; ++k)
	{
		switch (k)
		{
		case 0:	/* CAPITALIST */
			if (s->day % 3 == 0)
			{
				int idx = rndRange(s, 0, N);
				int amount = s->npcMoney[k] / s->price[idx] / 10;

				if (amount > 0)
				{
					s->npcMoney[k] -= amount * s->price[idx];
					s->npcHold[k][idx] += amount;
					s->pressure[idx] += amount / 200;
				}
			}
			break;

		case 1:	/* SPECULATOR */
			if (s->newsIdx >= 0)
			{
				int ni = s->newsIdx;

				if (s->newsPct > 0 && s->npcMoney[k] > s->price[ni] * 100)
				{
					int amount = s->npcMoney[k] / s->price[ni] / 2;

					s->npcMoney[k] -= amount * s->price[ni];
					s->npcHold[k][ni] += amount;
					s->pressure[ni] += amount / 200;
				}
				else if (s->newsPct < 0 && s->npcHold[k][ni] > 0)
				{
					s->npcMoney[k] += s->npcHold[k][ni] * s->price[ni];
					s->pressure[ni] -= s->npcHold[k][ni] / 200;
					s->npcHold[k][ni] = 0;
				}
			}
			for (i = 0; i < N; ++i)
			{
				int sl = slopeOf(s, i);

				if (sl > 1 && s->npcMoney[k] > s->price[i] * 50)
				{
					int amount = s->npcMoney[k] / s->price[i] / 3;

					s->npcMoney[k] -= amount * s->price[i];
					s->npcHold[k][i] += amount;
					s->pressure[i] += amount / 200;
				}
				else if (sl < -1 && s->npcHold[k][i] > 0)
				{
					s->npcMoney[k] += s->npcHold[k][i] * s->price[i];
					s->pressure[i] -= s->npcHold[k][i] / 200;
					s->npcHold[k][i] = 0;
				}
			}
			break;

		case 2:	/* SMART */
			for (i = 0; i < N; ++i)
			{
				int sum = 0, j, avg;

				for (j = 0; j < HIST; ++j) sum += s->hist[i][j];
				avg = sum / HIST;

				if (s->price[i] < avg && s->npcMoney[k] > s->price[i] * 10)
				{
					int amount = s->npcMoney[k] / s->price[i] / 20;

					if (amount < 1) amount = 1;
					s->npcMoney[k] -= amount * s->price[i];
					s->npcHold[k][i] += amount;
					s->pressure[i] += amount / 400;
				}
				else if (s->npcHold[k][i] > 0 && s->price[i] > avg * 103 / 100)
				{
					int amount = s->npcHold[k][i] / 2;

					if (amount < 1) amount = 1;
					s->npcHold[k][i] -= amount;
					s->npcMoney[k] += amount * s->price[i];
					s->pressure[i] -= amount / 400;
				}
			}
			break;

		case 3:	/* GAMBLER */
			if (rndRange(s, 0, 100) < 45)
			{
				int idx = rndRange(s, 0, N);
				int amount = s->npcMoney[k] / s->price[idx];

				if (amount > 0)
				{
					s->npcMoney[k] -= amount * s->price[idx];
					s->npcHold[k][idx] += amount;
					s->pressure[idx] += amount / 100;
				}
			}
			else
			{
				for (i = 0; i < N; ++i)
					if (s->npcHold[k][i] > 0)
					{
						s->npcMoney[k] += s->npcHold[k][i] * s->price[i];
						s->pressure[i] -= s->npcHold[k][i] / 100;
						s->npcHold[k][i] = 0;
					}
			}
			break;

		default:	/* QUANT 槽位：C 侧不实现，Python 训练时由外部驱动 */
			break;
		}
	}
}

__declspec(dllexport) SIM* sim_new(int difficulty, unsigned int seed)
{
	SIM* s = (SIM*)calloc(1, sizeof(SIM));
	int i, j;

	if (!s) return 0;

	s->rng = seed ? seed : 12345u;
	s->v = (difficulty == 0) ? 3 : (difficulty == 2 ? 7 : 5);
	s->cash = 20000;
	s->day = 1;
	s->newsIdx = -1;

	for (i = 0; i < N; ++i)
	{
		s->price[i] = basePrice(i);
		s->change[i] = 0;
		s->shares[i] = 0;
		for (j = 0; j < HIST; ++j)
			s->hist[i][j] = s->price[i];
	}

	s->npcMoney[0] = 500000;
	s->npcMoney[1] = 120000;
	s->npcMoney[2] = 60000;
	s->npcMoney[3] = 8000;
	s->npcMoney[4] = 20000;

	return s;
}

__declspec(dllexport) void sim_free(SIM* s) { free(s); }

__declspec(dllexport) void sim_step(SIM* s)
{
	int i, j;
	int drift;

	/* 新闻 + 情绪 */
	s->newsIdx = -1;
	s->newsPct = 0;
	if (rndRange(s, 0, 100) < 45)
	{
		s->newsIdx = rndRange(s, 0, N);
		s->newsPct = rndRange(s, 10, 26);
		if (rndRange(s, 0, 2) == 0) s->newsPct = -s->newsPct;
		s->mood100 = (int)(s->mood100 * 0.85f + (s->newsPct / 25.0f) * 15.f);
		if (s->mood100 > 100) s->mood100 = 100;
		if (s->mood100 < -100) s->mood100 = -100;
	}
	else
	{
		s->mood100 = (int)(s->mood100 * 0.98f);
	}

	drift = s->mood100 * 2 / 100;

	for (i = 0; i < N; ++i)
	{
		int vv = s->v * kVolPct[i] / 100;

		if (vv < 1) vv = 1;

		int change = rndRange(s, -vv, vv + 1) + drift + kBiasPct[i];

		if (rndRange(s, 0, 20) == 0) change += 1;
		if (i == s->newsIdx) change += s->newsPct * kNewsSens[i] / 100;

		/* 动量：延续昨天方向的 30% */
		{
			int prev = 0;

			if (s->hist[i][HIST - 2] > 0)
				prev = (s->hist[i][HIST - 1] - s->hist[i][HIST - 2]) * 100 / s->hist[i][HIST - 2];
			change += prev * 3 / 100;
		}

		/* 均值回归：偏离 20 日均线越多，回拉越强 */
		{
			int sum = 0, j, ma;

			for (j = 0; j < HIST; ++j) sum += s->hist[i][j];
			ma = sum / HIST;
			if (ma > 0)
				change -= ((s->price[i] * 100 / ma) - 100) * 20 / 100;
		}

		change += s->pressure[i];
		s->pressure[i] = 0;

		{
			int bp = basePrice(i);

			if (s->price[i] < bp / 5) change += 3;
			if (s->price[i] > bp * 5) change -= 3;
		}

		if (change < -100) change = -100;
		if (change > 30) change = 30;

		if ((long long)s->price[i] * (100 + change) / 100 < 1)
		{
			s->shares[i] = 0;
			s->price[i] = basePrice(i);
			s->change[i] = 0;
			for (j = 0; j < HIST; ++j) s->hist[i][j] = s->price[i];
			continue;
		}

		{
			int close = (int)((long long)s->price[i] * (100 + change) / 100);

			if (close < 0) close = 0;
			for (j = 0; j < HIST - 1; ++j) s->hist[i][j] = s->hist[i][j + 1];
			s->hist[i][HIST - 1] = close;
			s->price[i] = close;
			s->change[i] = change;
		}
	}

	npcAct(s);
	s->day++;
}

__declspec(dllexport) int sim_day(SIM* s) { return s->day; }
__declspec(dllexport) int sim_price(SIM* s, int i) { return s->price[i]; }
__declspec(dllexport) int sim_change(SIM* s, int i) { return s->change[i]; }
__declspec(dllexport) int sim_shares(SIM* s, int i) { return s->shares[i]; }
__declspec(dllexport) int sim_mood100(SIM* s) { return s->mood100; }
__declspec(dllexport) int sim_news_idx(SIM* s) { return s->newsIdx; }
__declspec(dllexport) int sim_news_pct(SIM* s) { return s->newsPct; }
__declspec(dllexport) long long sim_cash(SIM* s) { return s->cash; }
__declspec(dllexport) long long sim_npc_money(SIM* s, int k) { return s->npcMoney[k]; }
__declspec(dllexport) int sim_slope(SIM* s, int i) { return slopeOf(s, i); }
__declspec(dllexport) int sim_base(int i) { return basePrice(i); }

__declspec(dllexport) void sim_get_hist(SIM* s, int i, int* out)
{
	int j;

	for (j = 0; j < HIST; ++j) out[j] = s->hist[i][j];
}

__declspec(dllexport) int sim_buy(SIM* s, int i, int qty)
{
	long long cost;

	if (i < 0 || i >= N || qty <= 0) return 0;
	cost = (long long)qty * s->price[i] * 1003 / 1000;	/* 0.3% 手续费 */
	if (cost > s->cash)
	{
		qty = s->cash / s->price[i];
		cost = (long long)qty * s->price[i] * 1003 / 1000;	/* 0.3% 手续费 */
	}
	if (qty <= 0) return 0;

	s->cash -= cost;
	s->shares[i] += qty;
	s->pressure[i] += qty / 100;
	return qty;
}

__declspec(dllexport) int sim_sell(SIM* s, int i, int qty)
{
	if (i < 0 || i >= N || qty <= 0) return 0;
	if (qty > s->shares[i]) qty = s->shares[i];
	if (qty <= 0) return 0;

	s->shares[i] -= qty;
	s->cash += (long long)qty * s->price[i] * 997 / 1000;
	s->pressure[i] -= qty / 100;
	return qty;
}
