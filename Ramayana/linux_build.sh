#!/bin/sh

INCLUDE="-Ilib/glew-1.9.0/include -Ilib/freeglut/include -Ilib/opencv-2.4.9/include -Ilib/opencv-2.4.9/include/opencv -Ilib/SDL-1.2.15/include -Ilib/SDL-1.2.15/include/SDL -Ilib/SDL_mixer-1.2.12/include -Ilib/SDL_mixer-1.2.12/include/SDL -Ilib/rapidxml-1.13/include -Ilib/steamwork-sdk/public -Ilib/openvr-master/headers"

LIBDIR="-Llib/glew-1.9.0/lib/x64 -Llib/freeglut/lib/x64 -Llib/opencv-2.4.9/x64/linux -Llib/SDL-1.2.15/lib/x64 -Llib/SDL_mixer-1.2.12/lib/x64 -Llib/steamwork-sdk/redistributable_bin/linux64 -Llib/steamwork-sdk/steam/lib/linux64"

LIB="-lGL -lGLEW -lGLU -lglut -lSDL -lSDL_mixer -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_video -lsteam_api"

FILES="*.cpp gltools/*.cpp"

echo "Building Ramayana ..."
g++ $FILES $INCLUDE $LIBDIR $LIB -o "linux64/Ramayana" -Wl,-rpath,./linux64

echo "Building Ramayana.Demo ..."
g++ -D_DEMO $FILES $INCLUDE $LIBDIR $LIB -o "linux64/Ramayana.Demo" -Wl,-rpath,./linux64
