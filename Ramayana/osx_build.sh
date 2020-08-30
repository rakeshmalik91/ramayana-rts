#!/bin/sh

INCLUDE="-I/usr/include/malloc -Ilib/opencv-2.4.9/include -Ilib/opencv-2.4.9/include/opencv -Ilib/SDL-1.2.15/include -Ilib/SDL-1.2.15/include/SDL -Ilib/SDL_mixer-1.2.12/include -Ilib/SDL_mixer-1.2.12/include/SDL -Ilib/rapidxml-1.13/include -Ilib/steamwork-sdk/public -Ilib/openvr-master/headers"

LIBDIR="-Llib/opencv-2.4.9/x86/linux -Llib/SDL-1.2.15/lib/x86 -Llib/SDL_mixer-1.2.12/lib/x86 -Llib/steamwork-sdk/redistributable_bin/osx32 -Llib/steamwork-sdk/steam/lib/osx32"

LIB="-framework OpenGL -framework GLUT -lglut -lSDL -lSDLMain -framework Cocoa -lSDL_mixer -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_video -lsteam_api"

FILES="*.cpp gltools/*.cpp"

echo "Building Ramayana ..."
g++ $* -w $FILES $INCLUDE $LIBDIR $LIB -o "osx32/Ramayana" -Wl,-rpath,./osx32

#echo "Building Ramayana.Demo ..."
#g++ $* -w -D_DEMO $FILES $INCLUDE $LIBDIR $LIB -o "osx32/Ramayana.Demo" -Wl,-rpath,./osx32
