from pathlib import Path
from enum import Enum
import subprocess
import shutil
import os
import sys


class ShaderType(Enum):
    VERTEX = "vert"
    PIXEL = "pixel"
    COMPUTE = "comp"


# hardcode for now
OUT_FOLDER = "/Users/brinq/.dev/projects/solar-sim/build/bin/data/"
OUT_EXTENSION = ".shader"

COMPILER = os.getenv("SSF_VULKAN_BIN")

if COMPILER == None:
    print("Non valid vulkan binary path. NOTE: set SSF_VULKAN_BIN in env")
    sys.exit(1)

COMPILER = COMPILER.__str__().__add__("/glslc")


if shutil.which(COMPILER.__str__()) == None:
    print("Error: unable to find shader compiler.")
    sys.exit(1)

shader_folder = Path(Path(__file__).parent.parent.__str__().__add__("/data/shaders"))

def compile_shader(path: str, name: str, type_arg: str):
    out = "-o" + OUT_FOLDER + name + OUT_EXTENSION

    subprocess.run( [COMPILER.__str__(), type_arg, path, out])
    print("shader compiled finished - " + name)
    return True

for file in shader_folder.iterdir():
    shader = file.name
    arr = shader.__str__().split(".")

    match arr[1]:
        case "vert":
            shader_arg = "-fshader-stage=vert"
        case "pixel":
            shader_arg = "-fshader-stage=frag"
        case _:
            shader_arg = "h"
            print("inalid shader found at path", file)
            continue

    compile_shader(file.__str__(), arr[0].__add__( "." + arr[1]), shader_arg)
