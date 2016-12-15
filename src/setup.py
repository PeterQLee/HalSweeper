from distutils.core import setup, Extension
import numpy as np

setup(name="BoardPy", version="1.0",
      include_dirs= [np.get_include(),'.'],
      ext_modules=[Extension("BoardPy", ["board_module.c","gen.c","types.c"])
      ])
