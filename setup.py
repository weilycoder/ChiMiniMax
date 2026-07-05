import os

from setuptools import setup, Extension

module = Extension(
    "chiminimax",
    sources=["chiminimax.cpp"],
    extra_compile_args=(
        ["/std:c++latest"] if os.name == "nt" else ["-std=c++23", "-Wno-mismatched-new-delete"]
    ),
)

setup(
    name="chiminimax",
    description="",
    ext_modules=[module],
)
