#include "Project_All.h"

/* 实体登记的是"数据本身的地址"：用静态对象，�?TuiQi / TDGame 一�?*/
static STOCK_GAME g_stockGame;
static MOON_TIMELOAD g_logicTps;
static MOON_IMAGE g_gameBack;
static unsigned int g_shaderCrt;

/* CR 管线：整个界面先画到 g_gameBack，再用这个小着色器贴回窗口 */
static const char* shader_pixel_crt =
"#version 330 core\n"
MoonString(
	in vec2 tex_uv;
out vec4 pixel_color;
uniform sampler2D moon_utexture;
uniform float u_time;
void main()
{
	vec2 uv = tex_uv * 2.0 - 1.0;
	float strength = -0.05;
	float r = length(uv);
	float new_r = r / (1.0 + strength * r * r);
	if (r > 0.01) {
		float scale = new_r / r;
		uv = uv * scale;
	}
	vec2 fish_uv = uv * 0.5 + 0.5;
	if (fish_uv.x < 0.0 || fish_uv.x > 1.0 || fish_uv.y < 0.0 || fish_uv.y > 1.0)
	{
		pixel_color = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}
	vec4 color = texture(moon_utexture, fish_uv);
	float scanline = sin(fish_uv.y * 900.0 + u_time * 3.0) * 0.10 + 0.90;
	float offset_rgb = 0.0006;
	float r_channel = texture(moon_utexture, fish_uv + vec2(offset_rgb, 0.0)).r;
	float b_channel = texture(moon_utexture, fish_uv - vec2(offset_rgb, 0.0)).b;
	float red_boost = 1.15;
	float blue_boost = 1.10;
	float green_weak = 1.33;
	vec2 center_vec = fish_uv * 2.0 - 1.0;
	float edge_factor = 1.0 - length(center_vec);
	edge_factor = clamp(edge_factor, 0.0, 1.0);
	float edge_bias = (1.0 - edge_factor) * 0.25;
	float luminance = color.r * 0.299 + color.g * 0.587 + color.b * 0.114;
	float shadow_bias = (1.0 - luminance) * 0.15;
	float highlight_bias = luminance * 0.10;
	float final_red_boost = red_boost + edge_bias + highlight_bias;
	float final_green_weak = green_weak - edge_bias * 0.5;
	float final_blue_boost = blue_boost + edge_bias + shadow_bias;
	vec3 chromatic;
	chromatic.r = r_channel * final_red_boost;
	chromatic.g = color.g * final_green_weak;
	chromatic.b = b_channel * final_blue_boost;
	float red_overflow = max(0.0, chromatic.r - 0.85) * 0.3;
	chromatic.r = min(chromatic.r + red_overflow, 1.0);
	chromatic.g = min(chromatic.g + red_overflow * 0.5, 1.0);
	vec2 uv_vignette = fish_uv * 2.0 - 1.0;
	float vignette = 1.0 - length(uv_vignette) * 0.16;
	vignette = clamp(vignette, 0.55, 1.0);
	vec3 final_color = chromatic * scanline * vignette * 1.25;
	final_color = pow(final_color, vec3(0.95));
	pixel_color = vec4(final_color, 1.0);
}
);

static const char* shader_vertex_crt =
"#version 330 core\n"
MoonString(
	layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 tex_uv;
void main()
{
	tex_uv = aTexCoord;
	gl_Position = vec4(aPos, 1.0);
}
);

extern MOON_PROJECTSETTING(_Resource)
{
	StockGameInit(&g_stockGame, 1);
	if (g_autoPlay)
		g_stockGame.screen = SCREEN_GAME;

	MoonCreateEntityIndex(&g_stockGame, "StockGame", sizeof(STOCK_GAME), "STOCK_GAME");
	MoonCreateEntityIndex(&g_logicTps, "LogicTps", sizeof(MOON_TIMELOAD), "MOON_TIMELOAD");

	MoonImageCreate(&g_gameBack, 1800, 900);
	MoonCreateEntityIndex(&g_gameBack, "GameBitmap", sizeof(MOON_IMAGE), "MOON_IMAGE");
	MoonCreateEntityIndex(&g_shaderCrt, "shader_crt", sizeof(unsigned int), "unsigned int");

	MoonShaderLoad((char**)&shader_vertex_crt, (char**)&shader_pixel_crt, &g_shaderCrt);
}
