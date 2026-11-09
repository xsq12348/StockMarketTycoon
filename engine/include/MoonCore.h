#ifndef MOON_CORE_DEF
#define MOON_CORE_DEF
#include <SDL3/SDL.h>
#include<GLAD/glad.h>
#include<GLFW/glfw3.h>
#include"MoonType.h"

/*
注意!
这个文件中的函数您不应该使用!

*/

#define MOON_SAMPLE_RATE 48000		//音频频率

typedef GLFWwindow MOON_HWND;		//句柄

//--------------------------对象--------------------------//

typedef struct
{
	char* nameid;
	char* type_name;
	int length;
	void* entityindex;
}MOON_ENTITYINDEX;					//实体

//--------------------------模块切换--------------------------//

typedef enum
{
	MOON_MODULE_DRAW = 1,
	MOON_MODULE_LOGIC,
	MOON_MODULE_ATTR,
}MOON_MODULE_FUNCTION;

//--------------------------内核音乐数据--------------------------//

typedef struct
{
	float* data;
	int length;
}MOON_CORE_MUSIC;

//--------------------------消息队列单体--------------------------//

typedef struct MOON_MESSAGE_SPECIFIC
{
	unsigned int message;				//消息类型
	MOON_METADATA metadata;				//数据元
}MOON_MESSAGE_SPECIFIC;

//--------------------------消息队列--------------------------//

typedef struct
{
	MOON_MESSAGE_SPECIFIC* message;
	unsigned int message_index;
}MOON_MESSAGE_ALL;

//--------------------------内核唯一单例--------------------------//

typedef struct MOON_ENGINECORE
{
	MOON_HWND* hwnd;			//窗口句柄
	int window_width;			//宽度
	int window_height;			//高度
	unsigned char dead;			//项目状态
	unsigned char thread_message_type_draw;//消息队列状态 MOON_FALSE为可用 MOON_TRUE为不可发送消息
	unsigned char thread_message_type_logic;//消息队列状态 MOON_FALSE为可用 MOON_TRUE为不可发送消息
	//unsigned char thread_message_type_attr;//消息队列状态 MOON_FALSE为可用 MOON_TRUE为不可发送消息	
	char gamepowermode;			//记录高性能模式旧模式
	char power;					//高性能模式
	int focus;					//焦点
	int(*Logic)();				//多线程逻辑函数
	int(*Drawing)();			//主线程绘图函数
	int(*Attr)();				//属性函数
	MOON_ENTITYINDEX* entityindex;		//对象池注册表
	unsigned int entityindex_number;	//对象池注册表数量
	MOON_MESSAGE_ALL message_draw;		//绘制消息队列
	MOON_MESSAGE_ALL message_logic;		//逻辑消息队列
	//MOON_MESSAGE_ALL message_attr;	//属性消息队列
	MOON_TIMELOAD timeload;				//计时器
}MOON_ENGINECORE;

//--------------------------纹理顶点--------------------------//

typedef struct
{
	float x, y, z, uv_x, uv_y;
}MOON_TEXTURE_VECTER;		//用于纹理顶点

//--------------------------图元顶点--------------------------//

//点结构体
typedef struct
{
	float x, y, z;
	float r, g, b, a;
}MOON_GRAPHIC_VECTER;

//--------------------------自动内存分配--------------------------//

typedef void* MOON_ALLOC;
typedef struct
{
	MOON_ALLOC* alloc;
	unsigned int index;
}MOON_ALLOC_REGISTRY;		//用于内部队列

/*
* 函數 MoonWindow
* 作用 創建窗口
* 使用方法
* MOON_HWND* hwnd = MoonWindow(L"MyGame", 100, 100, 800, 600);
*/
extern MOON_HWND* MoonWindow(const char* name, int window_coord_x, int window_coord_y, int window_width, int window_height);					//创建窗口

/*
* 函數 MoonPrompt
* 作用 内部提示
* 使用方法
* MoonPrompt(text);
*/
extern void MoonPrompt(char* text);

/*
* 函數 MoonProjectError
* 作用 錯誤處理函數
* 使用方法
* MoonProjectError(NULL, 1, "Something went wrong!");
*/
extern int MoonProjectError(void* alpha, int degree, char* text);	//错误处理

/*
* 注意!這個函數對你的代碼沒有任何作用!僅僅是引擎内部初始化使用的,你可以使用MoonProjectFunctionSwitch來獲得更好的效果
* 函數 MoonProjectPause
* 作用 暫停/恢復項目運行
* 使用方法
* MoonProjectPause(1, &project->Logic, NULL, NULL);
*/
extern int MoonProjectPause(int mode, int (**function_1)(), int (*function_2)(), int (*function_3)());		//暂停函数

/*
* 注意!這個函數對你的代碼可能沒有任何作用!
* 函數 MoonUtilityFidCore
* 作用 初始化所有内部工具函數
* 使用方法
* MoonUtilityFidCore(core);
*/
extern void MoonUtilityCoreLoad();

/*
* 注意!這個函數對你的代碼可能沒有任何作用!
* 函數 MoonUtilityLoad
* 作用 链接工具库
* 使用方法
* MoonUtilityLoad(core);
*/
extern void MoonUtilityLoad(MOON_ENGINECORE* core);

/*
* 注意!這個函數對你的代碼可能沒有任何作用!
* 函數 MoonUtilityOver
* 作用 清理所有内部资源
* 使用方法
* MoonUtilityOver();
*/
extern void MoonUtilityOver();

/*
* 注意!這個函數對你的代碼可能沒有任何作用!
* 函數 MoonDrawLoad
* 作用 初始化所有内部绘图函數
* 使用方法
* MoonDrawLoad(project);
*/
extern void MoonDrawLoad();

/*
* 注意!這個函數對你的代碼可能沒有任何作用!
* 函數 MoonDrawOver
* 作用 回收绘图模块的资源
* 使用方法
* MoonDrawOver();
*/
extern void MoonDrawOver();

/*
* 注意!這個函數對你的代碼可能沒有任何作用!
* 函數 MoonImageShader
* 作用 切換成開發者設定的Shader
* 使用方法
* MoonImageShader( 0, shader);
*/
extern void MoonImageShader(unsigned int shader);//设置著色器

/*
* 注意!這個函數對你的代碼可能沒有任何作用!
* 函數 MoonImageDesignated
* 作用 手動指定當前的渲染目標紋理
* 使用方法
* MoonImageDesignated(&backBuffer);
*/
extern void MoonImageDesignated(MOON_IMAGE* image);//设置绘图对象

/*
* 注意!這個函數對你的代碼可能沒有任何作用!
* 函數 MoonProjectFunctionSwitch
* 作用 切換項目中的函數指針
*		通过第二个参数,指定需要更换的模块
*		参数为
			MOON_MODULE_DRAW
			MOON_MODULE_LOGIC
* 使用方法
* MoonProjectFunctionSwitch(project, MOON_MODULE_DRAW, NewDrawingFunction);
*/
extern void MoonProjectFunctionSwitch(MOON_MODULE_FUNCTION module, int (*function_2)());//函数切换

/*
* 注意!這個函數對你的代碼可能沒有任何作用!
* 函數 MoonProjectDead
* 作用 在内核快速杀死项目
* 使用方法
* MoonProjectDead();
*/
extern void MoonProjectDead();

/*
* 注意!這個函數對你的代碼可能沒有任何作用!
* 函數 MoonProjectGetMessage
* 作用 获取消息并处理
* 使用方法
* MoonProjectGetMessage(message, handle);
*/
extern int MoonProjectGetMessage(MOON_MESSAGE_ALL* message, unsigned char* type, void(*Handle)(MOON_MESSAGE_ALL*, unsigned char*));	//获取消息
extern void MoonDrawMessageHandle(MOON_MESSAGE_ALL* message, unsigned char* type);	//处理绘制线程消息
extern void MoonlogicMessageHandle(MOON_MESSAGE_ALL* message, unsigned char* type);	//处理逻辑线程消息

//-------------------------------------------------------------------------------------------绘制函数--------------------------------------------------------------------------------//

extern void MoonCoreDrawArea(MOON_TEXTURE_VECTER* vertexs, unsigned int vertex_number, MOON_METADATA* metadata);					//画板
extern int MoonCoreGraphic(MOON_GRAPHIC_VECTER* vertexs, unsigned int vertex_number, MOON_METADATA* metadata, unsigned int message_type);	//绘制图案
extern void MoonCoreFont(MOON_METADATA* metadata);																					//渲染默认字体

#endif
