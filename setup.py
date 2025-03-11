from setuptools import setup, Extension
import pybind11
from setuptools.command.build_ext import build_ext
import os
import subprocess

class CMakeBuild(build_ext):
    def build_extension(self, ext):
        build_temp = self.build_temp
        os.makedirs(build_temp, exist_ok=True)
        subprocess.check_call(["cmake", ".."], cwd=build_temp)
        subprocess.check_call(["make"], cwd=build_temp)
        subprocess.check_call(["make", "install"], cwd=build_temp)

setup(
    name="pymahjong",
    version="0.1",
    author="Mitsuhiro Taniguchi",
    description="fast mahjong tools",
    ext_modules=[
        Extension(
            "pymahjong",
            [
                "src/bindings.cpp",
                "src/calsht_dw.cpp",
             ],
            include_dirs=[pybind11.get_include()],  # pybind11のヘッダーパスを取得
            language="c++",
            extra_compile_args=["-std=c++20"]
        ),
    ],
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
)
