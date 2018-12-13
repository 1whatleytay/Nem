@echo off

echo Deleting existing library folders.
rmdir glfw /q /s
rmdir gl3w /q /s

echo Downloading GLFW for windows.
git clone https://github.com/glfw/glfw.git

echo Downloading GL3W for OpenGL on Windows.
git clone https://github.com/skaslev/gl3w.git

echo Building GL3W from specification. Requires python.
cd gl3w
py gl3w_gen.py

echo If you don't see any errors above, you should be good to go!
pause