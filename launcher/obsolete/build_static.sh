make
gcc -o h2launcher callbacks.o config_file.o interface.o launch_bin.o main.o\
 support.o -Wl,-Bstatic -L/usr/lib -L/usr/X11R6/lib -lgtk -lgdk\
 -rdynamic -lgmodule -lglib -Wl,-Bdynamic -ldl -lXi -lXext -lX11 -lm