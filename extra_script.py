import struct
import subprocess
import os

from SCons.Script import BUILD_TARGETS
from platformio.public import list_serial_ports

Import("env")

env.Replace(PROGNAME=f'firmware_{env["PIOENV"]}')

if not env.subst("$PIOENV").startswith("seeed_xiao_samd21"):
    pass
elif not (bool(BUILD_TARGETS) and "upload" in BUILD_TARGETS):
    pass
else:
    build_dir = env.subst("$BUILD_DIR")
    eeprom_path = os.path.join(build_dir, "eeprom_backup.bin")
    flash_dump_path = os.path.join(build_dir, "flash_dump.bin")
    EEPROM_LOCATOR_MAGIC = b'\x4d\x41\x58\x44'  # "MAXD"

    def _find_locator(data, label=""):
        DXAM_MAGIC = b'\x44\x58\x41\x4d'  # "DXAM"
        MIN_FLASH = 0x2000
        MAX_FLASH = 0x40000
        matches = []
        start = 0
        while True:
            idx = data.find(EEPROM_LOCATOR_MAGIC, start)
            if idx < 0:
                break
            if idx + 16 > len(data):
                break
            if data[idx + 12:idx + 16] != DXAM_MAGIC:
                start = idx + 1
                continue
            addr = struct.unpack_from('<I', data, idx + 4)[0]
            size = struct.unpack_from('<I', data, idx + 8)[0]
            if not (MIN_FLASH <= addr < MAX_FLASH) or not (0 < size <= MAX_FLASH - addr):
                start = idx + 1
                continue
            matches.append((idx, addr, size))
            start = idx + 1
        if not matches:
            return None
        if len(matches) > 1:
            off = ", ".join(f"0x{o:x}(addr=0x{a:x})" for o, a, _ in matches)
            raise RuntimeError(f"{label}{len(matches)} EEPROM locators found at offsets {off}")
        return (matches[0][1], matches[0][2])

    def _save_and_embed(target, source, env):
        r = subprocess.run(
            ["arm-none-eabi-nm", os.path.join(
                env.subst("$BUILD_DIR"),
                env.subst("$PROGNAME") + ".elf"
            ), "--defined-only"],
            capture_output=True, text=True, timeout=15
        )
        if "EEPROM_init" not in r.stdout:
            return

        env.AutodetectUploadPort()
        upload_port = env.subst("$UPLOAD_PORT")
        if not upload_port:
            return

        before = list_serial_ports()
        env.TouchSerialPort(upload_port, 1200)
        bl_port = env.WaitForNewSerialPort(before)
        if not bl_port:
            return

        env.Replace(UPLOAD_PORT=bl_port)
        r = subprocess.run(
            ["bossac", "-p", bl_port, "-r", flash_dump_path],
            capture_output=True, text=True, timeout=60
        )
        if r.returncode != 0:
            return

        with open(flash_dump_path, "rb") as f:
            flash_dump = f.read()
        os.remove(flash_dump_path)

        loc = _find_locator(flash_dump, "flash dump: ")
        if loc is None:
            return

        old_addr, size = loc
        offset = old_addr - 0x2000
        eeprom = flash_dump[offset:offset + size]
        if len(eeprom) != size:
            return

        print(f"[EEPROM] Located emulated EEPROM at flash 0x{old_addr:05X}"
              f" ({size} bytes) in running firmware — saving")

        with open(eeprom_path, "wb") as f:
            f.write(eeprom)

        with open(eeprom_path, "rb") as f:
            eeprom = f.read()
        os.remove(eeprom_path)

        fw_bin = os.path.join(
            env.subst("$BUILD_DIR"),
            env.subst("$PROGNAME") + ".bin"
        )
        with open(fw_bin, "rb") as f:
            fw = f.read()

        loc = _find_locator(fw, "new firmware .bin: ")
        if loc is None:
            return

        new_addr, new_size = loc
        if new_size != len(eeprom):
            return

        print(f"[EEPROM] New firmware EEPROM at flash 0x{new_addr:05X}"
              f" — embedding preserved data")
        new_offset = new_addr - 0x2000
        end = new_offset + new_size
        if len(fw) >= end:
            combined = fw[:new_offset] + eeprom + fw[end:]
        else:
            combined = fw + b'\xFF' * (new_offset - len(fw)) + eeprom

        with open(fw_bin, "wb") as f:
            f.write(combined)

    env.AddPreAction("upload", _save_and_embed)
