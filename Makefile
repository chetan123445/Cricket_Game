CC=gcc
CFLAGS=-I. -Wall -Wextra -std=c99
LDFLAGS_RAYLIB = -lraylib -lwinmm -lgdi32
DEPS = accounts.h common.h history.h match.h teams.h tournament.h admin.h ui.h

main.exe: main.c $(DEPS)
	$(CC) -o main.exe main.c $(CFLAGS)

gui.exe: gui.c raylib.h $(DEPS)
	$(CC) -o gui.exe gui.c $(CFLAGS) -L. $(LDFLAGS_RAYLIB)

clean:
	del *.exe
