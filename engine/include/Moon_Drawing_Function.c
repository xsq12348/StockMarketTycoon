#define STB_IMAGE_IMPLEMENTATION
#include"Moon.h"
#include"MoonCore.h"
//为了方便和防止冲突,所以改名
#include"Moon_stb_image.h"
#include"MoonFontttf.h"

static MOON_IMAGE moon_simple_font;
static MOON_GRAPHIC_VECTER* moon_vertex;
static MOON_TEXTURE_VECTER* moon_vertex_texture;
static unsigned int
solid_color_shader, texture_shader,
moon_vbo_solid, moon_vao_solid,
moon_vbo_texture, moon_vao_texture,
moon_vertex_index, moon_vertex_texture_index;
static unsigned int moon_vertices;
static void MoonVertexinitTemp(MOON_GRAPHIC_VECTER* vertex, unsigned int index_offset, float vx, float vy, float r, float g, float b, float a);					//构建图元顶点
static inline void MoonTextureVertexinitTemp(MOON_TEXTURE_VECTER* vertex, unsigned int index_offset, float vx, float vy, float uv_x, float uv_y);				//构建纹理顶点
static inline unsigned char MoonSetTemp(MOON_IMAGE** image_old, MOON_METADATA* metadata, int offset);															//全局设置
static void MoonDrawAreaTemp(unsigned int message_type, unsigned int index, MOON_MESSAGE_ALL* message, MOON_IMAGE* image_old, MOON_IMAGE* image_resource_old);	//MoonCoreDrawArea辅助函数
static void MoonCoreGraphicTemp(unsigned int message_type, unsigned int index, MOON_MESSAGE_ALL* message, MOON_IMAGE* image_old);								//MoonCoreGraphic辅助函数

extern void MoonDrawLoad()
{
	//获取引擎核心着色器
	MoonHashFindEntity("ProjectShader_SolidColor", unsigned int, shader_program_1);
	MoonHashFindEntity("ProjectShader_Texture", unsigned int, shader_program_2);
	solid_color_shader = *shader_program_1;
	texture_shader = *shader_program_2;

	{
		if (!moon_vertices)
			moon_vertices = MOON_VERTICES_MAX;
		MoonPrompt((char*)MOON_NULL);
		printf("[MoonDrawLoad]函数成功,图元顶点数量为[%u]\n", moon_vertices);

	}

	//动态分配资源
	{
		{
			MOON_GRAPHIC_VECTER* moon_vertex_buffer = (MOON_GRAPHIC_VECTER*)realloc(moon_vertex, sizeof(MOON_GRAPHIC_VECTER) * moon_vertices);
			if (!moon_vertex_buffer)
			{
				MoonPrompt((char*)"[MoonDrawLoad]函数错误,动态分配图元顶点失败!");
				MoonProjectDead();
				return;
			}
			else
				moon_vertex = moon_vertex_buffer;
		}

		{
			MOON_TEXTURE_VECTER* moon_vertex_texture_buffer = (MOON_TEXTURE_VECTER*)realloc(moon_vertex_texture, sizeof(MOON_TEXTURE_VECTER) * moon_vertices);
			if (!moon_vertex_texture_buffer)
			{
				MoonPrompt((char*)"[MoonDrawLoad]函数错误,动态分配纹理顶点失败!");
				MoonProjectDead();
				return;
			}
			else
				moon_vertex_texture = moon_vertex_texture_buffer;
		}

	}

	//texture
	{
		glad_glGenVertexArrays(1, &moon_vao_texture);
		glad_glGenBuffers(1, &moon_vbo_texture);

		glad_glBindVertexArray(moon_vao_texture);

		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_texture);
		glad_glBufferData(GL_ARRAY_BUFFER, sizeof(MOON_TEXTURE_VECTER) * moon_vertices, NULL, GL_DYNAMIC_DRAW);

		glad_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glad_glEnableVertexAttribArray(0);
		glad_glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glad_glEnableVertexAttribArray(1);

		glad_glBindVertexArray(0);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	//solid
	{
		glad_glGenVertexArrays(1, &moon_vao_solid);
		glad_glGenBuffers(1, &moon_vbo_solid);

		glad_glBindVertexArray(moon_vao_solid);

		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_solid);
		glad_glBufferData(GL_ARRAY_BUFFER, sizeof(MOON_GRAPHIC_VECTER) * moon_vertices, NULL, GL_DYNAMIC_DRAW);

		glad_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
		glad_glEnableVertexAttribArray(0);
		glad_glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
		glad_glEnableVertexAttribArray(1);

		glad_glBindVertexArray(0);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	//加载默认纹理
	{
		//创建内部字体

		glad_glGenTextures(1, &moon_simple_font.image.texture);
		glad_glBindTexture(GL_TEXTURE_2D, moon_simple_font.image.texture);
		glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		unsigned char* data = moon_simple_font_data;
		glad_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)MOON_FONT_CHAR_COUNT * MOON_FONT_CHAR_SIZE_W, (GLsizei)MOON_FONT_CHAR_SIZE_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glad_glGenFramebuffers(1, &moon_simple_font.image.fbo);
		glad_glBindFramebuffer(GL_FRAMEBUFFER, moon_simple_font.image.fbo);
		glad_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, moon_simple_font.image.texture, 0);

		moon_simple_font.image_size.w = MOON_FONT_CHAR_COUNT * MOON_FONT_CHAR_SIZE_W;
		moon_simple_font.image_size.h = MOON_FONT_CHAR_SIZE_H;
	}
}

extern void MoonDrawOver()
{
	{
		glad_glDeleteVertexArrays(1, &moon_vao_texture);
		glad_glDeleteBuffers(1, &moon_vbo_texture);
	}

	{
		glad_glDeleteVertexArrays(1, &moon_vao_solid);
		glad_glDeleteBuffers(1, &moon_vbo_solid);
	}

	MoonImageDelete(&moon_simple_font);

	//回收动态分配资源
	{
		free(moon_vertex);
		free(moon_vertex_texture);
	}
}

extern void MoonShaderLoad(char** vertex_shader, char** pixel_shader, unsigned int* shader_program)
{
	GLint success;

	//編譯頂點著色器
	unsigned int vertex_shader_uint = glad_glCreateShader(GL_VERTEX_SHADER);
	glad_glShaderSource(vertex_shader_uint, 1, vertex_shader, (const GLint*)MOON_NULL);
	glad_glCompileShader(vertex_shader_uint);
	glad_glGetShaderiv(vertex_shader_uint, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char error_log[1024];
		glad_glGetShaderInfoLog(vertex_shader_uint, 1024, (GLsizei*)MOON_NULL, error_log);
		MoonPrompt(error_log);
		MoonProjectError(shader_program, 1, (char*)"[MoonRendererLoad]函数错误,顶点着色器编译失败!");
		MoonProjectDead();
		return;
	}
	success = 0;
	MoonPrompt((char*)"顶点着色器编译完成");

	//編譯像素著色器
	unsigned int pixel_shader_uint = glad_glCreateShader(GL_FRAGMENT_SHADER);
	glad_glShaderSource(pixel_shader_uint, 1, pixel_shader, (const GLint*)MOON_NULL);
	glad_glCompileShader(pixel_shader_uint);
	glad_glGetShaderiv(pixel_shader_uint, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char error_log[1024];
		glad_glGetShaderInfoLog(pixel_shader_uint, 1024, (GLsizei*)MOON_NULL, error_log);
		MoonPrompt(error_log);
		MoonProjectError(shader_program, 1, (char*)"[MoonRendererLoad]函数错误,像素着色器编译失败!");
		MoonProjectDead();
		return;
	}
	success = 0;
	MoonPrompt((char*)"像素着色器编译完成");


	//組合著色器
	if (*shader_program)
		glad_glDeleteProgram(*shader_program);
	*shader_program = glad_glCreateProgram();
	glad_glAttachShader(*shader_program, vertex_shader_uint);
	glad_glAttachShader(*shader_program, pixel_shader_uint);
	glad_glLinkProgram(*shader_program);
	glad_glGetProgramiv(*shader_program, GL_LINK_STATUS, &success);
	if (!success)
	{
		char error_log[1024];
		glad_glGetProgramInfoLog(*shader_program, 1024, 0, error_log);
		MoonPrompt(error_log);
		MoonProjectError(shader_program, 1, (char*)"[MoonRendererLoad]函数错误,着色器鏈接失败!");
		MoonProjectDead();
		return;
	}
	success = 0;
	MoonPrompt((char*)"着色器链接完成");


	//刪除著色器對象
	glad_glDeleteShader(vertex_shader_uint);
	glad_glDeleteShader(pixel_shader_uint);
}

extern void MoonProjectSetVertices(unsigned int num)
{
	if(!moon_vertices)
		moon_vertices = num;
}

extern void MoonDrawArea(MOON_IMAGE* image_goal, MOON_IMAGE* image_source,int x,int y,int width ,int height)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
	metadata.draw.shader = texture_shader;
	metadata.draw.image_goal = image_goal;
	metadata.draw.image.image_resources = image_source;
	metadata.draw.image.x = x;
	metadata.draw.image.y = y;
	metadata.draw.image.apx = 0.f;
	metadata.draw.image.apy = 0.f;
	metadata.draw.image.width = width;
	metadata.draw.image.height = height;
	metadata.draw.image.uv_w = 1.f;
	metadata.draw.image.uv_h = 1.f;
	metadata.draw.image.deg = 0;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE, metadata);
}

extern void MoonDrawAreaRound(MOON_IMAGE* image_goal, MOON_IMAGE* image_source, int x, int y, float apx, float apy, int width, int height, int deg)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
	metadata.draw.shader = texture_shader;
	metadata.draw.image_goal = image_goal;
	metadata.draw.image.image_resources = image_source;
	metadata.draw.image.x = x;
	metadata.draw.image.y = y;
	metadata.draw.image.width = width;
	metadata.draw.image.height = height;
	metadata.draw.image.apx = apx;
	metadata.draw.image.apy = apy;
	metadata.draw.image.uv_w = 1.f;
	metadata.draw.image.uv_h = 1.f;
	metadata.draw.image.deg = deg;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE, metadata);
}

extern void MoonDrawAreaPlgBit(MOON_IMAGE* image_goal, MOON_IMAGE* image_source, MOON_POINT2D point[4])
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
	metadata.draw.shader = texture_shader;
	metadata.draw.image_goal = image_goal;
	metadata.draw.image_pig.image_resources = image_source;
	metadata.draw.image_pig.point[0] = point[0];
	metadata.draw.image_pig.point[1] = point[1];
	metadata.draw.image_pig.point[2] = point[2];
	metadata.draw.image_pig.point[3] = point[3];
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE_PIG, metadata);
}

extern void MoonDrawAreaUV(MOON_IMAGE* image_goal, MOON_IMAGE* image_source, int x, int y, int width, int height, float uv_x, float uv_y, float uv_width, float uv_height)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
	metadata.draw.shader = texture_shader;
	metadata.draw.image_goal = image_goal;
	metadata.draw.image.image_resources = image_source;
	metadata.draw.image.x = x;
	metadata.draw.image.y = y;
	metadata.draw.image.width = width;
	metadata.draw.image.height = height;
	metadata.draw.image.apx = uv_x;
	metadata.draw.image.apy = uv_y;
	metadata.draw.image.uv_w = uv_width;
	metadata.draw.image.uv_h = uv_height;
	metadata.draw.image.deg = 0;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE_UV, metadata);
}

extern void MoonDrawPix(MOON_IMAGE* image, int x, int y, unsigned int color)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
	metadata.draw.graphic.color_1 = color;
	metadata.draw.image_goal = image;
	metadata.draw.shader = solid_color_shader;
	metadata.draw.graphic.x1 = x;
	metadata.draw.graphic.y1 = y;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_PIX, metadata);
}

extern void MoonDrawLine(MOON_IMAGE* image, int x1, int y1, int x2, int y2, unsigned int color)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
	metadata.draw.graphic.color_1 = color;
	metadata.draw.graphic.color_2 = color;
	metadata.draw.image_goal = image;
	metadata.draw.shader = solid_color_shader;
	metadata.draw.graphic.x1 = x1;
	metadata.draw.graphic.y1 = y1;
	metadata.draw.graphic.x2 = x2;
	metadata.draw.graphic.y2 = y2;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_LINE, metadata);
}

extern void MoonDrawBox(MOON_IMAGE* image, int x1, int y1, int x2, int y2, unsigned int color)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
	metadata.draw.graphic.color_1 = color;
	metadata.draw.graphic.color_2 = color;
	metadata.draw.image_goal = image;
	metadata.draw.shader = solid_color_shader;
	metadata.draw.graphic.x1 = x1;
	metadata.draw.graphic.y1 = y1;
	metadata.draw.graphic.x2 = x2;
	metadata.draw.graphic.y2 = y1;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_LINE, metadata);
	metadata.draw.graphic.x1 = x2;
	metadata.draw.graphic.y2 = y2;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_LINE, metadata);
	metadata.draw.graphic.x1 = x2;
	metadata.draw.graphic.y1 = y2;
	metadata.draw.graphic.x2 = x1;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_LINE, metadata);
	metadata.draw.graphic.x1 = x1;
	metadata.draw.graphic.y1 = y2;
	metadata.draw.graphic.x2 = x1;
	metadata.draw.graphic.y2 = y1;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_LINE, metadata);
}

extern void MoonDrawTriFull(MOON_IMAGE* image, int x1, int y1, int x2, int y2, int x3, int y3, unsigned  int color)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
	metadata.draw.graphic.color_1 = color;
	metadata.draw.graphic.color_2 = color;
	metadata.draw.graphic.color_3 = color;
	metadata.draw.image_goal = image;
	metadata.draw.shader = solid_color_shader;
	metadata.draw.graphic.x1 = x1;
	metadata.draw.graphic.y1 = y1;
	metadata.draw.graphic.x2 = x2;
	metadata.draw.graphic.y2 = y2;
	metadata.draw.graphic.x3 = x3;
	metadata.draw.graphic.y3 = y3;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_TRI_FULL, metadata);
}

extern void MoonDrawBoxFull(MOON_IMAGE* image, int x1, int y1, int x2, int y2, unsigned int color)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
	metadata.draw.graphic.color_1 = color;
	metadata.draw.graphic.color_2 = color;
	metadata.draw.graphic.color_3 = color;
	metadata.draw.image_goal = image;
	metadata.draw.shader = solid_color_shader;
	metadata.draw.graphic.x1 = x1;
	metadata.draw.graphic.y1 = y1;
	metadata.draw.graphic.x2 = x1;
	metadata.draw.graphic.y2 = y2;
	metadata.draw.graphic.x3 = x2;
	metadata.draw.graphic.y3 = y1;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_TRI_FULL, metadata);
	metadata.draw.graphic.x1 = x2;
	metadata.draw.graphic.y3 = y2;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_TRI_FULL, metadata);
}

extern int MoonAnimeRun(MOON_IMAGE* image, MOON_ANIME* anime, int animeswitch, int x, int y, int width, int height)
{
	if (!animeswitch)
		return 0;
	else
	{
		anime->number %= anime->totalnumber;
		MOON_METADATA metadata = { 0 };
		metadata.draw.shader = texture_shader;
		metadata.draw.image_goal = image;
		metadata.draw.image.image_resources = &anime->sequenceframes[anime->number];
		metadata.draw.image.x = x;
		metadata.draw.image.y = y;
		metadata.draw.image.apx = 0.f;
		metadata.draw.image.apy = 0.f;
		metadata.draw.image.uv_w = 1.f;
		metadata.draw.image.uv_h = 1.f;
		metadata.draw.image.width = width;
		metadata.draw.image.height = height;
		metadata.draw.image.deg = 0;
		MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE, metadata);
	}
	if (MoonTimeLoad(&(anime->timeload), 1)) ++anime->number;
	return (int)anime->number;
}

extern void MoonDrawBox_Round(MOON_IMAGE* image, int x1, int y1, int x2, int y2, unsigned int r, unsigned int color)
{
	int x = MoonMin(x1, x2),
		y = MoonMin(y1, y2),
		w = abs(x2 - x1),
		h = abs(y2 - y1);

	int round_left = x + r,
		round_top = y + r,
		round_right = x + w - r,
		round_bottom = y + h - r;

	//四条直边
	{
		MoonDrawLine(image, round_left, y, round_right, y, color);
		MoonDrawLine(image, round_left, y + h, round_right, y + h, color);
		MoonDrawLine(image, x, round_top, x, round_bottom, color);
		MoonDrawLine(image, x + w, round_top, x + w, round_bottom, color);
	}
	MOON_POINT2D point[37];
	for (int index = 0; index < 36; ++index)
	{
		float rad = MoonDegRad((float)(index * 10));
		point[index].x = (int)(cosf(rad) * r);
		point[index].y = (int)(sinf(rad) * r);
	}

	point[36] = point[0];

	for (int index = 0; index < 9; ++index)
		MoonDrawLine(image, point[index].x + round_right, point[index].y + round_bottom, point[index + 1].x + round_right, point[index + 1].y + round_bottom, color);

	for (int index = 9; index < 18; ++index)
		MoonDrawLine(image, point[index].x + round_left, point[index].y + round_bottom, point[index + 1].x + round_left, point[index + 1].y + round_bottom, color);

	for (int index = 18; index < 27; ++index)
		MoonDrawLine(image, point[index].x + round_left, point[index].y + round_top, point[index + 1].x + round_left, point[index + 1].y + round_top, color);

	for (int index = 27; index < 36; ++index)
		MoonDrawLine(image, point[index].x + round_right, point[index].y + round_top, point[index + 1].x + round_right, point[index + 1].y + round_top, color);
}

extern void MoonDrawBoxFull_Round(MOON_IMAGE* image, int x1, int y1, int x2, int y2, unsigned int r, unsigned int color)
{
	int x = (x2 - x1 > 0 ? x1 : x2),
		y = (y2 - y1 > 0 ? y1 : y2),
		w = abs(x2 - x1),
		h = abs(y2 - y1);

	int round_left = x + r,
		round_top = y + r,
		round_right = x + w - r,
		round_bottom = y + h - r;

	MOON_POINT2D point[37];
	for (int index = 0; index < 36; ++index)
	{
		float rad = MoonDegRad((float)(index * 10));
		point[index].x = (int)(cosf(rad) * r);
		point[index].y = (int)(sinf(rad) * r);
	}

	point[36] = point[0];

	MoonDrawBoxFull(image, round_left, y, round_right, y + h, color);
	MoonDrawBoxFull(image, x, round_top, x + r, round_bottom, color);
	MoonDrawBoxFull(image, round_right, round_top, x + w, round_bottom, color);

	for (int index = 0; index < 9; ++index)
		MoonDrawTriFull(image, round_right, round_bottom, point[index + 1].x + round_right, point[index + 1].y + round_bottom, point[index].x + round_right, point[index].y + round_bottom, color);

	for (int index = 9; index < 18; ++index)
		MoonDrawTriFull(image, round_left, round_bottom, point[index + 1].x + round_left, point[index + 1].y + round_bottom, point[index].x + round_left, point[index].y + round_bottom, color);

	for (int index = 18; index < 27; ++index)
		MoonDrawTriFull(image, round_left, round_top, point[index + 1].x + round_left, point[index + 1].y + round_top, point[index].x + round_left, point[index].y + round_top, color);

	for (int index = 27; index < 36; ++index)
		MoonDrawTriFull(image, round_right, round_top, point[index + 1].x + round_right, point[index + 1].y + round_top, point[index].x + round_right, point[index].y + round_top, color);
}

extern void MoonDrawCircleFull(MOON_IMAGE* image, int x, int y, int r, unsigned int color)
{
	MOON_POINT2D point[37];
	for (int index = 0; index < 36; ++index)
	{
		float rad = MoonDegRad((float)(index * 10));
		point[index].x = (int)(cosf(rad) * r);
		point[index].y = (int)(sinf(rad) * r);
	}

	point[36] = point[0];

	for (int index = 0; index < 36; ++index)
		MoonDrawTriFull(image, x, y, point[index + 1].x + x, point[index + 1].y + y, point[index].x + x, point[index].y + y, color);
}


extern void MoonDrawCircle(MOON_IMAGE* image, int x, int y, int r, unsigned int color)
{
	if (r <= 0)
		return;
	MOON_POINT2D points[37];
	for (int index = 0; index < 36; ++index)
	{
		points[index].x = (int)(cosf(MoonDegRad((float)(index * 10))) * r) + x;
		points[index].y = (int)(sinf(MoonDegRad((float)(index * 10))) * r) + y;
	}
	points[36] = points[0];
	for (int index = 0; index < 36; ++index)
		MoonDrawLine(image, points[index].x, points[index].y, points[index + 1].x, points[index + 1].y, color);
}


extern void MoonDrawTextFont(MOON_IMAGE* image, const char* text, int x, int y, int sizewidth, int sizeheight, unsigned int color)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = color;
	metadata.draw.image_goal = image;
	metadata.draw.text.coord.x = x;
	metadata.draw.text.coord.y = y;
	metadata.draw.text.size_w = sizewidth;
	metadata.draw.text.size_h = sizeheight;
	int len = (int)strlen(text);
	if (!len)
		return;
	char* textbuffer = (char*)malloc(sizeof(char) * (len + 1));
	if (!textbuffer)
		return;
	strcpy(textbuffer, text);
	textbuffer[len] = '\0';
	metadata.draw.text.text = textbuffer;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_TEXT, metadata);
}

extern void MoonImageCreate(MOON_IMAGE* image, int bmpwidth, int bmpheight)
{
	glad_glGenTextures(1, &image->image.texture);
	glad_glBindTexture(GL_TEXTURE_2D, image->image.texture);
	glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	int data_size = bmpwidth * bmpheight * 4;
	unsigned char* data = (unsigned char*)calloc(data_size, 1);
	if (data)
	{
		glad_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)bmpwidth, (GLsizei)bmpheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		free(data);
	}
	else
		MoonPrompt((char*)"[MoonImageCreate]函数警告,内存分配失败");
	glad_glGenFramebuffers(1, &image->image.fbo);
	glad_glBindFramebuffer(GL_FRAMEBUFFER, image->image.fbo);
	glad_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, image->image.texture, 0);

	image->image_size.w = bmpwidth;
	image->image_size.h = bmpheight;

	char text[255];
	snprintf(text, 255, "创建了一个纹理,值为%d", image->image.texture);
	MoonPrompt(text);
}

extern void MoonImageDelete(MOON_IMAGE* image)
{
	char text[255];
	snprintf(text, 255, "销毁了一个纹理,值为%d", image->image.texture);
	MoonPrompt(text);
	glad_glDeleteTextures(1, &image->image.texture);
	glad_glDeleteFramebuffers(1, &image->image.fbo);
	image->image.texture = 0;
	image->image.fbo = 0;
	image->image_size.h = 0;
	image->image_size.w = 0;
}

extern void MoonImageLoad(MOON_IMAGE* image, const char** imagefile, int imagenumber)
{
	for (int index = 0; index < imagenumber; ++index)
	{
		unsigned int old_id = image[index].image.texture;
		MOON_IMAGE image_buffer;
		int bmpwidth, bmpheight, bmpchannels;
		glad_glGenTextures(1, &image_buffer.image.texture);
		glad_glBindTexture(GL_TEXTURE_2D, image_buffer.image.texture);
		glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		stbi_set_flip_vertically_on_load(1);
		unsigned char* data = stbi_load(imagefile[index], &bmpwidth, &bmpheight, &bmpchannels, 4);

		image_buffer.image_size.w = bmpwidth;
		image_buffer.image_size.h = bmpheight;

		glad_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmpwidth, bmpheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glad_glGenFramebuffers(1, &image_buffer.image.fbo);
		glad_glBindFramebuffer(GL_FRAMEBUFFER, image_buffer.image.fbo);
		glad_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, image_buffer.image.texture, 0);

		stbi_image_free(data);

		MoonImageDelete(&image[index]);
		image[index] = image_buffer;

		char text[255];
		snprintf(text, 255, "加载了一个纹理,值为%d,原值为%d", image[index].image.texture, old_id);
		MoonPrompt(text);
	}
}

extern void MoonImageLoadBatch(MOON_IMAGE* image, int totalnumber, const char** name, int width, int height)
{
	for (int index = 0; index < totalnumber; ++index)
		MoonImageCreate(&image[index], width, height);
	MoonImageLoad(image, name, totalnumber);
}

extern int MoonAnimeInit(MOON_ANIME* anime, MOON_IMAGE* sequenceframes, int timeload, int totalnumber, int width, int height)
{
	if (totalnumber <= 0) { MoonPrompt((char*)"[AnimeInit函数错误]动画序列帧总数有问题!\n"); return MOON_Error; }
	if (sequenceframes == NULL) { MoonPrompt((char*)"[AnimeInit函数错误]动画序列帧有问题,请检查名为动画是否存在!\n"); return MOON_Error; }
	anime->sequenceframes = sequenceframes;
	anime->totalnumber = totalnumber;
	anime->number = 0;
	for (int index = 0; index < anime->totalnumber; ++index)
	{
		anime->sequenceframes[index].image_size.x = width;
		anime->sequenceframes[index].image_size.y = height;
	}
	MoonTimeLoadInit(&(anime->timeload), timeload);		//设置定时器
	return MOON_YES;
}

extern void MoonAnimeDelete(MOON_ANIME* anime)
{
	if (anime == MOON_NULL) return;
	for (int index = 0; index < anime->totalnumber; ++index)
		MoonImageDelete(&anime->sequenceframes[index]);
	anime->sequenceframes = (MOON_IMAGE*)MOON_NULL;
	anime->totalnumber = 0;
	anime->number = 0;
}

extern void MoonAnimeCreate(
	MOON_IMAGE* image, MOON_ANIME* anime,
	int totalnumber, const char** animename, char* entityname,
	int timeload, int width, int height)
{
	MoonImageLoadBatch(image, totalnumber, animename, width, height);
	MoonAnimeInit(anime, image, timeload, totalnumber, width, height);
	MoonCreateEntityIndex(anime, entityname, sizeof(MOON_ANIME), (char*)"MOON_ANIME");
}

extern void MoonImageDesignated(MOON_IMAGE* image)
{
	//glad_glBindFramebuffer	紋理作爲畫布(寫入)
	//glad_glBindTexture		紋理作爲資源(讀取)
	glad_glBindFramebuffer(GL_FRAMEBUFFER, (unsigned int)image->image.fbo);
}

extern void MoonImageShader(unsigned int shader)
{
	glad_glUseProgram(shader);
}

extern void MoonCoreDrawArea(MOON_TEXTURE_VECTER* vertexs, unsigned int vertex_number, MOON_METADATA* metadata)
{
	if (!metadata->draw.image_goal || !metadata->draw.image.image_resources)
	{
		MoonPrompt((char*)"无效的纹理对象");
		return;
	}

	MoonImageShader(metadata->draw.shader);

	{
		float
			color_buffer = 1.f / 255.f;
		float
			r = ((metadata->draw.color >> 0) & 0xFF) * color_buffer,
			g = ((metadata->draw.color >> 8) & 0xFF) * color_buffer,
			b = ((metadata->draw.color >> 16) & 0xFF) * color_buffer,
			a = ((metadata->draw.color >> 24) & 0xFF) * color_buffer;
		glad_glBindVertexArray(moon_vao_texture);
		glad_glBindTexture(GL_TEXTURE_2D, metadata->draw.image.image_resources->image.texture);
		glad_glUniform4f(glad_glGetUniformLocation(metadata->draw.shader, "moon_ucolor"), r, g, b, a);
		glad_glUniform1i(glad_glGetUniformLocation(metadata->draw.shader, "moon_utexture"), 0);
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_texture);
		glad_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MOON_TEXTURE_VECTER) * vertex_number, vertexs);
		glad_glDrawArrays(GL_TRIANGLES, 0, vertex_number);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
		glad_glBindVertexArray(0);
	}
}

extern int MoonCoreGraphic(MOON_GRAPHIC_VECTER* vertexs, unsigned int vertex_number, MOON_METADATA* metadata, unsigned int message_type)
{
	if (!metadata->draw.image_goal)
	{
		MoonPrompt((char*)"无效的纹理对象");
		return MOON_Error;
	}

	MoonImageShader(metadata->draw.shader);
	
	GLenum graphic_mode = 0;

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

	{
		glad_glBindVertexArray(moon_vao_solid);
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_solid);
		glad_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MOON_GRAPHIC_VECTER) * vertex_number, vertexs);
		glad_glDrawArrays(graphic_mode, 0, vertex_number);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
		glad_glBindVertexArray(0);
	}
	return MOON_TRUE;
}

extern void MoonCoreFont(MOON_METADATA* metadata)
{
	if (!metadata->draw.image_goal)
	{
		MoonPrompt((char*)"无效的纹理对象");
		return;
	}

	float
		color_buffer = 1.f / 255.f;

	float
		r = ((metadata->draw.color >> 0) & 0xFF) * color_buffer,
		g = ((metadata->draw.color >> 8) & 0xFF) * color_buffer,
		b = ((metadata->draw.color >> 16) & 0xFF) * color_buffer,
		a = ((metadata->draw.color >> 24) & 0xFF) * color_buffer,
		font_w = 1.f / MOON_FONT_CHAR_COUNT,
		uv_h = 1.f;

	MOON_POINT2D image_buffer_size;

	image_buffer_size.w = metadata->draw.text.size_w;
	image_buffer_size.h = metadata->draw.text.size_h;

	unsigned int 
		text_index = 0,
		len = (unsigned int)strlen((const char*)metadata->draw.text.text);
	int start_x = metadata->draw.text.coord.x,
		n = 0,
		ch_index = 0;
	MOON_TEXTURE_VECTER* text_font_image = (MOON_TEXTURE_VECTER*)malloc(sizeof(MOON_TEXTURE_VECTER) * (len + 1) * 6);


	for (unsigned int index = 0; index < len; ++index)
	{
		int ch = metadata->draw.text.text[index];
		if (ch == '\n')
		{
			++n;
			ch_index = 0;  // 换行，重置行内计数
			continue;             // 不生成换行符的顶点
		}

		int x = image_buffer_size.w * ch_index + metadata->draw.text.coord.x;
		float
			w_buffer = 1.f / metadata->draw.image_goal->image_size.w,
			h_buffer = 1.f / metadata->draw.image_goal->image_size.h;

		float
			vx1 = MoonLerp(-1.f, 1.f, x * w_buffer),
			vy1 = MoonLerp(1.f, -1.f, (metadata->draw.text.coord.y + image_buffer_size.h * n) * h_buffer),
			vx2 = MoonLerp(-1.f, 1.f, (x + image_buffer_size.w) * w_buffer),
			vy2 = MoonLerp(1.f, -1.f, (metadata->draw.text.coord.y + image_buffer_size.h * (n + 1)) * h_buffer),
			uv_w = font_w,
			apx = ch * font_w;

		float
			uv_left = MoonRange(apx, 0, 1.f),
			uv_right = MoonRange(apx + uv_w, 0, 1.f),
			uv_top = 0.f,
			uv_bottom = 1.f;
		
		MoonTextureVertexinitTemp(text_font_image, text_index + 0, vx1, vy2, uv_left, uv_bottom);
		MoonTextureVertexinitTemp(text_font_image, text_index + 1, vx2, vy2, uv_right, uv_bottom);
		MoonTextureVertexinitTemp(text_font_image, text_index + 2, vx1, vy1, uv_left, uv_top);
		MoonTextureVertexinitTemp(text_font_image, text_index + 3, vx2, vy1, uv_right, uv_top);
		MoonTextureVertexinitTemp(text_font_image, text_index + 4, vx1, vy1, uv_left, uv_top);
		MoonTextureVertexinitTemp(text_font_image, text_index + 5, vx2, vy2, uv_right, uv_bottom);

		text_index += 6;
		++ch_index;
	}

	MoonImageShader(texture_shader);

	{
		glad_glBindVertexArray(moon_vao_texture);
		glad_glBindTexture(GL_TEXTURE_2D, moon_simple_font.image.texture);
		glad_glUniform4f(glad_glGetUniformLocation(texture_shader, "moon_ucolor"), r, g, b, a);
		glad_glUniform1i(glad_glGetUniformLocation(texture_shader, "moon_utexture"), 0);
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_texture);
		glad_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MOON_TEXTURE_VECTER) * text_index, text_font_image);
		glad_glDrawArrays(GL_TRIANGLES, 0, text_index);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
		glad_glBindVertexArray(0);
	}

	free(text_font_image);
}

extern void MoonDrawMessageHandle(MOON_MESSAGE_ALL* message, unsigned char* type)
{
	MOON_IMAGE* image_old = (MOON_IMAGE*)MOON_NULL;
	MOON_IMAGE* image_resource_old = (MOON_IMAGE*)MOON_NULL;
	for (unsigned int index = 0; index < message->message_index; ++index)
		if (*type)
		{
			switch (message->message[index].message)
			{
			case MOON_MESSAGE_DRAW_END: *type = MOON_FALSE; break;
			//case MOON_MESSAGE_DRAW_SETDRAW:				MoonProjectFunctionSwitch(MOON_MODULE_DRAW, message->message[index].metadata.function);			break;

			case MOON_MESSAGE_DRAW_IMAGE:
			{
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 0))
					break;
				image_resource_old = message->message[index].metadata.draw.image.image_resources;

				MOON_METADATA* metadata = &message->message[index].metadata;

				{

					float
						w_buffer = 1.f / metadata->draw.image_goal->image_size.w,
						h_buffer = 1.f / metadata->draw.image_goal->image_size.h;
					float
						vx1 = MoonLerp(-1.f, 1.f, metadata->draw.image.x * w_buffer),
						vy1 = MoonLerp(1.f, -1.f, metadata->draw.image.y * h_buffer),
						vx2 = MoonLerp(-1.f, 1.f, (metadata->draw.image.x + metadata->draw.image.image_resources->image_size.w) * w_buffer),
						vy2 = MoonLerp(1.f, -1.f, (metadata->draw.image.y + metadata->draw.image.image_resources->image_size.h) * h_buffer);
					float
						vx3 = vx1,
						vy3 = vy2,
						vx4 = vx2,
						vy4 = vy1;

					{
						//这里处理旋转

						float
							cosrad = (float)cosf(MoonDegRad((float)(metadata->draw.image.deg))),
							sinrad = (float)sinf(MoonDegRad((float)(metadata->draw.image.deg)));
						float matrix2d[4] = { cosrad,-sinrad,sinrad,cosrad };
						int	apx = (int)MoonLerp(0, metadata->draw.image.width, metadata->draw.image.apx),
							apy = (int)MoonLerp(0, metadata->draw.image.height, metadata->draw.image.apy);

						MOON_POINT2D points[4];

						int left = 0 - apx,
							top = 0 - apy,
							right = metadata->draw.image.width - apx,
							bottom = metadata->draw.image.height - apy;

						points[0].x = (int)(left * matrix2d[0] + top * matrix2d[1] + metadata->draw.image.x);
						points[0].y = (int)(left * matrix2d[2] + top * matrix2d[3] + metadata->draw.image.y);
						points[1].x = (int)(right * matrix2d[0] + top * matrix2d[1] + metadata->draw.image.x);
						points[1].y = (int)(right * matrix2d[2] + top * matrix2d[3] + metadata->draw.image.y);
						points[2].x = (int)(left * matrix2d[0] + bottom * matrix2d[1] + metadata->draw.image.x);
						points[2].y = (int)(left * matrix2d[2] + bottom * matrix2d[3] + metadata->draw.image.y);
						points[3].x = (int)(right * matrix2d[0] + bottom * matrix2d[1] + metadata->draw.image.x);
						points[3].y = (int)(right * matrix2d[2] + bottom * matrix2d[3] + metadata->draw.image.y);

						vx1 = MoonLerp(-1.f, 1.f, points[0].x * w_buffer);
						vy1 = MoonLerp(1.f, -1.f, points[0].y * h_buffer);
						vx2 = MoonLerp(-1.f, 1.f, points[3].x * w_buffer);
						vy2 = MoonLerp(1.f, -1.f, points[3].y * h_buffer);
						vx3 = MoonLerp(-1.f, 1.f, points[2].x * w_buffer);
						vy3 = MoonLerp(1.f, -1.f, points[2].y * h_buffer);
						vx4 = MoonLerp(-1.f, 1.f, points[1].x * w_buffer);
						vy4 = MoonLerp(1.f, -1.f, points[1].y * h_buffer);

					}

					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 0, vx1, vy1, 0.f, 1.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 1, vx3, vy3, 0.f, 0.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 2, vx4, vy4, 1.f, 1.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 3, vx2, vy2, 1.f, 0.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 4, vx4, vy4, 1.f, 1.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 5, vx3, vy3, 0.f, 0.f);
				}

				moon_vertex_texture_index += 6;

				MoonDrawAreaTemp(MOON_MESSAGE_DRAW_IMAGE, index, message, image_old, image_resource_old);

			}
			break;

			case MOON_MESSAGE_DRAW_IMAGE_UV:
			{
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 0))
					break;
				image_resource_old = message->message[index].metadata.draw.image.image_resources;

				MOON_METADATA* metadata = &message->message[index].metadata;

				{
					MOON_POINT2D image_buffer_size;

					image_buffer_size.w = (int)(metadata->draw.image.image_resources->image_size.w * metadata->draw.image.uv_w);
					image_buffer_size.h = (int)(metadata->draw.image.image_resources->image_size.h * metadata->draw.image.uv_h);

					metadata->draw.image.apy = 1.f - metadata->draw.image.apy;

					float
						w_buffer = 1.f / metadata->draw.image_goal->image_size.w,
						h_buffer = 1.f / metadata->draw.image_goal->image_size.h;
					float
						vx1 = MoonLerp(-1.f, 1.f, metadata->draw.image.x * w_buffer),
						vy1 = MoonLerp(1.f, -1.f, metadata->draw.image.y * h_buffer),
						vx2 = MoonLerp(-1.f, 1.f, (metadata->draw.image.x + image_buffer_size.w + metadata->draw.image.width) * w_buffer),
						vy2 = MoonLerp(1.f, -1.f, (metadata->draw.image.y + image_buffer_size.h + metadata->draw.image.height) * h_buffer),
						uv_w = metadata->draw.image.uv_w,
						uv_h = metadata->draw.image.uv_h;

					float
						uv_left = MoonRange(metadata->draw.image.apx, 0, 1.f),
						uv_right = MoonRange(metadata->draw.image.apx + uv_w, 0, 1.f),
						uv_top = MoonRange(metadata->draw.image.apy, 0, 1.f),
						uv_bottom = MoonRange(metadata->draw.image.apy - uv_h, 0, 1.f);

					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 0, vx1, vy2, uv_left, uv_bottom);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 1, vx2, vy2, uv_right, uv_bottom);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 2, vx1, vy1, uv_left, uv_top);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 3, vx2, vy1, uv_right, uv_top);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 4, vx1, vy1, uv_left, uv_top);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 5, vx2, vy2, uv_right, uv_bottom);
				}

				moon_vertex_texture_index += 6;

				MoonDrawAreaTemp(MOON_MESSAGE_DRAW_IMAGE_UV, index, message, image_old, image_resource_old);

			}
			break;
			
			case MOON_MESSAGE_DRAW_IMAGE_PIG:
			{
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 0))
					break;
				image_resource_old = message->message[index].metadata.draw.image.image_resources;

				MOON_METADATA* metadata = &message->message[index].metadata;

				{
					float
						w_buffer = 1.f / metadata->draw.image_goal->image_size.w,
						h_buffer = 1.f / metadata->draw.image_goal->image_size.h;
					float
						vx1 = MoonLerp(-1.f, 1.f, metadata->draw.image_pig.point[2].x * w_buffer),
						vy1 = MoonLerp(1.f, -1.f, metadata->draw.image_pig.point[2].y * h_buffer),
						vx2 = MoonLerp(-1.f, 1.f, metadata->draw.image_pig.point[1].x * w_buffer),
						vy2 = MoonLerp(1.f, -1.f, metadata->draw.image_pig.point[1].y * h_buffer),
						vx3 = MoonLerp(-1.f, 1.f, metadata->draw.image_pig.point[0].x * w_buffer),
						vy3 = MoonLerp(1.f, -1.f, metadata->draw.image_pig.point[0].y * h_buffer),
						vx4 = MoonLerp(-1.f, 1.f, metadata->draw.image_pig.point[3].x * w_buffer),
						vy4 = MoonLerp(1.f, -1.f, metadata->draw.image_pig.point[3].y * h_buffer);

					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 0, vx1, vy1, 0.f, 1.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 1, vx3, vy3, 0.f, 0.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 2, vx4, vy4, 1.f, 1.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 3, vx2, vy2, 1.f, 0.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 4, vx4, vy4, 1.f, 1.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 5, vx3, vy3, 0.f, 0.f);
				}
			
				moon_vertex_texture_index += 6;

				MoonDrawAreaTemp(MOON_MESSAGE_DRAW_IMAGE_PIG, index, message, image_old, image_resource_old);

			}
			break;
			
			case MOON_MESSAGE_DRAW_IMAGE_CLEAN:
			{
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, -1))
					break;

				float
					color_buffer = 1.f / 255.f;

				float
					r = ((message->message[index].metadata.draw.color >> 0) & 0xFF) * color_buffer,
					g = ((message->message[index].metadata.draw.color >> 8) & 0xFF) * color_buffer,
					b = ((message->message[index].metadata.draw.color >> 16) & 0xFF) * color_buffer,
					a = ((message->message[index].metadata.draw.color >> 24) & 0xFF) * color_buffer;
				glad_glClearColor(r, g, b, a);
				glad_glClear(GL_COLOR_BUFFER_BIT);
			}
			break;
			
			case MOON_MESSAGE_DRAW_PIX:
			{
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 1))
					break;

				float
					color_buffer = 1.f / 255.f;

				float
					r = ((message->message[index].metadata.draw.color >> 0) & 0xFF) * color_buffer,
					g = ((message->message[index].metadata.draw.color >> 8) & 0xFF) * color_buffer,
					b = ((message->message[index].metadata.draw.color >> 16) & 0xFF) * color_buffer,
					a = ((message->message[index].metadata.draw.color >> 24) & 0xFF) * color_buffer,
					vx1 = MoonLerp(-1.f, 1.f, message->message[index].metadata.draw.graphic.x1 * 1.f / message->message[index].metadata.draw.image_goal->image_size.w),
					vy1 = MoonLerp(1.f, -1.f, message->message[index].metadata.draw.graphic.y1 * 1.f / message->message[index].metadata.draw.image_goal->image_size.h);

				float
					r_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 0) & 0xFF) * color_buffer * r,
					g_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 8) & 0xFF) * color_buffer * g,
					b_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 16) & 0xFF) * color_buffer * b,
					a_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 24) & 0xFF) * color_buffer * a;

				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 0, vx1, vy1, r_1, g_1, b_1, a_1);

				moon_vertex_index += 1;

				MoonCoreGraphicTemp(MOON_MESSAGE_DRAW_PIX, index, message, image_old);

			}
			break;

			case MOON_MESSAGE_DRAW_LINE:
			{
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 2))
					break;

				float
					color_buffer = 1.f / 255.f,
					w_buffer = 1.f / message->message[index].metadata.draw.image_goal->image_size.w,
					h_buffer = 1.f / message->message[index].metadata.draw.image_goal->image_size.h;

				float
					r = ((message->message[index].metadata.draw.color >> 0) & 0xFF) * color_buffer,
					g = ((message->message[index].metadata.draw.color >> 8) & 0xFF) * color_buffer,
					b = ((message->message[index].metadata.draw.color >> 16) & 0xFF) * color_buffer,
					a = ((message->message[index].metadata.draw.color >> 24) & 0xFF) * color_buffer,
					vx1 = MoonLerp(-1.f, 1.f, message->message[index].metadata.draw.graphic.x1 * w_buffer),
					vy1 = MoonLerp(1.f, -1.f, message->message[index].metadata.draw.graphic.y1 * h_buffer),
					vx2 = MoonLerp(-1.f, 1.f, message->message[index].metadata.draw.graphic.x2 * w_buffer),
					vy2 = MoonLerp(1.f, -1.f, message->message[index].metadata.draw.graphic.y2 * h_buffer);

				float
					r_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 0) & 0xFF) * color_buffer * r,
					g_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 8) & 0xFF) * color_buffer * g,
					b_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 16) & 0xFF) * color_buffer * b,
					a_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 24) & 0xFF) * color_buffer * a,
					r_2 = ((message->message[index].metadata.draw.graphic.color_2 >> 0) & 0xFF) * color_buffer * r,
					g_2 = ((message->message[index].metadata.draw.graphic.color_2 >> 8) & 0xFF) * color_buffer * g,
					b_2 = ((message->message[index].metadata.draw.graphic.color_2 >> 16) & 0xFF) * color_buffer * b,
					a_2 = ((message->message[index].metadata.draw.graphic.color_2 >> 24) & 0xFF) * color_buffer * a;

				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 0, vx1, vy1, r_1, g_1, b_1, a_1);
				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 1, vx2, vy2, r_2, g_2, b_2, a_2);

				moon_vertex_index += 2;

				MoonCoreGraphicTemp(MOON_MESSAGE_DRAW_LINE, index, message, image_old);

			}
			break;

			case MOON_MESSAGE_DRAW_TRI_FULL:
			{
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 3))
					break;

				float
					color_buffer = 1.f / 255.f,
					w_buffer = 1.f / message->message[index].metadata.draw.image_goal->image_size.w,
					h_buffer = 1.f / message->message[index].metadata.draw.image_goal->image_size.h;

				float
					r = ((message->message[index].metadata.draw.color >> 0) & 0xFF) * color_buffer,
					g = ((message->message[index].metadata.draw.color >> 8) & 0xFF) * color_buffer,
					b = ((message->message[index].metadata.draw.color >> 16) & 0xFF) * color_buffer,
					a = ((message->message[index].metadata.draw.color >> 24) & 0xFF) * color_buffer,
					vx1 = MoonLerp(-1.f, 1.f, message->message[index].metadata.draw.graphic.x1 * w_buffer),
					vy1 = MoonLerp(1.f, -1.f, message->message[index].metadata.draw.graphic.y1 * h_buffer),
					vx2 = MoonLerp(-1.f, 1.f, message->message[index].metadata.draw.graphic.x2 * w_buffer),
					vy2 = MoonLerp(1.f, -1.f, message->message[index].metadata.draw.graphic.y2 * h_buffer),
					vx3 = MoonLerp(-1.f, 1.f, message->message[index].metadata.draw.graphic.x3 * w_buffer),
					vy3 = MoonLerp(1.f, -1.f, message->message[index].metadata.draw.graphic.y3 * h_buffer);

				float
					r_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 0) & 0xFF) * color_buffer * r,
					g_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 8) & 0xFF) * color_buffer * g,
					b_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 16) & 0xFF) * color_buffer * b,
					a_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 24) & 0xFF) * color_buffer * a,
					r_2 = ((message->message[index].metadata.draw.graphic.color_2 >> 0) & 0xFF) * color_buffer * r,
					g_2 = ((message->message[index].metadata.draw.graphic.color_2 >> 8) & 0xFF) * color_buffer * g,
					b_2 = ((message->message[index].metadata.draw.graphic.color_2 >> 16) & 0xFF) * color_buffer * b,
					a_2 = ((message->message[index].metadata.draw.graphic.color_2 >> 24) & 0xFF) * color_buffer * a,
					r_3 = ((message->message[index].metadata.draw.graphic.color_3 >> 0) & 0xFF) * color_buffer * r,
					g_3 = ((message->message[index].metadata.draw.graphic.color_3 >> 8) & 0xFF) * color_buffer * g,
					b_3 = ((message->message[index].metadata.draw.graphic.color_3 >> 16) & 0xFF) * color_buffer * b,
					a_3 = ((message->message[index].metadata.draw.graphic.color_3 >> 24) & 0xFF) * color_buffer * a;

				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 0, vx1, vy1, r_1, g_1, b_1, a_1);
				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 1, vx2, vy2, r_2, g_2, b_2, a_2);
				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 2, vx3, vy3, r_3, g_3, b_3, a_3);

				moon_vertex_index += 3;

				MoonCoreGraphicTemp(MOON_MESSAGE_DRAW_TRI_FULL, index, message, image_old);
			}
			break;

			case MOON_MESSAGE_DRAW_TEXT:
			{
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, -1))
					break;
				MoonCoreFont(&message->message[index].metadata);
				free(message->message[index].metadata.draw.text.text);
			}
			break;
			
			case MOON_MESSAGE_DRAW_UNIFORM:
			{
				MOON_UNIFORM_DATA* data = &message->message[index].metadata.uniform.data;
				MoonImageShader(message->message[index].metadata.uniform.shader);
				int location = glad_glGetUniformLocation(message->message[index].metadata.uniform.shader, (const GLchar*)message->message[index].metadata.uniform.var);
				switch (data->type)
				{
				case MOON_UNIFORM_TYPE_NONE:
					break;
				case MOON_UNIFORM_TYPE_VECTOR1_FLOAT:
					glad_glUniform1f(location, data->vec_float.x);
					break;
				case MOON_UNIFORM_TYPE_VECTOR2_FLOAT:
					glad_glUniform2f(location, data->vec_float.x, data->vec_float.y);
					break;
				case MOON_UNIFORM_TYPE_VECTOR3_FLOAT:
					glad_glUniform3f(location, data->vec_float.x, data->vec_float.y, data->vec_float.z);
					break;
				case MOON_UNIFORM_TYPE_VECTOR4_FLOAT:
					glad_glUniform4f(location, data->vec_float.x, data->vec_float.y, data->vec_float.z, data->vec_float.w);
					break;
				case MOON_UNIFORM_TYPE_VECTOR1_INT:
					glad_glUniform1i(location, data->vec_int.x);
					break;
				case MOON_UNIFORM_TYPE_VECTOR2_INT:
					glad_glUniform2i(location, data->vec_int.x, data->vec_int.y);
					break;
				case MOON_UNIFORM_TYPE_VECTOR3_INT:
					glad_glUniform3i(location, data->vec_int.x, data->vec_int.y, data->vec_int.z);
					break;
				case MOON_UNIFORM_TYPE_VECTOR4_INT:
					glad_glUniform4i(location, data->vec_int.x, data->vec_int.y, data->vec_int.z, data->vec_int.w);
					break;
				case MOON_UNIFORM_TYPE_VECTOR1_UINT:
					glad_glUniform1ui(location, data->vec_uint.x);
					break;
				case MOON_UNIFORM_TYPE_VECTOR2_UINT:
					glad_glUniform2ui(location, data->vec_uint.x, data->vec_uint.y);
					break;
				case MOON_UNIFORM_TYPE_VECTOR3_UINT:
					glad_glUniform3ui(location, data->vec_uint.x, data->vec_uint.y, data->vec_uint.z);
					break;
				case MOON_UNIFORM_TYPE_VECTOR4_UINT:
					glad_glUniform4ui(location, data->vec_uint.x, data->vec_uint.y, data->vec_uint.z, data->vec_uint.w);
					break;
				case MOON_UNIFORM_TYPE_TEXTURE:
					if (!data->texture.slot || !data->texture.image)
						break;
					glad_glActiveTexture(GL_TEXTURE0 + data->texture.slot);
					glad_glBindTexture(GL_TEXTURE_2D, data->texture.image->image.texture);
					glad_glUniform1i(location, data->texture.slot);
					break;
				default:
					MoonPrompt((char*)"[MoonShaderUniform] 错误的[MOON_UNIFORM_TYPE]类型输入");
					break;
				}
			}
			break;
			
			case MOON_MESSAGE_CULL_FACE:
			{
				if (message->message[index].metadata.cull_face)
					glad_glEnable(GL_CULL_FACE);
				else
					glad_glDisable(GL_CULL_FACE);
			}
			break;

			case MOON_MESSAGE_DRAW_OPEN:
			{
				message->message[index].metadata.function_open();
			}
			break;


			}
		}
		else return;
}

//此函数仅作为辅助函数
static inline void MoonVertexinitTemp(
	MOON_GRAPHIC_VECTER* vertex,
	unsigned int index_offset, 
	float vx, float vy, 
	float r, float g, float b, float a)
{
	vertex[index_offset].x = vx;
	vertex[index_offset].y = vy;
	vertex[index_offset].z = 0.f;
	vertex[index_offset].a = a;
	vertex[index_offset].r = r;
	vertex[index_offset].g = g;
	vertex[index_offset].b = b;
}


//此函数仅作为辅助函数
static inline void MoonTextureVertexinitTemp(
	MOON_TEXTURE_VECTER* vertex, 
	unsigned int index_offset, 
	float vx, float vy, float uv_x, float uv_y)
{
	vertex[index_offset].x = vx;
	vertex[index_offset].y = vy;
	vertex[index_offset].z = 0.f;
	vertex[index_offset].uv_x = uv_x;
	vertex[index_offset].uv_y = uv_y;
}

//此函数仅作为辅助函数
static inline unsigned char MoonSetTemp(
	MOON_IMAGE** image_old, 
	MOON_METADATA* metadata, 
	int offset)
{
	unsigned char out_temp = MOON_TRUE;
	if (metadata->draw.image_goal)
	{
		MOON_IMAGE* image_new = metadata->draw.image_goal;

		if (*image_old != image_new)
		{
			MoonImageDesignated(image_new);
			*image_old = image_new;
		}
		static MOON_POINT2D view;
		if (view.w != image_new->image_size.w
			|| view.h != image_new->image_size.h)
		{
			glad_glViewport(0, 0, image_new->image_size.w, image_new->image_size.h);
			view = image_new->image_size;
		}
	}
	else
	{
		MoonPrompt((char*)"[MoonSetTemp] goal传入的空指针错误");
		out_temp = MOON_FALSE;
	}

	//没有偏移,是纹理
	if (!offset)
	{
		if (!metadata->draw.image.image_resources)
		{
			MoonPrompt((char*)"[MoonSetTemp] image_resources传入的空指针错误");
			out_temp = MOON_FALSE;
		}
		else
			if (*image_old == metadata->draw.image.image_resources)
			{
				MoonPrompt((char*)"[MoonSetTemp] 禁止将image_resources设定为goal");
				out_temp = MOON_FALSE;
			}
	}
	else
		//检查图元顶点
		if (offset > 0 && moon_vertex_index >= (unsigned int)moon_vertices - offset)
		{
			MoonPrompt((char*)"[MoonSetTemp] 图形顶点溢出");
			out_temp = MOON_FALSE;
		}

	//检查纹理顶点
	if (moon_vertex_texture_index >= (unsigned int)moon_vertices - 6)
	{
		MoonPrompt((char*)"[MoonSetTemp] 纹理顶点溢出");
		out_temp = MOON_FALSE;
	}

	return out_temp;
}

//此函数仅作为辅助函数
static void MoonDrawAreaTemp(
	unsigned int message_type, 
	unsigned int index, 
	MOON_MESSAGE_ALL* message, 
	MOON_IMAGE* image_old, 
	MOON_IMAGE* image_resource_old)
{
	if ((index == message->message_index - 1)
		|| message->message[index + 1].message != message_type
		|| message->message[index + 1].metadata.draw.image_goal != image_old
		|| message->message[index + 1].metadata.draw.image.image_resources != image_resource_old
		|| message->message[index + 1].metadata.draw.color != message->message[index].metadata.draw.color)
	{
		MoonCoreDrawArea(moon_vertex_texture, moon_vertex_texture_index, &message->message[index].metadata);
		moon_vertex_texture_index = 0;
	}
}

//此函数仅作为辅助函数
static void MoonCoreGraphicTemp(
	unsigned int message_type, 
	unsigned int index, 
	MOON_MESSAGE_ALL* message, 
	MOON_IMAGE* image_old)
{
	if (
		(index == message->message_index - 1)
		|| message->message[index + 1].message != message_type
		|| message->message[index + 1].metadata.draw.image_goal != image_old)
	{
		MoonCoreGraphic(moon_vertex, moon_vertex_index, &message->message[index].metadata, message_type);
		moon_vertex_index = 0;
	}
}
