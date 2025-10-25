#!/usr/bin/env python3
"""
Setup script for AI Audio Generator Python module
"""

from setuptools import setup, Extension
from pybind11.setup_helpers import Pybind11Extension, build_ext
from pybind11 import get_cmake_dir
import pybind11
import os
import sys

# Get the directory containing this file
here = os.path.abspath(os.path.dirname(__file__))

# Define the extension module
ext_modules = [
    Pybind11Extension(
        "aiaudio_python",
        [
            "src/python_bindings.cpp",
        ],
        include_dirs=[
            "include",
            pybind11.get_include(),
        ],
        libraries=["aiaudio_core"],
        library_dirs=["build/src"],
        language='c++',
        cxx_std=17,
    ),
]

# Read the README file
with open(os.path.join(here, "README.md"), "r", encoding="utf-8") as fh:
    long_description = fh.read()

setup(
    name="aiaudio-generator",
    version="1.0.0",
    author="AI Audio Generator Team",
    author_email="team@aiaudio.com",
    description="AI-powered audio generation system with Python-C++ integration",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/aiaudio/generator",
    packages=[""],
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: C++",
        "Topic :: Multimedia :: Sound/Audio",
        "Topic :: Scientific/Engineering :: Artificial Intelligence",
    ],
    python_requires=">=3.8",
    install_requires=[
        "numpy>=1.24.0",
        "pybind11>=2.10.0",
        "pyyaml>=6.0",
        "flask>=2.3.0",
        "flask-cors>=4.0.0",
        "pygame>=2.5.0",
        "sounddevice>=0.4.6",
        "soundfile>=0.12.1",
        "requests>=2.28.0",
    ],
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    include_package_data=True,
    package_data={
        "": ["*.json", "*.yaml", "*.md"],
    },
    entry_points={
        "console_scripts": [
            "aiaudio-generate=main:main",
            "aiaudio-server=web_server:main",
        ],
    },
)