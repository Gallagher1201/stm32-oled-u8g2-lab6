"""Run the Keil ARM binary's display functions; I2C/delays are mocked, not board-tested."""
from pathlib import Path
import json
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, "E:/Unicorn/python_deps")
from elftools.elf.elffile import ELFFile
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_THUMB, UC_MODE_MCLASS, UC_HOOK_CODE
from unicorn.arm_const import UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2
from unicorn.arm_const import UC_ARM_REG_R3, UC_ARM_REG_SP, UC_ARM_REG_LR
from unicorn.arm_const import UC_ARM_REG_PC, UC_ARM_REG_XPSR
from PIL import Image, ImageDraw, ImageFont

PIC = ROOT.parent / "picture"
uc = Uc(UC_ARCH_ARM, UC_MODE_THUMB | UC_MODE_MCLASS)
uc.mem_map(0x08000000, 0x20000)
uc.mem_map(0x20000000, 0x10000)
uc.mem_map(0xE0000000, 0x10000)
with (ROOT / "MDK-ARM/Objects/Lab6_OLED.axf").open("rb") as stream:
    elf = ELFFile(stream)
    symbols = {s.name: s["st_value"] for s in elf.get_section_by_name(".symtab").iter_symbols() if s.name}
    for section in elf.iter_sections():
        if section["sh_flags"] & 2 and section["sh_type"] != "SHT_NOBITS" and section["sh_size"]:
            uc.mem_write(section["sh_addr"], section.data())

STOP = 0x0801FFF0
display = symbols["display"]
transfers = []
present = {0x78}


def hook(cpu, address, size, _):
    result = None
    if address == symbols["HAL_I2C_Master_Transmit"] & ~1:
        addr = cpu.reg_read(UC_ARM_REG_R1)
        pointer = cpu.reg_read(UC_ARM_REG_R2)
        length = cpu.reg_read(UC_ARM_REG_R3)
        assert 0 < length <= 128
        transfers.append((addr, bytes(cpu.mem_read(pointer, length))))
        result = 0
    elif address == symbols["HAL_I2C_IsDeviceReady"] & ~1:
        result = 0 if cpu.reg_read(UC_ARM_REG_R1) in present else 1
    elif address in {symbols["HAL_Delay"] & ~1, symbols["DelayCycles"] & ~1}:
        result = 0
    if result is not None:
        cpu.reg_write(UC_ARM_REG_R0, result)
        cpu.reg_write(UC_ARM_REG_PC, cpu.reg_read(UC_ARM_REG_LR))


uc.hook_add(UC_HOOK_CODE, hook)


def call(name, *args):
    for reg, value in zip((UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3), args):
        uc.reg_write(reg, value & 0xFFFFFFFF)
    uc.reg_write(UC_ARM_REG_SP, 0x20004F00)
    uc.reg_write(UC_ARM_REG_LR, STOP | 1)
    uc.reg_write(UC_ARM_REG_XPSR, 0x01000000)
    uc.emu_start(symbols[name] | 1, STOP, count=5000000)
    assert uc.reg_read(UC_ARM_REG_PC) == STOP, f"Instruction limit: {name}"
    return uc.reg_read(UC_ARM_REG_R0)


def screen():
    data = uc.mem_read(buffer, 1024)
    image = Image.new("RGB", (128, 64), "black")
    for y in range(64):
        for x in range(128):
            if data[(y // 8) * 128 + x] & (1 << (y % 8)):
                image.putpixel((x, y), (255, 255, 255))
    return image


def draw(name, *args):
    call(name, display, *args)
    return screen()


assert call("OLED_Init", display) == 1
buffer = call("u8g2_m_16_8_f", 0x20004000)
assert len(transfers) > 0
present = {0x7A}
assert call("OLED_Init", display) == 1
assert transfers[-1][0] == 0x7A
present = set()
assert call("OLED_Init", display) == 0
present = {0x78}
assert call("OLED_Init", display) == 1

frames = [("Identity", draw("Demo_Identity")), ("U8g2 Demo", draw("Demo_Graphics", 80))]
call("u8g2_SetFont", display, symbols["lab6_font"])
codes = [int(x) for x in (ROOT / "Assets/font_map.txt").read_text().split(",")[1:]]
for code in codes:
    assert call("u8g2_GetGlyphWidth", display, code) > 0, hex(code)
width = call("u8g2_GetUTF8Width", display, symbols["long_text"])
assert width > 128
left = draw("Demo_Horizontal", 0)
right = draw("Demo_Horizontal", 128 - width)
assert left.tobytes() != right.tobytes()
assert left.crop((0, 0, 128, 17)).tobytes() == right.crop((0, 0, 128, 17)).tobytes()
frames.extend([("Long text: start", left), ("Long text: end", right)])
up = draw("Demo_Vertical", 33)
down = draw("Demo_Vertical", -27)
assert up.tobytes() != down.tobytes()
assert up.crop((0, 0, 128, 17)).tobytes() == down.crop((0, 0, 128, 17)).tobytes()
frames.extend([("Vertical scroll", up), ("XBM avatar", draw("Demo_Avatar", 36)),
               ("Hardware scroll RAM", draw("Demo_HardwarePage"))])

for index, (title, img) in enumerate(frames):
    assert sum(pixel != (0, 0, 0) for pixel in img.get_flattened_data()) > 100
    img.resize((768, 384), Image.Resampling.NEAREST).save(PIC / f"Preview_{index + 1:02d}.png")

transfers.clear()
call("OLED_ScrollStart", 1)
call("OLED_ScrollStop")
call("OLED_ScrollStart", 0)
assert transfers[0][1] == bytes([0, 0x2E, 0x27, 0, 4, 0, 7, 0, 0xFF, 0x2F])
assert transfers[1][1] == bytes([0, 0x2E])
assert transfers[2][1][2] == 0x26

animation = [draw("Demo_Avatar", frame).resize((512, 256), Image.Resampling.NEAREST) for frame in range(0, 144, 3)]
assert len({img.tobytes() for img in animation}) > 20
animation[0].save(PIC / "Preview_Animation.gif", save_all=True, append_images=animation[1:], duration=150, loop=0)

font = ImageFont.truetype("C:/Windows/Fonts/consola.ttf", 20)
sheet = Image.new("RGB", (1120, 1430), "#202020")
pen = ImageDraw.Draw(sheet)
pen.text((24, 16), "STM32F103C8T6 / Keil ARM binary / U8g2", fill="white", font=font)
pen.text((24, 44), "SOFTWARE PREVIEW - I2C mocked; no physical board verification", fill="#FFCF66", font=font)
for index, (title, img) in enumerate(frames):
    x, y = 24 + (index % 2) * 550, 92 + (index // 2) * 328
    pen.text((x, y), title, fill="white", font=font)
    sheet.paste(img.resize((512, 256), Image.Resampling.NEAREST), (x, y + 30))
sheet.save(PIC / "07_ARM_SoftwarePreview.png")
result = {
    "result": "PASS", "scope": "Keil ARM binary; HAL I2C and delays mocked; not a physical OLED/I2C test",
    "glyphs_checked": len(codes), "long_text_width_px": width,
    "checks": ["0x3C address", "0x3D address fallback", "missing OLED returns failure", "all Chinese glyphs exist",
               "horizontal negative coordinate rendering", "vertical clipping preserves title", "avatar animation changes",
               "SSD1306 hardware scroll command bytes"],
}
(ROOT / "Logs/software_verification.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
print(json.dumps(result, indent=2))
