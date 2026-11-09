#pragma once
#ifndef MOON_ENGINE
#define MOON_ENGINE
#define _CRT_SECURE_NO_WARNINGS

#ifdef MOON_ENGINE_CONFIG

#include"Moon_Configuration.h"

#endif

#ifdef MOONCOMPATIBLE

#include"Moon_Compatible_Function.h"

#endif

#include "MoonUtility.h"
#include "MoonDraw.h"

/*
如果您感兴趣,还可以查看另一个功能更加强大但是已经落后的项目:Star,项目请见:https://github.com/xsq12348/star
MoonEngine是一个简单的框架/轻量化引擎

作者:xsq12348 / XSQ / xsq1234(这三个都是别名)
Email:1993346266@qq.com
创建日期:2025.10.29
版本,如果没有日期,那就是前一个日期一起写的/If there is no date, it is written together with the previous date.
(0)是修改BUG/重构函数/更新些许小功能,(1)是添加函数,(2)是重构,(3)是正式的大版本号
一般来说只有写着日期的日志才是我正式工作的日期,如果要判断工时,请以此为据
请注意,本引擎/框架本质上是作为内核/类内核使用的,实际只提供最小功能,比如基础图元,工具函数,大部分情况仍需您自主实现
引擎本身是通用的,或者说并没有对2D或者3D以及任何其他类型的应用进行特殊优化,所以如果您想要使用这个引擎,请自行实现您的应用逻辑,本引擎只提供最基础的功能
您可以用于开发任意类型的应用,包括但不限于2D游戏,3D游戏,图形编辑器,音频播放器,GUI等图形化应用,只要您愿意,本引擎都可以作为您的基础框架使用

	大版本(3).主要版本(2).小版本(1).小更新(0)

	本项目地址 https://github.com/xsq12348/Moon_Engine2.0

[简易文档]
最后一次更新日期 : 2026.5.22
最后一次更新日期 : 2026.5.26
最后一次更新日期 : 2026.6.4
最后一次更新日期 : 2026.6.9
最后一次更新日期 : 2026.6.10
最后一次更新日期 : 2026.6.14
最后一次更新日期 : 2026.6.24
最后一次更新日期 : 2026.6.26
最后一次更新日期 : 2026.6.27
最后一次更新日期 : 2026.8.9
最后一次更新日期 : 2026.9.14
最后一次更新日期 : 2026.9.16
最后一次更新日期 : 2026.9.19

[1]
	MoonEngine以左上角为原点,与GDI和SDL看齐,反转Y轴
	而不是传统图形库的中心原点,Y轴为正

[2]
	如果要使用这个引擎,有几个概念您得先搞懂
	实体系统是重中之重,几乎所有核心内容都需要实体系统传递,包括多文件下您自身的变量与结构体
	引擎主画布类似于屏幕的后缓冲区,是一张纹理图层,用于规范绘制,防止直接操作后缓冲区造成混乱,以及适配可能不同的绘图后端

	引擎内部提供的实体有以下这些
	帧率			ProjectFPS					int
	鼠标坐标		ProjectMouseCoord			MOON_POINT2D
	引擎主画布	ProjectBitmap				MOON_IMAGE
	纯色着色器	ProjectShader_SolidColor	unsigned int
	纹理着色器	ProjectShader_Texture		unsigned int
	按键状态		ProjectKey					unsigned char

	您可以通通过类似于
	static MOON_IMAGE* engineback;
	MoonHashFindEntity(project, "ProjectBitmap", MOON_IMAGE, engineback_2);
	engineback = engineback_2;
	的方法来获取实体系统的资源

[3]
	[3.1]
	引擎内部总是维护两个内部函数,用于逻辑与绘制线程
	这两个函数通过指针切换特定签名的函数来实现模块的切换
	单一线程只能操纵单一线程类型的消息

	绘制线程只能够发送绘制消息
	逻辑线程发送其余的所有消息
	当然,绘制线程也可以发送其他消息
	进过多次优化,并发问题实际上已经彻底解决了
	发送消息时遇到BUSY的概率极小
	
	逻辑线程禁止发送绘制消息,会导致频闪
	请注意,这个频闪不是由于即时模式的那种没有双缓冲导致的
	恰恰是由双缓冲本身导致的
	具体原因是因为线程的异步导致消息队列的异步
	间隔帧会存在黑屏导致的频闪
	
	[3.2]
	如要切换模块[示例]

	逻辑线程
		MOON_METADATA metadata;
		metadata.function = GameLogicAll;
		MoonProjectSendMessage(MOON_MESSAGE_SETLOGIC, metadata);
	
	绘制线程
		MOON_METADATA metadata;
		metadata.function = GameDrawAll;
		int alpha = MoonProjectSendMessage(MOON_MESSAGE_SETDRAW, metadata);
	
		MOON_IMAGE的实际功能更偏向于图层,配套函数也是基于图层的逻辑来写的

	[3.3]
		请注意,绘制模块是必选项
		逻辑模块是可选项
		如果不传入逻辑模块,那么也不用担心功能的丧失
		引擎内部会使用默认的空函数来填充
	
[4]
		引擎固定的用于输出的纹理渲染器unifrom变量是
			moon_utexture	对应		纹理
			moon_ucolor		对应		metadata.draw.color

		如果您需要自行编写着色器并使用消息
			MOON_DRAW_IMAGE
			MOON_DRAW_IMAGE_UV
			MOON_DRAW_IMAGE_PIG
		来进行渲染
		使用此功能必须搭配引擎源码而非dll
		那么请参考此条目

[5]
		如果您需要使用更强大的MOON_METADATA功能
		请务查看以下案例
		[案例]
			[5.1]
					MoonDrawAreaRound(&enginebackalpha, image_dd, 400, 300, .5f, .5f, image_dd->image_size.w, image_dd->image_size.h, 45);
					MOON_METADATA metadata;
					metadata.draw.shader = shader2;
					metadata.draw.image_goal = engineback;
					metadata.draw.image.image_resources = &enginebackalpha;
					metadata.draw.image.x = 0;
					metadata.draw.image.y = 0;
					metadata.draw.image.width = enginebackalpha.image_size.w;
					metadata.draw.image.height = enginebackalpha.image_size.h;
					MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE, metadata);
			[5.2]		
					MoonDrawAreaRound(&enginebackalpha, image_dd, 400, 300, .5f, .5f, image_dd->image_size.w, image_dd->image_size.h, 45);
					MOON_METADATA metadata = { MOON_NULL };
					metadata.draw.shader = shader2;
					metadata.draw.image_goal = engineback;
					metadata.draw.image.image_resources = &enginebackalpha;
					metadata.draw.image.deg = 0;
					metadata.draw.image.x = 0;
					metadata.draw.image.y = 0;
					metadata.draw.image.width = enginebackalpha.image_size.w;
					metadata.draw.image.height = enginebackalpha.image_size.h;
					MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE, metadata);
			[5.3]		
					MoonDrawAreaRound(&enginebackalpha, image_dd, 400, 300, .5f, .5f, image_dd->image_size.w, image_dd->image_size.h, 45);
					MOON_METADATA metadata;
					metadata.draw.shader = shader2;
					metadata.draw.image_goal = engineback;
					metadata.draw.image.image_resources = &enginebackalpha;
					metadata.draw.image.deg = 0;
					metadata.draw.image.apx = 0;
					metadata.draw.image.apy = 0;
					metadata.draw.image.uv_w = 1.f;
					metadata.draw.image.uv_h = 1.f;
					metadata.draw.image.x = 0;
					metadata.draw.image.y = 0;
					metadata.draw.image.width = enginebackalpha.image_size.w;
					metadata.draw.image.height = enginebackalpha.image_size.h;
					MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE, metadata);

			请注意,[5.1]可能不是您想要的效果,因为没有像[5.2]与[5.3]那样完全初始化,可能有状态残留
			尤其是可能出现顶点紊乱,方向错误,黑屏,奇怪的旋转等等
			实际使用时,请务必注意是否初始化

[6]
			如果要嵌入自定义消息队列
			请使用消息
				MOON_MESSAGE_ATTR_OPEN
				MOON_MESSAGE_DRAW_OPEN
			并发送到
			metadata.function_open中
			请注意,function_open相当于注入队列的一个小型主循环
			主要功能是用于补全一些缺失的功能,比如即时绘图模式或者其他功能
			如果在其中使用消息式绘图,SendMessage几乎一定会返回MOON_MESSAGE_THREAD_TYPE_BUSY

[7]
			关于MoonDrawtextFont
			由于目前的技术力,暂时还不打算接受任意长度的字符串数据
			因此请您注意是否超出MOON_MESSAGE_TEXT_MAX
			或者说您可以自行切割字符串以支持任意长度

[8]
			鼠标默认行为在窗口内部
			您可以通过MoonSetMouseCoord来更改

[9]
			由于逻辑线程没有OpenGL上下文
			所以MoonShaderLoad这类需要OpenGL上下文的函数应尽量在
				ProjectSetting_1
				ProjectSetting_2
				绘制线程内部
			里面完成

[10]
			加载图片
			您首先需要定义
			MOON_IMAGE image;

			其次使用 MoonImageCreate 给予它范围

[11] 消息式绘图{ 警告!!本条目为AI自动填充,如有错误,作者不对此负有主要责任 }

	MoonEngine 2.0 开始采用消息式绘图架构，所有绘图操作不再直接执行，而是通过发送消息由引擎在绘制线程中统一处理。

	[11.1] 基本概念

		消息式绘图的核心是“延迟 + 批量”：
		- 延迟：绘图请求先存入队列，不立即执行
		- 批量：引擎自动合并相同状态的绘制，减少 GPU 状态切换

		开发者只需要调用 MoonProjectSendMessage 发送消息，引擎在每帧绘制阶段自动处理。

	[11.2] 发送消息

		使用 MoonProjectSendMessage 函数发送消息：

		MOON_METADATA metadata = { 0 };
		metadata.draw.xxx = ...;
		MoonProjectSendMessage(MOON_MESSAGE_XXX, metadata);

		返回值类型为 MOON_MESSAGE_THREAD_TYPE：
			MOON_MESSAGE_THREAD_TYPE_TRUE				发送成功
			MOON_MESSAGE_THREAD_TYPE_BUSY				线程忙，稍后重试
			MOON_MESSAGE_THREAD_TYPE_REALLOC_FAILURE	内存分配失败
			MOON_MESSAGE_THREAD_TYPE_FALSE				无效消息

	[11.3] 可用的绘图消息

		MOON_MESSAGE_DRAW_IMAGE			绘制纹理图层
		MOON_MESSAGE_DRAW_IMAGE_UV		绘制 UV 裁切后的纹理图层
		MOON_MESSAGE_DRAW_IMAGE_PIG		四点透视变换纹理贴图
		MOON_MESSAGE_DRAW_IMAGE_CLEAN	清屏
		MOON_MESSAGE_DRAW_PIX			绘制点
		MOON_MESSAGE_DRAW_LINE			绘制线
		MOON_MESSAGE_DRAW_TRI_FULL		绘制填充三角形
		MOON_MESSAGE_DRAW_TEXT			绘制默认字体文字
		MOON_MESSAGE_DRAW_OPEN			注入自定义绘制模块（一次性）
		MOON_MESSAGE_DRAW_END			提前终止本帧绘制（谨慎使用）

	[11.4] 使用示例

		[11.4.1] 绘制纹理

			MOON_METADATA metadata = { 0 };
			metadata.draw.color = 0xffffffff;
			metadata.draw.shader = texture_shader;			// 纹理着色器
			metadata.draw.image_goal = engineback;			// 目标纹理
			metadata.draw.image.image_resources = sprite;	// 源纹理
			metadata.draw.image.x = 100;
			metadata.draw.image.y = 200;
			metadata.draw.image.width = 64;
			metadata.draw.image.height = 64;
			metadata.draw.image.uv_w = 1.f;				// UV 宽度
			metadata.draw.image.uv_h = 1.f;				// UV 高度
			metadata.draw.image.deg = 0;				// 旋转角度
			MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE, metadata);

		[11.4.2] 绘制旋转纹理

			MOON_METADATA metadata = { 0 };
			metadata.draw.color = 0xffffffff;
			metadata.draw.shader = texture_shader;
			metadata.draw.image_goal = engineback;
			metadata.draw.image.image_resources = sprite;
			metadata.draw.image.x = 100;
			metadata.draw.image.y = 200;
			metadata.draw.image.apx = 0.5f;				// 锚点 X（0~1）
			metadata.draw.image.apy = 0.5f;				// 锚点 Y（0~1）
			metadata.draw.image.width = 64;
			metadata.draw.image.height = 64;
			metadata.draw.image.uv_w = 1.f;
			metadata.draw.image.uv_h = 1.f;
			metadata.draw.image.deg = 45;				// 45 度旋转
			MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE, metadata);

		[11.4.3] 绘制 UV 裁切纹理

			MOON_METADATA metadata = { 0 };
			metadata.draw.color = 0xffffffff;
			metadata.draw.shader = texture_shader;
			metadata.draw.image_goal = engineback;
			metadata.draw.image.image_resources = sprite;
			metadata.draw.image.x = 100;
			metadata.draw.image.y = 200;
			metadata.draw.image.width = 64;
			metadata.draw.image.height = 64;
			metadata.draw.image.apx = 0.5f;				// UV 起始 X
			metadata.draw.image.apy = 0.0f;				// UV 起始 Y
			metadata.draw.image.uv_w = 0.25f;			// UV 宽度
			metadata.draw.image.uv_h = 0.25f;			// UV 高度
			metadata.draw.image.deg = 0;
			MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE_UV, metadata);

		[11.4.4] 绘制透视变换纹理

			MOON_METADATA metadata = { 0 };
			metadata.draw.color = 0xffffffff;
			MOON_POINT2D points[4] = {
				{100, 200},	// 左上
				{300, 180},	// 右上
				{80,  280},	// 左下
				{320, 300}	// 右下
			};
			metadata.draw.shader = texture_shader;
			metadata.draw.image_goal = engineback;
			metadata.draw.image_pig.image_resources = sprite;
			metadata.draw.image_pig.point[0] = points[0];
			metadata.draw.image_pig.point[1] = points[1];
			metadata.draw.image_pig.point[2] = points[2];
			metadata.draw.image_pig.point[3] = points[3];
			MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE_PIG, metadata);

		[11.4.5] 绘制点

			MOON_METADATA metadata = { 0 };
			metadata.draw.color = 0xFFFFFFFF;			// 颜色调制器，白色表示不调制
			metadata.draw.shader = solid_color_shader;		// 纯色着色器
			metadata.draw.image_goal = engineback;
			metadata.draw.graphic.x1 = 100;
			metadata.draw.graphic.y1 = 200;
			metadata.draw.graphic.color_1 = 0xFFFF0000;		// 红色
			MoonProjectSendMessage(MOON_MESSAGE_DRAW_PIX, metadata);

		[11.4.6] 绘制线

			MOON_METADATA metadata = { 0 };
			metadata.draw.color = 0xFFFFFFFF;
			metadata.draw.shader = solid_color_shader;
			metadata.draw.image_goal = engineback;
			metadata.draw.graphic.x1 = 100;
			metadata.draw.graphic.y1 = 200;
			metadata.draw.graphic.x2 = 300;
			metadata.draw.graphic.y2 = 400;
			metadata.draw.graphic.color_1 = 0xFFFFFFFF;		// 白色
			metadata.draw.graphic.color_2 = 0xFFFFFFFF;
			MoonProjectSendMessage(MOON_MESSAGE_DRAW_LINE, metadata);

		[11.4.7] 绘制填充三角形

			MOON_METADATA metadata = { 0 };
			metadata.draw.color = 0xFFFFFFFF;
			metadata.draw.shader = solid_color_shader;
			metadata.draw.image_goal = engineback;
			metadata.draw.graphic.x1 = 100;
			metadata.draw.graphic.y1 = 200;
			metadata.draw.graphic.x2 = 300;
			metadata.draw.graphic.y2 = 200;
			metadata.draw.graphic.x3 = 200;
			metadata.draw.graphic.y3 = 400;
			metadata.draw.graphic.color_1 = 0xFFFF0000;		// 顶点1 红色
			metadata.draw.graphic.color_2 = 0xFF00FF00;		// 顶点2 绿色
			metadata.draw.graphic.color_3 = 0xFF0000FF;		// 顶点3 蓝色
			MoonProjectSendMessage(MOON_MESSAGE_DRAW_TRI_FULL, metadata);

		[11.4.8] 清屏

			MOON_METADATA metadata = { 0 };
			metadata.draw.color = 0xFF000000;			// 黑色
			metadata.draw.image_goal = engineback;
			MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE_CLEAN, metadata);

	[11.5] 颜色说明

		颜色格式为 0xAARRGGBB：(此处为AI错误,应该为ABGR格式)
			AA = Alpha（透明度，00=全透明，FF=不透明）
			RR = Red（红色）
			GG = Green（绿色）
			BB = Blue（蓝色）

		使用 MoonRGBA 宏可以方便地构造颜色值：
			unsigned int red = MoonRGBA(255, 0, 0, 255);

		注意：
			draw.color 是颜色调制器，最终颜色 = 顶点颜色 × draw.color
			如果不想要调制效果，请将 draw.color 设为 0xFFFFFFFF（白色）
			如果 draw.color 为 0x00000000，将导致完全透明/黑色，什么都不显示

	[11.6] 批量渲染优化

		引擎会自动合并连续的相同类型的绘制消息：
		- 相同目标纹理（image_goal）
		- 相同源纹理（image_resources）
		- 相同消息类型

		合并后的绘制会一次性提交给 GPU，减少 DrawCall 数量。
		开发者无需手动处理，引擎自动完成。

	[11.7] 封装函数

		为了方便使用，引擎提供了一系列封装函数，内部自动填充 metadata 并发送消息：

			MoonDrawArea			-> MOON_MESSAGE_DRAW_IMAGE
			MoonDrawAreaRound		-> MOON_MESSAGE_DRAW_IMAGE
			MoonDrawAreaUV			-> MOON_MESSAGE_DRAW_IMAGE_UV
			MoonDrawAreaPlgBit		-> MOON_MESSAGE_DRAW_IMAGE_PIG
			MoonDrawPix				-> MOON_MESSAGE_DRAW_PIX
			MoonDrawLine			-> MOON_MESSAGE_DRAW_LINE
			MoonDrawBox				-> MOON_MESSAGE_DRAW_LINE（4次）
			MoonDrawBoxFull			-> MOON_MESSAGE_DRAW_TRI_FULL（2次）
			MoonDrawTriFull			-> MOON_MESSAGE_DRAW_TRI_FULL
			MoonDrawTextFont		-> MOON_MESSAGE_DRAW_TEXT

		推荐优先使用封装函数，代码更简洁，且已正确处理 draw.color 的默认值。

	[11.8] 注意事项

		[11.8.1] metadata 必须完全初始化
			建议使用 MOON_METADATA metadata = { 0 }; 后再填充需要的字段
			未初始化的字段可能导致状态残留，出现顶点紊乱、黑屏等问题

		[11.8.2] draw.color 不可为 0
			即使不需要调制，也请设置为 0xFFFFFFFF
			封装函数已自动处理，直接发送消息时需注意

		[11.8.3] 逻辑线程与绘制线程
			虽然可以从逻辑线程发送绘制消息，但引擎文档提示可能造成频闪
			建议在绘制线程内部发送绘制消息，或使用封装函数让引擎自动处理

		[11.8.4] 文字长度限制
			单条 MOON_MESSAGE_DRAW_TEXT 消息最多支持 MOON_MESSAGE_TEXT_MAX 个字符
			超出部分请自行分割为多条消息

		[11.8.5] 顶点数量限制
			单帧顶点总数不能超过 MOON_VERTICES_MAX（65536）
			超出会触发顶点溢出警告

	[11.9] 与旧版本的区别

		MoonEngine 2.0 之前采用即时模式绘图，函数直接执行绘制操作。
		2.0 之后改为消息式绘图，所有绘制操作都需要通过消息发送。

		如果您需要兼容旧代码，可以在配置中开启 MOONCOMPATIBLE 宏，
		引擎会提供兼容层支持旧版 API ,但不保证参数相同。

[12]
	[12.1]
		注册按钮十分简单
		static MOON_BUTTON button;
		MOON_BUTTON_CREATE(project, "button", button, 500, 100, 150, 150, MOON_NULL, MOON_NULL, MOON_NULL);
		即可
	[12.2]
		按钮可以在任意线程使用

[13]
	如何在没有控制台的情况下查看版本号?
	请如下操作
		unsigned int vsn = MoonVsn();
		printf("%d.%d.%d.%d\n", vsn & 0xff, (vsn & 0xff00) >> 8, (vsn & 0xff0000) >> 16, (vsn & 0xff000000) >> 24);

[14]
	硬件检测
	使用消息系统回调
		{
			MOON_METADATA metadata = { 0 };
			static int ic;
			metadata.key.token = MOON_KEY_MOUSE_LEFT;
			metadata.key.worth = &ic;
			MoonProjectSendMessage(MOON_MESSAGE_KEY, metadata);
		}
	或者是
	直接使用
		MoonKeyState
		MoonKeyReal
	都可以检测按鍵
	它们都是线程安全的

[15]
	[15.1]
		加载音乐很简单
		您只需要
			MOON_MUSIC music;
			MoonMusicInit(&music);

	[15.2]
		使用音乐
			MoonMusic(&music);

	[15.3]
		默认是播放整首
		您可以通过
			MoonMusicSet(&music, .5f, 1.f);
		来调整播放区间

	[15.4]
		您可以写下如下代码来实现循环播放
			if(music.mode == MOON_MUSIC_MODE_FALSE)
				MoonMusicAgain(&music);

[16]{ 警告!!本条目为AI自动填充,如有错误,作者不对此负有主要责任 }
		文件的加载与使用

		MoonEngine 提供了轻量级的文本文件加载与行索引工具，专为脚本解析和配置文件读取设计。
		与直接将整个文件切分为字符串数组不同，引擎采用“完整文本块 + 行偏移索引”的方式，
		在保持内存连续性的同时，提供了 O(1) 的行定位能力。

	[16.1]
		核心数据结构

		MOON_FILE 是文件内容在内存中的镜像，包含原始文本缓冲区和每行的起始偏移量：

		typedef struct
		{
			char* file_buffer;				// 文件完整内容（连续内存块）
			unsigned int file_size;			// 文件总长度（字节数）
			unsigned int* line_index;		// 每行起始偏移量数组
			unsigned int line_all;			// 总行数
		}MOON_FILE;

		行号从 1 开始编号（而非 0），方便人类阅读和调试。
		line_index[0] = 0		第一行起点
		line_index[1] = 2		第二行起点
		...
		line_index[line_all - 1]	最后一行的起点
		line_index[line_all]		= file_size（哨兵，用于计算最后一行长度）

	[16.2]
		文件加载函数

		[16.2.1]
			MoonFileLoad_TEXT（基础函数）
			功能：将文本文件读入用户提供的缓冲区，自动处理 BOM 头。
			原型：MoonFileLoad_TEXT(const char* file_name, char* text, unsigned int text_size);
			参数：
				file_name	文件路径
				text		用户分配的缓冲区
				text_size	缓冲区大小（必须足够容纳文件内容 + 结尾 '\0'）
			返回值：成功 MOON_TRUE，失败 MOON_FALSE
			注意：此函数仅做原始读取，不构建行索引。

		[16.2.2]
			MoonFileRead_TEXT（推荐使用）
			功能：加载文本文件并自动构建 MOON_FILE 结构体，包含完整的行索引。
			原型：MoonFileRead_TEXT(MOON_FILE* file, const char* file_name);
			参数：
				file		指向 MOON_FILE 的指针（需提前声明）
				file_name	文件路径
			返回值：成功 MOON_TRUE，失败 MOON_FALSE
			内部处理：
				- 自动跳过 UTF-8 BOM（0xEF 0xBB 0xBF）
				- 将 CRLF（\r\n）中的 \r 替换为空格，保持文件长度不变
				- 为每行建立起始偏移索引，最后一行的终点指向 file_size
			内存管理：
				- 使用 MoonAlloc 分配内存，由引擎注册表自动回收
				- 失败时自动回滚已分配的资源

		[16.2.3]
			MoonFileRead_Line（读取指定行）
			功能：从已加载的 MOON_FILE 中提取指定行的内容。
			原型：MoonFileRead_Line(MOON_FILE* file, char* file_buffer, unsigned int line);
			参数：
				file		已加载的 MOON_FILE
				file_buffer	用户分配的缓冲区（需确保足够容纳该行 + '\0'）
				line		行号（从 1 开始）
			返回值：成功 行长度，失败 MOON_FALSE
			注意：
				- 当file_buffer为MOON_NULL时,仅返回行长度
				- 行号从 1 开始，对应文件的第一行
				- 如果 line 超出范围（line > file->line_all），返回 MOON_FALSE 并打印提示
				- 缓冲区由调用者管理，引擎不负责检查其大小

		引擎退出时，MoonAlloc 注册表会自动释放 file.file_buffer 和 file.line_index，
		您无需手动调用 MoonFree（但也可以在不再使用时主动释放以减少内存占用）。

[17]	自动居中
		当您希望窗口可以自动居中显示时,请在MoonProjectInit中将任意坐标设置为负数即可

[18]	回收资源
			引擎会回收一些特定类型的资源,只要它们在实体注册表中
			如下
				MOON_IMAGE
				MOON_ANIME
				MOON_VECTOR

[19]	维护代码
			本引擎维护标准 以下是部分建议
			
			您可以在Github上提交pr或者私信邮箱来提交维护申请
			
			必须提供您的完整的,可独立运行的测试.c文件或者完整项目
				即最小可复现示例（Minimal Reproducible Example）
			
			禁止使用任何cpp,但您可以保持兼容cpp
			
			对于用不到的枚举或者代码块,最好注释掉而不是删除,方便代码回溯错误或者供给给他人学习

			标准如下

				对于if
					如果单行if很长,必须换行
						例如
							if ((index == message->message_index - 1)
								|| message->message[index + 1].message != message_type
								|| message->message[index + 1].metadata.draw.image_goal != image_old
								|| message->message[index + 1].metadata.draw.image.image_resources != image_resource_old
								|| message->message[index + 1].metadata.draw.color != message->message[index].metadata.draw.color)
					
					如果只有一行操作,那么不添加括号

				for
					如果没有其他特殊要求,使用index来作为循环变量
					如果只有一行操作,那么不添加括号


				对于case语句
					每个case后方必须接一个break,哪怕case后面直接是return
					如果case内有多行代码,必须用大括号括起来,以便于阅读
					break和case必须单独占一行,以便于阅读
					例如
						switch (message_type)
						{
						case MOON_MESSAGE_DRAW_LINE:
							graphic_mode = GL_LINES;
							break;
						case MOON_MESSAGE_DRAW_PIX:
							graphic_mode = GL_POINTS;
							break;
						case MOON_MESSAGE_DRAW_TRI_FULL:
							graphic_mode = GL_TRIANGLES;
							break;
						default:
							MoonPrompt((char*)"无效的绘制命令");
							return MOON_Error;
								break;
						}

				函数
					对于添加新的公开函数,描述必须如下,且必须以extern修饰
						* 函數 xxx
						* 作用 xxx
						* 使用方法
						* xxx();
					公开函数必须以大写字母开头,且必须使用驼峰命名法,家族函数以_为功能后缀或者直接写名称
						例如
							MoonProjectRun
							MoonVector_Dot

					公开函数必须有描述,描述必须包含函数名,作用,使用方法,使用示例
					公开函数必须有使用示例,使用示例必须包含完整的调用过程
					公开函数内任何独立功能,比如更新数组循环,且与函数内其他部分没有耦合,最好用大括号括起来,以便于阅读
					例如
						{
							MoonDrawOver();
							MoonUtilityOver();
						}

						{
							SDL_Quit();
						}

						{
							glad_glDeleteProgram(shader_program_vectex);
							glad_glDeleteProgram(shader_program_pixel);
							glfwTerminate();
						}

						//释放消息队列
						{
							free(moon_engine_core.message_draw.message);
							free(moon_engine_core.message_logic.message);
						}

			对于变量
				任何变量,如果存在临时副本或者缓冲,必须以_buffer结尾
				全局变量
					必须以static开头
					extern禁止修饰任何变量,包括静态变量,包括全局变量,包括结构体,包括联合体,包括枚举体等
						跨文件传递变量,必须通过函数接口或者实体系统接口调用
					全局变量的名称必须有意义,不要吝啬名字长度
					返回小错误或者提示用MoonPrompt
					遇到重大问题调用MoonProjectDead立即杀死项目

				对于任何已知不会出现负数的变量,务必用unsigned修饰
				所有bool类型全部以unsigned char表示
				如果一次性声明变量很多
					最好以如下方式组织代码,保持整齐
					例如
						float
							vx1 = MoonLerp(-1.f, 1.f, x * w_buffer),
							vy1 = MoonLerp(1.f, -1.f, (metadata->draw.text.coord.y + image_buffer_size.h * n) * h_buffer),
							vx2 = MoonLerp(-1.f, 1.f, (x + image_buffer_size.w) * w_buffer),
							vy2 = MoonLerp(1.f, -1.f, (metadata->draw.text.coord.y + image_buffer_size.h * (n + 1)) * h_buffer),
							uv_w = font_w,
							apx = ch * font_w;

						int a = 100,
							b = 100,
							c = 100;

				变量名称最好有意义,哪怕是临时变量,非必要不使用单字母
					除非它们组在一起没有歧义
						例如
							w h 一起
							x y z 一起
							r g b a 一起
							alpha beta gamma作为临时操作变量等


* 0.0.0.0
* 1.0.0.0  2025.10.29  完成了基本框架的搭建																		.Completed the setup of the basic framework
* 1.0.0.1  2025.10.30  解决了按键检测,窗口输入,消息循环BUG,添加了一个线程用来记录项目运行中的各种属性					.Fixed bugs with key detection, window input, and message loop, and added a thread to record various properties during project execution.
* 1.0.0.2  2025.10.31  解决了帧数显示不出来的问题																.Fixed the issue where the frame count was not displaying.
* 1.0.0.3  2025.11.2   加入了动画系统																			.Added an animation system
* 1.0.0.3  2025.11.3   修改了帧率控制不稳定的问题																.Fixed the issue of unstable frame rate control
* 1.0.0.4              将工具函数转移到了新的文件里																.Moved the utility functions to a new file
* 1.0.0.5  2025.11.4   添加了字符函数																			.Added character function
* 1.0.0.6  2025.11.5   添加了图片旋转																			.Added image rotation
* 1.0.0.7              添加了获取像素颜色																		.Added functionality to get pixel color
* 1.0.0.8              修复了HashFindEntity的BUG																.Fixed the bug in HashFindEntity
* 1.0.0.9              添加了三角形碰撞检测																		.Added triangle collision detection
* 1.0.0.10             添加了运行外部程序																		.Added the ability to run external programs
* 1.1.0.0              引擎几乎搭建完成																		.The engine is almost fully built
* 1.1.0.1  2025.11.6   修复了DringAreaAlpha的BUG																.Fixed the bug in DringAreaAlpha
* 1.1.0.2              更新了ProjectOver函数																	.Updated the ProjectOver function
* 1.1.0.3  2025.11.7   添加了ProjectPause/暂停函数																.Added ProjectPause function
* 1.1.0.4              重构了ProjectGod结构体																	.Refactored the ProjectGod structure
* 1.1.0.5              重构了ProjectRun函数																	.Refactored the ProjectRun function
* 1.1.0.5              修复了ProjectPause的BUG																.Fixed the bug in ProjectPause
* 1.1.0.6              添加了ProjectFunctionSwitch/函数切换函数													.Added ProjectFunctionSwitch function
* 1.1.1.0  2025.11.10  添加了SDL支持																			.Added SDL support
* 1.1.1.1  2025.11.27  解决了StarEngine一直没有解决的窗口焦点问题													.Resolved the window focus issue that StarEngine had never fixed
* 1.1.3.0  2025.11.29  添加了按钮控件																			.Added a button control
* 1.1.3.1              更新了启动图案																			.Updated the startup pattern
* 1.1.3.2  2025.11.29  更新了启动图案																			.Updated the startup pattern
* 1.1.3.3  2025.11.30  修复了模式切换时只能切换初始逻辑函数而不是设计的切换当前逻辑函数的BUG							.Fixed the bug where only the initial logic function could be switched during mode switching, instead of switching to the currently designed logic function.
* 1.1.4.0              添加了实体统计函数																		.Added ProjectFindEntityAllNumber function
* 1.1.4.1  2025.12.1   更新了按钮控件,添加了触发																	.Updated the button control and added a trigger
* 1.1.4.2              将按钮函数更换到了工具区																	.Moved the button function to the toolbar
* 1.1.5.0              添加了触发方式函数																		.Added ButtonSetTriggerMode function
* 1.1.5.1              添加了PROJECTMODULE宏,与PROJECTSETTING功能一样											.Added the PROJECTMODULE macro, which functions the same as PROJECTSETTING
* 1.1.5.2              更新了实体系统																			.Updated the entity system
* 1.1.6.0              实现了C++的适配,如果要使用C++版本的,请确保您所有.c的文件后缀已经改成.cpp						.C++ adaptation has been implemented. If you want to use the C++ version, please make sure all your .c file extensions have been changed to .cpp.
* 1.1.6.1              修复了ProjectFindEntityAllNumber函数的计数错误											.Fixed the counting error in the ProjectFindEntityAllNumber function
* 1.1.6.2              更新了实体系统,现在不再会有堆栈溢出的问题了。
					   因为PROJECTGOD里的ENTITYINDEX entityindex[MOON_ENTITY_NUMBER];变成了ENTITYINDEX* entityindex;.The entity system has been updated, and there will no longer be stack overflow issues because ENTITYINDEX entityindex[MOON_ENTITY_NUMBER]; in PROJECTGOD has been changed to ENTITYINDEX* entityindex;
* 1.1.6.3  2025.12.2   修复了引擎内部实体类型不正确的BUG															.Fixed a bug where the internal entity type of the engine was incorrect
* 1.1.6.4              更新了ProjectFindEntityAllNumber函数,现在会打印更详细的内容								.Updated the ProjectFindEntityAllNumber function, it will now print more detailed informatio
* 1.1.6.5              将Hash槽位改成质数,减少Hash冲突															.Change the hash slots to prime numbers to reduce hash collisions
* 1.1.6.6  2025.12.4   优化了主线程逻辑																		.Optimized the main thread logic
* 1.1.7.0              添加了动画创建函数AnimeCreate,现在创建动画不再是手动档了										.Added the animation creation function AnimeCreate, now creating animations is no longer done manually.
* 1.1.8.0              AnimeCreate函数功能实现错误,应该实现的是ImageLoadBatch函数,现在将重新实现AnimeCreate函数		.The AnimeCreate function is implemented incorrectly; it should implement the ImageLoadBatch function. The AnimeCreate function will now be re-implemented.
* 1.1.8.1  2025.12.5   解决了引擎内部的一个隐性BUG,即引擎内部的属性函数错误的保存了初始的函数导致的无论如何调整,
*                      都会回到最初的引擎传入最初的模块,即中央模块,现已修复。
*                      但我们仍然保留了它,如果您想要使用,可以在配置栏里将MOONCENTRALDISPATCHER改成1。					.Fixed an internal hidden bug in the engine, where the engine's internal property function incorrectly saved the initial function, causing it to always revert to the original module passed by the engine, namely the central module, regardless of any adjustments. This has now been fixed. However, we have kept it; if you want to use it, you can change MOONCENTRALDISPATCHER to 1 in the configuration panel.
* 1.1.8.2              添加了函数切换宏,防止您因手动切换模块导致的二级指针问题										.Added a function toggle macro to prevent secondary pointer issues caused by manually switching modules.
* 1.1.8.3              解决了鼠标坐标总是不稳定的问题																.Resolved the issue of the mouse coordinates being unstable.
* 1.1.8.4              修复了按钮系统的空回调函数指针BUG															.Fixed the bug of null callback function pointer in the button system
* 1.1.8.5  2025.12.6   更新了按钮系统,修复了一些BUG																.Updated the button system and fixed some bugs
* 1.1.8.6              修复了MOONCENTRALDISPATCHER的笔误														.Fixed a typo in MOONCENTRALDISPATCHER
* 1.1.8.7  2025.12.9   修复了ButtonDetection函数因为忘记取模造成的Hash溢出时的BUG									.Fixed a bug in the ButtonDetection function where forgetting to use the modulo caused a hash overflow
* 1.1.9.0  2025.12.10  添加了字符转换函数CharToWchar															.Added character conversion function CharToWchar
* 1.1.9.1  2026.1.14   添加了开发者控制台调控模式																.Added developer console control mode
* 1.2.0.0  2026.1.23   
*                      1.更换了API的名称,如果需要继续使用旧API,请导入Moon_Compatible_Function.c并且把MOONCOMPATIBLE配置打开
*                      2.正在逐步添加SDL支持
*                      3.删除了MOONCENTRALDISPATCHER配置选项
*                      4.基本完成引擎核心函数的SDL重写
*					   1. The API name has been changed. If you need to continue using the old API, please import Moon_Compatible_Function.c and enable the MOONCOMPATIBLE configuration.
*					   2. Gradually adding SDL support.
*					   3. The MOONCENTRALDISPATCHER configuration option has been removed.
*					   4. The SDL rewrite of the engine core functions is basically completed.
* 
* 1.2.0.1  2026.1.24   
*                      1.完成了SDL下引擎原本就有休眠模式功能的重写
*                      2.解决了SDL的消息队列
*					   1. Completed the rewriting of the engine's original sleep mode function under SDL
*					   2. Fixed SDL's message queue
* 1.2.0.2              解决了SDL下鼠标坐标的问题																.Solved the issue of mouse coordinates in SDL
* 1.2.0.3  2026.1.25   1.重构了绘图逻辑,将绘图逻辑改到单独的线程中,属性线程逻辑移到了主线程
*                      2.已测试重构后的GDI部分,和原来的效果相同
*					   1. Refactored the drawing logic, moving the drawing logic to a separate thread and the property thread logic to the main thread
*					   2. The refactored GDI part has been tested and the effect is the same as before
* 1.2.0.4  2026.1.26   解决了纹理创建失败的问题																	.Resolved the issue of texture creation failure
* 1.2.0.5  2026.1.27   解决了SDL矩形绘制不了的问题,原因是参数SDL_Rect改成了SDL_FRect								.Solved the problem of SDL rectangles not being drawable; the reason was that the parameter SDL_Rect was changed to SDL_FRect.
* 1.2.0.6              更新了MoonSetDrawImage函数																.Updated the MoonSetDrawImage function
* 1.2.0.7  2026.1.28   正在测试DrawingArea系列函数
*                      1.MoonDrawingArea      已通过
*                      2.MoonDrawingAreaRound 已通过
*					   Testing the DrawingArea series of functions
*					   1. MoonDrawingArea passed
*					   2. MoonDrawingAreaRound passed
* 1.2.0.8  2026.1.29   1.MoonDrawingAreaAlpha 已通过
*                      2.修复了MoonDrawingAreaRound的锚点旋转,保持和GDI版本功能的相同
*                      3.动画系统测试完成
*					   1. MoonDrawingAreaAlpha has passed
*					   2. Fixed the anchor rotation of MoonDrawingAreaRound to maintain the same functionality as the GDI version
*					   3. Animation system testing completed
* 1.2.0.9  2026.1.30   修改了MoonAnimeRun的参数,现在倍数参数size从int改成了float									.Modified the parameters of MoonAnimeRun, now the multiplier parameter size has been changed from int to float
* 1.2.1.0              添加了MoonSDLTextFont系列函数															.Added the MoonSDLTextFont series of functions
* 1.2.1.1              现在MoonDrawingAreaAlpha的最后一个参数有用了,它将用来控制Alpha通道							.Now the last parameter of MoonDrawingAreaAlpha is useful; it will be used to control the Alpha channel.
* 1.2.1.2              取消了MoonDrawingAreaAlpha函数最后一个参数的作用,该功能将集成到MoonModifyAlpha中,原因是性能太低,只要一次Alpha修饰就足够了.The effect of the last parameter of the MoonDrawingAreaAlpha function has been removed. This functionality will be integrated into MoonModifyAlpha because the performance is too low, and a single Alpha modification is sufficient.
* 1.2.1.3  2026.1.31   修复了MoonImageLoadBatch的BUG															.Fixed the bug in MoonImageLoadBatch
* 1.2.2.0  2026.2.1    新添加了MoonSetDrawImage和MoonImagePlgBit函数											.Newly added MoonSetDrawImage and MoonImagePlgBit functions
* 1.2.2.1              修复了MoonImagePlgBit函数在透明度变化时不能显示的BUG										.Fixed the bug in the MoonImagePlgBit function where it could not display when the opacity changed
* 1.2.2.2  2026.3.16   修复了在切換回GDI時的未定義BUG															.Fixed an undefined bug when switching back to GDI
* 1.2.2.3              修復了MoonTextFont函數的BUG																.Fixed a bug in the MoonTextFont function
* 1.2.2.4  2026.4.5    新增了MoonDrawingAreaUV函數,現在可以使用UV矩形坐標											.Added the MoonDrawingAreaUV function, now UV rectangular coordinates can be used
* 1.2.2.5  2026.4.7    將SDL的透明混合模式改成了SDL_SCALEMODE_NEAREST											.Changed SDL's alpha blending mode to SDL_SCALEMODE_NEAREST
* 1.2.2.6  2026.4.9    修復了幀率控制失效的問題																	.Fixed the issue frame rate control not working
* 1.2.2.7  2026.4.13   新增了	MoonCircle函數,用於繪製圓形														.Added the MoonCircle function, used for drawing circles
* 1.2.2.8  2026.4.14   新增了image_old功能,減少SDL_SetRenderTarget帶來的狀態切換提升性能							.Added the image_old feature, reducing state switches caused by SDL_SetRenderTarget to improve performance
* 請注意,由於本人的疏忽,忘記了更新函數不是小版本號,1.2.2.4版本應該是1.2.3.0,那麽1.2.2.7應該是1.2.4.0,但是本人不會再更改舊版本號,從這裏開始應用新版本號
* 這算是一個小問題,只有1.2.3.x和1.2.4.0有問題
* 1.2.4.1  2026.4.15	將MoonImagePlgBit函數的名字改成了MoonDrawingAreaPlgBit,您可以開啓兼容模式以使用原來的名字
* 1.2.4.2				完成了SDL模式下的MoonGetColor,現在這個函數在SDL下可以使用了
* 1.2.5.0				新增了MoonImageHandlePassageMatrix函數,可以修飾紋理顔色通道
* 1.2.5.1  2026.4.16	修復了一些函數返回值的錯誤,比如int (*ProjectSetting_2)(PROJECTGOD*)改成了void (*ProjectSetting_2)(PROJECTGOD*)
*						這是一些在C++下的小錯誤
*						從這個版本開始,我們將逐步把GDI換成DirectX 11
*						因爲DirectX 11涉及到C++,可能要換語言
*						這是極其一個重大的更新,遠非重構可比
*						因此版本號為2.x.x.x
*						未來的繪圖架構將采用消息式
*						開發者不再能夠自由的調用繪圖函數,繪圖函數底層將會變成向引擎呼叫消息
*						引擎根據消息機中的消息來繪圖
*						这样就能够在游戏中直接呼叫
*						不過表現形式上與以前沒有區別
*						大概率依舊會用傳統的C語法來寫作
*						1.2.5.1版本可能是1.0最後一個版本
* 
* 2.0.0.beta	2026.4.16	正式開始DirectX的編寫
* 2.0.0.beta	2026.4.20	基本完成DirectX的前置準備工作
* 2.0.0.beta	2026.4.21	更改了宏的名稱,您可以通過開啓兼容模式來兼容舊名稱
*							修復了一個小錯誤,MoonDeletImage的命名改爲MoonDeleteImage
*							正在重寫全部的繪圖函數
*							修復了projectbitmap提前釋放導致的0xC0000005
* 2.0.0.beta	2026.4.22	正在重寫全部的繪圖函數
*							正在測試繪圖函數
*							DirectX太难了
*							現在轉向OpenGL
* 2.0.0.beta	2026.4.23	如果使用OpenGL,那麽就不會使用C++來編寫
							但因爲這次依舊是架構升級,會包含很多前所未聞的新東西
							簡單來説就是除了繪圖要更改,還要建立一套全引擎通信系統
							開發者可以嵌入自己的消息和處理方式,由引擎統一調用
* 2.0.0.beta	2026.4.24	完成OpenGL初始化設置
*							添加了新函數MoonPrompt,用於引擎内部提示
* 2.0.0.beta	2026.4.25	修改了MOON_POINT3D的内容,增加了r,g,b,a成員
*							添加了MoonType.h頭文件,現在所有的類型定義都在裏面
*							爲了類型安全,實體類型加入字符串來區分類型,同時保留length字段快速初步判斷
* 2.0.0.beta	2026.4.26	MoonCreateImage與MoonDeleteImage已經編寫完成
* 2.0.0.beta	2026.4.28	新加入了MoonString宏,用於字符串拼接
*							正在實現紋理渲染
*							簡化了projectgod結構體
*							FUNCTIONSWITCH改成了MOON_FUNCTIONSWITCH
*							HashFindEntity改成了MoonHashFindEntity
*							PROJECTMODULE改成了MOON_PROJECTMODULE
*							PROJECTSETTING改成了MOON_PROJECTSETTING
*							所有更換的名字,您都可以開啓兼容模式來避免可能的報錯
*							將MoonSetDrawImage改爲了公共函數,參數略有不同
* 2.0.0.beta	2026.4.29	將所有頭文件放入了MoonType.h,保持主頭文件的乾净
*							窗口句柄改成了MOON_HWND
*							將MoonCreateEntityIndex 的 int length改成 size_t size_len,明確語義
*							MOONBUTTON改成了MOON_BUTTON
*							將OPEN_SDL更改爲了MOON_OPEN_SDL
*							主循環編寫完成
*							紋理到后緩衝區與圖案輸出至紋理的鏈路以基本打通
* 2.0.0.beta	2026.4.30	修復了SDL模式下無法調整窗口位置的BUG
* 2.0.0.beta	2026.5.4	打通了绘制到纹理与纹理绘制后缓冲区的链路
* 2.0.0.beta	2026.5.5	目前核心部分已经完全砍掉SDL,尽管它已经很成熟,和核心绘图代码有了深入的嵌合
*							SDL将作为外设使用,比如音乐	
*							彻底解决核心部分绘制管线问题
*							下一步的工作是开始消息式绘图
*							更改了工具函数
*							新添加了MoonUtilityLoad,用于加载工具区函数可能所需的资源
* 2.0.0.beta	2026.5.6	废弃了MoonCharToLPCWSTR函数
*							废弃了MoonWindowRun函数
*							将一些核心函数转移到了MoonCore.h中
*							将MoonProjectFunctionSwitch的参数由
*								MoonProjectFunctionSwitch(int (**function_1)(MOON_PROJECTGOD*), int (*function_2)(MOON_PROJECTGOD*))
*							更改为
*								MoonProjectFunctionSwitch(MOON_PROJECTGOD* project, char mode, int (*function_2)(MOON_PROJECTGOD*))
*							移除了MOON_FUNCTIONSWITCH
*							正式开始设计消息式绘图
*							也许最后所有的操作函数都会消失
*							整个引擎变成一个巨大的消息机
*							谁知道呢
* 2.0.0.beta	2026.5.7	继续设计消息机制
*							大幅削减了PROJECTGOD的能力
* 2.0.0.beta	2026.5.8	继续设计消息机制
*							添加了MoonProjectSendMessage函数,用于发送消息
*							将MOON_PROJECTGOD中的DEAD参数放到内核中
*							将MoonProjectFunctionSwitch的mode参数改为module,明确语义
*							添加了MoonProjectGetMessage函数,用于内部处理消息
*							解决了发送消息的问题
*							解决了处理消息的问题
*							解决了焦点问题
*							将MOON_COMMAND类型的枚举值改成了MOON_MESSAGE类型的枚举值
* 2.0.0.beta	2026.5.9	移除了Moon_SDL_Utility_Function.c文件
*							通过SDL_Delay解决了Sleep函数精确度不高的问题
*							去除了核心部分一些短路求值的写法
*							去除了对Win的多线程依赖,改成了依赖SDL3
*							由于函数模块的转移
*								MoonProjectFunctionSwitch的参数由
*									MoonProjectFunctionSwitch(MOON_PROJECTGOD* project, char module, int (*function_2)(MOON_PROJECTGOD*))
*								改成
*									MoonProjectFunctionSwitch(char module, int (*function_2)(MOON_PROJECTGOD*))
*							
*							MoonProjectFunctionSwitch移到了核心层
* 
*							修改了MoonProjectSendMessage的参数
*								由
*									MoonProjectSendMessage(unsigned char MOON_MODULE_X, MOON_MESSAGE message, MOON_METADATA metadata)
*								改为
*									MoonProjectSendMessage(MOON_MESSAGE message, MOON_METADATA metadata)
*							现在不必操心传入的是什么模块 
* 2.0.0.beta	2026.5.17	修改了MoonDrawingAreaAlpha的参数
*								由
*									MoonDrawingAreaAlpha(MOON_IMAGE* image_1, MOON_IMAGE* image_2, int x, int y, int width, int height, int transparent_color)
*								改为
*									MoonDrawingAreaAlpha(MOON_IMAGE* image_1, MOON_IMAGE* image_2, int x, int y, int width, int height)
* 
*							修改了MoonRendererLoad的参数
*								由
*									MoonRendererLoad(MOON_PROJECTGOD* project, char** vertex_shader, char** pixel_shader, unsigned int* shader_program)
*								改为
*									MoonRendererLoad(char** vertex_shader, char** pixel_shader, unsigned int* shader_program)
*								
*							MoonRendererLoad更改名称为MoonShaderLoad
*							MoonDrawingArea类函数改名为MoonDrawArea
*							完成了绘制消息队列的绘制实验
*							添加了MoonDrawLoad与MoonDrawOver,负责绘制模块的初始化与终止
*							删除了MoonDrawAreaAlpha,现在功能合并到MoonDrawArea中了
* 2.0.0.beta	2026.5.18	正在编写绘图函数
*							添加了MoonCore家族函数,用于底层绘图
*							目前已经完成了Pixs,Lines
*							实现了绘图的基本合并
*							完成了Box的绘制
*							添加了MoonTriFull函数,用于绘制填充三角形
*							
*							修改了MoonLine的参数
*								由
*									MoonLine(MOON_IMAGE* image, int x1, int y1, int x2, int y2, int width, int color)
*								改为
*									MoonLine(MOON_IMAGE* image, int x1, int y1, int x2, int y2, int color)
*
*							修改了MoonBox的参数
*								由
*									MoonBox(MOON_IMAGE* image, int x1, int y1, int x2, int y2, int width, int color)
*								改为
*									MoonBox(MOON_IMAGE* image, int x1, int y1, int x2, int y2, int color)
*							
*							目前已经完成了Tri,BoxFull,Box
*							实现了纹理到纹理
*							正在解决坐标问题
*							解决了坐标问题,原因是未重置视口
*							添加了MoonRGBA宏,方便写入颜色值
* 2.0.0.beta	2026.5.19	正在补充属性消息的具体处理
*							给所有绘图函数加上了MoonDraw前缀
*							MoonDeleteImage的名称改为MoonImageDelete
*							MoonCreateImage的名称改为MoonImageCreate
*							可以通过兼容模式适配新名称
*							改善了MoonDrawArea的内部处理,现在宽度与长度直接作用于逻辑坐标,可以实现拉伸
*							实现了MoonImageLoad函数
*							使用stb_image库以实现更多格式的图片加载
*							实现了MoonImageLoadBatch函数
*							删除了MOON_IMAGE的逻辑尺寸
*							修复了因直接修改image_size造成的隐性BUG,虽然看起来消息队列会主动重新赋值且并未发生问题
*							这是一个很隐性的BUG,如果没有严格的推导,在应用层根本看不出区别
*							这是一个"静默数据污染"（Silent Data Corruption），是最难发现的BUG类型之一
*							image_size应该保持内部一致性,即内部只读,只能由外部也就是应用层来修改
*							删除了MOON_TRANSPARENTCOLOR宏
*							MoonRANGE更改名称为MoonRange
*							消息部分添加了着色器对象,现在可以传入自定义着色器
* 2.0.0.beta	2026.5.20	整合了MOON_METADATA的graphic与image,现在它们共享某些字段
*							MoonsetDrawImage更改名称为MoonImageShader
*							将MoonDrawAreaRound的ap参数修改成了float类型
*							将MoonDrawAreaUV转成了通用函数
*							添加了MoonMin,MoonMax宏
*							完成了MoonDrawAreaUV函数
*							完成了MoonDrawAreaRound函数
*							完成了MoonDrawAreaPlgBit函数
* 2.0.0.beta	2026.5.21	所有绘图函数的color参数都由int改成了unsigned int类型
*							修改了MoonTextFont(MOON_IMAGE* image, const char* text, int x, int y, unsigned int color, float sizewidth, float sizeheight)的参数
*								由
*									MoonTextFont(MOON_IMAGE* image, int x, int y, const wchar_t* text, COLORREF color, BOOL back, const wchar_t* font, int sizewidth, int sizeheight, int texttilt, int fonttilt, int FW_, int underline, int deleteline, int DEFAULT_)
*								改为
*									MoonTextFont(MOON_IMAGE* image, const char* text, int x, int y, float sizewidth, float sizeheight)
*							添加了MOON_MESSAGE_DRAW_TEXT消息,用于绘制文字(内置默认字体)
*							发现了某种奇怪的BUG
*							如下图
*							int w = 200 + 200, h = 200 + 200;
									MOON_POINT2D points[4] = 
									{
										{200, 0},
										{w	, 0},
										{200, h},
										{w	, h},
									};

									MOON_POINT2D points_2[4] =
									{
										{200, 0},
										{400, 0},
										{200, 400},
										{400, 400},
									};
									倘若分别使用如下函数
									MoonDrawAreaPlgBit(&enginebackalpha, image_dd, points);
									MoonDrawAreaPlgBit(&enginebackalpha, image_dd, points_2);
									那么points就不能渲染,甚至导致黑屏
									而points_2就可以
									这似乎是MSVC编译器内部初始化的问题,那么解决不了
									如果您必须使用,比如应用偏移量
									那么可以如下操作
									[示例]
											static int x, y;
											if (MoonKeyState(MOON_KEY_UP))y --;
											if (MoonKeyState(MOON_KEY_DOWN))y ++;
											if (MoonKeyState(MOON_KEY_LEFT))x --;
											if (MoonKeyState(MOON_KEY_RIGHT))x ++;

											static MOON_POINT2D points[4] =
											{
												{200, 0},
												{400, 0},
												{200, 400},
												{400, 400},
											};
											points[0].x += x;
											points[0].y += y;
											points[1].x += x;
											points[1].y += y;
											points[2].x += x;
											points[2].y += y;
											points[3].x += x;
											points[3].y += y;
											MoonDrawAreaPlgBit(&enginebackalpha, image_dd, points);
							添加了MoonMouseState,用于检测鼠标
* 2.0.0.beta	2026.5.22	MoonTextFont的名称改为MoonDrawTextFont
*							解决了大量难题,包括默认字体库(ASCII码 38000+行),单条消息绘制等等
*							完成了MoonDrawtextFont函数
*							废弃了MoonGetColor,这个功能使用频次太低
*							废弃了MoonMouseState函数
*							新增了MoonKeyReal函数,用于检测硬件状态
*							功能与MoonKeyState的单次检测不同,是即时检测
*							MoonAnimeRun函数的size参数改为int类型
* 
* -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
* 从2.0开始,添加一条新规则,
* 如果是稳定版,则为2.x.x.x+
* 如果是破坏性版本,即参数数量(不包含类型)对不上,则为2.x.x.x-
* 反之则为普通版本,不一定稳定
* Moon Engine 2.0 正式版
* 
* 2.1.0.0		2026.5.22	2.0版本正式完成
* 2.1.0.1		2026.5.24	MoonSETMOUSECOORD更改名称为MoonSetMouseCoord
* 2.1.0.2					去除了对Windows.h头文件的依赖
* 2.1.0.3					去除了MoonRunProgram函数
* 2.1.0.3					去除了MoonCMD函数
* 2.1.0.4					添加了MOON_CURSOR_MODE枚举
* 2.1.0.5+					修正了MoonAnimeInit一个不合理的参数
*							MoonSetMouseCoord更改名称为MoonSetMouse
*							更改了部分注释的位置,修复了IDE下注释可能显示不正确的体验
*							MOON_POINT2D添加了成员
							struct { float fx, fy; };
							struct { float fw, fh; };
							MoonImageShader更改名称为MoonImageShader
* 2.1.0.6		2026.5.26	MoonProjectFunctionSwitch新增空指针预防与提示
* 2.1.0.7					MoonProjectFunctionSwitch新增字符串超出具体提示
* 2.1.0.8					规范了MoonShaderLoad内部消息的初始化
* 2.1.0.9					由于模块切换的同步问题很难解决,所以
*								回归了
*									MOON_MESSAGE_ATTR_SETLOGIC
*									MOON_MESSAGE_ATTR_SETDRAW
*								注释掉了
*									MOON_MESSAGE_LOGIC_SETDRAW
*									MOON_MESSAGE_DRAW_SETDRAW
*							统一由属性线程进行切换
* 2.1.0.10					将ProjectSetting_2纳入OpenGL上下文范围
* 2.1.0.11					针对一些警告,做了语法上的优化
* 2.1.0.12					去除了对这两个链接库的依赖
*							#pragma comment( lib,"Winmm.lib")
*							#pragma comment(lib, "Msimg32.lib")
* 2.1.0.13					去除了对这两个链接库的依赖
**								设计了
*									MOON_MESSAGE_LOGIC_SETLOGIC
*									MOON_MESSAGE_LOGIC_SETDRAW
*								注释掉了
*									MOON_MESSAGE_ATTR_SETDRAW
*									MOON_MESSAGE_ATTR_SETDRAW
* 2.1.0.14+					为了彻底解决并发问题,完全去除了属性消息
* 2.1.0.15		2026.5.28	MoonFindEntity添加了空指针预防
* 2.1.0.16					MoonDrawTextFont修复了内部textbuffer变量的使用错误
* 2.1.0.17		2026.5.28	添加了两个新的头文件
*								MoonUtility.h
*								MoonDraw.h
*							让文件结构更清楚
* 2.1.0.18		2026.5.29	添加了未响应机制
*							如果每帧时间超过2000ms
*							那么自动跳入未响应状态
* 2.1.0.19-		2026.5.30	重构了按钮系统
*								
							原来的
								enum
								{
									MOON_BUTTONPRESS = 1,																																				//按下
									MOON_BUTTONRHOVER,																																					//悬停
								};

								typedef struct MOONBUTTON
								{
									char nameid[255];
									int x;
									int y;
									int width;
									int height;
									char mode;
									unsigned char triggermode;
									int (*ButtonModePress)   (PROJECTGOD* project, struct MOONBUTTON* buton);	//按下
									int (*ButtonModeRelease) (PROJECTGOD* project, struct MOONBUTTON* buton);	//松开
									int (*ButtonModeHover)   (PROJECTGOD* project, struct MOONBUTTON* buton);	//悬停
								}MOONBUTTON;

								更改为
								enum
								{
									MOON_BUTTON_FALSE,			//不存在按钮
									MOON_BUTTON_PRESS = 1,		//按下
									MOON_BUTTON_PRESS_LONG,		//长按
									MOON_BUTTON_RHOVER,			//悬停
								};

								typedef struct MOONBUTTON
								{
									int x;
									int y;
									int width;
									int height;
									char mode;
									unsigned char triggermode;
									int (*ButtonModePress)   (struct MOONBUTTON* buton, void* context);	//按下
									int (*ButtonModePressL)  (struct MOONBUTTON* buton, void* context);	//长按
									int (*ButtonModeHover)   (struct MOONBUTTON* buton, void* context);	//悬停
								}MOON_BUTTON;
*							
*							修改了MoonButtonDetection的参数
*								由
*									MoonButtonDetection(PROJECTGOD* project, char* name)
*								改为
*									MoonButtonDetection(MOON_BUTTON* button, int x, int y, char* context)
* 2.1.1.0					添加了MoonFileLoad_TEXT函数,用于加载文本文件
* 2.1.1.1		2026.5.31	修复了UV原点在左下角的问题
* 2.1.1.2-					UV函数优化了width和height的参数体验
*							现在不再是在原图层上裁切相同大小的区域,而是匹配传入的width和height
* 2.1.1.3-					
*				2026.6.4	修改了MoonImageCreate的参数
*								由
*									MoonImageCreate(MOON_PROJECTGOD* project, MOON_IMAGE* image, int bmpwidth, int bmpheight)
*								改为
*									MoonImageCreate(MOON_IMAGE* image, int bmpwidth, int bmpheight)
* 
*							修改了MoonImageLoadBatch的参数
*								由
*									MoonImageLoadBatch(MOON_PROJECTGOD* project, MOON_IMAGE* image, int totalnumber, const char** name, int width, int height)
*								改为
*									MoonImageLoadBatch(MOON_IMAGE* image, int totalnumber, const char** name, int width, int height)
* 2.1.1.4		2026.6.6	修正了定义参数与实际参数不一致的情况
* 2.1.1.5		2026.6.7	实现了纹理的批量化渲染
* 2.1.1.6		2026.6.8	优化了渲染队列,现在不再是每帧realloc/free,而是在原先扩容的基础上分配
* 2.1.1.7		2026.6.9	优化了渲染机制,现在每个顶点都会分配一个单独的颜色,而metadata.draw.color则用于颜色通道
* 2.1.1.8					修复大量不规范的语法问题
* 2.1.1.9					对纹理也应用了metadata.draw.color
* 2.1.1.10					优化了
*									MoonCoreDrawArea
*									MoonCoreDrawAreaUV
*									MoonCoreDrawAreaPlgBit
*							统一优化成了
*									MoonCoreDrawArea
*							当然这对您的应用层代码没有影响
* 2.1.1.11-					修改了MoonDrawTextFont的参数
*								由
*									MoonDrawTextFont(MOON_IMAGE* image, const char* text, int x, int y, int sizewidth, int sizeheight)
*								改为
*									MoonDrawTextFont(MOON_IMAGE* image, const char* text, int x, int y, int sizewidth, int sizeheight,unsigned int color)
* 2.1.1.12					修复了纹理批处理时依旧无法合并的BUG
*								原因是
*									忘记更新image_resource_old
*							现在是真正的纹理批量绘制
* 2.1.1.13		2026.6.10	将
*								MOON_MESSAGE_ALL
*								MOON_MESSAGE_SPECIFIC
*							转入了
*								MoonCore
* 2.1.1.14					将
*								MoonCorePixs
*								MoonCoreLines
*								MoonCoreTriFulls
*							合并为
*								MoonCoreGraphic
* 2.1.1.15-					MOONBUTTONCREATE 重命名为 MOON_BUTTON_CREATE
* 2.1.1.16-					修改了MoonButtonSetTriggerMode的参数
*								由
*									MoonButtonSetTriggerMode(MOON_PROJECTGOD* project, char* name, unsigned char key)
*								改为
*									MoonButtonSetTriggerMode(MOON_PROJECTGOD* project, MOON_BUTTON* button, unsigned int key)
* 2.1.2.0+		2026.6.11	添加了MoonStrMatch家族函数
*								MoonStrMatch_Prefix				匹配前缀,返回长度
*								MoonStrMatch_PrefixIgnore		匹配前缀,忽略特定字符,返回长度
*								MoonStrMatch_Replace			替换字符
* 2.1.2.1		2026.6.12	修复了MoonSetTemp函数传入空指针崩溃的BUG
* 2.1.3.0					添加了函数
*								MoonDrawBox_Round
*								MoonDrawBoxFull_Round
*							用于绘制圆角矩形
* 2.1.4.0					添加了函数
*								MoonVsn
*							用于查看版本号
* 2.1.4.1-		2026.6.13	按钮结构体添加了成员
* 2.1.4.2					修复了未响应机制
* 2.1.4.3					添加了背面剔除,您的代码可能需要调整
* 2.1.4.4					现在进入暂停会提示
* 2.1.4.5					优化了暂停机制
* 2.1.4.6					添加了MOON_KEY_MODE枚举与按钮消息MOON_MESSAGE_KEY
* 2.1.4.7					修复了按钮系统因为MoonKey家族函数导致的状态污染问题
*								原因是在不同线程修改的状态不同
*							它们不是线程安全的
* 2.1.5.0		2026.6.14	添加了函数
*								StrMatch_PrefixIgnoreStr
* 2.1.6.0					添加了函数
*								MoonAlloc
*							支持 malloc/calloc/realloc 三种分配模式
*							自动注册分配的内存，引擎退出时统一释放
*							防止内存泄漏，简化内存管理
* 2.1.7.0					添加了函数
*								MoonFree
*							用于提前回收
*								MoonAlloc
*							分配的内存
* 2.1.7.1		2026.6.15	优化了代码布局
* 2.1.7.2					重写了MoonDegRad函数
* 2.1.7.3					优化了冗余代码
* 2.1.8.0					添加了函数
*								MoonShaderUniform
*							用于自定义着色器
* 2.1.8.1		2026.6.16	将函数
*								MoonImageDesignated
*							移入了核心区
* 2.1.8.2					将函数
*								MoonImageShader
*							移入了核心区
* 2.1.8.3					ENTITYNUMBER
*								改名为
*							MOON_ENTITY_NUMBER
*							您可以通过开启兼容模式来解决报错问题
* 2.1.8.4					现在可以通过消息系统来设置
*								MoonShaderUniform
* 2.1.8.5					MoonAlloc_Registry
*								改为了内部函数,dll不再能够导出
* 2.1.8.6		2026.6.17	MOON_POINT3D和内部渲染脱轨,渲染图元将使用内部的MOON_GRAPHIC_VECTER结构体
* 2.1.9.0		2026.6.19	添加了函数
*								MoonMatrix4_4Mul
*							用于矩阵计算
* 2.1.9.1		2026.6.21	修复了按钮宏初始化时的错误
* 2.1.9.2					添加了uniform可能有时失效的BUG
* 2.1.9.3					修复了
*								MoonCoreGraphic
*							声明与形参不同的BUG
* 2.1.9.4					修复了
*								按键消息单次按鍵的单帧不连续性
*							现在按下一个按钮在单帧内一定输出确定的,持久性的消息
*							而不是之前的读取MOON_KEY_MODE_PRESS后
*							后续读取的到MOON_KEY_MODE_FALSE的问题
* 2.1.9.5					取消了消息
*								MOON_MESSAGE_LOGIC_OPEN
*							因为看起来似乎没什么用
* 2.1.9.6					修复了
*								MoonFree
*							释放内存后可能存在的野指针问题
* 2.1.9.7-		2026.6.22	修改了MoonMusic的参数
*								由
*									MoonMusic(const char* File)
*								改为
*									MoonMusic(MOON_MUSIC* music)
* 2.1.9.8-					MOON_METADATA中
*								dead成员被认为是多余的
*							已注释掉
* 2.1.9.9					添加了MOON_MUSIC结构体
* 2.1.9.10					MoonMusic的返回值改为int
* 2.1.9.11					MoonMusic函数可以使用了
* 2.1.10.0					添加了函数
*								MoonMusicInit_Wav
*							用于初始化音频
* 2.1.11.0					添加了函数
*								MoonDead
*							用于杀死项目
* 2.1.12.0					添加了函数
*								MoonMusicSet
*							用于设置音频状态
* 2.1.12.1-		2026.6.23	修改了MoonDrawTextFont的参数
*								由
*									MoonMusicSet(MOON_MUSIC* music, unsigned char on_or_off)
*								改为
*									MoonMusicSet(MOON_MUSIC* music, MOON_MUSIC_MODE on_or_off, float start, float end)
* 2.1.12.2					添加了枚举
*								MOON_MUSIC_MODE
* 2.1.12.3					优化了音频播放
* 2.1.13.0+					添加了函数
*								MoonMusicAgain
*							用于从头播放音乐
* 2.1.13.1		2026.6.24	修复了部分语法规范
* 2.1.13.2					摆脱了Moon_Compatible_Function.h的依赖
*								运行引擎现在不必需要Moon_Compatible_Function.h
* 2.1.13.3					删除了
*								#pragma comment(lib, "SDL3.lib")
*								#pragma comment(lib, "glfw3.lib")
*								#pragma comment(lib, "opengl32.lib")
*							因为在IDE中配置更加便捷
* 2.1.13.4					MOON_PROJECTGOD中移除了hwnd成员
*								因为用不到,引擎不应该暴露这种底层
* 2.1.13.5					将案件枚举囊括为了MOON_KEY_TYPE
* 2.1.13.6					设置了image_resources和goal的检查
*								image_resources和goal不得相同
* 2.1.14.0					添加了函数
*								MoonDrawCircleFull
*								MoonDrawCircle
*							用于绘制圆
* 2.1.15.0					添加了函数
*								MoonGetFps
*							用于获取帧率
* 2.1.15.1		2026.6.25	修复了MoonUtilityLoad的参数不匹配的BUG
* 2.1.15.2-					修改了MoonFileLoad_TEXT的参数
*								由
*									MoonFileLoad_TEXT(char* file_name, char* text, unsigned int text_size)
*								改为
*									MoonFileLoad_TEXT(const char* file_name, char* text, unsigned int text_size)
* 2.1.15.3					添加了MOON_FILE结构体,用于文件管理
* 2.1.16.0					添加了函数
*								MoonFileRead_TEXT
*							用于加载文件
* 2.1.17.0		2026.6.26	添加了函数
*								MoonFileRead_Line
*							用于读取文件中任意一行
* 2.1.17.1					修改了
*								MoonFileRead_Line的返回值
*								现在返回这一行的长度
*								当file_buffer为MOON_NULL时
*								仅返回长度
* 2.1.17.2					优化了
*								MoonCreateEntityIndex函数
*							输出更加人性化且修复了在处理错误时崩溃的BUG
* 2.1.17.3					优化了对内部的自增逻辑,由后缀自增改为前缀自增
* 2.1.17.4					MOON_MESSAGE_THREAD_TYPE添加了枚举
*								MOON_MESSAGE_THREAD_TYPE_CACHE
*							内核态将维护一个新的消息队列
*								logic_message_cache
*							当发送到逻辑线程且逻辑线程返回
*								MOON_MESSAGE_THREAD_TYPE_CACHE时
*							意味着逻辑线程处于
*								MOON_MESSAGE_THREAD_TYPE_BUSY
*							但是不再像之前的版本一样直接丢弃消息
*							而是暂存入
*								logic_message_cache
*							由于绘制线程有强烈的顺序性要求
*							暂时不考虑缓存机制
* 2.1.17.5					彻底修复了按键检测不灵敏的问题
*								MoonKeyState
*								MoonKeyReal
*							改成了线程安全的,与直接使用消息无区别
* 2.1.17.6-					projectgod.entityindex移进了内核
* 2.1.17.7-					projectgod.window_width		窗口宽度
*							projectgod.window_height	窗口高度
*							移进了内核
* 2.1.17.8-					去除了按下~进入控制台的功能
* 2.1.18.0					添加了函数
* 2.1.18.1					修改了内置字体
* 
* ---------------------------------重要重构版本分割线---------------------------------
* 
* 2.2.0.0					去除了MOON_PROJECTGOD结构体
* 2.2.0.1		2026.6.30	添加了MOON_VECTOR结构体
* 2.2.0.2					添加了MoonCrashed宏,用于立即崩溃
* 2.2.0.3					逻辑线程的卡顿阈值提升到30s,防止游戏AI计算耗时过长导致误判
* 2.2.1.0-					将MoonRandom改为了函数
* 2.2.2.0					添加了函数
*								MoonCursorOffect
*							用于获取偏移值并限制在区域内部
*							该函数在
*								MOON_CURSOR_MODE_DISABLED
*							模式下很有用
* 2.2.3.0					添加了函数
*								MoonCursorGet
*							用于获取光标位置
* 2.2.4.0					添加了
*								MoonVector家族函数
*							用于计算向量
* 2.2.4.1		2026.7.1	因为暂时不考虑3D方向,所以关闭了背面剔除
* 2.2.4.2-					为了保持对Cpp的兼容
*							已经将所有的_Bool和其他不符合Cpp语法的地方修改了
*							如果您的代码依赖_Bool强制转换为1 or 0
*								比如 
*									int alpha = 100;
*									int beta = (_Bool)alpha;
*							我们建议您改为
*									int alpha = 100;
*									int beta = !!alpha;
* 2.2.4.3					修复了
*								MoonVector_SetDim
*							与预期相同维度下输出TRUE反而输出FALSE的BUG
* 2.2.4.4					实现了字体的批量渲染
*							现在每条字体消息无论有多少字(不超过上限)
*							都只渲染一次				
* 2.2.4.5					优化了纹理渲染,内部除法大大减少
*							在内部测试中(i7 3720QM, Quadro K2000m)
*								单张1200*800纹理渲染帧数提升了54.63%
* 2.2.5.0		2026.7.3	添加了
*								MoonSetPower
*							用于设置高性能模式,当参数大于0时,开启高性能模式,当参数小于等于0时,关闭高性能模式
* 2.2.5.1		2026.7.4	扩容了顶点总数
* 2.2.6.0					添加了
*								MoonProjectBuffer
*							用于直接获取后缓冲区
* 2.2.6.1		2026.7.6	修复了一个在2.1.17.1版本被标记为修复实则没修复的BUG
* 2.2.6.2					修复了MoonDrawArea纹理渲染顶点坐标错误的问题
*							原因是纹理坐标在旋转时与宽高严重耦合
* 2.2.7.0+					添加了
*								MoonShaderTexture
*								MoonShaderSolid
*							用于查询默认shader
* 2.2.7.1		2026.7.10	解决了绘制文字的消息上限
*							现在只要您的内存或者顶点足够
*							可以绘制不限量文字
* 2.2.8.0					添加了
*								MoonRoundOff
*							用于四舍五入
* 2.2.8.1		2026.7.12	实现了渲染文字时会根据换行符换行
* 2.2.8.2					修复了MoonFree提前释放注册表导致的后续注册失败
*								1,MoonAlloc 内部存在逻辑缺陷
*									采用或而非与来检测名字
*									导致奇怪的返回
*										[MoonAlloc] 无效的参数[calloc]
*								2,未检查注册表是否非空
*									导致再分配检查时
*										if (*ptr == moon_alloc.alloc[index])
*									分支实际查询到的是
*										NULL == NULL
*									造成的分配失败
* 2.2.8.3					调整了字体
* 2.2.9.0					添加了
*								MoonFileClose
*							用于提前回收文件
* 2.2.10.0					添加了
*								MoonThreadCreate
*							用于创建多线程函数
*							现在创建多线程函数将作为引擎的基本功能而不仅仅只是需要 SDL3 头文件的附加
* 2.2.11.0+					添加了
*								MoonLinked家族函数
*							用于双向链表
* 2.2.11.1		2026.7.17	优化了MoonVector函数
* 2.2.11.2					优化掉了带inline的函数
* 2.2.11.3					将MoonVector重构了部分
*							使其更加符合数学直觉与性能提升
*							例如
*								MoonVector_NormSize
*								MoonVector_Scale
*							不再强制赋值维度导致因为截断而丢失的数据
* 2.2.11.3					修复了MoonVector_Norm的BUG,此前输出的模长总是预定数值的sqrt2倍
*							原因是代码执行顺序错误
* 2.2.12.0+					添加了
*								MoonLogCreate
*							用于创建开发日志
* 2.2.12.1		2026.7.31	添加了
*								MOON_UNIFORM_TYPE_TEXTURE
*							类型
*							允许在着色器中传入多种纹理,而不仅仅只是之前的一个纹理
* 2.2.13.0					添加了
*								MoonProjectRun_Single
*							用于创建多线程函数
*							添加了单线程模式,现在允许只使用一个线程来进行游戏,获取更高的稳定性
* 2.2.13.1		2026.8.1	优化了单线程模式下的流程
* 2.2.14.0					添加了
*								MoonCtz
*							用于计算末尾0的数量
* 2.2.14.1		2026.8.9	修正了部分注释内容
* 2.2.14.2					修正了按鍵回调部分
*							保证其在单线程模式下符合单次触发的最初设想
*							而不是像多线程那样的短暂时间窗口下的短按和长按
*							是真正稳定的单次按下与长按
* 2.2.15.0					添加了
*								MoonStrStr
*							用于strstr的封装,但是返回索引而不是指针
* 2.2.16.0		2026.8.22	添加了
*								MoonStrReverse
*							用于反转字符串
* 2.2.16.1		2026.9.13	优化了部分代码
* 2.2.16.2					MOON_POINT2D的long int x, y
*							改为 int x, y
* 2.2.16.3					添加了
*								MoonCode_OFF/MoonCode_ON宏
*							用于关闭/开启代码块
* 2.2.16.4					添加了
*								MoonClamp
*							用于限制数值在指定范围内,与Range完全相同,但是语义上更清晰
* 2.2.16.5					修改了内置字体,现在更加美观
* 2.2.17.0+					添加了
*								MoonFindEntity_Id
*							用于直接查询实体
* 2.2.17.1		2026.9.14	添加了
*								MOON_MESSAGE_CULL_FACE
*							用于设置是否开启背面剔除
* 2.2.18.0		2026.9.15	添加了
*								MoonProjectSetEntityIndex
* 							用于设置实体索引的数量
* 2.2.18.1					将entityindex设置为了动态分配,不再是固定的数组,现在可以创建任意数量的实体
*							您可以使用新添加的MoonProjectSetEntityIndex来设置实体索引的数量,如果您不调用该函数,则实体索引数量为默认值
* 2.2.18.2					修改了MoonUtilityCoreLoad的参数
*								由
*									MoonUtilityCoreLoad(MOON_ENGINECORE* core)
*								改为
*									MoonUtilityCoreLoad()
*							这只是内核函数的一个优化,对您的应用层代码完全没有影响
* 2.2.18.3					将纹理和图元顶点设置为了动态分配
*							
*							2026.10.6
*							这是停更的最久的一次
*							后续可能还会又更长时间的停更,首先是目前正在准备一次更大规模的更新
*							正在准备思路和具体实现,以及可能会有架构级别的重构
*							其次是我个人的生活和工作问题,可能会导致我没有多余的时间来像之前一样每日更新
*							不过这个项目我不会放弃,我会继续维护它
* 2.2.18.4		2026.10.6	优化了MoonPrompt的内部实现
* 2.2.19.0					添加了
*								MoonProjectSetVertices
*							用于设置顶点的数量,如果您不调用该函数,则顶点数量为默认值
*							
* 为了迎接2.2.20.0
* 更新了命令行美术风格,从抽象的圆球回归正常的字母
* 原	2.0画面
	  ██████████
  ████      ████████
  ██    ██████    ██
████  ██    ████    ██
██████████████  ██  ██
██  ██  ██  ██  ██  ██
██  ██  ██████████████
██    ████    ██  ████
  ██    ██████    ██
  ████████      ████
	  ██████████
	被弃用
*
* 2.2.20.0		2026.10.27	添加了
*								MoonVector_Copy
*							用于复制向量
*/



//-------------------------------------------------------------------------------------------流程函数--------------------------------------------------------------------------------//


/*
* 函數 MoonProjectInit
* 作用 初始化遊戲項目
* 使用方法
* PROJECTGOD game;
* MoonProjectInit(&game, L"MyGame", 100, 100, 800, 600, 60, GameSetting);
*/
extern void MoonProjectInit(const char* project_name, int x, int y, int width, int height, int fps,void (*ProjectSetting_1)());//创建项目

/*
* 函數 MoonProjectRun
* 作用 運行遊戲項目主循環
* 使用方法
* MoonProjectRun(&game, GameInit, GameLogic, GameDrawing);
*/
extern void MoonProjectRun(void (*ProjectSetting_2)(), int(*ProjectLogic)(), int(*ProjectDrawing)());//运行项目

/*
* 函數 MoonProjectRun_Single
* 作用 運行遊戲項目主循環(单线程)
* 使用方法
* MoonProjectRun_Single(&game, GameInit, GameLogic, GameDrawing);
*/
extern void MoonProjectRun_Single(void (*ProjectSetting_2)(), int(*ProjectLogic)(), int(*ProjectDrawing)());//运行项目

/*
* 函數 MoonProjectOver
* 作用 結束遊戲項目，釋放資源
* 使用方法
* MoonProjectOver(&game, GameCleanup);
*/
extern void MoonProjectOver(void (*ProjectOverSetting)());//结束项目

/*
* 函數
* 作用
* 使用方法

*/
#define MOON_PROJECTSETTING(NAME) void NAME()//创建设置选项

/*
* 函數
* 作用
* 使用方法

*/
#define MOON_PROJECTMODULE(NAME)  int NAME()//配置模块

/*
* 函數 MoonProjectFindEntityAllNumber
* 作用 統計項目中已註冊的實體總數
* 使用方法
* int count = MoonProjectFindEntityAllNumber(project);
*/
extern int MoonProjectFindEntityAllNumber();//统计实体总数


/*
* 函數 MoonProjectSendMessage
* 作用 向引擎核心发送消息
* 根据返回值来判断是否发送成功
* 	
	MOON_MESSAGE_THREAD_TYPE_FALSE				无效
	MOON_MESSAGE_THREAD_TYPE_TRUE				成功
	MOON_MESSAGE_THREAD_TYPE_REALLOC_FAILURE	内存分配失败
	MOON_MESSAGE_THREAD_TYPE_BUSY				线程忙
* 
* 使用方法
		MoonProjectSendMessage(MOON_MESSAGE_ATTR_DEAD, metadata);
*/
extern MOON_MESSAGE_THREAD_TYPE MoonProjectSendMessage(MOON_MESSAGE message, MOON_METADATA metadata);

/*
* 函數 MoonVsn
* 作用 統計項目中已註冊的實體總數
* 使用方法
* MoonVsn(vsn);
*/
extern unsigned int MoonVsn();

/*
* 函數 MoonDead
* 作用 杀死项目
* 使用方法
* MoonDead();
*/
extern void MoonDead();

/*
* 函數 MoonProjectWindowSize
* 作用 查询窗口大小
* 使用方法
* MOON_POINT2D size = MoonProjectWindowSize();
* printf("%d, %d",size.w, size.h);
*/
extern MOON_POINT2D MoonProjectWindowSize();

/*
* 函數 MoonProjectBuffer
* 作用 查询后缓冲区
* 使用方法
* MOON_IMAGE* size = MoonProjectBuffer();
*/
extern MOON_IMAGE* MoonProjectBuffer();

/*
* 函數 MoonProjectTextureShader
* 作用 查询引擎纹理shader
* 使用方法
* unsigned int shader = MoonProjectTextureShader();
*/
extern unsigned int MoonShaderTexture();

/*
* 函數 MoonShaderSolid
* 作用 查询引擎图元shader
* 使用方法
* unsigned int shader = MoonShaderSolid();
*/
extern unsigned int MoonShaderSolid();

/*
* 函數 MoonProjectSetEntityIndex
* 作用 动态设置实体索引的数量
* 使用方法
* 在MoonProjectInit之前调用,即可设置实体索引的数量
* 如果您不调用该函数,则实体索引数量为默认值
* 默认值有三个
*	997			min
*	10007		mid
*	1000003		max
*/
extern void MoonProjectSetEntityIndex(unsigned int num);

/*
* 函數 MoonProjectSetVertices
* 作用 动态设置顶点的数量
* 使用方法
* 在MoonProjectInit之前调用,即可设置顶点的数量
* 如果您不调用该函数,则顶点数量为默认值
* 默认值有为MOON_VERTICES_MAX
*/
extern void MoonProjectSetVertices(unsigned int num);

#endif
