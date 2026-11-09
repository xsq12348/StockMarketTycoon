#include"Moon.h"
#include"MoonCore.h"

static unsigned char Moon_Engine_VSn[4] = { 2,2,20,0 };
static MOON_TIMELOAD projectfps;
static int fpsmax, fpsmax2;
static MOON_IMAGE projectdoublebuffer;
static MOON_POINT2D projectmousecoord;
static MOON_ENTITYINDEX* entityindex;
static MOON_ENGINECORE moon_engine_core;
static MOON_MESSAGE_ALL logic_message_cache;
static unsigned char thread_draw_type, thread_attr_type;
static unsigned char logic_dead, single_switch;
static unsigned char moon_key_type[MOON_KEY_LAST];

static const char* moon_vertex_shader2d_code =
"#version 330 core\n"
MoonString(
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
out vec4 vcolor;
void main()
{
	vcolor = color;
	gl_Position = vec4(position, 1.0);
}
);

static const char* moon_pixel_shader2d_code =
"#version 330 core\n"
MoonString(
out vec4 pixel_color;
in vec4 vcolor;
void main()
{
	pixel_color = vcolor;
}
);

static const char* moon_vertex_shader2d_texture_code =
"#version 330 core\n"
MoonString(
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 atex_uv;
out vec2 tex_uv;
void main()
{
	tex_uv = atex_uv;
	gl_Position = vec4(position, 1.0);
}
);

static const char* moon_pixel_shader2d_texture_code =
"#version 330 core\n"
MoonString(
in vec2 tex_uv;
out vec4 pixel_color;
uniform sampler2D moon_utexture;
uniform vec4 moon_ucolor;
void main()
{
	pixel_color = texture(moon_utexture, tex_uv) * moon_ucolor;
}
);

static unsigned int shader_program_vectex, shader_program_pixel;

static MOON_PROJECTMODULE(MoonLogicPause);															//暂停逻辑线程
static MOON_PROJECTMODULE(MoonDrawingPause);														//暂停绘制线程
static MOON_CREATETHREADFUNCTION(ProjectLogicThread);												//逻辑线程
static void MoonKeyCallback(GLFWwindow* hwnd, int key, int scancode, int action, int mods);			//键盘按键回调
static void MoonMouseCallback(GLFWwindow* hwnd, int button, int action, int mods);					//鼠标按键回调
static inline void MoonPollButton();																//轮询按鍵
static MOON_PROJECTMODULE(MoonLogicNull);															//当不存在逻辑函数时,设置为此函数

extern MOON_HWND* MoonWindow(const char* name, int window_coord_x, int window_coord_y, int window_width, int window_height)
{
	MOON_HWND* hwnd;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
	hwnd = glfwCreateWindow(window_width, window_height, name, (GLFWmonitor*)MOON_NULL, (GLFWwindow*)MOON_NULL);
	glfwSetWindowPos(hwnd, window_coord_x, window_coord_y);
	glfwMakeContextCurrent(hwnd);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		MoonProjectError(glfwGetProcAddress, 1, (char*)"[MoonWindow]严重错误,glfwGetProcAddress初始化失败");
		return 0;
	}
	//glfwShowWindow(hwnd);
	glfwSetKeyCallback(hwnd, MoonKeyCallback);
	glfwSetMouseButtonCallback(hwnd, MoonMouseCallback);
	printf("OpenGL: %s\n", glGetString(GL_VERSION));
	unsigned int vao;
	glad_glGenVertexArrays(1, &vao);
	glad_glBindVertexArray(vao);
	glad_glClearColor(0.f, 0.f, 0.f, 1.f);
	glad_glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(hwnd);
	return hwnd;
}

extern void MoonProjectInit(const char* project_name, int x, int y, int width, int height, int fps, void (*ProjectSetting_1)())
{
	MoonPrompt((char*)"[ProjectInit]初始化函数进入成功");
	//printf("\n\033[1;33m    ████████████      \n  ████      ████████  \n  ██    ██████    ██  \n████  ██    ████    ███\n██████████████  ██  ███\n██  ██  ██  ██  ██  ███\n██  ██  ███████████████\n██    ████    ██  █████\n  ██    ██████    ██  \n  ████████      ████  \n      ████████████      \n\033[0m\n");
	printf("\033[1;33m%s\n", "▌▌▌╗   ▌▌▌╗  ▌▌▌▌▌▌╗   ▌▌▌▌▌▌╗  ▌▌▌╗   ▌▌╗");
	printf("%s\n",           "▌▌▌▌╗ ▌▌▌▌║ ▌▌╔═══▌▌╗ ▌▌╔═══▌▌╗ ▌▌▌▌╗  ▌▌║");
	printf("%s\n",           "▌▌╔▌▌▌▌╔▌▌║ ▌▌║   ▌▌║ ▌▌║   ▌▌║ ▌▌╔▌▌╗ ▌▌║");
	printf("%s\n",           "▌▌║╚▌▌╔╝▌▌║ ▌▌║   ▌▌║ ▌▌║   ▌▌║ ▌▌║╚▌▌╗▌▌║");
	printf("%s\n",           "▌▌║ ╚═╝ ▌▌║ ╚▌▌▌▌▌▌╔╝ ╚▌▌▌▌▌▌╔╝ ▌▌║ ╚▌▌▌▌║");
	printf("%s\n",           "╚═╝     ╚═╝  ╚═════╝   ╚═════╝  ╚═╝  ╚═══╝");
	printf("%s\033[0m\n", "                                       ");
	printf("MoonEngine[%d.%d.%d.%d]\n", Moon_Engine_VSn[0], Moon_Engine_VSn[1], Moon_Engine_VSn[2], Moon_Engine_VSn[3]);


	if (!glfwInit())
	{
		MoonProjectError(glfwInit, 1, (char*)"[MoonWindow]严重错误,glfwInit初始化失败");
		moon_engine_core.dead = MOON_TRUE;
		return;
	}

	GLFWmonitor* moon_monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* moon_screen = glfwGetVideoMode(moon_monitor);

	if (x < 0 || y < 0)
	{
		x = (moon_screen->width - width) / 2;
		y = (moon_screen->height - height) / 2;
	}

	moon_engine_core.hwnd = MoonWindow(project_name, x, y, width, height);
	if (moon_engine_core.hwnd == MOON_NULL)
	{
		MoonProjectError(moon_engine_core.hwnd, 1, (char*)"[MoonWindow]严重错误,窗口创建失败");
		moon_engine_core.dead = MOON_TRUE;
		return;
	}

	//分配实体注册表
	{
		if(!moon_engine_core.entityindex_number)
			moon_engine_core.entityindex_number = MOON_ENTITY_NUMBER;

		MOON_ENTITYINDEX* entityindex_buffer = (MOON_ENTITYINDEX*)realloc(entityindex, sizeof(MOON_ENTITYINDEX) * moon_engine_core.entityindex_number);
		if(!entityindex_buffer)
		{
			MoonProjectError(entityindex_buffer, 1, (char*)"[MoonProjectInit]严重错误,entityindex分配失败");
			MoonProjectDead();
			return;
		}
		memset(entityindex_buffer, 0, sizeof(MOON_ENTITYINDEX) * moon_engine_core.entityindex_number);
		entityindex = entityindex_buffer;

		MoonPrompt((char*)"[MoonProjectInit]实体注册表分配成功");
		printf("大小为[%d]个实体 预计占用[%f]MiB\n", moon_engine_core.entityindex_number, sizeof(MOON_ENTITYINDEX) * moon_engine_core.entityindex_number * 1.f / (1024 * 1024));
	}

	MoonUtilityLoad(&moon_engine_core);
	SDL_Init(SDL_INIT_AUDIO);

	MoonPrompt((char*)"projectgod初始化成功");
	moon_engine_core.window_height = height;
	moon_engine_core.window_width = width;
	moon_engine_core.dead = MOON_FALSE;
	moon_engine_core.entityindex = entityindex;
	MoonTimeLoadInit(&moon_engine_core.timeload, (int)(1000.f / (fps > 0 ? fps : 60)));
	MoonTimeLoadInit(&projectfps, 1000);
	MoonCreateEntityIndex(&fpsmax2, (char*)"ProjectFPS", sizeof(int), (char*)"int");
	MoonCreateEntityIndex(&projectmousecoord, (char*)"ProjectMouseCoord", sizeof(MOON_POINT2D), (char*)"MOON_POINT2D");
	MoonCreateEntityIndex(&projectdoublebuffer, (char*)"ProjectBitmap", sizeof(MOON_IMAGE), (char*)"MOON_IMAGE");
	MoonCreateEntityIndex(moon_key_type, (char*)"ProjectKey", sizeof(unsigned char), (char*)"unsigned char");

	MoonImageCreate(&projectdoublebuffer, moon_engine_core.window_width, moon_engine_core.window_height);

	MoonCreateEntityIndex(&shader_program_vectex, (char*)"ProjectShader_SolidColor", sizeof(unsigned int), (char*)"unsigned int");
	MoonCreateEntityIndex(&shader_program_pixel, (char*)"ProjectShader_Texture", sizeof(unsigned int), (char*)"unsigned int");
	MoonShaderLoad((char**)&moon_vertex_shader2d_code, (char**)&moon_pixel_shader2d_code, &shader_program_vectex);					//加载渲染器
	MoonShaderLoad((char**)&moon_vertex_shader2d_texture_code, (char**)&moon_pixel_shader2d_texture_code, &shader_program_pixel);	//加载渲染器
	MoonUtilityCoreLoad();
	MoonDrawLoad();

	if (ProjectSetting_1 != MOON_NULL)
		ProjectSetting_1();
	MoonPrompt((char*)"[ProjectInit]初始化完成");
}

static MOON_CREATETHREADFUNCTION(ProjectLogicThread)
{
	int runload[3] = { 0 };//帧率计时器

	//逻辑线程
	MOON_GETTHREADRESOURCE(MOON_ENGINECORE*, core);

	MoonPrompt((char*)"加载了逻辑线程");

	while (!moon_engine_core.dead)
	{
		runload[0] = clock();

		if (moon_engine_core.Logic)
		{
			moon_engine_core.Logic();
			if (moon_engine_core.Logic != MoonLogicPause)
			{
				//自动锁
				MoonProjectGetMessage(&moon_engine_core.message_logic, &moon_engine_core.thread_message_type_logic, MoonlogicMessageHandle);
				MOON_MESSAGE_ALL buffer = logic_message_cache;
				logic_message_cache = moon_engine_core.message_logic;
				moon_engine_core.message_logic = buffer;
			}
		}
		
		runload[1] = clock();
		runload[2] = runload[1] - runload[0];
		if (runload[2] >= 30000)
		{
			MoonPrompt((char*)"[逻辑线程]时间过长,超过30ms,现在转入暂停");
			moon_engine_core.power = MOON_Error;
			moon_engine_core.gamepowermode = MOON_Error;
			moon_engine_core.Logic = MoonLogicPause;
		}
	}
	return 1;
}

static MOON_CREATETHREADFUNCTION(ProjectDrawingThread)
{
	//绘制线程
	//OpenGL
	MOON_GETTHREADRESOURCE(MOON_ENGINECORE*, core);
	glfwMakeContextCurrent(core->hwnd);

	MoonUtilityLoad(core);//重新绑定

	MoonHashFindEntity("ProjectBitmap", MOON_IMAGE, projectbitmap);
	MoonHashFindEntity("ProjectShader_SolidColor", unsigned int, shader_program_1);
	MoonHashFindEntity("ProjectShader_Texture", unsigned int, shader_program_2);
	unsigned int moon_vbo, moon_ebo, moon_vao, solid_color_shader, texture_shader;
	solid_color_shader = (*shader_program_1);
	texture_shader = (*shader_program_2);
	float vertexs[20] =
	{
		 1.f,  1.f, 0.f,  1.f, 1.f,  // 右上
		 1.f, -1.f, 0.f,  1.f, 0.f,  // 右下
		-1.f, -1.f, 0.f,  0.f, 0.f,  // 左下
		-1.f,  1.f, 0.f,  0.f, 1.f,  // 左上
	};
	unsigned int vertex_index[6] = { 0,2,1,2,0,3 };
	int runload[3] = { 0 };//帧率计时器

	{
		glad_glGenBuffers(1, &moon_vbo);
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo);
		glad_glBufferData(GL_ARRAY_BUFFER, sizeof(vertexs), vertexs, GL_STATIC_DRAW);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);

		glad_glGenBuffers(1, &moon_ebo);
		glad_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moon_ebo);
		glad_glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertex_index), vertex_index, GL_STATIC_DRAW);
		glad_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glad_glGenVertexArrays(1, &moon_vao);
		glad_glBindVertexArray(moon_vao);
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo);
		glad_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moon_ebo);

		glad_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glad_glEnableVertexAttribArray(0);

		glad_glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glad_glEnableVertexAttribArray(1);

		glad_glEnable(GL_BLEND);
		//glad_glEnable(GL_CULL_FACE);
		glad_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glad_glDisable(GL_DEPTH_TEST);
		//glad_glEnable(GL_DEPTH_TEST);
		//glad_glDepthFunc(GL_LEQUAL);
	}

	MoonPrompt((char*)"加载了绘制线程");

	glad_glViewport(0, 0, projectbitmap->image_size.w, projectbitmap->image_size.h);

	//主循环
	while (!moon_engine_core.dead)
	{
		runload[0] = clock();
		{
			if (!MoonTimeLoad(&projectfps, MOON_TRUE))++fpsmax;
			else { fpsmax2 = fpsmax; fpsmax = 0; }
		}

		if(moon_engine_core.Drawing)
		{
			moon_engine_core.Drawing();
			if (moon_engine_core.Drawing != MoonDrawingPause)
			{

				//自动锁
				MoonProjectGetMessage(&moon_engine_core.message_draw, &moon_engine_core.thread_message_type_draw, MoonDrawMessageHandle);

				glad_glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glad_glViewport(0, 0, projectbitmap->image_size.w, projectbitmap->image_size.h);
				glad_glBindTexture(GL_TEXTURE_2D, projectbitmap->image.texture);
				MoonImageShader(texture_shader);
				glad_glUniform4f(glad_glGetUniformLocation(texture_shader, "moon_ucolor"), 1.0f, 1.0f, 1.0f, 1.0f);
				glad_glUniform1i(glad_glGetUniformLocation(texture_shader, "moon_utexture"), 0);
				glad_glBindVertexArray(moon_vao);
				glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo);
				glad_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moon_ebo);
				glad_glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glfwSwapBuffers(moon_engine_core.hwnd);
				MoonImageDesignated(projectbitmap);
				glad_glClearColor(0.f, 0.f, 0.f, 1.f);
				glad_glClear(GL_COLOR_BUFFER_BIT);
			}
		}

		runload[1] = clock();
		runload[2] = runload[1] - runload[0];
		if (moon_engine_core.power <= 0)
			if ((unsigned int)runload[2] < moon_engine_core.timeload.timeload)
				MoonSleep((moon_engine_core.timeload.timeload - runload[2]));

		if (runload[2] >= 2000)
		{
			MoonPrompt((char*)"[绘制线程]时间过长,超过2000ms,现在转入暂停");
			moon_engine_core.power = MOON_Error;
			moon_engine_core.gamepowermode = MOON_Error;
			moon_engine_core.Drawing = MoonDrawingPause;
		}
	}

	{
		glad_glDeleteVertexArrays(1, &moon_vao);
		glad_glDeleteBuffers(1, &moon_vbo);
		glad_glDeleteBuffers(1, &moon_ebo);
		glfwMakeContextCurrent((GLFWwindow*)MOON_NULL);
	}

	thread_draw_type = MOON_TRUE;

	return 1;
}

extern void MoonProjectRun_Single(void (*ProjectSetting_2)(), int(*ProjectLogic)(), int(*ProjectDrawing)())
{
	if (moon_engine_core.dead)
	{
		MoonPrompt((char*)"[ProjectRun]引擎流程函数进入失败!");
		return;
	}

	MoonHashFindEntity("ProjectBitmap", MOON_IMAGE, projectbitmap);
	MoonHashFindEntity("ProjectShader_SolidColor", unsigned int, shader_program_1);
	MoonHashFindEntity("ProjectShader_Texture", unsigned int, shader_program_2);
	MoonHashFindEntity("ProjectMouseCoord", MOON_POINT2D, mousecoord);
	MoonHashFindEntity("ProjectFPS", int, fpsnumber);

	{
		MoonPrompt((char*)"[ProjectRun]引擎流程函数进入成功!\n目前是单线程模式");

		//默认在窗口内部
		glfwSetInputMode(moon_engine_core.hwnd, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);

		if (ProjectDrawing == MOON_FALSE)
		{
			MoonProjectError(ProjectDrawing, 1, (char*)"绘图函数传入失败!");
			return;
		}
		moon_engine_core.Drawing = ProjectDrawing;

		if (ProjectSetting_2)
			ProjectSetting_2();

		//默认逻辑函数
		if (ProjectLogic)
			moon_engine_core.Logic = ProjectLogic;
		else
			moon_engine_core.Logic = MoonLogicNull;
	}

	{
		single_switch = MOON_TRUE;
	}

	int
	(*drawing)() = 0, (*logic)() = 0,
	modetemp = MOON_FALSE, runload[3] = { 0 },//帧率计时器
	moon_vbo, moon_ebo, moon_vao, solid_color_shader, texture_shader;
	solid_color_shader = (*shader_program_1);
	texture_shader = (*shader_program_2);

	double mousecoord_x_2, mousecoord_y_2;
	moon_engine_core.gamepowermode = moon_engine_core.power;

	drawing = moon_engine_core.Drawing;
	logic = moon_engine_core.Logic;
	
	{
		float vertexs[20] =
		{
			 1.f,  1.f, 0.f,  1.f, 1.f,  // 右上
			 1.f, -1.f, 0.f,  1.f, 0.f,  // 右下
			-1.f, -1.f, 0.f,  0.f, 0.f,  // 左下
			-1.f,  1.f, 0.f,  0.f, 1.f,  // 左上
		};
		unsigned int vertex_index[6] = { 0,2,1,2,0,3 };

		glad_glGenBuffers(1, &moon_vbo);
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo);
		glad_glBufferData(GL_ARRAY_BUFFER, sizeof(vertexs), vertexs, GL_STATIC_DRAW);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);

		glad_glGenBuffers(1, &moon_ebo);
		glad_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moon_ebo);
		glad_glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertex_index), vertex_index, GL_STATIC_DRAW);
		glad_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glad_glGenVertexArrays(1, &moon_vao);
		glad_glBindVertexArray(moon_vao);
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo);
		glad_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moon_ebo);

		glad_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glad_glEnableVertexAttribArray(0);

		glad_glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glad_glEnableVertexAttribArray(1);

		glad_glEnable(GL_BLEND);
		//glad_glEnable(GL_CULL_FACE);
		glad_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glad_glDisable(GL_DEPTH_TEST);
		//glad_glEnable(GL_DEPTH_TEST);
		//glad_glDepthFunc(GL_LEQUAL);
	}

	glad_glViewport(0, 0, projectbitmap->image_size.w, projectbitmap->image_size.h);

	int run_logic = 0,
		run_draw = 0;

	while (!moon_engine_core.dead)
	{

		{
			runload[0] = clock();
			{
				if (!MoonTimeLoad(&projectfps, MOON_TRUE))++fpsmax;
				else { fpsmax2 = fpsmax; fpsmax = 0; }
			}

		}

		//属性
		{
			glfwPollEvents();

			{
				if (moon_engine_core.Logic != MoonLogicPause)
					logic = moon_engine_core.Logic;
				if (moon_engine_core.Drawing != MoonDrawingPause)
					drawing = moon_engine_core.Drawing;
				moon_engine_core.dead = (unsigned char)(glfwWindowShouldClose(moon_engine_core.hwnd)) || logic_dead;
				moon_engine_core.focus = (unsigned char)(!glfwGetWindowAttrib(moon_engine_core.hwnd, GLFW_FOCUSED));
			}

			{
				{
					glfwGetCursorPos(moon_engine_core.hwnd, &mousecoord_x_2, &mousecoord_y_2);
					mousecoord->x = (int)mousecoord_x_2;
					mousecoord->y = (int)mousecoord_y_2;
				}

				{
					if (moon_engine_core.focus)
						moon_engine_core.power = MOON_NOTFOUND;
					else
						moon_engine_core.power = moon_engine_core.gamepowermode;
				}

				//当Power改变时
				{
					if (moon_engine_core.power != modetemp)
					{
						MoonProjectPause(moon_engine_core.power < 0, &moon_engine_core.Logic, MoonLogicPause, logic);
						MoonProjectPause(moon_engine_core.power < 0, &moon_engine_core.Drawing, MoonDrawingPause, drawing);
					}
					modetemp = moon_engine_core.power;
				}
			}
			if (moon_engine_core.Attr)
				moon_engine_core.Attr();
		}

		//逻辑
		{
			if (moon_engine_core.Logic)
			{
				moon_engine_core.Logic();
				if (moon_engine_core.Logic != MoonLogicPause)
					run_logic = MOON_TRUE;
			}
		}

		//绘制
		{
			if (moon_engine_core.Drawing)
			{
				moon_engine_core.Drawing();
				if (moon_engine_core.Drawing != MoonDrawingPause)
					run_draw = MOON_TRUE;
			}


		}

		//自动锁
		{
			if(run_logic)
				MoonProjectGetMessage(&moon_engine_core.message_logic, &moon_engine_core.thread_message_type_logic, MoonlogicMessageHandle);
			if(run_draw)
			{
				MoonProjectGetMessage(&moon_engine_core.message_draw, &moon_engine_core.thread_message_type_draw, MoonDrawMessageHandle);
				{
					glad_glBindFramebuffer(GL_FRAMEBUFFER, 0);

					glad_glViewport(0, 0, projectbitmap->image_size.w, projectbitmap->image_size.h);
					glad_glBindTexture(GL_TEXTURE_2D, projectbitmap->image.texture);
					MoonImageShader(texture_shader);
					glad_glUniform4f(glad_glGetUniformLocation(texture_shader, "moon_ucolor"), 1.0f, 1.0f, 1.0f, 1.0f);
					glad_glUniform1i(glad_glGetUniformLocation(texture_shader, "moon_utexture"), 0);
					glad_glBindVertexArray(moon_vao);
					glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo);
					glad_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moon_ebo);
					glad_glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
					glfwSwapBuffers(moon_engine_core.hwnd);
					MoonImageDesignated(projectbitmap);
					glad_glClearColor(0, 0, 0, 1);
					glad_glClear(GL_COLOR_BUFFER_BIT);
				}
			}

			run_logic = MOON_FALSE;
			run_draw = MOON_FALSE;
		}		

		//轮询按鍵
		{
			MoonPollButton();
		}

		{
			runload[1] = clock();
			runload[2] = runload[1] - runload[0];
			if (moon_engine_core.power <= 0)
				if ((unsigned int)runload[2] < moon_engine_core.timeload.timeload)
					MoonSleep((moon_engine_core.timeload.timeload - runload[2]));
		}
	}

	{
		glad_glDeleteVertexArrays(1, &moon_vao);
		glad_glDeleteBuffers(1, &moon_vbo);
		glad_glDeleteBuffers(1, &moon_ebo);
		glfwMakeContextCurrent((GLFWwindow*)MOON_NULL);
	}

	thread_draw_type = MOON_TRUE;
	thread_attr_type = MOON_TRUE;
}

extern void MoonProjectRun(void (*ProjectSetting_2)(), int(*ProjectLogic)(), int(*ProjectDrawing)())
{
	if (moon_engine_core.dead)
	{
		MoonPrompt((char*)"[ProjectRun]引擎流程函数进入失败!");
		return;
	}

	MoonPrompt((char*)"[ProjectRun]引擎流程函数进入成功!");

	//默认在窗口内部
	glfwSetInputMode(moon_engine_core.hwnd, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);

	if (ProjectDrawing == MOON_FALSE)
	{
		MoonProjectError(ProjectDrawing, 1, (char*)"绘图函数传入失败!");
		return;
	}
	moon_engine_core.Drawing = ProjectDrawing;

	if (ProjectSetting_2)
		ProjectSetting_2();

	glfwMakeContextCurrent((GLFWwindow*)MOON_NULL);


	//默认逻辑函数
	if (ProjectLogic)
		moon_engine_core.Logic = ProjectLogic;
	else
		moon_engine_core.Logic = MoonLogicNull;

	MOON_CREATETHREAD(ProjectDrawingThread, "DrawingThread", &moon_engine_core);	//加载绘制线程
	MOON_CREATETHREAD(ProjectLogicThread, "LogicThread", &moon_engine_core);		//加载逻辑线程

	MoonPrompt((char*)"加载了属性线程");

	MoonHashFindEntity("ProjectMouseCoord", MOON_POINT2D, mousecoord);
	MoonHashFindEntity("ProjectBitmap", MOON_IMAGE, projectbitmap);
	MoonHashFindEntity("ProjectFPS", int, fpsnumber);
	static int(*drawing)() = 0, (*logic)() = 0;
	drawing = moon_engine_core.Drawing;
	logic = moon_engine_core.Logic;
	static double mousecoord_x_2, mousecoord_y_2;
	moon_engine_core.gamepowermode = moon_engine_core.power;
	int modetemp = MOON_FALSE, runload[3] = { 0 };//帧率计时器
	//属性线程
	while (!moon_engine_core.dead)
	{
		glfwPollEvents();

		{
			if (moon_engine_core.Logic != MoonLogicPause)
				logic = moon_engine_core.Logic;
			if (moon_engine_core.Drawing != MoonDrawingPause)
				drawing = moon_engine_core.Drawing;
			moon_engine_core.dead = (unsigned char)(glfwWindowShouldClose(moon_engine_core.hwnd)) || logic_dead;
			moon_engine_core.focus = (unsigned char)(!glfwGetWindowAttrib(moon_engine_core.hwnd, GLFW_FOCUSED));
		}

		//轮询按鍵
		{
			MoonPollButton();
		}

		{
			{
				glfwGetCursorPos(moon_engine_core.hwnd, &mousecoord_x_2, &mousecoord_y_2);
				mousecoord->x = (int)mousecoord_x_2;
				mousecoord->y = (int)mousecoord_y_2;
			}

			{
				if (moon_engine_core.focus)
					moon_engine_core.power = MOON_NOTFOUND;
				else
					moon_engine_core.power = moon_engine_core.gamepowermode;
			}

			//当Power改变时
			{
				if (moon_engine_core.power != modetemp)
				{
					MoonProjectPause(moon_engine_core.power < 0, &moon_engine_core.Logic, MoonLogicPause, logic);
					MoonProjectPause(moon_engine_core.power < 0, &moon_engine_core.Drawing, MoonDrawingPause, drawing);
				}
				modetemp = moon_engine_core.power;
			}
		}
		if (moon_engine_core.Attr)
			moon_engine_core.Attr();
		MoonSleep(1);
	}
	thread_attr_type = MOON_TRUE;
}

extern void MoonProjectOver(void (*ProjectOverSetting)())
{
	while (1)
		if (thread_attr_type && thread_draw_type)
		{
			glfwMakeContextCurrent(moon_engine_core.hwnd);
			break;
		}
		else
			MoonSleep(1);

	MoonPrompt((char*)"[ProjectOver]引擎流程结束!\n[ProjectOver]结束函数进入成功");

	if (ProjectOverSetting)
		ProjectOverSetting();

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

	//釋放所有實體
	for (unsigned int index = 0; index < moon_engine_core.entityindex_number; ++index)
	{
		if (moon_engine_core.entityindex[index].type_name != 0)
			if (!strcmp(moon_engine_core.entityindex[index].type_name, (char*)"MOON_IMAGE"))
				MoonImageDelete((MOON_IMAGE*)moon_engine_core.entityindex[index].entityindex);
			else if (!strcmp(moon_engine_core.entityindex[index].type_name, (char*)"MOON_ANIME"))
				MoonAnimeDelete((MOON_ANIME*)moon_engine_core.entityindex[index].entityindex);
			else if (!strcmp(moon_engine_core.entityindex[index].type_name, (char*)"MOON_VECTOR"))
				MoonVectorFree((MOON_VECTOR*)moon_engine_core.entityindex[index].entityindex);

		moon_engine_core.entityindex[index].length = 0;
		moon_engine_core.entityindex[index].nameid = (char*)MOON_NULL;
		moon_engine_core.entityindex[index].entityindex = MOON_NULL;
	}

	//释放实体注册表
	{
		free(moon_engine_core.entityindex);
	}

	MoonPrompt((char*)"[ProjectOver]资源清理完成");
	MoonPrompt((char*)"程序已退出");
}

extern int MoonProjectError(void* alpha, int degree, char* text)
{
	enum
	{
		Serious = 1,
		General,
		Mild,
	};
	printf("\n来自[%p]的[%s]发生错误!现在转入错误处理函数[ProjectError]", alpha, text);
	switch (degree)
	{
	case Serious:
		printf("\t等级[Serious/严重]\n"); 
		break;
	case General:
		printf("\t等级[General/一般]\n"); 
		break;
	case Mild:
		printf("\t等级[Mild/轻微]\n"); 
		break;
	}
	printf("按下[esc]退出");
	while (!MoonKeyState(MOON_KEY_ESCAPE))
		MoonSleep(1);
	return degree;
}

extern int MoonProjectPause(int mode, int (**function_1)(), int (*function_2)(), int (*function_3)())
{
	if (mode) *function_1 = function_2;
	else *function_1 = function_3;
	return 1;
}

extern void MoonProjectFunctionSwitch(MOON_MODULE_FUNCTION module, int (*function_2)())
{
	if (function_2 == MOON_NULL)
	{
		char text[255] = { 0 };
		snprintf(text, 255, "[MoonProjectFunctionSwitch]函数,空指针错误,请勿传入空指针,来自模块[%s]", (module == MOON_MODULE_DRAW ? "Draw" : (module == MOON_MODULE_ATTR ? "Attr" : "Logic")));
		MoonPrompt(text);
	}
	switch (module)
	{
	case MOON_MODULE_DRAW:	moon_engine_core.Drawing = function_2; break;
	case MOON_MODULE_LOGIC:	moon_engine_core.Logic = function_2; break;
	case MOON_MODULE_ATTR:	break;
	}
}

extern int MoonProjectFindEntityAllNumber()
{
	int all_number = 0;
	printf("\n\033[4;7;105m   序号|地址            |索引      |名称                          |类型                          |Hash      |类型大小  \033[0m\n");
	for (unsigned int index = 0; index < moon_engine_core.entityindex_number; ++index)
		if (moon_engine_core.entityindex[index].length != 0)
		{
			++all_number;
			printf("\033[4;7;105m%-7d|%p|%-10d|%-30s|%-30s|%-10d|%-10d\033[0m\n", 
				all_number, moon_engine_core.entityindex[index].entityindex, index, moon_engine_core.entityindex[index].nameid, 
				moon_engine_core.entityindex[index].type_name, MoonHash(moon_engine_core.entityindex[index].nameid), moon_engine_core.entityindex[index].length);
		}
	printf("\n[ProjectFindEntityAllNumber函数]进入成功!\n统计到的实体总数为[%d]\n", all_number);
	return all_number;
}

extern unsigned int MoonVsn()
{
	unsigned int vsn =
		(Moon_Engine_VSn[0] << 0)
		| (Moon_Engine_VSn[1] << 8)
		| (Moon_Engine_VSn[2] << 16)
		| (Moon_Engine_VSn[3] << 24);
	return vsn;
}

extern void MoonPrompt(char* text)
{
	printf("\n\033[31m[MoonEngine]提示\033[0m\n");
	if(text)
		printf("%s\n", text);
}

static MOON_PROJECTMODULE(MoonLogicPause)
{
	//printf("[MoonLogicPause函数]进入成功!\n");
	MoonSleep(1);
	return MOON_NOTFOUND;
}

static MOON_PROJECTMODULE(MoonDrawingPause)
{
	//printf("[MoonDrawingPause函数]进入成功!\n");
	/*
	char text_buffer[255] = { 0 };
	snprintf(text_buffer, 255, "MoonEngine %d.%d.%d.%d", Moon_Engine_VSn[0], Moon_Engine_VSn[1], Moon_Engine_VSn[2], Moon_Engine_VSn[3]);
	MoonDrawTextFont(&projectdoublebuffer, (const char*)text_buffer, 0, 0, 16, 16, MoonRGBA(255, 0, 0, 255));
	MoonDrawTextFont(&projectdoublebuffer, "PAUSE", 0, 16, 16, 16, MoonRGBA(255, 0, 0, 255));
	snprintf(text_buffer, 255, "Window w:%d h:%d", project->window_width, project->window_height);
	MoonDrawTextFont(&projectdoublebuffer, (const char*)text_buffer, 0, 32, 16, 16, MoonRGBA(255, 255, 0, 255));
	snprintf(text_buffer, 255, "FPS:%d", fpsmax2);
	MoonDrawTextFont(&projectdoublebuffer, (const char*)text_buffer, 0, 48, 16, 16, MoonRGBA(255, 255, 0, 255));
	snprintf(text_buffer, 255, "mouse coord:%d %d", projectmousecoord.x, projectmousecoord.y);
	MoonDrawTextFont(&projectdoublebuffer, (const char*)text_buffer, 0, 64, 16, 16, MoonRGBA(255, 255, 0, 255));
	*/
	MoonSleep(1);
	return MOON_NOTFOUND;
}

extern MOON_MESSAGE_THREAD_TYPE MoonProjectSendMessage(MOON_MESSAGE message, MOON_METADATA metadata)
{
	static unsigned int
		message_index_max_draw,
		message_index_max_logic;
	if (message > MOON_MESSAGE_DRAW_START && message <= MOON_MESSAGE_DRAW_END)
		if (!moon_engine_core.thread_message_type_draw)
		{
			if (moon_engine_core.message_draw.message_index >= message_index_max_draw)
			{
				MOON_MESSAGE_SPECIFIC* buffer = (MOON_MESSAGE_SPECIFIC*)realloc(moon_engine_core.message_draw.message, (moon_engine_core.message_draw.message_index + 1) * sizeof(MOON_MESSAGE_SPECIFIC));
				if (!buffer)
					return MOON_MESSAGE_THREAD_TYPE_REALLOC_FAILURE;
				else
				{
					message_index_max_draw += 1;
					moon_engine_core.message_draw.message = buffer;
				}
			}
			moon_engine_core.message_draw.message[moon_engine_core.message_draw.message_index].message = message;
			moon_engine_core.message_draw.message[moon_engine_core.message_draw.message_index].metadata = metadata;
			moon_engine_core.message_draw.message_index += 1;
			return MOON_MESSAGE_THREAD_TYPE_TRUE;
		}
		else
			return MOON_MESSAGE_THREAD_TYPE_BUSY;

	if (message > MOON_MESSAGE_LOGIC_START && message <= MOON_MESSAGE_LOGIC_END)
		if (!moon_engine_core.thread_message_type_logic)
		{
			if (moon_engine_core.message_logic.message_index >= message_index_max_logic)
			{
				logic_message_cache.message_index = moon_engine_core.message_logic.message_index;
				MOON_MESSAGE_SPECIFIC* buffer = (MOON_MESSAGE_SPECIFIC*)realloc(moon_engine_core.message_logic.message, (moon_engine_core.message_logic.message_index + 1) * sizeof(MOON_MESSAGE_SPECIFIC));
				MOON_MESSAGE_SPECIFIC* buffer_2 = (MOON_MESSAGE_SPECIFIC*)realloc(logic_message_cache.message, (logic_message_cache.message_index + 1) * sizeof(MOON_MESSAGE_SPECIFIC));
				if (!buffer)
					return MOON_MESSAGE_THREAD_TYPE_REALLOC_FAILURE;
				else
				{
					message_index_max_logic += 1;
					moon_engine_core.message_logic.message = buffer;
				}
				if (buffer_2)
					logic_message_cache.message = buffer_2;
			}
			moon_engine_core.message_logic.message[moon_engine_core.message_logic.message_index].message = message;
			moon_engine_core.message_logic.message[moon_engine_core.message_logic.message_index].metadata = metadata;
			moon_engine_core.message_logic.message_index += 1;
			return MOON_MESSAGE_THREAD_TYPE_TRUE;
		}
		else
		{
			logic_message_cache.message[logic_message_cache.message_index].message = message;
			logic_message_cache.message[logic_message_cache.message_index].metadata = metadata;
			logic_message_cache.message_index += 1;
			return MOON_MESSAGE_THREAD_TYPE_CACHE;
		}

	return MOON_MESSAGE_THREAD_TYPE_FALSE;
}

extern int MoonProjectGetMessage(MOON_MESSAGE_ALL* message, unsigned char* type, void(*Handle)(MOON_MESSAGE_ALL*, unsigned char*))
{
	//type的作用
	//防止随意操作标志导致消息处理紊乱
	*type = MOON_TRUE;
	if (Handle && message->message_index > 0)
	{
		//Handle内部应该处理完所有消息,因为该函数结束后,线程循环也差不多结束
		Handle(message, type);
		if (message->message)
			memset(message->message, 0, sizeof(MOON_MESSAGE_SPECIFIC) * message->message_index);
		message->message_index = 0;
	}
	*type = MOON_FALSE;
	return MOON_FALSE;
}

extern void MoonlogicMessageHandle(MOON_MESSAGE_ALL* message, unsigned char* type)
{
	for (unsigned int index = 0; index < message->message_index; ++index)
	{
		if (*type)
			switch (message->message[index].message)
			{
			case MOON_MESSAGE_LOGIC_END: *type = MOON_FALSE; break;
				//case MOON_MESSAGE_LOGIC_SETLOGIC:				MoonProjectFunctionSwitch(MOON_MODULE_LOGIC, message->message[index].metadata.function);				break;
			case MOON_MESSAGE_SETLOGIC:
				MoonProjectFunctionSwitch(MOON_MODULE_LOGIC, message->message[index].metadata.function);
				break;
			case MOON_MESSAGE_SETDRAW:
				MoonProjectFunctionSwitch(MOON_MODULE_DRAW, message->message[index].metadata.function);
				break;
				//case MOON_MESSAGE_LOGIC_OPEN:				message->message[index].metadata.function_open(project); break;
			case MOON_MESSAGE_DEAD:
				moon_engine_core.dead = MOON_TRUE;
				logic_dead = MOON_TRUE;
				break;
			case MOON_MESSAGE_POWER:
			{
				int buffer = MoonMax(message->message[index].metadata.power, 0);
				moon_engine_core.gamepowermode = buffer;
			}
			break;
			case MOON_MESSAGE_SETFPS:
			{
				int fps = (int)(1000.f / message->message[index].metadata.fps);
				if (fps <= 0)fps = (int)(1000.f / 60);
				moon_engine_core.timeload.timeload = fps;
			}
			break;
			case MOON_MESSAGE_ATTR_OPEN:
				moon_engine_core.Attr = message->message[index].metadata.function_open;
				break;
			case MOON_MESSAGE_KEY:
			{
				int token = message->message[index].metadata.key.token,
					* state = message->message[index].metadata.key.worth;
				*state = moon_key_type[token];
			}
			break;
			}
		else
			return;
	}
	/*
	for (unsigned int index = 0; index < MOON_KEY_LAST; ++index)
	{
		if (key_on_bufer[index] && !moon_key_type[index])
			moon_key_type[index] = MOON_KEY_MODE_PRESS;
		key_on_bufer[index] = MOON_FALSE;
	}
*/
};

extern void MoonProjectDead()
{
	moon_engine_core.dead = MOON_TRUE;
	logic_dead = MOON_TRUE;
	thread_attr_type = MOON_TRUE;
	thread_draw_type = MOON_TRUE;
}

extern void MoonDead()
{
	MOON_METADATA metadata = { 0 };
	MoonProjectSendMessage(MOON_MESSAGE_DEAD, metadata);
}

static void MoonKeyCallback(GLFWwindow* hwnd, int key, int scancode, int action, int mods)
{
	switch (action)
	{

	case GLFW_RELEASE:
		moon_key_type[key] = MOON_KEY_MODE_FALSE;
		break;

	case GLFW_PRESS:
		moon_key_type[key] = MOON_KEY_MODE_PRESS;
		break;
	}

}

static void MoonMouseCallback(GLFWwindow* hwnd, int button, int action, int mods)
{
	switch (action)
	{
	case GLFW_RELEASE:
		moon_key_type[button] = MOON_KEY_MODE_FALSE;
		break;

	case GLFW_PRESS:
		moon_key_type[button] = MOON_KEY_MODE_PRESS;
		break;
	}
}

static inline void MoonPollButton()
{
	static int moon_key_press_time[MOON_KEY_LAST];
	static const short int press_tps = 17;//单次检测持续的时间,~13.5ms
	for (unsigned int index = 0; index < MOON_KEY_LAST; ++index)
	{
		switch (moon_key_type[index])
		{
		case GLFW_MOUSE_BUTTON_LEFT:
		case GLFW_MOUSE_BUTTON_RIGHT:
		case GLFW_KEY_A:
		case GLFW_KEY_B:
		case GLFW_KEY_C:
		case GLFW_KEY_D:
		case GLFW_KEY_E:
		case GLFW_KEY_F:
		case GLFW_KEY_G:
		case GLFW_KEY_H:
		case GLFW_KEY_I:
		case GLFW_KEY_J:
		case GLFW_KEY_K:
		case GLFW_KEY_L:
		case GLFW_KEY_M:
		case GLFW_KEY_N:
		case GLFW_KEY_O:
		case GLFW_KEY_P:
		case GLFW_KEY_Q:
		case GLFW_KEY_R:
		case GLFW_KEY_S:
		case GLFW_KEY_T:
		case GLFW_KEY_U:
		case GLFW_KEY_V:
		case GLFW_KEY_W:
		case GLFW_KEY_X:
		case GLFW_KEY_Y:
		case GLFW_KEY_Z:
		case GLFW_KEY_0:
		case GLFW_KEY_1:
		case GLFW_KEY_2:
		case GLFW_KEY_3:
		case GLFW_KEY_4:
		case GLFW_KEY_5:
		case GLFW_KEY_6:
		case GLFW_KEY_7:
		case GLFW_KEY_8:
		case GLFW_KEY_9:
		case GLFW_KEY_F1:
		case GLFW_KEY_F2:
		case GLFW_KEY_F3:
		case GLFW_KEY_F4:
		case GLFW_KEY_F5:
		case GLFW_KEY_F6:
		case GLFW_KEY_F7:
		case GLFW_KEY_F8:
		case GLFW_KEY_F9:
		case GLFW_KEY_F10:
		case GLFW_KEY_F11:
		case GLFW_KEY_F12:
		case GLFW_KEY_F13:
		case GLFW_KEY_F14:
		case GLFW_KEY_F15:
		case GLFW_KEY_F16:
		case GLFW_KEY_F17:
		case GLFW_KEY_F18:
		case GLFW_KEY_F19:
		case GLFW_KEY_F20:
		case GLFW_KEY_F21:
		case GLFW_KEY_F22:
		case GLFW_KEY_F23:
		case GLFW_KEY_F24:
		case GLFW_KEY_F25:
		case GLFW_KEY_UP:
		case GLFW_KEY_DOWN:
		case GLFW_KEY_LEFT:
		case GLFW_KEY_RIGHT:
		case GLFW_KEY_HOME:
		case GLFW_KEY_END:
		case GLFW_KEY_PAGE_UP:
		case GLFW_KEY_PAGE_DOWN:
		case GLFW_KEY_INSERT:
		case GLFW_KEY_DELETE:
		case GLFW_KEY_SPACE:
		case GLFW_KEY_APOSTROPHE:// '
		case GLFW_KEY_COMMA:// ,
		case GLFW_KEY_MINUS:// -
		case GLFW_KEY_PERIOD:// .
		case GLFW_KEY_SLASH:// /
		case GLFW_KEY_SEMICOLON:// ;
		case GLFW_KEY_EQUAL:// =
		case GLFW_KEY_LEFT_BRACKET:// [
		case GLFW_KEY_BACKSLASH:// '\'
		case GLFW_KEY_RIGHT_BRACKET:// ]
		case GLFW_KEY_GRAVE_ACCENT:// `
		case GLFW_KEY_WORLD_1:
		case GLFW_KEY_WORLD_2:
		case GLFW_KEY_KP_0:
		case GLFW_KEY_KP_1:
		case GLFW_KEY_KP_2:
		case GLFW_KEY_KP_3:
		case GLFW_KEY_KP_4:
		case GLFW_KEY_KP_5:
		case GLFW_KEY_KP_6:
		case GLFW_KEY_KP_7:
		case GLFW_KEY_KP_8:
		case GLFW_KEY_KP_9:
		case GLFW_KEY_KP_DECIMAL:// .
		case GLFW_KEY_KP_DIVIDE:// /
		case GLFW_KEY_KP_MULTIPLY:// *
		case GLFW_KEY_KP_SUBTRACT:// -
		case GLFW_KEY_KP_ADD:// +
		case GLFW_KEY_KP_ENTER:
		case GLFW_KEY_KP_EQUAL:// =
		case GLFW_KEY_ESCAPE:
		case GLFW_KEY_ENTER:
		case GLFW_KEY_TAB:
		case GLFW_KEY_BACKSPACE:
		case GLFW_KEY_CAPS_LOCK:
		case GLFW_KEY_SCROLL_LOCK:
		case GLFW_KEY_NUM_LOCK:
		case GLFW_KEY_PRINT_SCREEN:
		case GLFW_KEY_PAUSE:
		case GLFW_KEY_MENU:
		case GLFW_KEY_LEFT_SHIFT:
		case GLFW_KEY_LEFT_CONTROL:
		case GLFW_KEY_LEFT_ALT:
		case GLFW_KEY_LEFT_SUPER:// Windows 键 / Command
		case GLFW_KEY_RIGHT_SHIFT:
		case GLFW_KEY_RIGHT_CONTROL:
		case GLFW_KEY_RIGHT_ALT:
		case GLFW_KEY_RIGHT_SUPER:
		case GLFW_KEY_UNKNOWN:
		{
			if (!moon_key_type[index])
			{
				moon_key_press_time[index] = 0;
				break;
			}

			if (moon_key_type[index] == MOON_KEY_MODE_PRESS_LONG)
				break;

			if (single_switch)
				moon_key_type[index] = MOON_KEY_MODE_PRESS_LONG;
			else
				if (!moon_key_press_time[index])
					moon_key_press_time[index] = clock();
				else
					if (clock() - moon_key_press_time[index] >= press_tps)
						moon_key_type[index] = MOON_KEY_MODE_PRESS_LONG;
		}
		break;
		default:
			continue;
			break;
		}
	}
}

static MOON_PROJECTMODULE(MoonLogicNull)
{
	MoonSleep(17);
	return 1;
}

extern MOON_POINT2D MoonProjectWindowSize()
{
	MOON_POINT2D size;
	size.w = moon_engine_core.window_width;
	size.h = moon_engine_core.window_height;
	return size;
}

extern void MoonProjectSetEntityIndex(unsigned int num)
{
	if (!moon_engine_core.entityindex_number)
		moon_engine_core.entityindex_number = num;
}

