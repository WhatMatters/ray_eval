#!/usr/bin/python
# -*- coding: utf-8 -*-

from setuptools import setup, Extension

setup(name='rayeval',
    install_requires=['joblib'],
    py_modules=['rayeval'],
    description='The Mighty Ray Eval Module',
    author='Aldanor',
    author_email='i.s.smirnov@gmail.com',
    ext_modules=[
        Extension('_rayeval',
            sources=['_rayevalmodule.c'],
            extra_compile_args=['-O3'],
            extra_link_args=['-O3'])
    ],
    keywords='poker porn eval ray sluts math shit monte-carlo crap')
