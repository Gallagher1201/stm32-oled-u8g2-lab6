# 实验六：STM32 I2C OLED 与 U8g2

学生：周新城；学号：632407030424。

本工程完成任务一的姓名学号、长文字滚动，以及任务三的 HAL/U8g2 移植、图形演示、四方向文字移动和动态图案。姓名学号页已上板实拍；AHT20 温湿度和任务二的仪器实测不在本次范围内。实验报告、博客草稿与本仓库说明见工程上级目录的 `report/`、`blog/` 和根目录 `README.md`。

## 打开与编译

直接打开 `MDK-ARM/Lab6_OLED.uvprojx`，在 Keil 中按 F7 编译，无需先运行 CubeMX。

本机已验证环境：Keil uVision 5、ARM Compiler 5.06 update 5 build 528、Keil.STM32F1xx_DFP 2.2.0。编译器使用 ARMCC5，C99，保留 `--no_multibyte_chars` 选项，源文件使用 UTF-8。不要随意改变此选项，否则中文字符串可能编译失败或编码不正确。

已生成的烧录文件：`MDK-ARM/Objects/Lab6_OLED.hex`，起始地址 `0x08000000`。`Lab6_OLED.axf` 用于 Keil 调试。工程使用相对源码路径，HAL、CMSIS 和裁剪后的 U8g2 源码均随工程提供。

## 硬件配置与接线

芯片为 STM32F103C8T6，Flash 64 KB，RAM 20 KB。程序使用内部 HSI 经 PLL 得到 64 MHz，不依赖板载外部晶振。APB1 为 32 MHz，I2C2 配置为 400 kHz。

OLED 按 **SSD1306、128×64、四针 I2C** 配置，启动时自动探测七位地址 `0x3C`，失败后尝试 `0x3D`。照片可确认四针接口，但不能据此确认控制器；若实际是 SH1106 或其他控制器，需要更换 U8g2 对应驱动后重新编译。

| OLED 丝印 | STM32 引脚 |
| --- | --- |
| GND | GND |
| VCC | 3.3V |
| SCL | PB10 / I2C2_SCL |
| SDA | PB11 / I2C2_SDA |

按模块丝印接线，不按线缆颜色判断。SCL/SDA 为开漏信号，需要到 3.3V 的上拉；模块一般已带上拉，如没有，可各接约 4.7 kΩ。

ST-LINK 使用 SWD：SWDIO 接 PA13、SWCLK 接 PA14、GND 共地，目标电压为 3.3V；BOOT0 保持 0。避免两个电源同时给开发板供电。2026-10-21 用户接入设备后追加授权烧录，已通过 Keil/ST-LINK 完成写入与校验，显示 Programming Done、Verify OK；尚未确认 OLED 实际显示效果。

## 程序显示顺序

上电后自动循环，无需按钮操作：

1. 显示“周新城”和“632407030424”，停留约 3 秒。
2. U8g2 图形 demo：边框、矩形、圆、实心圆、直线和动态进度条。
3. 长句“Hello，欢迎来到重庆交通大学物联网205实训室！”先向左、再向右移动，完整句宽 337 像素。
4. 多行中文和学号先向上、再向下移动，标题区域固定。
5. 32×32 笑脸点阵移动、上下浮动并眨眼。
6. SSD1306 硬件左滚、右滚，各约 4 秒，上半屏固定。

长句超出屏幕宽度，使用 U8g2 坐标移动和裁剪显示；另提供 SSD1306 硬件滚屏示例。硬件滚屏只能移动显存中已有内容，不能凭空保存整段超宽长句，所以分别演示这两种方法。停止硬件滚屏后重新写入显存，再开始下一段显示。

代码采用简单主循环和 HAL_Delay，不使用 RTOS、菜单或额外传感器。OLED 未应答时，主程序每隔约 500 ms 重试初始化，不显示虚构数据。

## 修改与 CubeMX

显示内容主要在 `Core/Src/oled_demo.c`；I2C 回调和硬件滚屏在 `Core/Src/oled_port.c`。已有中文字体仅保留本实验需要的 21 个非 ASCII 字符。更换姓名或增加汉字时，也要补充字库，不能只修改字符串。

`Lab6_OLED.ioc` 为实际 CubeMX 配置，使用 STM32CubeMX 6.18.1、STM32Cube FW_F1 V1.8.0 生成。自写 main.c 代码放在 USER CODE 区域。

日常使用直接编译即可。若重新生成 CubeMX 工程，应先备份当前工程，确保生成位置仍为本目录，保留 USER CODE，关闭 Keil 后使用 PowerShell 7 执行 `Tools/prepare_project.ps1`，恢复裁剪、字库和 Keil 自定义源码分组，再打开 Keil 重新编译。新增汉字需另外取得包含这些字符的合法 BDF 字库；当前子集不含任意汉字。

## 验证记录

完整重新编译结果为 **0 errors、0 warnings**。Code=11648、RO-data=5204、RW-data=28、ZI-data=4044 字节；Flash 装载约 16.5 KiB，静态 RAM 含栈堆约 4 KiB。

姓名学号页已在实际硬件上显示，照片为 `../picture/11_OLED_Identity_Photo.jpg`。照片中“周新城”“632407030424”和板卡信息完整，说明 I2C 地址、SSD1306 初始化、字库和显存布局正常。长句滚动、上下滚动、图形 demo、XBM 动画和硬件滚屏未逐页拍照，以软件核对图呈现。

`Tools/verify_firmware.py` 使用 Unicorn 运行编译后的 ARM 显示函数，模拟 HAL I2C 和延时，检查地址探测、缺屏分支、字形、坐标裁剪、动画和滚屏命令。记录为 `Logs/software_verification.json`。这是软件验证，不是 Keil 外设仿真或实物测试；不验证电气连接、I2C 实际时序或控制器兼容性。

可选验证工具的依赖位于 `E:/Unicorn/python_deps`，由本机已有 Python 配合 Pillow 使用；固件编译和烧录不依赖 Python 或 Unicorn。复查命令：

```powershell
& 'C:/Users/Gallagher/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' 'E:/Embedded/Test/6/project/Tools/verify_firmware.py'
```

真实软件操作截图和软件预览位于相邻的 `../picture/`，详见工程文件索引。烧录写入校验已通过，姓名学号页已上板实拍；其余页面以软件核对为准，真实 I2C 时序尚未用仪器测量。若烧录后程序未自动运行，可按开发板 RESET 键启动，BOOT0 保持 0。
