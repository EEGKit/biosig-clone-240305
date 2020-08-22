# TODO:
#   windows, how to add libbiosig.dll.a and alike
#   https://github.com/lebigot/uncertainties/blob/master/setup.py

try:
    from setuptools import setup, Extension
except ImportError:
    from distutils.core import setup
    from distutils.extension import Extension

import os
import numpy.distutils.misc_util

module_biosig = Extension('biosig',
        define_macros = [('MAJOR_VERSION', '1'), ('MINOR_VERSION', '9')],
        include_dirs = ['./..', numpy.distutils.misc_util.get_numpy_include_dirs()[0]],
        libraries    = ['biosig'],
        library_dirs = ['./..','../lib'],
        sources      = ['biosigmodule.c'])

def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()

setup (name = 'Biosig',
        version = '2.0.5',
        description = 'BioSig - tools for biomedical signal processing',
        author = 'Alois Schloegl',
        author_email = 'alois.schloegl@gmail.com',
        license = 'GPLv3+',
        url = 'https://biosig.sourceforge.io',
        #long_description='Import filters of biomedical signal formats',
        long_description=read('README.md'),
        long_description_content_type="text/markdown",
        include_package_data = True,
        keywords = 'EEG ECG EKG EMG EOG Polysomnography ECoG biomedical signals SCP EDF GDF HEKA CFS ABF',
        install_requires=['numpy','setuptools>=6.0'],
        classifiers=[
          'Programming Language :: Python',
          'License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)',
          'Operating System :: OS Independent'
        ],
        ext_modules = [module_biosig])
