# 参考与来源

## 作业参考

- OLED 基础与接口：https://blog.csdn.net/qq_45237293/article/details/111712565
- OLED 点阵与字符显示：https://blog.csdn.net/qq_43279579/article/details/111414037
- HAL/U8g2 移植：https://blog.csdn.net/black_sneak/article/details/126312657

实现参考 HAL 回调、I2C2 和 U8g2 裁剪方法，显示流程与适配代码保存在本工程。未直接使用未提供具体仓库地址的“江大科协”代码。作业中的 U82G 按实际开源库 U8g2 处理。

## 第三方源码

| 组件 | 来源与版本 | 许可证位置 |
| --- | --- | --- |
| STM32 HAL/CMSIS | 本机 STM32Cube_FW_F1_V1.8.0，CubeMX 6.18.1 生成 | Drivers/CMSIS/LICENSE.txt、ST_PACKAGE_LICENSE.md、各源码版权头 |
| U8g2、bdfconv | https://github.com/olikraus/u8g2 ，提交 d6c8499c5f2707cac8eccd09fd8f677d12b17977 | Middlewares/U8g2/LICENSE |
| 文泉驿点阵宋体 | WenQuanYi Bitmap Song 0.9.9.8，16 像素；https://github.com/olikraus/u8g2/tree/d6c8499c5f2707cac8eccd09fd8f677d12b17977/tools/font/bdf | Assets/lab6_subset.bdf 原始版权头；GPL v2 with font embedding exception |
| 6×12 字体 | U8g2 u8g2_font_6x12_tf | 源文件注明 Public domain terminal emulator font |

U8g2 修改：仅保留本工程使用的 C 模块；setup 和内存文件只保留一个 SSD1306 初始化与全屏缓冲函数；补充文件结尾换行。文泉驿字体修改：保留 95 个 ASCII 字符和 21 个实验需要的非 ASCII 字符，原 BDF 版权注释保留；生成的压缩字库为 2259 字节。

STM32F1 HAL 为 BSD-3-Clause，CMSIS 为 Apache-2.0，U8g2 为 BSD 风格许可。各第三方材料仍遵循其原许可证。可选软件验证使用 Unicorn、pyelftools 与 Pillow，它们不链接进 STM32 固件。
