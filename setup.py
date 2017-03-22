#! /usr/bin/env python
from __future__ import print_function

from distutils.core import setup
from distutils.extension import Extension

# Complain on 32-bit systems. See README for more details
import struct
if struct.calcsize('P') < 8:
    raise RuntimeError(
        'Simhash-py does not work on 32-bit systems. See README.md')

ext_files = [
    'simhash/simhash-cpp/src/permutation.cpp',
    'simhash/simhash-cpp/src/simhash.cpp'
]

kwargs = {}

try:
    from Cython.Distutils import build_ext
    print('Building from Cython')
    ext_files.append('simhash/simhash.pyx')
    kwargs['cmdclass'] = {'build_ext': build_ext}
except ImportError:
    print('Buidling from C++')
    ext_files.append('simhash/simhash.cpp')

ext_modules = [
    Extension(
        'simhash.simhash', ext_files,
        language='c++',
        extra_compile_args=['-std=c++11'],
        include_dirs=['simhash/simhash-cpp/include']
    )
]

setup(
    name='simhash-py',
    version='0.4.0',
    description='Near-Duplicate Detection with Simhash',
    url='http://github.com/seomoz/simhash-py',
    author='Dan Lecocq',
    author_email='dan@moz.com',
    classifiers=[
        'Programming Language :: Python',
        'Intended Audience :: Developers',
        'Operating System :: OS Independent',
        'Topic :: Internet :: WWW/HTTP'
    ],
    ext_modules=ext_modules,
    packages=[
        'simhash'
    ],
    package_dir={
        'simhash': 'simhash'
    },
    tests_require=[
        'coverage',
        'nose',
        'nose-timer',
        'rednose'
    ],
    **kwargs
)
