#!/bin/bash

echo Deleting existing library folders...
rm -rf glfw

echo Downloading GLFW for windows.
git clone https://github.com/glfw/glfw.git

echo If you don't see any errors above, you should be good to go!
read -p "Press any key to continue..."