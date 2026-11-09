#include <windows.h>
#include "Project_All.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

static MOON_IMAGE* backbuffer = MOON_NULL;
static MOON_IMAGE* screen = MOON_NULL;
static MOON_IMAGE* canvas = MOON_NULL;
static unsigned int* shaderCrt = MOON_NULL;
static STOCK_GAME* game = MOON_NULL;

#define COLOR_BG		MoonRGBA(16, 20, 28, 255)
#define COLOR_TITLE		MoonRGBA(255, 210, 90, 255)
#define COLOR_TEXT		MoonRGBA(225, 228, 235, 255)
#define COLOR_DIM		MoonRGBA(140, 148, 160, 255)
#define COLOR_UP		MoonRGBA(235, 90, 90, 255)
#define COLOR_DOWN		MoonRGBA(90, 200, 120, 255)
#define COLOR_PRED		MoonRGBA(110, 170, 250, 255)
#define COLOR_CELL		MoonRGBA(24, 30, 40, 255)
#define COLOR_BORDER	MoonRGBA(60, 72, 90, 255)
#define COLOR_GRID		MoonRGBA(45, 55, 70, 255)
#define COLOR_PANEL		MoonRGBA(22, 28, 38, 255)

#define CHAR_W 16
#define CHAR_H 32

static void drawLeaderboard();


/* 原来按 1280x800 排版，现在按 1800x900 等比拉伸；字号保持 16x32 */
#define UI_BASE_W 1280
#define UI_BASE_H 800
#define UI_W 1800
#define UI_H 900
#define UI_SX(v) ((v) * UI_W / UI_BASE_W)
#define UI_SY(v) ((v) * UI_H / UI_BASE_H)

static void uiLine(int x1, int y1, int x2, int y2, unsigned int c)
{
	MoonDrawLine(backbuffer, UI_SX(x1), UI_SY(y1), UI_SX(x2), UI_SY(y2), c);
}

static void uiBox(int x1, int y1, int x2, int y2, unsigned int c)
{
	MoonDrawBox(backbuffer, UI_SX(x1), UI_SY(y1), UI_SX(x2), UI_SY(y2), c);
}

static void uiBoxFull(int x1, int y1, int x2, int y2, unsigned int c)
{
	MoonDrawBoxFull(backbuffer, UI_SX(x1), UI_SY(y1), UI_SX(x2), UI_SY(y2), c);
}

static void text(const char* s, int x, int y, unsigned int color)
{
	MoonDrawTextFont(backbuffer, s, UI_SX(x), UI_SY(y), CHAR_W, CHAR_H, color);
}

static void textBig(const char* s, int x, int y, unsigned int color)
{
	MoonDrawTextFont(backbuffer, s, UI_SX(x), UI_SY(y), 24, 48, color);
}



volatile unsigned char g_keyLatch[KEY_LATCH_MAX];
static unsigned char g_keySeen[KEY_LATCH_MAX];

/* 用 Win32 实时键盘状态采样，边沿触发：一次按下只置位一次 */
static void latchKeys()
{
	static const struct { int moon; int vk; } keys[] = {
		{ MOON_KEY_RETURN, VK_RETURN },
		{ MOON_KEY_ESCAPE, VK_ESCAPE },
		{ MOON_KEY_TAB, VK_TAB },
		{ MOON_KEY_H, 'H' },
		{ MOON_KEY_Q, 'Q' },
		{ MOON_KEY_E, 'E' },
		{ MOON_KEY_N, 'N' },
		{ MOON_KEY_UP, VK_UP },
		{ MOON_KEY_DOWN, VK_DOWN },
		{ MOON_KEY_LEFT, VK_LEFT },
		{ MOON_KEY_RIGHT, VK_RIGHT },
		{ MOON_KEY_1, '1' },
		{ MOON_KEY_2, '2' },
		{ MOON_KEY_3, '3' },
		{ MOON_KEY_4, '4' },
		{ MOON_KEY_5, '5' },
		{ MOON_KEY_6, '6' }
	};
	unsigned int i;

	for (i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i)
	{
		int k = keys[i].moon;

		if (GetAsyncKeyState(keys[i].vk) & 0x8000)
		{
			if (!g_keySeen[k])
			{
				g_keySeen[k] = 1;
				g_keyLatch[k] = 1;
			}
		}
		else
		{
			g_keySeen[k] = 0;
		}
	}
}

static void clearScreen()
{
	MOON_METADATA metadata = { 0 };

	metadata.draw.image_goal = backbuffer;
	metadata.draw.color = COLOR_BG;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE_CLEAN, metadata);
}

static int barIndexAt(COMPANY* c, int i, int total)
{
	return (c->barNext - total + i + HISTORY_MAX) % HISTORY_MAX;
}

static float trendSlope(COMPANY* c, int n)
{
	int i;
	float sx = 0, sy = 0, sxx = 0, sxy = 0;

	if (n < 2)
		return 0.f;

	for (i = 0; i < n; ++i)
	{
		int idx = barIndexAt(c, i, n);
		float y = (float)c->bars[idx].c;

		sx += (float)i;
		sy += y;
		sxx += (float)(i * i);
		sxy += (float)i * y;
	}

	return (n * sxy - sx * sy) / (n * sxx - sx * sx);
}

static void drawKLine(COMPANY* c, int x, int y, int w, int h)
{
	int i;
	int n = c->barCount < 20 ? c->barCount : 20;
	int slots = n + 6;
	float spacing = (float)w / (float)slots;
	int minv = 0, maxv = 0, first = 1;
	int baseY;
	float slope;
	int predLast, predMin, predMax;
	int lastX, lastY;

	if (n <= 0)
		return;

	for (i = 0; i < n; ++i)
	{
		BAR* b = &c->bars[barIndexAt(c, i, n)];

		if (first) { minv = b->l; maxv = b->h; first = 0; }
		if (b->l < minv) minv = b->l;
		if (b->h > maxv) maxv = b->h;
	}

	slope = trendSlope(c, n);
	predLast = c->bars[barIndexAt(c, n - 1, n)].c;
	predMin = predMax = predLast;
	for (i = 1; i <= 5; ++i)
	{
		int p = (int)(predLast + slope * i);
		if (p < predMin) predMin = p;
		if (p > predMax) predMax = p;
	}
	if (predMin < minv) minv = predMin;
	if (predMax > maxv) maxv = predMax;
	if (maxv - minv < 4) maxv = minv + 4;

	baseY = y + h - 4;

	for (i = 0; i < n; ++i)
	{
		BAR* b = &c->bars[barIndexAt(c, i, n)];
		int cx = x + (int)(spacing * (i + 0.5f));
		int half = (int)(spacing * 0.3f);
		int yh = baseY - (int)((float)(h - 8) * (b->h - minv) / (maxv - minv));
		int yl = baseY - (int)((float)(h - 8) * (b->l - minv) / (maxv - minv));
		int yo = baseY - (int)((float)(h - 8) * (b->o - minv) / (maxv - minv));
		int yc = baseY - (int)((float)(h - 8) * (b->c - minv) / (maxv - minv));
		unsigned int color = b->c >= b->o ? COLOR_UP : COLOR_DOWN;
		int top = yo < yc ? yo : yc;
		int bottom = yo < yc ? yc : yo;

		if (half < 1) half = 1;
		if (bottom - top < 1) bottom = top + 1;

		uiLine(cx, yh, cx, yl, color);
		uiBoxFull(cx - half, top, cx + half, bottom, color);
	}

	lastX = x + (int)(spacing * (n - 0.5f));
	lastY = baseY - (int)((float)(h - 8) * (predLast - minv) / (maxv - minv));

	for (i = 1; i <= 5; ++i)
	{
		int px = x + (int)(spacing * (n - 0.5f + i));
		int py = baseY - (int)((float)(h - 8) * (predLast + slope * i - minv) / (maxv - minv));

		uiLine(lastX, lastY, px, py, COLOR_PRED);
		lastX = px;
		lastY = py;
	}
}

static void drawVolume(COMPANY* c, int x, int y, int w, int h)
{
	int i;
	int n = c->barCount < 20 ? c->barCount : 20;
	float spacing = (float)w / (float)(n + 6);
	int maxv = 1;

	for (i = 0; i < n; ++i)
	{
		int v = c->bars[barIndexAt(c, i, n)].vol;
		if (v > maxv) maxv = v;
	}

	for (i = 0; i < n; ++i)
	{
		BAR* b = &c->bars[barIndexAt(c, i, n)];
		int cx = x + (int)(spacing * (i + 0.5f));
		int half = (int)(spacing * 0.3f);
		int bh = (h - 2) * b->vol / maxv;
		unsigned int color = b->c >= b->o ? COLOR_UP : COLOR_DOWN;

		if (half < 1) half = 1;
		if (bh < 1) bh = 1;

		uiBoxFull(cx - half, y + h - bh, cx + half, y + h, color);
	}
}

static void drawCell(int cellX, int cellY, int cellW, int cellH, int companyIndex, int selected)
{
	COMPANY* c = &game->company[companyIndex];
	char buffer[128];
	unsigned int chgColor = c->change > 0 ? COLOR_UP : (c->change < 0 ? COLOR_DOWN : COLOR_DIM);

	uiBoxFull(cellX, cellY, cellX + cellW, cellY + cellH, MoonRGBA(24, 30, 40, 255));
	uiBox(cellX, cellY, cellX + cellW, cellY + cellH, selected ? COLOR_TITLE : COLOR_BORDER);

	text(c->name, cellX + 12, cellY + 6, selected ? COLOR_TITLE : COLOR_TEXT);

	snprintf(buffer, sizeof(buffer), "%d", c->price);
	text(buffer, cellX + 190, cellY + 6, COLOR_TEXT);

	snprintf(buffer, sizeof(buffer), "%+d%%", c->change);
	text(buffer, cellX + 300, cellY + 6, chgColor);

	snprintf(buffer, sizeof(buffer), "HOLD %d", c->shares);
	text(buffer, cellX + 420, cellY + 6, COLOR_DIM);

	uiLine(cellX + 12, cellY + 44, cellX + cellW - 12, cellY + 44, COLOR_GRID);

	drawKLine(c, cellX + 12, cellY + 52, 380, 150);
	drawVolume(c, cellX + 12, cellY + 208, 380, 46);

	snprintf(buffer, sizeof(buffer), "VALUE %lld", (long long)c->shares * c->price);
	text(buffer, cellX + 410, cellY + 60, COLOR_TEXT);

	snprintf(buffer, sizeof(buffer), "DAY %d", game->day);
	text(buffer, cellX + 410, cellY + 100, COLOR_DIM);

	snprintf(buffer, sizeof(buffer), "PRED %+.1f", trendSlope(c, c->barCount < 20 ? c->barCount : 20) * 5.f);
	text(buffer, cellX + 410, cellY + 140, COLOR_PRED);
}


/* 结束时的收益曲线：每条曲线是 (净值/起始净值 - 1) 的百分比 */
static float logOf(long long v)
{
	return logf(v > 0 ? (float)v : 1.f);
}

static void drawEquityChart()
{
	static const unsigned int colors[1 + NPC_COUNT] = {
		MoonRGBA(240, 240, 240, 255),
		MoonRGBA(255, 210, 90, 255),
		MoonRGBA(235, 90, 90, 255),
		MoonRGBA(90, 200, 120, 255),
		MoonRGBA(190, 130, 240, 255),
		MoonRGBA(110, 170, 250, 255)
	};
	static const char* names[1 + NPC_COUNT] = { "YOU", "CAPITALIST", "SPECULATOR", "SMART GUY", "GAMBLER", "QUANT" };
	int k, d;
	long long hi = 10000;
	long long lo = 1000;
	int x0 = 30, x1 = 1250, y0 = 60, y1 = 690;
	float llo, lhi;

	for (k = 0; k <= NPC_COUNT; ++k)
		for (d = 0; d < 365; ++d)
			if (game->equity[k][d] > hi)
				hi = game->equity[k][d];

	/* 把上限抬到下一个 1/2/5 × 10^n 刻度，保证最高一条曲线不会顶到框外 */
	{
		long long base = 1000;
		long long pick = base;

		while (1)
		{
			long long mult[3] = { 1, 2, 5 };
			int m;
			int done = 0;

			for (m = 0; m < 3; ++m)
			{
				long long v = base * mult[m];

				if (v >= hi)
				{
					pick = v;
					done = 1;
					break;
				}
			}

			if (done || base > hi * 10)
				break;
			base *= 10;
		}

		hi = pick;
	}

	llo = logOf(lo);
	lhi = logOf(hi);

	/* 背景 */
	uiBoxFull(x0, y0, x1, y1, MoonRGBA(18, 24, 34, 255));

	/* 竖线：每 5 个交易日 */
	for (d = 0; d <= 364; d += 5)
	{
		int gx = x0 + (int)((long long)(x1 - x0) * d / 364);

		uiLine(gx, y0, gx, y1, MoonRGBA(38, 48, 62, 255));
	}

	/* 横线：对数刻度 1/2/5 × 10^n */
	{
		long long base = 1000;

		while (base <= hi)
		{
			long long mult[3] = { 1, 2, 5 };
			int m;

			for (m = 0; m < 3; ++m)
			{
				long long v = base * mult[m];
				int gy;

				if (v > hi)
					continue;

				gy = y1 - (int)((logOf(v) - llo) / (lhi - llo) * (y1 - y0));
				if (gy < y0 || gy > y1)
					continue;

				uiLine(x0, gy, x1, gy, v == 1000 ? MoonRGBA(70, 85, 105, 255) : MoonRGBA(38, 48, 62, 255));

				{
					char buf[32];
					snprintf(buf, sizeof(buf), "%lld", v / 1000);
					text(buf, x0 + 8, gy - 8, COLOR_DIM);
				}
			}
			base *= 10;
		}
	}

	uiBox(x0, y0, x1, y1, COLOR_BORDER);

	/* 曲线：对数纵轴 */
	for (k = 0; k <= NPC_COUNT; ++k)
	{
		int px = -1, py = 0, started = 0;

		for (d = 0; d < 365; ++d)
		{
			int nx, ny;
			long long v = game->equity[k][d];

			if (v <= 0)
				continue;
			if (v < lo) v = lo;

			nx = x0 + (int)((long long)(x1 - x0) * d / 364);
			ny = y1 - (int)((logOf(v) - llo) / (lhi - llo) * (y1 - y0));
			if (ny < y0) ny = y0;
			if (ny > y1) ny = y1;

			if (started && px >= 0)
				uiLine(px, py, nx, ny, colors[k]);

			px = nx;
			py = ny;
			started = 1;
		}
	}

	/* 图例：左下角，两行三列，用缩写 */
	{
		static const char* shortNames[1 + NPC_COUNT] = { "YOU", "CAP", "SPEC", "SMART", "GAMB", "QUANT" };
		int i;

		uiBoxFull(40, 596, 470, 682, MoonRGBA(18, 24, 34, 255));

		for (i = 0; i <= NPC_COUNT; ++i)
		{
			int cx = 56 + (i % 3) * 140;
			int cy = 604 + (i / 3) * 40;

			uiLine(cx, cy + 12, cx + 22, cy + 12, colors[i]);
			text(shortNames[i], cx + 30, cy, colors[i]);
		}
	}

}

static void drawGameScreen()
{
	char buffer[128];
	int slot;

	text("STOCK MARKET TYCOON", 40, 20, COLOR_TITLE);

	snprintf(buffer, sizeof(buffer), "DAY %d / 365", game->day);
	text(buffer, 300, 20, COLOR_TEXT);

	snprintf(buffer, sizeof(buffer), "CASH %d", game->money);
	text(buffer, 520, 20, COLOR_TEXT);

	snprintf(buffer, sizeof(buffer), "NET %d", game->netWorth);
	text(buffer, 760, 20, COLOR_TEXT);

	snprintf(buffer, sizeof(buffer), "PAGE %d/%d", game->page + 1, PAGE_COUNT);
	text(buffer, 980, 20, COLOR_DIM);

	snprintf(buffer, sizeof(buffer), "MOOD %+0.1f", game->sentiment * 100.f);
	text(buffer, 1120, 20, game->sentiment >= 0 ? COLOR_UP : COLOR_DOWN);

	if (game->newsText)
	{
		if (game->newsIndex >= 0)
			snprintf(buffer, sizeof(buffer), "%s  %+d%%  (%s)",
				game->newsText, game->newsPercent, game->company[game->newsIndex].name);
		else
			snprintf(buffer, sizeof(buffer), "%s  %+d%%  (MARKET WIDE)",
				game->newsText, game->newsPercent);

		text(buffer, 40, 60, game->newsPercent > 0 ? COLOR_UP : COLOR_DOWN);
	}
	else
	{
		text("NEWS: a quiet day, no major headlines", 40, 60, COLOR_DIM);
	}

	for (slot = 0; slot < PAGE_SIZE; ++slot)
	{
		int idx = game->page * PAGE_SIZE + slot;
		int cx = 40 + (slot % 2) * 620;
		int cy = 110 + (slot / 2) * 330;

		drawCell(cx, cy, 600, 320, idx, slot == game->slot);
	}

	if (game->showBoard)
		drawLeaderboard();

	if (game->gameOver)
	{
		static const char* items[3] = { "RESTART", "MAIN MENU", "QUIT" };
		int i;

		text("FINAL RESULTS", 720, 90, COLOR_TITLE);

		drawEquityChart();

		snprintf(buffer, sizeof(buffer), "FINAL NET WORTH: %d", game->netWorth);
		text(buffer, 60, 705, COLOR_TEXT);

		if (game->netWorth > game->startMoney)
			text("YOU BEAT THE MARKET.", 560, 705, COLOR_UP);
		else
			text("THE MARKET BEAT YOU.", 560, 705, COLOR_DOWN);

		for (i = 0; i < 3; ++i)
		{
			int bw = 300;						/* 每个选项的框宽 */
			int bx = 360 + i * 340;
			int len = (int)strlen(items[i]);
			int tx = bx + (bw - len * CHAR_W) / 2;	/* 文字在框内居中 */

			if (i == game->menuIndex)
				uiBoxFull(bx, 745, bx + bw, 795, MoonRGBA(52, 84, 140, 255));

			text(items[i], tx, 755, i == game->menuIndex ? COLOR_TITLE : COLOR_DIM);
		}
	}
}

/* 排行榜：标题 + 玩家 + NPC，按字符尺寸算好面板大小 */
static void drawLeaderboard()
{
	char lines[NPC_COUNT + 2][64];
	int rows = NPC_COUNT + 2;
	int i;
	int maxLen = 0;
	int rowH = 40;
	int pad = 20;
	int w, h, x, y;

	snprintf(lines[0], sizeof(lines[0]), "RANKING");
	snprintf(lines[1], sizeof(lines[1]), "YOU%18d", game->netWorth);

	for (i = 0; i < NPC_COUNT; ++i)
		snprintf(lines[i + 2], sizeof(lines[i + 2]), "%s%*d",
			game->npc[i].name, (int)(18 - strlen(game->npc[i].name)), game->npc[i].netWorth);

	for (i = 0; i < rows; ++i)
	{
		int len = (int)strlen(lines[i]);
		if (len > maxLen) maxLen = len;
	}

	w = maxLen * CHAR_W + pad * 2;
	h = rows * rowH + pad * 2;
	x = (1280 - w) / 2;
	y = (800 - h) / 2;

	uiBoxFull(x, y, x + w, y + h, COLOR_PANEL);
	uiBox(x, y, x + w, y + h, COLOR_BORDER);

	for (i = 0; i < rows; ++i)
		text(lines[i], x + pad, y + pad + i * rowH, i == 0 ? COLOR_TITLE : (i == 1 ? COLOR_TEXT : COLOR_DIM));
}


/* 主菜单两侧的小动画：左边滚动 K 线，右边滚动行情条 */
static int g_animFrame = 0;

static void drawMenuDecor()
{
	static const char* tick[8] = {
		"NVIDIA  +2.4", "APPLE   -0.8", "TESLA   +1.9", "CATL    +0.6",
		"MSFT    -1.2", "HUAWEI  +3.1", "CHANEL  -0.4", "IBM     +0.9"
	};
	int i;

	/* 左侧：滚动的小 K 线 */
	for (i = 0; i < 9; ++i)
	{
		int x = 50 + i * 26;
		float a = sinf((g_animFrame * 0.03f) + i * 0.8f);
		float b = sinf((g_animFrame * 0.03f) + (i - 1) * 0.8f);
		int top = 430 - (int)(a * 70.f);
		int bot = 430 - (int)(b * 70.f);
		int hi = (top < bot ? top : bot) - 18;
		int lo = (top < bot ? bot : top) + 18;
		unsigned int c = (top <= bot) ? COLOR_UP : COLOR_DOWN;
		int bodyTop = (top < bot) ? top : bot;
		int bodyH = abs(bot - top);

		if (bodyH < 6) bodyH = 6;

		uiLine(x + 5, hi, x + 5, lo, c);
		uiBoxFull(x, bodyTop, x + 11, bodyTop + bodyH, c);
	}

	{
		int label = 270 + (int)(sinf(g_animFrame * 0.02f) * 6.f);
		text("LIVE FEED", 60, label, COLOR_DIM);
		text("MARKET", 60, label + 34, COLOR_DIM);
	}

	/* 右侧：滚动的行情条 */
	{
		int offset = (g_animFrame / 2) % 64;

		for (i = 0; i < 10; ++i)
		{
			int y = (i * 32 - offset + 640) % 640;
			int idx = i % 8;

			if (y < 230 || y > 640)
				continue;

			text(tick[idx], 1040, y, strchr(tick[idx], '+') ? COLOR_UP : COLOR_DOWN);
			uiLine(1030, y + 12, 1036, y + 12, COLOR_BORDER);
		}
	}
}

static void drawMainMenu()
{
	static const char* items[MAIN_ITEM_COUNT] = { "EASY   50000 CASH", "NORMAL 20000 CASH", "HARD    5000 CASH", "QUIT" };
	int i;
	int baseY = 320;

	drawMenuDecor();

	textBig("STOCK MARKET TYCOON", 470, 120, COLOR_TITLE);
	text("a tiny stock trading simulator", 430, 190, COLOR_DIM);
	text("SELECT DIFFICULTY", 500, 260, COLOR_TEXT);

	for (i = 0; i < MAIN_ITEM_COUNT; ++i)
	{
		unsigned int color = (i == game->menuIndex) ? COLOR_TITLE : COLOR_DIM;

		if (i == game->menuIndex)
			uiBoxFull(440, baseY + i * 60 - 6, 840, baseY + i * 60 + 42, MoonRGBA(52, 84, 140, 255));

		text(items[i], 460, baseY + i * 60, color);
	}

	text("UP/DOWN select   RETURN start   H help   ESC quit", 360, 620, COLOR_DIM);
}

static void drawPauseMenu()
{
	static const char* items[PAUSE_ITEM_COUNT] = { "RESUME", "RESTART", "MAIN MENU", "HELP", "QUIT GAME" };
	int i;
	int baseY = 300;

	uiBoxFull(380, 220, 900, 620, COLOR_PANEL);
	uiBox(380, 220, 900, 620, COLOR_BORDER);

	text("PAUSED", 600, 250, COLOR_TITLE);

	for (i = 0; i < PAUSE_ITEM_COUNT; ++i)
	{
		unsigned int color = (i == game->menuIndex) ? COLOR_TITLE : COLOR_DIM;

		if (i == game->menuIndex)
			uiBoxFull(420, baseY + i * 60 - 6, 860, baseY + i * 60 + 42, MoonRGBA(52, 84, 140, 255));

		text(items[i], 440, baseY + i * 60, color);
	}
}

static void drawHelpScreen()
{
	textBig("HELP", 600, 80, COLOR_TITLE);

	text("GAME", 120, 170, COLOR_TITLE);
	text("LEFT / RIGHT   move, at the edge it turns page", 140, 210, COLOR_TEXT);
	text("UP / DOWN      pick top or bottom stock", 140, 245, COLOR_TEXT);
	text("ENTER          go to next trading day", 140, 280, COLOR_TEXT);
	text("TAB            show / hide ranking", 140, 315, COLOR_TEXT);
	text("ESC            pause menu", 140, 350, COLOR_TEXT);

	text("TRADE", 120, 410, COLOR_TITLE);
	text("1 / 2          buy 1 share / sell 1 share", 140, 450, COLOR_TEXT);
	text("3 / 4          buy 50 shares / sell 50 shares", 140, 485, COLOR_TEXT);
	text("5 / 6          buy with all cash / sell all holdings", 140, 520, COLOR_TEXT);

	text("READING THE CHART", 120, 580, COLOR_TITLE);
	text("candles: red = close up, green = close down; volume bars below", 140, 620, COLOR_TEXT);
	text("blue line: linear-regression forecast of the next 5 days", 140, 655, COLOR_TEXT);

	text("ESC / ENTER back", 520, 720, COLOR_DIM);
}

extern MOON_PROJECTMODULE(StockDrawLoad)
{
	MOON_IMAGE* image = MOON_NULL;

	MoonHashFindEntity("ProjectBitmap", MOON_IMAGE, image_2);
	image = image_2;

	screen = image;

	MoonHashFindEntity("GameBitmap", MOON_IMAGE, canvas_2);
	canvas = canvas_2;

	MoonHashFindEntity("shader_crt", unsigned int, shader_2);
	shaderCrt = shader_2;

	MoonHashFindEntity("StockGame", STOCK_GAME, game_2);
	game = game_2;

	backbuffer = canvas;

	return 1;
}

extern MOON_PROJECTMODULE(StockDraw)
{
	if (!backbuffer || !game || !canvas || !shaderCrt)
		return 1;

	latchKeys();
	g_animFrame++;

	clearScreen();

	switch (game->screen)
	{
	case SCREEN_MAIN:
		drawMainMenu();
		break;
	case SCREEN_PAUSE:
		drawGameScreen();
		drawPauseMenu();
		break;
	case SCREEN_HELP:
		drawHelpScreen();
		break;
	default:
		drawGameScreen();
		break;
	}

	/* 离屏画布 -> 窗口，走 CRT 着色器 */
	{
		MOON_METADATA blit = { 0 };
		MOON_UNIFORM_DATA uniform;

		blit.draw.image_goal = screen;
		blit.draw.image.image_resources = canvas;
		blit.draw.image.x = 0;
		blit.draw.image.y = 0;
		blit.draw.image.width = 1800;
		blit.draw.image.height = 900;
		blit.draw.shader = *shaderCrt;
		MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE, blit);

		uniform.type = MOON_UNIFORM_TYPE_VECTOR1_FLOAT;
		uniform.vec_float.x = (float)clock() / (float)CLOCKS_PER_SEC;
		MoonShaderUniform(*shaderCrt, "u_time", &uniform);
	}

	return 1;
}
