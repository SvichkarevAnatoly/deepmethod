gcc -Wall -o g_spawn_r2 g_spawn_r2.c $(pkg-config --cflags --libs glib-2.0)
./g_spawn_r2
