#!/usr/bin/python
# -*- coding: utf-8 -*-

from setuptools import Extension, find_packages, setup
from distutils.command.install_data import install_data
import sys
import os
from os.path import join as file_join
from setuptools.command.install import install as _install
import imp
from pkg_resources import resource_filename
import platform


if platform.system() == 'Darwin':
    extra_compile_args = ['-O3', '-msse4', '-fPIC', '-g', '-gdwarf-2', '-stdlib=libstdc++']
    extra_link_args = ['-O3', '-msse4', '-fPIC', '-g', '-gdwarf-2', '-stdlib=libstdc++']
else:
    extra_compile_args = ['-O3', '-msse4', '-fPIC', '-g', '-gdwarf-2']
    extra_link_args = ['-O3', '-msse4', '-fPIC', '-g', '-gdwarf-2']


INSTALL_LIB_PATH = ''
INSTALL_DATA_FILES_PATH = ''
ARGV_OPTIONS = {
    'no_ranks_test': False,
    'without_build_ranks': False,
    'ranks_dir': None,
    'hand_ranks_7_file_name': 'rayeval_hand_ranks_7.dat',
    'hand_ranks_9_file_name': 'rayeval_hand_ranks_9.dat',
    'use_ranks_7': None,
    'use_ranks_9': None
}
GENERATE_HAND_RANKS = False


filtered_args = []
for arg in sys.argv:
    if arg == '--no-ranks-test':
        ARGV_OPTIONS['no_ranks_test'] = True
    elif arg == '--without-build-ranks':
        ARGV_OPTIONS['without_build_ranks'] = True
    elif arg.startswith('--ranks-dir'):
        ARGV_OPTIONS['ranks_dir'] = arg.split('=')[1]
    elif arg.startswith('--ranks-7-filename'):
        ARGV_OPTIONS['hand_ranks_7_file_name'] = arg.split('=')[1]
    elif arg.startswith('--ranks-9-filename'):
        ARGV_OPTIONS['hand_ranks_9_file_name'] = arg.split('=')[1]
    elif arg.startswith('--use-ranks-7'):
        ARGV_OPTIONS['use_ranks_7'] = arg.split('=')[1]
    elif arg.startswith('--use-ranks-9'):
        ARGV_OPTIONS['use_ranks_9'] = arg.split('=')[1]
    else:
        filtered_args.append(arg)

sys.argv = filtered_args


def build_and_install_ranks():
    print "Running: build_and_install_ranks"
    # Loading rayeval from the path it was installed to
    f, filename, description = imp.find_module('rayeval', [INSTALL_LIB_PATH])
    if f is None:
        f, filename, description = imp.find_module('rayeval', [file_join(INSTALL_LIB_PATH, 'rayeval')])
    rayeval_module = imp.load_module('rayeval', f, filename, description)

    global INSTALL_DATA_FILES_PATH
    global ARGV_OPTIONS
    if ARGV_OPTIONS['use_ranks_7'] is None:
        ranks_7_path = file_join(INSTALL_DATA_FILES_PATH, ARGV_OPTIONS['hand_ranks_7_file_name'])
    else:
        if not os.path.exists(ARGV_OPTIONS['use_ranks_7']):
            raise IOError, "Hand ranks 7 file not found: %s" % ARGV_OPTIONS['use_ranks_7']
        ranks_7_path = os.path.abspath(ARGV_OPTIONS['use_ranks_7'])

    if ARGV_OPTIONS['use_ranks_9'] is None:
        ranks_9_path = file_join(INSTALL_DATA_FILES_PATH, ARGV_OPTIONS['hand_ranks_9_file_name'])
    else:
        if not os.path.exists(ARGV_OPTIONS['use_ranks_9']):
            raise IOError, "Hand ranks 9 file not found: %s" % ARGV_OPTIONS['use_ranks_9']
        ranks_9_path = os.path.abspath(ARGV_OPTIONS['use_ranks_9'])

    if GENERATE_HAND_RANKS and ARGV_OPTIONS['use_ranks_7'] is None:
        rayeval_module.generate_handranks_7(ranks_7_path, not ARGV_OPTIONS['no_ranks_test'])

    if GENERATE_HAND_RANKS and ARGV_OPTIONS['use_ranks_9'] is None:
        rayeval_module.generate_handranks_9(ranks_9_path, ranks_7_path, not ARGV_OPTIONS['no_ranks_test'])

    if GENERATE_HAND_RANKS or ARGV_OPTIONS['use_ranks_7'] is not None or ARGV_OPTIONS['use_ranks_9'] is not None:
        f = open(resource_filename('rayeval', "__rayeval_ranks_path__.txt"), 'w')
        f.write(ranks_7_path + "\n")
        f.write(ranks_9_path)
        f.close()


class install_hand_ranks(install_data):
    def run(self):
        filtered_data_files = []
        for item in self.data_files:
            if len(item) == 2 and len(item[1]) > 0 and item[1][0] == ':hand_ranks':
                global INSTALL_DATA_FILES_PATH
                global GENERATE_HAND_RANKS
                global ARGV_OPTIONS
                GENERATE_HAND_RANKS = not ARGV_OPTIONS['without_build_ranks']

                if ARGV_OPTIONS['ranks_dir'] is not None:
                    INSTALL_DATA_FILES_PATH = ARGV_OPTIONS['ranks_dir']
                else:
                    INSTALL_DATA_FILES_PATH = file_join(self.install_dir, item[0])

                if not os.path.exists(INSTALL_DATA_FILES_PATH):
                    os.mkdir(INSTALL_DATA_FILES_PATH)
            else:
                filtered_data_files.append(item)

        self.data_files = filtered_data_files
        install_data.run(self)


class install(_install):
    def run(self):
        global INSTALL_LIB_PATH
        INSTALL_LIB_PATH = self.install_lib
        _install.run(self)


setup(name='rayeval',
      version='0.1',
      cmdclass={'install': install, 'install_data': install_hand_ranks},
      description='Ray Eval',
      author='Aldanor',
      author_email='i.s.smirnov@gmail.com',
      url='https://github.com/WhatMatters/ray_eval',
      install_requires=['joblib'],
      packages=find_packages(),
      package_data = {'rayeval': ['rayeval/__rayeval_ranks_path__.txt']},
      data_files=[('shared', [':hand_ranks'])],
      ext_modules=[
          Extension('_rayeval',
                    sources=[
                        'src/_rayevalmodule.cpp',
                        'src/arrays.cpp',
                        'src/rayutils.cpp',
                        'src/raygen7.cpp',
                        'src/raygen9.cpp'],
                    extra_compile_args=extra_compile_args,
                    extra_link_args=extra_link_args)
      ],
      keywords='poker monte-carlo eval omaha holdem',
      license='LICENSE.txt'
)


if GENERATE_HAND_RANKS or ARGV_OPTIONS['use_ranks_7'] is not None or ARGV_OPTIONS['use_ranks_9'] is not None:
    build_and_install_ranks()
