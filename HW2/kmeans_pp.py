import numpy as np
import mykmeanssp as k  # still works with the warning - as long as we launched build_ext and the .so file is present

print(f"fit() returned {k.fit(5,6)}")   # currently adds the two integers in C

# Try running this regularly. 
# If "mykmeanssp module doesn't exist" - run "python3 setup.py build_ext --inplace" in the folder and try again.