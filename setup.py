# Released under the BSD license, see LICENSE for details

import os
import sys

from distutils.core import setup, Extension
from shutil import copy
from subprocess import PIPE, Popen

WINDOWS = os.sys.platform == 'win32' 


APP_NAME = os.path.basename(os.getcwd())
SHARE_DIR = os.path.join('share', 'spicedoc')

SPICE_MODULE_C = 'spicemodule.c'
ROOT_DIR = os.path.dirname(__file__)
MODULE_PATH = os.path.join(ROOT_DIR, SPICE_MODULE_C)

CSPICE_SRC = os.environ.get('CSPICE_SRC', os.path.join(ROOT_DIR, 'cspice'))

if not os.path.exists(CSPICE_SRC):
    message = ('Unable to find CSPICE source at %s.  '
               'Please untar the source there or set the environment '
               'variable CSPICE_SRC') % (CSPICE_SRC,)
    sys.exit(message)

# check to see if the lib files are named lib*
remove_files = []

class LibError(Exception): pass

def build_cspice():
    libfile_paths = (os.path.join(CSPICE_SRC, 'lib', 'cspice.a'), 
                     os.path.join(CSPICE_SRC, 'lib', 'cspice.lib'),
                     )
    if any(os.path.exists(x) for x in libfile_paths):
        return

    curr_dir = os.getcwd()
    try:
        if WINDOWS:
            makeall = Popen('mkprodct_cspice.bat', shell=True)

        else:
            os.chdir(CSPICE_SRC)
            makeall = Popen('/bin/csh makeall.csh', shell=True)
        status = os.waitpid(makeall.pid, 0)[1]

        if status != 0:
            sys.stderr.write('warning: cspice build exit status: %d' % status)
    finally:
        os.chdir(curr_dir)

def find_libs():
    if WINDOWS: 
        libfile_list = ('cspice.lib', )
    else:
        libfile_list = ('cspice.a', )
    for libfile in libfile_list:
        libfile_path = os.path.join(CSPICE_SRC, 'lib', libfile)
        lib_libfile_path = os.path.join(CSPICE_SRC, 'lib', 'lib' + libfile)

        if not os.path.exists(lib_libfile_path):
            if not os.path.exists(libfile_path):
                sys.exit('unable to find %s' % (libfile_path,))

            try:
                copy(libfile_path, lib_libfile_path)
                remove_files.append(lib_libfile_path)
            except:
                raise LibError('Unable to copy %s to %s' % (libfile_path,
                                                            lib_libfile_path))

def make_spice_module():
    if not os.path.exists(MODULE_PATH):
        import mkwrapper
        print 'making wrapper'
        f = open(MODULE_PATH, 'wb')
        f.write(mkwrapper.main(CSPICE_SRC))
        f.close()

def cleanup():
    for path in remove_files:
        os.remove(path)

def set_build_paths():
    if not sys.argv[1].startswith('build'):
        return

    for flag, dirname in (('-I', 'include'), ('-L', 'lib')):
        path = os.path.join(CSPICE_SRC, dirname)
        sys.argv.append('%s%s' % (flag, path))

spice_custom = Extension(
    '_spice',
    sources = ['pyspice.c', 'spicemodule.c', 
               'spicemodule_custom.c'
               ],
    libraries = ['cspice'],
    include_dirs=[os.path.join(CSPICE_SRC, 'include'),],
    library_dirs=[os.path.join(CSPICE_SRC, 'lib'),],
    define_macros=[('MSDOS', None), 
                   ('_COMPLEX_DEFINED', None),
                   ('OMIT_BLANK_CC', None)],
)

try:
    build_cspice()
    make_spice_module()
    find_libs()
    set_build_paths()

    module1 = Extension(
        '_spice',
        sources = ['pyspice.c', 'spicemodule.c'],
        libraries = ['cspice'],
    )

    setup(
        name = 'Spice',
        version = '1.0',
        description = 'Spice Wrapper Module',
        packages = ['spice'],
        ext_modules = [spice_custom]
    )
finally:
    cleanup()
