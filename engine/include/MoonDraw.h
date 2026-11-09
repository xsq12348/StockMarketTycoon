#include"MoonType.h"

//-------------------------------------------------------------------------------------------绘制函数--------------------------------------------------------------------------------//

/*
* 函數 MoonDrawingArea
* 作用 將image_2繪製到image_1上，不帶透明度
* 使用方法
* MoonDrawingArea(&backBuffer, &sprite, 10, 20, 64, 64);
*/
extern void MoonDrawArea(MOON_IMAGE* image_goal, MOON_IMAGE* image_source, int x, int y, int width, int height);//画板

/*
* 函數 MoonDrawingAreaRound
* 作用 將image_2旋轉後繪製到image_1上
* 使用方法
* MoonDrawingAreaRound(&backBuffer, &sprite, 100, 100, 32, 32, 64, 64, 45);
*/
extern void MoonDrawAreaRound(MOON_IMAGE* image_goal, MOON_IMAGE* image_source, int x, int y, float apx, float apy, int width, int height, int deg);//旋转的画板

/*
* 函數 MoonDrawingAreaPlgBit
* 作用 將image_2進行四點透視變換後繪製到image_1上（紋理貼圖）
* 注意事项
* UV坐标如下
	point[0]	0,0
	point[1]	1,0
	point[2]	0,1
	point[3]	1,1
* 使用方法
* MOON_POINT2D points[4] = {{0,0},{100,0},{0,100},{100,100}};
* MoonDrawingAreaPlgBit(&backBuffer, &texture, points, RGB(255,255,255));
*/
extern void MoonDrawAreaPlgBit(MOON_IMAGE* image_goal, MOON_IMAGE* image_source, MOON_POINT2D point[4]);//纹理贴图

/*
* 函數 MoonDrawAreaUV
* 作用 將image_2的指定UV區域繪製到image_1上
* 使用方法
* MoonDrawAreaUV(&backBuffer, &sprite, 10, 20, 64, 64, 0, 0, 32, 32);
*/
extern void MoonDrawAreaUV(MOON_IMAGE* image_goal, MOON_IMAGE* image_source, int x, int y, int width, int height, float uv_x, float uv_y, float uv_width, float uv_height);	//UV纹理贴图

/*
* 函數 MoonPix
* 作用 在圖像上繪製一個像素點
* 使用方法
* MoonPix(&image, 10, 10, RGB(255,0,0));
*/
extern void MoonDrawPix(MOON_IMAGE* image, int x, int y, unsigned int color);//绘制点

/*
* 函數 MoonLine
* 作用 在圖像上繪製線條
* 使用方法
* MoonLine(&image, 0, 0, 100, 100, 2, 0xffffffff);
*/
extern void MoonDrawLine(MOON_IMAGE* image, int x1, int y1, int x2, int y2, unsigned int color);//绘制线

/*
* 函數 MoonDrawBox
* 作用 在圖像上繪製矩形邊框
* 使用方法
* MoonBox(&image, 10, 10, 100, 100, 2, 0xffffffff);
*/
extern void MoonDrawBox(MOON_IMAGE* image, int x1, int y1, int x2, int y2, unsigned int color);//绘制矩形

/*
* 函數 MoonDrawBoxFull
* 作用 在圖像上繪製填充矩形
* 使用方法
* MoonBoxFull(&image, 10, 10, 100, 100, 0xffffffff);
*/
extern void MoonDrawBoxFull(MOON_IMAGE* image, int x1, int y1, int x2, int y2, unsigned int color);//绘制填充矩形

/*
* 函數 MoonDrawBox_Round
* 作用 在圖像上繪製矩形邊框
* 使用方法
* MoonBox(&image, 10, 10, 100, 100, 2, 0xffffffff);
*/
extern void MoonDrawBox_Round(MOON_IMAGE* image, int x1, int y1, int x2, int y2, unsigned int r, unsigned int color);

/*
* 函數 MoonDrawBoxFull_Round
* 作用 在圖像上繪製圆角填充矩形
* 使用方法
* MoonBoxFull(&image, 10, 10, 100, 100, 0xffffffff);
*/
extern void MoonDrawBoxFull_Round(MOON_IMAGE* image, int x1, int y1, int x2, int y2, unsigned int r, unsigned int color);

/*
* 函數 MoonTriFull
* 作用 在圖像上繪製填充矩形
* 使用方法
* MoonTriFull(&image, 10, 10, 100, 100, 200, 200, 0xffffffff);
*/
extern void MoonDrawTriFull(MOON_IMAGE* image, int x1, int y1, int x2, int y2, int x3, int y3, unsigned int color);//绘制三角形

/*
* 函數 MoonDrawCircleFull
* 作用 在圖像上繪製填充圆
* 使用方法
* MoonDrawCircleFull(&image, 10, 10, 200, 0xffffffff);
*/
extern void MoonDrawCircleFull(MOON_IMAGE* image, int x, int y, int r, unsigned int color);//绘制圆

/*
* 函數 MoonDrawCircle
* 作用 在圖像上繪製圆
* 使用方法
* MoonDrawCircle(&image, 10, 10, 200, 0xffffffff);
*/
extern void MoonDrawCircle(MOON_IMAGE* image, int x, int y, int r, unsigned int color);//绘制圆

/*
* 函數 MoonDeawTextFont
* 作用 在圖像上顯示文字
* 使用方法
* MoonDeawTextFont(&image, 10, 10, L"Hello", RGB(255,255,255), TRUE, L"宋體", 24, 24, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, color);
*/
extern void MoonDrawTextFont(MOON_IMAGE* image, const char* text, int x, int y, int sizewidth, int sizeheight, unsigned int color);//显示字符

//------------------------------------动画------------------------------------------------//


/*
* 函數 MoonAnimeInit
* 作用 初始化動畫結構體
* 使用方法
* ANIME anim;
* MoonAnimeInit(&anim, "Walk", frames, 100, 8, 64, 64);
*/
extern int MoonAnimeInit(MOON_ANIME* anime, MOON_IMAGE* sequenceframes, int timeload, int totalnumber, int width, int height);//初始化动画

/*
* 函數 MoonAnimeRun
* 作用 運行動畫，繪製當前幀
* 使用方法
* MoonAnimeRun(&backBuffer, &anim, 1, x, y, 1.0f, 1.0f);
*/
extern int MoonAnimeRun(MOON_IMAGE* image, MOON_ANIME* anime, int animeswitch, int x, int y, int width, int height);//运行动画

/*
* 函數 MoonAnimeDelete
* 作用 刪除動畫並釋放所有序列幀紋理
* 使用方法
* MoonAnimeDelete(&anim);
*/
extern void MoonAnimeDelete(MOON_ANIME* anime);//删除动画

/*
* 函數 MoonAnimeCreate
* 作用 一步創建完整的動畫（批量加載圖片+初始化動畫+註冊實體）
* 使用方法
* IMAGE frames[10];
* ANIME anim;
* const wchar_t* names[] = {L"1.bmp", L"2.bmp", ...};
* MoonAnimeCreate(project, frames, &anim, 10, names, "PlayerAnim", 100, 64, 64);
*/
extern void MoonAnimeCreate(MOON_IMAGE* image, MOON_ANIME* anime, int totalnumber, const char** animename, char* entityname, int timeload, int width, int height);//创建动画
