from distutils.core import setup, Extension
import numpy as np
import os
img_bins=os.listdir('bin/')
for i in range(len(img_bins)):
    img_bins[i]='bin/'+img_bins[i]
print (img_bins)
setup(name="BoardPy", version="1.0",
      include_dirs= [np.get_include(),'src/'],
      ext_modules=[Extension("BoardPy", ["src/board_module.c","src/gen.c","src/types.c"],extra_objects=img_bins)
      ])
