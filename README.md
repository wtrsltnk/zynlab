# zynlib
Rearrangement of the zynaddsubfx classes and seperated into libraries.

```
set root_dir=%CD%
set "root_dir=%root_dir:\=/%"

git clone https://github.com/wtrsltnk/zynlab.git --recursive 
git clone https://github.com/wtrsltnk/zynaddsubfx-deps.git

mkdir zynlab-build

cd zynlab-build
cmake -DCMAKE_PREFIX_PATH=%root_dir%/zynaddsubfx-deps/mingw-32bit -DCMAKE_MODULE_PATH=%root_dir%/zynaddsubfx-deps/mingw-32bit/cmake-modules -G "MinGW Makefiles" ../zynlab
cmake --build ./
cd ../

robocopy "zynaddsubfx-deps/mingw-32bit/bin" zynlab-build/app *.dll
robocopy "zynaddsubfx-deps/mingw-32bit/bin" zynlab-build/app.threedee *.dll
```