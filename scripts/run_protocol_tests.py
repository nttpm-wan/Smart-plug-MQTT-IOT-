"""Compile and run the actual MQTT protocol header on the development computer."""

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default=os.environ.get("CXX", "c++"))
    parser.add_argument("--compiler-arg", action="append", default=[])
    args = parser.parse_args()

    compiler = shutil.which(args.compiler)
    if compiler is None:
        print("C++ compiler not found. Install GCC/Clang or use --compiler PATH.", file=sys.stderr)
        return 2

    project = Path(__file__).resolve().parents[1]
    output_dir = project / "build" / "tests"
    output_dir.mkdir(parents=True, exist_ok=True)
    executable = output_dir / ("mqtt_protocol_tests.exe" if os.name == "nt" else "mqtt_protocol_tests")
    command = [compiler, *args.compiler_arg, "-std=c++11", "-Wall", "-Wextra",
               "-Werror", "-pedantic", "-I", str(project / "include"),
               str(project / "tests" / "mqtt_protocol_tests.cpp"), "-o", str(executable)]
    print("Building MQTT protocol tests...", flush=True)
    try:
        subprocess.run(command, cwd=project, check=True)
        subprocess.run([str(executable)], cwd=project, check=True)
    except subprocess.CalledProcessError as error:
        return error.returncode or 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
