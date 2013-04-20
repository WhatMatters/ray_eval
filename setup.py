#!/usr/bin/python
# -*- coding: utf-8 -*-

from setuptools import setup, Extension

setup(name='rayeval',
    install_requires=['joblib'],
    py_modules=['rayeval'],
    description='Ray Eval',
    author='Aldanor',
    author_email='i.s.smirnov@gmail.com',
    ext_modules=[
        Extension('_rayeval',
            sources=['_rayevalmodule.cpp', 'arrays.cpp', 'rayutils.cpp',
                'raygen7.cpp', 'raygen9.cpp'],
            extra_compile_args=['-O3', '-msse4', '-fPIC', '-g',
                '-Wshorten-64-to-32', '-gdwarf-2', '-Qunused-arguments'],
            extra_link_args=['-O3', '-msse4', '-fPIC', '-g',
                '-Wshorten-64-to-32', '-gdwarf-2', '-Qunused-arguments'])
    ],
    keywords='poker monte-carlo eval omaha holdem'
)
