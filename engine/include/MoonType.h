#ifndef MOON_TYPE_DEF
#define MOON_TYPE_DEF
#include<stdio.h>
#include<locale.h>
#include<limits.h>
#include<string.h>
#include<stdlib.h>
#include<wchar.h>
#include<math.h>
#include<time.h>

#ifndef MOON_ENGINE_CONFIG

#define MOONSTANDARDENTITY 1

#endif

#if	   MOONMANYENTITY

#undef MOONSTANDARDENTITY
#undef MOONFEWENTITY
#define MOON_ENTITY_NUMBER	1000003

#elif  MOONSTANDARDENTITY

#undef MOONMANYENTITY
#undef MOONFEWENTITY
#define MOON_ENTITY_NUMBER	10007

#elif  MOONFEWENTITY

#undef MOONMANYENTITY
#undef MOONSTANDARDENTITY
#define MOON_ENTITY_NUMBER	997

#endif


#define MOON_TRUE			(1)
#define MOON_FALSE			(0)
#define MOON_NULL			((void*)0)
#define MOON_Error			(-1)
#define MOON_NOTFOUND		(-1)
#define MOON_YES			MOON_TRUE
#define MOON_NO				MOON_FALSE
#define MOON_ON				MOON_TRUE
#define MOON_OFF			MOON_FALSE
#define MOON_Pi				(3.1415926f)		//Pi
#define MOON_VERTICES_MAX	(65536 * 4)			//顶点上限
#define MOON_MESSAGE_TEXT_MAX	(32)			//单条消息最大字符数
#define MOON_MESSAGE_BYTE_MAX	(64)			//单条消息最大字节
#define MOON_UINT_MAX	UINT_MAX				//无类型整数最大值

#define MOON_FONT_CHAR_SIZE_H 16				//字体高度
#define MOON_FONT_CHAR_SIZE_W 8					//字体宽度

#define MoonCrashed			{volatile int *ptr = 0; *ptr = 0;}	//立即崩溃
#define MoonCode_ON			if(1)				//代码块开启
#define MoonCode_OFF		if(0)				//代码块关闭

//创建线程函数关键字
typedef int MOON_THREAD;

//--------------------------定时器--------------------------//

typedef struct
{
	unsigned int time1;
	unsigned int time2;
	unsigned int timeload;
	unsigned char timeswitch;
}MOON_TIMELOAD;


//--------------------------点结构体--------------------------//

typedef struct
{
	float x, y, z;
	float r, g, b, a;
}MOON_POINT3D;

typedef struct
{
	union 
	{
		struct { int x, y; };
		struct { int w, h; };
		struct { float fx, fy; };
		struct { float fw, fh; };
	};
}MOON_POINT2D;

//--------------------------双缓冲绘图--------------------------//

typedef struct
{
	unsigned int texture, fbo;
}MOON_DOUBLEBUFFER;

//--------------------------图片--------------------------//

typedef struct
{
	MOON_POINT2D image_size;		//物理尺寸
	MOON_DOUBLEBUFFER image;		//图片
}MOON_IMAGE;

//--------------------------动画结构体--------------------------//

typedef struct
{
	MOON_IMAGE* sequenceframes;		//序列帧数组
	long long int number;			//当前序列帧
	int totalnumber;				//序列帧总数
	MOON_TIMELOAD timeload;			//定时器
}MOON_ANIME;

//--------------------------按钮状态--------------------------//

typedef enum
{
	MOON_BUTTON_FALSE,			//不存在按钮
	MOON_BUTTON_PRESS,			//按下
	MOON_BUTTON_PRESS_LONG,		//长按
	MOON_BUTTON_RHOVER,			//悬停
}MOON_BUTTON_TYPE;

//--------------------------按钮控件--------------------------//

typedef struct MOONBUTTON
{
	int x;
	int y;
	int width;
	int height;
	MOON_BUTTON_TYPE mode;
	unsigned int triggermode;
	int (*ButtonModePress)   (struct MOONBUTTON* button, void* context);	//按下
	int (*ButtonModePressL)  (struct MOONBUTTON* button, void* context);	//长按
	int (*ButtonModeHover)   (struct MOONBUTTON* button, void* context);	//悬停
	int (*ButtonModeFalse)   (struct MOONBUTTON* button, void* context);	//空状态时
}MOON_BUTTON;

//--------------------------文件结构体--------------------------//

typedef struct
{
	char* file_buffer;			//文件具体内容
	unsigned int file_size;		//文件长度
	unsigned int* line_index;		//每行起始索引
	unsigned int line_all;		//总行数
}MOON_FILE;

//--------------------------音乐状态--------------------------//

typedef enum
{
	MOON_MUSIC_MODE_FALSE,	//不播放
	MOON_MUSIC_MODE_RUN,	//继续播放
	MOON_MUSIC_MODE_AGAIN,	//重新开始
}MOON_MUSIC_MODE;

//--------------------------音乐--------------------------//

typedef struct
{
	unsigned int id;	//音乐
	float start, end;	//音频起点和终点,范围0.f~1.f
	MOON_MUSIC_MODE	mode;
}MOON_MUSIC;

//--------------------------向量--------------------------//

typedef struct
{
	float* vector;			//具体数值
	unsigned int dim;		//维度
}MOON_VECTOR;

//--------------------------链表/网--------------------------//

//二叉树
typedef struct MOON_BINARY_TREE
{
	struct MOON_BINARY_TREE* parent;		//链接的父节点
	struct MOON_BINARY_TREE* left;			//链接的子节点_左
	struct MOON_BINARY_TREE* right;			//链接的子节点_右
	unsigned char assign;					//动态分配标志位, 若为TRUE则尝试free resource
	void* resource;							//资源
}MOON_BINARY_TREE;

//链表函数返回状态
typedef enum
{
	MOON_LINKED_TYPE_FALSE,			//失败
	MOON_LINKED_TYPE_TRUE,			//成功
	MOON_LINKED_TYPE_NULL_PTR,		//空指针
	MOON_LINKED_TYPE_ILLEGAL,		//非法的节点,无法操作
}MOON_LINKED_TYPE;

//链表选择模式
typedef enum
{
	MOON_LINKED_MODE_FALSE,			//不插入
	MOON_LINKED_MODE_FRONT,			//节点前
	MOON_LINKED_MODE_AFTER,			//节点后
}MOON_LINKED_MODE;

//链表
typedef struct MOON_LINKED
{
	struct MOON_LINKED* prev;		//链接的父节点
	struct MOON_LINKED* next;		//链接的子节点
	unsigned char assign;			//动态分配标志位, 若为TRUE则尝试free resource
	void* resource;					//资源
}MOON_LINKED;

//--------------------------uniform type--------------------------//

typedef enum
{
	MOON_UNIFORM_TYPE_NONE,				//非法
	MOON_UNIFORM_TYPE_VECTOR1_FLOAT,	//float
	MOON_UNIFORM_TYPE_VECTOR2_FLOAT,	//2D向量 (x, y)
	MOON_UNIFORM_TYPE_VECTOR3_FLOAT,	//3D向量 (x, y, z)
	MOON_UNIFORM_TYPE_VECTOR4_FLOAT,	//4D向量 (x, y, z, w)
	MOON_UNIFORM_TYPE_VECTOR1_INT,		//int
	MOON_UNIFORM_TYPE_VECTOR2_INT,		//2D整数向量
	MOON_UNIFORM_TYPE_VECTOR3_INT,		//3D整数向量
	MOON_UNIFORM_TYPE_VECTOR4_INT,		//4D整数向量
	MOON_UNIFORM_TYPE_VECTOR1_UINT,		//unsigned int
	MOON_UNIFORM_TYPE_VECTOR2_UINT,		//2D无符号整数向量
	MOON_UNIFORM_TYPE_VECTOR3_UINT,		//3D无符号整数向量
	MOON_UNIFORM_TYPE_VECTOR4_UINT,		//4D无符号整数向量
	MOON_UNIFORM_TYPE_TEXTURE,			//传递纹理
}MOON_UNIFORM_TYPE;

//--------------------------uniform--------------------------//

typedef struct
{
	MOON_UNIFORM_TYPE type;
	union
	{
		struct
		{
			int x, y, z, w;
		}vec_int;
		struct
		{
			unsigned int x, y, z, w;
		}vec_uint;
		struct
		{
			float x, y, z, w;
		}vec_float;
		struct
		{
			int slot;      //绑定的OpenGL纹理单元编号,不得为0,若为0则无效
			MOON_IMAGE* image;
		}texture;
	};
}MOON_UNIFORM_DATA;

//--------------------------MOON_METADATA--------------------------//

typedef struct
{
	union
	{
		struct
		{
			MOON_IMAGE* image_goal;					//目标纹理
			unsigned int
				shader,								//着色器
				color;								//颜色
			union
			{
				struct
				{
					MOON_IMAGE* image_resources;						//源纹理资源
					int x, y, width, height, deg;
					float
						apx, apy,					//锚点
						uv_w, uv_h;					//uv裁切宽度
				}image;													//纹理
				struct
				{
					int x1, y1, x2, y2, x3, y3;
					unsigned int color_1, color_2, color_3;
				}graphic;												//绘图
				struct
				{
					MOON_IMAGE* image_resources;	//源纹理资源
					MOON_POINT2D point[4];			//顶点
				}image_pig;												//纹理映射

				struct
				{
					MOON_POINT2D coord;						//坐标
					int size_w, size_h;
					char* text;
				}text;													//纹理
			};
		}draw;

		struct
		{
			unsigned int token;						//键码
			int* worth;								//需要修改的值
		}key;															//按鍵
		
		struct
		{
			MOON_UNIFORM_DATA data;					//具体数据
			unsigned int shader;					//着色器
			char var[MOON_MESSAGE_TEXT_MAX];		//变量名
		}uniform;														//uniform
		
		union
		{
			int (*function_open)();					//自定义消息队列
			int (*function)();						//切换模块
			int power;
			int fps;
			unsigned int cull_face;
			//int dead;
		};
		unsigned char data[MOON_MESSAGE_BYTE_MAX];	//占位,消息最大为64byte
	};
}MOON_METADATA;

//--------------------------光标状态--------------------------//

typedef enum
{
	//MOON_CURSOR_MODE_NULL,		//正常模式
	MOON_CURSOR_MODE_HIDDEN,		//在窗口位置隐藏光标,不限制位置
	MOON_CURSOR_MODE_DISABLED,		//禁用模式,全程隐藏光标并不受主窗口限制
	MOON_CURSOR_MODE_CAPTURED,		//捕获模式,防止光标跑出窗口,不隐藏
}MOON_CURSOR_MODE;

//--------------------------线程状态--------------------------//

typedef enum
{
	MOON_MESSAGE_THREAD_TYPE_FALSE,
	MOON_MESSAGE_THREAD_TYPE_TRUE,
	MOON_MESSAGE_THREAD_TYPE_REALLOC_FAILURE,
	MOON_MESSAGE_THREAD_TYPE_BUSY,
	MOON_MESSAGE_THREAD_TYPE_CACHE,
}MOON_MESSAGE_THREAD_TYPE;

//--------------------------支持的消息类型--------------------------//

typedef enum
{
	/*
	请注意
	大部分情况下您都不必使用_END类消息,引擎会自动处理
	而且使用它们需要非常小心,可能会造成引擎内部自动锁失效
	核心只保证一定程度的稳定性
	向属性线程发送消息时要非常小心并发问题
	如果不是并发问题
	更改函数指针的消息也不会独立出来
	MOON_MESSAGE_ATTR_SETLOGIC
	MOON_MESSAGE_ATTR_SETDRAW

	如果按照我设定的工程规范
	只能由单一线程
	一般是逻辑线程来发送属性线程的消息

	*/
	MOON_MESSAGE_NULL = MOON_FALSE,
	
	MOON_MESSAGE_DRAW_START,//起始符,无其他含义
	//MOON_MESSAGE_DRAW_SETDRAW,
	MOON_MESSAGE_DRAW_OPEN,						//在消息队列注入自定义模块函数,一次性
	MOON_MESSAGE_DRAW_UNIFORM,					//glad_glUniform
	MOON_MESSAGE_CULL_FACE,						//设置剔除模式
	MOON_MESSAGE_DRAW_IMAGE,					//绘制纹理图层
	//MOON_MESSAGE_DRAW_IMAGE_ALPHA,
	//MOON_MESSAGE_DRAW_IMAGE_ROUND,
	MOON_MESSAGE_DRAW_IMAGE_UV,					//绘制UV纹理图层
	MOON_MESSAGE_DRAW_IMAGE_PIG,				//绘制自定义顶点纹理图层
	MOON_MESSAGE_DRAW_IMAGE_CLEAN,				//清屏纹理图层
	MOON_MESSAGE_DRAW_PIX,						//绘制点
	MOON_MESSAGE_DRAW_LINE,						//绘制线
	//MOON_MESSAGE_DRAW_CIRCLE,
	//MOON_MESSAGE_DRAW_BOX,
	//MOON_MESSAGE_DRAW_BOX_FULL,
	MOON_MESSAGE_DRAW_TRI_FULL,					//绘制填充三角形

	MOON_MESSAGE_DRAW_LINE_3D,					//绘制线 3D
	MOON_MESSAGE_DRAW_PIX_3D,					//绘制点 3D
	MOON_MESSAGE_DRAW_TRI_FULL_3D,				//绘制填充三角形 3D

	MOON_MESSAGE_DRAW_TEXT,						//绘制默认字体
	//MOON_MESSAGE_DRAW_ANIME_RUN,
	//MOON_MESSAGE_DRAW_ANIME_MODE,
	//MOON_MESSAGE_DRAW_ANIME_DELETE,
	MOON_MESSAGE_DRAW_END,//终止符,提前返回队列,提前解锁,需要很小心的使用,最好只在本线程使用,只在绘制线程使用,因为是串行,会提前处理,所以没问题,其他线程使用可能会破坏自动锁

	MOON_MESSAGE_LOGIC_START,//起始符,无其他含义
	MOON_MESSAGE_SETLOGIC,						//设置逻辑模块
	MOON_MESSAGE_SETDRAW,						//设置绘图模块
	//MOON_MESSAGE_LOGIC_OPEN,					//在消息队列注入自定义模块函数,一次性
	MOON_MESSAGE_ATTR_OPEN,						//在属性线程注入自定义模块函数,长期,可以传入MOON_NULL来禁用注入的函数
	MOON_MESSAGE_DEAD,							//项目退出,不论传入的任何值
	MOON_MESSAGE_POWER,							//设置高性能模式/暂停/锁帧
	MOON_MESSAGE_SETFPS,						//设置帧数
	MOON_MESSAGE_KEY,							//回调按鍵
	MOON_MESSAGE_LOGIC_END,//终止符,提前返回队列,提前解锁,需要很小心的使用,最好只在本线程使用,只在绘制线程使用,因为是串行,会提前处理,所以没问题,其他线程使用可能会破坏自动锁
}MOON_MESSAGE;

//--------------------------鍵盤掃描碼--------------------------//

typedef enum
{
	//鼠标
	MOON_KEY_MOUSE_LEFT = 0,   //GLFW_MOUSE_BUTTON_LEFT
	MOON_KEY_MOUSE_RIGHT = 1,  //GLFW_MOUSE_BUTTON_RIGHT

	//字母键
	MOON_KEY_A = 65,   //GLFW_KEY_A
	MOON_KEY_B = 66,   //GLFW_KEY_B
	MOON_KEY_C = 67,   //GLFW_KEY_C
	MOON_KEY_D = 68,   //GLFW_KEY_D
	MOON_KEY_E = 69,   //GLFW_KEY_E
	MOON_KEY_F = 70,   //GLFW_KEY_F
	MOON_KEY_G = 71,   //GLFW_KEY_G
	MOON_KEY_H = 72,   //GLFW_KEY_H
	MOON_KEY_I = 73,   //GLFW_KEY_I
	MOON_KEY_J = 74,   //GLFW_KEY_J
	MOON_KEY_K = 75,   //GLFW_KEY_K
	MOON_KEY_L = 76,   //GLFW_KEY_L
	MOON_KEY_M = 77,   //GLFW_KEY_M
	MOON_KEY_N = 78,   //GLFW_KEY_N
	MOON_KEY_O = 79,   //GLFW_KEY_O
	MOON_KEY_P = 80,   //GLFW_KEY_P
	MOON_KEY_Q = 81,   //GLFW_KEY_Q
	MOON_KEY_R = 82,   //GLFW_KEY_R
	MOON_KEY_S = 83,   //GLFW_KEY_S
	MOON_KEY_T = 84,   //GLFW_KEY_T
	MOON_KEY_U = 85,   //GLFW_KEY_U
	MOON_KEY_V = 86,   //GLFW_KEY_V
	MOON_KEY_W = 87,   //GLFW_KEY_W
	MOON_KEY_X = 88,   //GLFW_KEY_X
	MOON_KEY_Y = 89,   //GLFW_KEY_Y
	MOON_KEY_Z = 90,   //GLFW_KEY_Z

	//数字键
	MOON_KEY_0 = 48,   //GLFW_KEY_0
	MOON_KEY_1 = 49,   //GLFW_KEY_1
	MOON_KEY_2 = 50,   //GLFW_KEY_2
	MOON_KEY_3 = 51,   //GLFW_KEY_3
	MOON_KEY_4 = 52,   //GLFW_KEY_4
	MOON_KEY_5 = 53,   //GLFW_KEY_5
	MOON_KEY_6 = 54,   //GLFW_KEY_6
	MOON_KEY_7 = 55,   //GLFW_KEY_7
	MOON_KEY_8 = 56,   //GLFW_KEY_8
	MOON_KEY_9 = 57,   //GLFW_KEY_9

	//功能键
	MOON_KEY_ESCAPE = 256,          //GLFW_KEY_ESCAPE
	MOON_KEY_RETURN = 257,          //GLFW_KEY_ENTER
	MOON_KEY_TAB = 258,             //GLFW_KEY_TAB
	MOON_KEY_BACK = 259,            //GLFW_KEY_BACKSPACE
	MOON_KEY_INSERT = 260,          //GLFW_KEY_INSERT
	MOON_KEY_DELETE = 261,          //GLFW_KEY_DELETE
	MOON_KEY_RIGHT = 262,           //GLFW_KEY_RIGHT
	MOON_KEY_LEFT = 263,            //GLFW_KEY_LEFT
	MOON_KEY_DOWN = 264,            //GLFW_KEY_DOWN
	MOON_KEY_UP = 265,              //GLFW_KEY_UP
	MOON_KEY_PRIOR = 266,           //GLFW_KEY_PAGE_UP
	MOON_KEY_NEXT = 267,            //GLFW_KEY_PAGE_DOWN
	MOON_KEY_HOME = 268,            //GLFW_KEY_HOME
	MOON_KEY_END = 269,             //GLFW_KEY_END
	MOON_KEY_CAPITAL = 280,         //GLFW_KEY_CAPS_LOCK
	MOON_KEY_SCROLL = 281,          //GLFW_KEY_SCROLL_LOCK
	MOON_KEY_NUMLOCK = 282,         //GLFW_KEY_NUM_LOCK
	MOON_KEY_SNAPSHOT = 283,        //GLFW_KEY_PRINT_SCREEN
	MOON_KEY_PAUSE = 284,           //GLFW_KEY_PAUSE

	//F功能键
	MOON_KEY_F1 = 290,   //GLFW_KEY_F1
	MOON_KEY_F2 = 291,   //GLFW_KEY_F2
	MOON_KEY_F3 = 292,   //GLFW_KEY_F3
	MOON_KEY_F4 = 293,   //GLFW_KEY_F4
	MOON_KEY_F5 = 294,   //GLFW_KEY_F5
	MOON_KEY_F6 = 295,   //GLFW_KEY_F6
	MOON_KEY_F7 = 296,   //GLFW_KEY_F7
	MOON_KEY_F8 = 297,   //GLFW_KEY_F8
	MOON_KEY_F9 = 298,   //GLFW_KEY_F9
	MOON_KEY_F10 = 299,  //GLFW_KEY_F10
	MOON_KEY_F11 = 300,  //GLFW_KEY_F11
	MOON_KEY_F12 = 301,  //GLFW_KEY_F12
	MOON_KEY_F13 = 302,  //GLFW_KEY_F13
	MOON_KEY_F14 = 303,  //GLFW_KEY_F14
	MOON_KEY_F15 = 304,  //GLFW_KEY_F15
	MOON_KEY_F16 = 305,  //GLFW_KEY_F16
	MOON_KEY_F17 = 306,  //GLFW_KEY_F17
	MOON_KEY_F18 = 307,  //GLFW_KEY_F18
	MOON_KEY_F19 = 308,  //GLFW_KEY_F19
	MOON_KEY_F20 = 309,  //GLFW_KEY_F20
	MOON_KEY_F21 = 310,  //GLFW_KEY_F21
	MOON_KEY_F22 = 311,  //GLFW_KEY_F22
	MOON_KEY_F23 = 312,  //GLFW_KEY_F23
	MOON_KEY_F24 = 313,  //GLFW_KEY_F24

	//小键盘
	MOON_KEY_NUMPAD0 = 320,      //GLFW_KEY_KP_0
	MOON_KEY_NUMPAD1 = 321,      //GLFW_KEY_KP_1
	MOON_KEY_NUMPAD2 = 322,      //GLFW_KEY_KP_2
	MOON_KEY_NUMPAD3 = 323,      //GLFW_KEY_KP_3
	MOON_KEY_NUMPAD4 = 324,      //GLFW_KEY_KP_4
	MOON_KEY_NUMPAD5 = 325,      //GLFW_KEY_KP_5
	MOON_KEY_NUMPAD6 = 326,      //GLFW_KEY_KP_6
	MOON_KEY_NUMPAD7 = 327,      //GLFW_KEY_KP_7
	MOON_KEY_NUMPAD8 = 328,      //GLFW_KEY_KP_8
	MOON_KEY_NUMPAD9 = 329,      //GLFW_KEY_KP_9
	MOON_KEY_DECIMAL = 330,      //GLFW_KEY_KP_DECIMAL
	MOON_KEY_DIVIDE = 331,       //GLFW_KEY_KP_DIVIDE
	MOON_KEY_MULTIPLY = 332,     //GLFW_KEY_KP_MULTIPLY
	MOON_KEY_SUBTRACT = 333,     //GLFW_KEY_KP_SUBTRACT
	MOON_KEY_ADD = 334,          //GLFW_KEY_KP_ADD
	MOON_KEY_SEPARATOR = 335,    //GLFW_KEY_KP_ENTER（小键盘 Enter）
	MOON_KEY_OEM_PLUS = 336,     //GLFW_KEY_KP_EQUAL（小键盘 =）

	//修饰键
	MOON_KEY_LSHIFT = 340,    //GLFW_KEY_LEFT_SHIFT
	MOON_KEY_LCONTROL = 341,  //GLFW_KEY_LEFT_CONTROL
	MOON_KEY_LMENU = 342,     //GLFW_KEY_LEFT_ALT
	MOON_KEY_LWIN = 343,      //GLFW_KEY_LEFT_SUPER
	MOON_KEY_RSHIFT = 344,    //GLFW_KEY_RIGHT_SHIFT
	MOON_KEY_RCONTROL = 345,  //GLFW_KEY_RIGHT_CONTROL
	MOON_KEY_RMENU = 346,     //GLFW_KEY_RIGHT_ALT
	MOON_KEY_RWIN = 347,      //GLFW_KEY_RIGHT_SUPER
	MOON_KEY_APPS = 348,      //GLFW_KEY_MENU（应用程序键）

	//符号键
	MOON_KEY_SPACE = 32,              //GLFW_KEY_SPACE
	MOON_KEY_OEM_7 = 39,              //GLFW_KEY_APOSTROPHE（'）
	MOON_KEY_OEM_COMMA = 44,          //GLFW_KEY_COMMA（,）
	MOON_KEY_OEM_MINUS = 45,          //GLFW_KEY_MINUS（-）
	MOON_KEY_OEM_PERIOD = 46,         //GLFW_KEY_PERIOD（.）
	MOON_KEY_OEM_2 = 47,              //GLFW_KEY_SLASH（/）
	MOON_KEY_OEM_1 = 59,              //GLFW_KEY_SEMICOLON（;）
	MOON_KEY_OEM_4 = 91,              //GLFW_KEY_LEFT_BRACKET（[）
	MOON_KEY_OEM_5 = 92,              //GLFW_KEY_BACKSLASH（\）
	MOON_KEY_OEM_6 = 93,              //GLFW_KEY_RIGHT_BRACKET（]）
	MOON_KEY_OEM_3 = 96,              //GLFW_KEY_GRAVE_ACCENT（`）

	MOON_KEY_LAST = 349   //GLFW_KEY_LAST + 1
}MOON_KEY_TYPE;

//--------------------------按键消息回调--------------------------//

typedef enum
{
	MOON_KEY_MODE_FALSE,
	MOON_KEY_MODE_PRESS,
	MOON_KEY_MODE_PRESS_LONG,
}MOON_KEY_MODE;

#endif
