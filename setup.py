import os
import sys
import subprocess
import setuptools
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


class CMakeBuild(build_ext):
    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
            "-DPYTHON_EXECUTABLE=" + sys.executable,
            ]

        build_temp = os.path.join(self.build_temp, ext.name)
        os.makedirs(build_temp, exist_ok=True)

        subprocess.check_call(["cmake", ".."] + cmake_args, cwd=build_temp)
        subprocess.check_call(["cmake", "--build", "."], cwd=build_temp)


ext_modules = [
    Extension("pymahjong", sources=[])  # CMake に全て任せるので空
]

setup(
    name="pymahjong",
    version="0.1.0",
    packages=setuptools.find_packages(),
    ext_modules=ext_modules,
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
)
