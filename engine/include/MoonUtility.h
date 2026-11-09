#include"MoonType.h"

//------------------------------------基础工具函数-----------------------------------------------//

/*
* 函數 MoonHash
* 作用 求字符串的Hash值
* 使用方法
* char arr[12] = "Hello_World";
* int alpha = Moonhash(arr)
*/
extern unsigned int MoonHash(char* text);

/*
* 函數 DegRad
* 作用 求弧度值
* 使用方法
* float alpha = cos(DegRad(45));
*/
extern float MoonDegRad(float phi); //角度转弧度

/*
* 函數 MoonKeyState
* 作用 獲取硬件狀態
* 使用方法
* unsigned char alpha = MoonKeyState(MOON_KEY_SPACE);
*/
extern unsigned char MoonKeyState(MOON_KEY_TYPE Key);//获取按键的值

/*
* 函數 MoonKeyReal
* 作用 獲取硬件狀態
*     即时检测
* 使用方法
* unsigned char alpha = MoonKeyReal(MOON_KEY_MOUSE_LEFT);
*/
extern unsigned char MoonKeyReal(MOON_KEY_TYPE Key);//获取按键的值

/*
* 函數 Lerp
* 作用 線性插值
* 使用方法
* float alpha = Lerp(0, 1, .5f);
*/
#define MoonLerp(alpha, beta, t) ((1.f - (t)) * (alpha) + (t) * (beta))				//线性插值

/*
* 函數 Moonmax
* 作用 限制範圍
* 使用方法
* int alpha = Moonmax(0, 100);
*/
#define MoonMax(a,b) (((a) > (b)) ? (a) : (b))

/*
* 函數 Moonmin
* 作用 限制範圍
* 使用方法
* int alpha = Moonmin(0, 100);
*/
#define MoonMin(a,b) (((a) < (b)) ? (a) : (b))

/*
* 函數 RANGE
* 作用 限制範圍
* 使用方法
* int alpha = MoonRange(beta, 0, 100);
*/
#define MoonRange(alpha, alpha_min, alpha_max) (MoonMin(MoonMax(alpha, alpha_min), alpha_max))//限制范围

/*
* 函數 Clamp
* 作用 限制範圍
* 使用方法
* int alpha = MoonClamp(beta, 0, 100);
*/
#define MoonClamp(alpha, alpha_min, alpha_max) (MoonMin(MoonMax(alpha, alpha_min), alpha_max))//限制范围

/*
* 函數 MoonSetMouse
* 作用 設置窗口输入中光标的状态
* 使用方法
* MoonSetMouse(x, y);
*/
extern void MoonSetMouse(MOON_CURSOR_MODE mode);

/*
* 函數 MoonRandom
* 作用 獲取隨機數
* 使用方法
* 种子只需要设置一次,不用担心重复设置的问题,函数内部会检查种子是否改变
* 区间为半开区间[start, end)
* int alpha = MoonRandom(0, 0, 100);
*/
extern int MoonRandom(unsigned int seed, int start, int end);

/*
* 函數 MoonMatrix4_4Mul
* 作用  4 * 4 矩阵乘法,列主序
*		如果mat4_return传入mat4_left或者mat4_right,不影响最终的值
* 使用方法
* MoonMatrix4_4Mul(mat4_return, mat4_left, mat4_right);
*/
extern unsigned char MoonMatrix4_4Mul(float* mat4_return, float* mat4_left, float* mat4_right);

/*
* 函數 MoonGetFps
* 作用 获取Fps
* 使用方法
* int fps = MoonGetFps();
*/
extern int MoonGetFps();

/*
* 函數 MoonTriangleDetection
* 作用 檢測點與三角形的位置關係
* 使用方法
* MOON_POINT2D point[4] =
* {
*	{0,0},
*	{10,0},
*	{0,10},
*	{5,50}
* };
* unsigned char = MoonTriangleDetection(point[0], point[1], point[2], point[3]);
*/
extern int MoonTriangleDetection(MOON_POINT2D a, MOON_POINT2D b, MOON_POINT2D c, MOON_POINT2D p);	//三角形碰撞检测

/*
* 函數 MoonString
* 作用 字符串拼接
* 使用方法
* MoonString(str)
*/
#define MoonString(str)	""#str

/*
* 函數 MoonRGBA
* 作用 写入颜色值
* 使用方法
* MoonRGBA(r,g,b,a)
*/
#define MoonRGBA(r,g,b,a) ((r)|((g) << 8)|((b) << 16)|((a) << 24))

/*
* 函數 MoonCursorOffect
* 作用 获取光标偏移值并限制在区域内部
* 使用方法
* MOON_POINT2D a = MoonCursorOffect(size);
*/
extern MOON_POINT2D MoonCursorOffect(MOON_POINT2D size);

/*
* 函數 MoonCursorGet
* 作用 获取光标偏移值位置
* 使用方法
* MOON_POINT2D a = MoonCursorGet();
*/
extern MOON_POINT2D MoonCursorGet();

/*
* 函數 MoonSetPower
* 作用 设置高性能模式
* 使用方法
* MoonSetPower(TRUE);
*/
extern void MoonSetPower(unsigned char power);

/*
* 函數 MoonRoundOff
* 作用 获取四舍五入的值
* 使用方法
* int a = MoonRoundOff(.5f);
* printf("a:%d",a);
* a:1
*/
extern int MoonRoundOff(float f);

/*
* 函數 MoonCtz
* 作用 计算末尾0的数量
* 使用方法
*	unsigned int alpha = MoonCtz((unsigned int)0b11000);
*/
extern unsigned int MoonCtz(unsigned int value);

//------------------------------------音乐函数--------------------------------------------------//


/*
* 函數 MoonMusicSet
* 作用 设置音樂播放状态
* 使用方法
* MoonMusicSet(music, false, 0, 1);
*/
extern void MoonMusicSet(MOON_MUSIC* music, MOON_MUSIC_MODE on_or_off, float start, float end);

/*
* 函數 MoonMusicAgain
* 作用 设置音樂播放状态,再次播放
* 使用方法
* MoonMusicAgain(music);
*/
extern void MoonMusicAgain(MOON_MUSIC* music);

/*
* 函數 MoonMusic
* 作用 播放音樂
* 使用方法
*   MOON_MUSIC music;
*   music.id = 0;
*   music.start = 0.3f;			// 从 30% 位置开始
*   music.end = 0.8f;			// 到 80% 位置结束
*   music.loop = MOON_TRUE;		//开启循环
* MoonMusic(&music);
*/
extern int MoonMusic(MOON_MUSIC* music);//播放音乐

/*
* 函數 MoonMusicInit_Wav
* 作用 初始化音樂
* 使用方法
* MoonMusicInit_Wav(music, "music.mp3");
*/
extern unsigned char MoonMusicInit_Wav(MOON_MUSIC* music, const char* File);

//------------------------------------自动内存--------------------------------------------------//


/*
* 函數 MoonAlloc
* 作用 内存分配,结束时会自动回收
* 使用方法
* 支持
* "malloc"
* "calloc"
* "realloc"
* MoonAlloc(ptr,sizeof(type),num,"malloc");
*/
extern unsigned char MoonAlloc(void** ptr, size_t size_len, unsigned int num, const char* alloc);

/*
* 函數 MoonFree
* 作用 提前将注册的内存回收
*		注意回收后仍在使用的野指针
* 使用方法
* MoonFree(ptr);
*/
extern unsigned char MoonFree(void* ptr);

//------------------------------------定时函数--------------------------------------------------//


/*
* 函數 MoonTimeLoadInit
* 作用 初始化定時器
* 使用方法
* TIMELOAD load;
* MoonTimeLoadInit(&load, 100);
*/
extern void MoonTimeLoadInit(MOON_TIMELOAD* Timeload, int load);		//初始化定时器

/*
* 函數 MoonTimeLoad
* 作用 運行定時器
* 使用方法
* MoonTimeLoad(&load, 1);
*/
extern int MoonTimeLoad(MOON_TIMELOAD* Timeload, int mode);//运行定时器

/*
* 函數 MoonSleep
* 作用 暫停
* 使用方法
* MoonSleep(1);
*/
extern int MoonSleep(int timeload);					//暂停

//------------------------------------实体函数--------------------------------------------------//


/*
* 函數 MoonFindEntity
* 作用 尋找實體
* 使用方法
* int* entity = (int*)MoonFindEntity((char*)nameid)
*/
extern void* MoonFindEntity(char* nameid);	//寻找实体

/*
* 函數 MoonFindEntity_Id
* 作用 尋找實體,通过id寻找
* 使用方法
* int* entity = (int*)MoonFindEntity_Id(unsigned int id)
*/
extern void* MoonFindEntity_Id(unsigned int id);	//寻找实体

/*
* 函數 HashFindEntity
* 作用 hash寻找实体
* 使用方法
* HashFindEntity("ProjectBitmap", int, engineback);
*/
#define MoonHashFindEntity(nameid, type, entity) type* entity = (type*)MoonFindEntity((char*)nameid)//hash寻找实体

/*
* 函數 MoonCreateEntityIndex
* 作用 注册实体
* 使用方法
* static TIMELOAD logictps;
* MoonCreateEntityIndex(&logictps, "LogicTps", sizeof(TIMELOAD));
*/
extern int MoonCreateEntityIndex(void* arrentity, char* nameid, size_t size_len, char* type_name);//注册实体

//------------------------------------图像操作函数------------------------------------------------//


/*
* 函數 MoonCreateImage
* 作用 创建双缓冲绘图绘图区
* 使用方法
* IMAGE buffer;
* MoonCreateImage(&buffer, window_width, window_height);
*/
extern void MoonImageCreate(MOON_IMAGE* image, int bmpwidth, int bmpheight);	//创建双缓冲绘图绘图区

/*
* 函數 MoonDeletImage
* 作用 删除双缓冲绘图绘图区/紋理
* 使用方法
* MoonDeletImage(bitmap);
*/
extern void MoonImageDelete(MOON_IMAGE* image);//删除双缓冲绘图绘图区

/*
* 函數 MoonImageLoad
* 作用 加載BMP圖片到紋理中
* 使用方法
* const wchar_t* files[] = {L"1.bmp", L"2.bmp"};
* MoonImageLoad(&image, files, 2);
*/
extern void MoonImageLoad(MOON_IMAGE* image, const char** imagefile, int imagenumber);	//加载图片

/*
* 函數 MoonImageLoadBatch
* 作用 批量創建紋理並加載圖片
* 使用方法
* IMAGE frames[10];
* const wchar_t* names[] = {L"frame1.bmp", L"frame2.bmp", ...};
* MoonImageLoadBatch(frames, 10, names, 64, 64);
*/
extern void MoonImageLoadBatch(MOON_IMAGE* image, int totalnumber, const char** name, int width, int height);//批量加载图片

//------------------------------------多线程函数------------------------------------------------//


/*
* 函數 创建多线程函数
* 作用 MOON_CREATETHREADFUNCTION
* 使用方法
* MOON_CREATETHREADFUNCTION(Thread);
*/
#define MOON_CREATETHREADFUNCTION(NAME)       MOON_THREAD NAME(void* lparam)//创建多线程函数

/*
* 函數 CREATETHREAD
* 作用 创建并运行多线程函数
* 使用方法
* CREATETHREAD(Thread, context);
*/
extern void MoonThreadCreate(MOON_THREAD(*fuction)(void*), const char* name, void* resource);
#define MOON_CREATETHREAD(fuction, name, resource)   MoonThreadCreate(fuction, (const char*)name, (void*)resource);//创建并运行多线程函数

/*
* 函數 GETTHREADRESOURCE
* 作用 获取多綫程函數外部导入的资源
* 使用方法
* static MOON_CREATETHREADFUNCTION(Thread)
* {
*	GETTHREADRESOURCE(int*,int);
*	return 1;
* }
*/
#define MOON_GETTHREADRESOURCE(type, resource) type resource = (type)lparam;	//获取外部导入的资源

//------------------------------------字符串操作函数------------------------------------------------//


/*
* 函數 MoonCharToWchar
* 作用 字符转换
* 使用方法
* wchar_t* text1;
* char* text2 = "Hello";
* MoonCharToWchar(text1, text2, 5);
*/
extern int MoonCharToWchar(wchar_t* text1, char* text2, int len);//字符转换

/*
* 函數
* 作用 字符转换
* 使用方法
* char* text1;
* wchar_t* text2 = L"Hello";
* MoonCharToWchar(text1, text2, 5);
*/
extern int MoonWcharToChar(char* text1, wchar_t* text2, int len);//字符转换

/*
* 函數 MoonStrMatch_Prefix
* 作用 匹配字符串前缀
*/
extern unsigned int MoonStrMatch_Prefix(const char* str_1, const char* str_2);

/*
* 函數 MoonStrMatch_PrefixIgnore
* 作用 匹配字符串前缀,忽略特定字符
* 使用方法
*	MoonStrMatch_PrefixIgnore(str,str_2,'\n');
*/
extern unsigned int MoonStrMatch_PrefixIgnore(const char* str_1, const char* str_2, char ch);

/*
* 函數 StrMatch_PrefixIgnoreStr
* 作用 忽略字符,比较时忽略 igno_str 中出现的任意字符
* 使用方法
*	MoonStrMatch_PrefixIgnoreStr(str,str_2, "\n \t");
*/
extern unsigned int MoonStrMatch_PrefixIgnoreStr(const char* str_1, const char* str_2, const char* igno_str);

/*
* 函數 MoonStrMatch_Replace
* 作用 替换字符
* 使用方法
*	MoonStrMatch_Replace(str,start,len,'\n', ' ');
*/
extern void MoonStrMatch_Replace(char* str, unsigned int start_index, unsigned int len, char ch_goal, char ch_replace);

/*
* 函數 MoonStrStr
* 作用 是strstr的封装,但是返回索引
* 使用方法
*	MoonStrStr(str_scource,str);
*/
extern int MoonStrStr(const char* str_scource, const char* str);

/*
* 函數 MoonStrReverse
* 作用 反转字符串
* 使用方法
*	MoonStrReverse(str_scource, str_len);
*/
extern int MoonStrReverse(char* str, unsigned int len);


//------------------------------------文件处理------------------------------------------------//


/*
* 函數 MoonFileLoad_TEXT
* 作用 加载文本文件
* 使用方法
*	MoonFileLoad_TEXT("a.txt", text, strlen(text));
*/
extern unsigned char MoonFileLoad_TEXT(const char* file_name, char* text, unsigned int text_size);

/*
* 函數 MoonFileRead_TEXT
* 作用 加载文本文件并存储到file
* 使用方法
*	MoonFileRead_TEXT(file, name);
*/
extern unsigned char MoonFileRead_TEXT(MOON_FILE* file, const char* file_name);

/*
* 函數 MoonFileRead_Line
* 作用 读取已经加载的文本文件的任意一行
* 使用方法
*	MoonFileRead_Line(file, text, 1);
*/
extern unsigned int MoonFileRead_Line(MOON_FILE* file, char* file_buffer, unsigned int line);

/*
* 函數 MoonFileClose
* 作用 提前手动回收文件
* 使用方法
*	MoonFileClose(file);
*/
extern unsigned int MoonFileClose(MOON_FILE* file);

/*
* 函數 MoonLogCreate
* 作用 生成日志
* 使用方法
*	MoonLogCreate(file,(char*)"hello world");
*/
extern int MoonLogCreate(const char* file_name, char* text);

//------------------------------------按钮控件------------------------------------------------//

/*
* 函數 MoonButtonInit
* 作用 初始化按鈕控件
* 使用方法
* MOONBUTTON btn;
* MoonButtonInit(&btn, 10, 10, 100, 50);
*/
extern int MoonButtonInit(MOON_BUTTON* button, int x, int y, int width, int height);		//初始化按钮

/*
* 函數 MoonButtonDetection
* 作用 檢測按鈕觸發狀態
* 使用方法
* if(MoonButtonDetection(button, x, y, context) { ... }
*/
extern int MoonButtonDetection(MOON_BUTTON* button, int x, int y, void* context);

/*
* 函數 MoonButtonSetTriggerMode
* 作用 更改按鈕觸發方式
* 使用方法
* MoonButtonSetTriggerMode("MyButton", MOON_BUTTONPRESS);
*/
extern int MoonButtonSetTriggerMode(MOON_BUTTON* button, unsigned int key);//更改触发方式

/*
* 函數 MOON_BUTTON_CREATE
* 作用 便捷注冊按鈕
* 使用方法
* static MOONBUTTON button
* MOONBUTTONCREATE(name, button, x, y, w, h, 0, 0, 0);
*/
#define MOON_BUTTON_CREATE(name, button, x, y, width, height, Press, PressL, Hover, False)  \
{\
	MoonButtonInit(&button,(x), (y), (width), (height));\
	MoonButtonSetTriggerMode(&button,MOON_KEY_MOUSE_LEFT);\
	button.ButtonModeHover = (Hover);\
	button.ButtonModePress = (Press);\
	button.ButtonModePressL = (PressL);\
	button.ButtonModeFalse = (False);\
	MoonCreateEntityIndex(&button, (char*)name, sizeof(MOON_BUTTON), "MOON_BUTTON");\
}
//------------------------------------着色器函数------------------------------------------------//

/*
* 注意!這個函數對您的代碼可能沒有任何作用!
* 如果要修改着色器,请如下操作來實現着色器重载 (Shader Hot-reloading),并且通過傳入的
* 函數 MoonShaderLoad
* 作用 初始化所有内部繪圖函數
* 使用方法
* MoonShaderLoad(shader, shader, program);
*/
extern void MoonShaderLoad(char** vertex_shader, char** pixel_shader, unsigned int* shader_program);//初始化所有内部繪圖函數

/*
* 注意!這個函數對您的代碼可能沒有任何作用!
* 函數 MoonShaderUniform
* 作用 OpenGL的uniform函数
* 使用方法
* MoonShaderUniform(shader, var, data);
*/
extern void MoonShaderUniform(unsigned int shader, const char* var, MOON_UNIFORM_DATA* data);

//------------------------------------向量计算------------------------------------------------//

/*
使用向量计算时,您不必担心维度的差异导致无法计算
一切都是自动的,且符合数学常识
例如
	1,存在一个二维向量,如果要与三维向量计算,那么低维向量会自动扩容
	2,如果试图读取二维向量的第三维,那么返回0.f
	3,只有当绝对不可能的事情发生时
		比如模长获取失败,那么返回-1.f
*/


/*
* 函數 MoonVectorInit
* 作用 初始化向量
* 使用方法
* MoonVectorInit(vector, arr, 2);
*/
extern unsigned char MoonVectorInit(MOON_VECTOR* vector, float* num, unsigned int num_size);

/*
* 函數 MoonVectorFree
* 作用 释放向量
* 使用方法
* MoonVectorFree(vector);
*/
extern void MoonVectorFree(MOON_VECTOR* vector);

/*
* 函數 MoonVector_Dim
* 作用 返回向量的维度（元素个数）
* 使用方法
* unsigned int dim = MoonVector_Dim(&vector);
*/
extern unsigned int MoonVector_Dim(MOON_VECTOR* vector);

/*
* 函數 MoonVector_Get
* 作用 获取向量中指定维度的元素值（维度从1开始计数），若维度越界则返回0.f
* 使用方法
* float val = MoonVector_Get(&vector, 2);   // 获取第2个元素
*/
extern float MoonVector_Get(MOON_VECTOR* vector, unsigned int dim);

/*
* 函數 MoonVector_SetDim
* 作用 重新设置向量的维度（只能扩容，不能缩容），新增维度自动初始化为0
* 使用方法
* MoonVector_SetDim(&vector, 5);   // 将向量扩容到5维
*/
extern unsigned char MoonVector_SetDim(MOON_VECTOR* vector, unsigned int dim);

/*
* 函數 MoonVector_SetEle
* 作用 设置向量中指定维度的值，若维度超出当前范围则自动扩容
* 使用方法
* MoonVector_SetEle(&vector, 3, 9.9f);   // 将第3维设为9.9
*/
extern unsigned char MoonVector_SetEle(MOON_VECTOR* vector, unsigned int dim, float num);

/*
* 函數 MoonVector_Norm
* 作用 计算向量的L2范数（模长），若向量为空或空指针则返回-1.f
* 使用方法
* float length = MoonVector_Norm(&vector);
*/
extern float MoonVector_Norm(MOON_VECTOR* vector);

/*
* 函數 MoonVector_NormSize
* 作用 将输入向量归一化为单位向量，结果输出到vector_out中（自动管理内存）
* 使用方法
* MoonVector_NormSize(&out, &in);
*/
extern void MoonVector_NormSize(MOON_VECTOR* vector_out, MOON_VECTOR* vector);

/*
* 函數 MoonVector_Add
* 作用 向量加法：vector_out = vector_1 + vector_2，结果维度自动取两者最大值，短向量缺失部分视为0
* 使用方法
* MoonVector_Add(&result, &v1, &v2);
*/
extern void MoonVector_Add(MOON_VECTOR* vector_out, MOON_VECTOR* vector_1, MOON_VECTOR* vector_2);

/*
* 函數 MoonVector_Sub
* 作用 向量减法：vector_out = vector_1 - vector_2，结果维度自动取两者最大值，短向量缺失部分视为0
* 使用方法
* MoonVector_Sub(&result, &v1, &v2);
*/
extern void MoonVector_Sub(MOON_VECTOR* vector_out, MOON_VECTOR* vector_1, MOON_VECTOR* vector_2);

/*
* 函數 MoonVector_Scale
* 作用 向量的数乘：vector_out = vector * num，维度保持不变
* 使用方法
* MoonVector_Scale(&result, &v, 2.5f);
*/
extern void MoonVector_Scale(MOON_VECTOR* vector_out, MOON_VECTOR* vector, float num);

/*
* 函數 MoonVector_Hadamard
* 作用 哈达玛积（逐元素相乘）：vector_out[i] = vector_1[i] * vector_2[i]，结果维度取最大值，短向量缺失部分视为0
* 使用方法
* MoonVector_Hadamard(&result, &v1, &v2);
*/
extern void MoonVector_Hadamard(MOON_VECTOR* vector_out, MOON_VECTOR* vector_1, MOON_VECTOR* vector_2);

/*
* 函數 MoonVector_Dot
* 作用 计算两个向量的点积（内积），只计算共同维度部分，短向量的溢出部分忽略
* 使用方法
* float dot = MoonVector_Dot(&v1, &v2);
*/
extern float MoonVector_Dot(MOON_VECTOR* vector_1, MOON_VECTOR* vector_2);

//------------------------------------链表/网------------------------------------------------//


/*
* 函數 MoonLinkedCreate
* 作用 插入空的子节点
* 使用方法
* MoonLinkedCreate(&linked, MOON_LINKED_MODE_AFTER);
*/
extern MOON_LINKED_TYPE MoonLinkedCreate(MOON_LINKED* linked, MOON_LINKED_MODE mode);

/*
* 函數 MoonLinkedCreate_Insert
* 作用 插入已存在的节点
* 使用方法
* MoonLinkedCreate_Insert(&linked, MOON_LINKED_MODE_AFTER);
*/
extern MOON_LINKED_TYPE MoonLinkedCreate_Insert(MOON_LINKED* linked, MOON_LINKED_MODE mode);

/*
* 函數 MoonLinkedDelete
* 作用 删除链表节点
* 使用方法
* MoonLinkedDelete(&linked);
*/
extern MOON_LINKED_TYPE MoonLinkedDelete(MOON_LINKED* linked);

/*
* 函數 MoonVector_Copy
* 作用 复制向量，返回新分配的向量指针
* 使用方法
* MOON_VECTOR* copy = MoonVector_Copy(&v1);
*/
extern MOON_VECTOR MoonVector_Copy(MOON_VECTOR* vector);

//------------------------------------编译器------------------------------------------------//
