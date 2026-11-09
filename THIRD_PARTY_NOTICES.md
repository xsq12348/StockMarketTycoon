# 第三方组件与许可

本项目本体以 MIT 许可发布（见仓库根目录 `LICENSE`）。引擎 MoonEngine 2.2.20.0 同样是 MIT 许可（见 `engine/LICENSE`）。

下面这些第三方组件随项目或发布包一起分发，各自保留原有许可：

| 组件 | 用途 | 许可 | 声明位置 |
|---|---|---|---|
| SDL3 | 窗口、输入、音频 | zlib | `engine/include/SDL3/LICENSE.txt` |
| GLFW | 窗口与输入（备选后端） | zlib | 见下方版权声明 |
| glad | OpenGL 函数加载 | MIT（生成的加载器为公有领域） | `engine/lib/glad.lib` |
| stb_image | 图像解码 | 公有领域 / MIT | `engine/include/Moon_stb_image.h` 头部 |
| Khronos 头文件 | OpenGL / EGL 类型定义 | Apache-2.0 | `engine/include/KHR`、`engine/include/GLAD` |

## SDL3

```
Copyright (c) 1997-2025 Sam Lantinga <slouken@libsdl.org>
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the
use of this software. Permission is granted to anyone to use this software for
any purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the restrictions in the zlib license.
```

## GLFW

```
Copyright (c) 2002-2006 Marcus Geelnard
Copyright (c) 2006-2019 Camilla Löwy
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the
use of this software. Permission is granted to anyone to use this software for
any purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the restrictions in the zlib license.
```

发布包 `dist/` 里的 `SDL3.dll` 与 `glfw3.dll` 是上述项目的未修改二进制，遵循各自的 zlib 许可。
