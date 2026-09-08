#!/usr/bin/env python3
"""
纹理转换工具 - 将 PNG/BMP 图片转换为 C 数组（RGB565 格式）
用法：python texture_converter.py input.png -o output.hpp -n kTextureBrick
"""

import argparse
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("错误：需要安装 Pillow 库")
    print("运行：pip install Pillow")
    sys.exit(1)


def rgb_to_rgb565(r: int, g: int, b: int) -> int:
    """将 RGB888 转换为 RGB565"""
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)


def convert_image(input_path: str, output_path: str, array_name: str, size: int = 64):
    """转换图片为 C 数组"""
    img = Image.open(input_path)

    # 缩放到指定大小
    img = img.resize((size, size), Image.Resampling.LANCZOS)

    # 转换为 RGB
    img = img.convert("RGB")

    # 转换像素数据
    pixels = []
    for y in range(size):
        for x in range(size):
            r, g, b = img.getpixel((x, y))
            rgb565 = rgb_to_rgb565(r, g, b)
            pixels.append(rgb565)

    # 生成 C 头文件
    with open(output_path, "w") as f:
        f.write(f"// 自动生成的纹理数据 - {input_path}\n")
        f.write(f"// 尺寸：{size}x{size}，格式：RGB565\n")
        f.write("#pragma once\n\n")
        f.write("#include <cstdint>\n\n")
        f.write(f"constexpr uint16_t {array_name}[] = {{\n")

        # 每行 16 个像素
        for i in range(0, len(pixels), 16):
            chunk = pixels[i:i+16]
            hex_values = ", ".join(f"0x{p:04X}" for p in chunk)
            f.write(f"    {hex_values},\n")

        f.write("};\n")
        f.write(f"\nconstexpr int {array_name.replace('kTexture', 'kTextureSize')} = {size};\n")

    print(f"成功：{input_path} -> {output_path}")
    print(f"像素数：{len(pixels)}，大小：{len(pixels) * 2} 字节")


def main():
    parser = argparse.ArgumentParser(description="纹理转换工具")
    parser.add_argument("input", help="输入图片路径")
    parser.add_argument("-o", "--output", help="输出头文件路径", default="texture.hpp")
    parser.add_argument("-n", "--name", help="C 数组名称", default="kTexture")
    parser.add_argument("-s", "--size", help="纹理大小（默认 64）", type=int, default=64)

    args = parser.parse_args()

    if not Path(args.input).exists():
        print(f"错误：输入文件不存在：{args.input}")
        sys.exit(1)

    convert_image(args.input, args.output, args.name, args.size)


if __name__ == "__main__":
    main()
