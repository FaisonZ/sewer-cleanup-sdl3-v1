gcc src/sewer-cleanup.c -o build/sewer-cleanup/sewer-cleanup `pkg-config --cflags --libs sdl3` -Wl,-rpath='$ORIGIN/lib' -g -Wall
