import subprocess
import os

os.mkdir("build")
subprocess.run(["ca65.exe", "main.s", "-o", "build/main.o"])
subprocess.run(["ca65.exe", "header.s", "-o", "build/header.o"])
subprocess.run(["ld65", "-C", "link.x", "main.o", "header.o", "-o", "build/rom.nes"])
