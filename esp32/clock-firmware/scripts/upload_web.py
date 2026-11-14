"""
Automates building and flashing the web LittleFS partition during
PlatformIO uploads.
"""

Import("env")

import os
import subprocess
import sys
from pathlib import Path

PROJECT_DIR = Path(env.subst("$PROJECT_DIR"))
BUILD_DIR = Path(env.subst("$BUILD_DIR"))

WEB_IMAGE = BUILD_DIR / "web.bin"

IDF_PATH = env.PioPlatform().get_package_dir("framework-espidf")
ESPTOOL_DIR = env.PioPlatform().get_package_dir("tool-esptoolpy")
ESPTOOL = Path(ESPTOOL_DIR) / "esptool.py"

def build_web_image(source, target, env):
    print("\n=== Building LittleFS web image ===")

    build_env = os.environ.copy()
    build_env["IDF_PATH"] = IDF_PATH

    subprocess.run(
        [
            "cmake",
            "--build",
            str(BUILD_DIR),
            "--target",
            "littlefs_web_bin",
        ],
        cwd=PROJECT_DIR,
        env=build_env,
        check=True,
    )

    if not WEB_IMAGE.is_file():
        raise RuntimeError(f"Web image was not generated: {WEB_IMAGE}")

    print(f"Web image ready: {WEB_IMAGE}")


def flash_web_image(source, target, env):
    print("\n=== Flashing LittleFS web image ===")

    port = env.subst("$UPLOAD_PORT")

    if not port or "$UPLOAD_PORT" in port:
        raise RuntimeError(
            "Upload port not available. Set upload_port in platformio.ini."
        )

    baud = str(env.get("UPLOAD_SPEED", 460800))

    command = [
        sys.executable,
        str(ESPTOOL),
        "--chip", "esp32c6",
        "--port", port,
        "--baud", baud,
        "write_flash",
        "0x250000",
        str(WEB_IMAGE),
    ]

    subprocess.run(command, cwd=PROJECT_DIR, check=True)


# Generate the filesystem before the normal firmware upload.
env.AddPreAction("upload", build_web_image)

# Flash the filesystem after the normal firmware upload.
env.AddPostAction("upload", flash_web_image)

