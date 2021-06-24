all:
	gcc -g -W -Wall -Wextra --pedantic -Wno-sign-compare -Wno-unused-parameter -fPIC -shared -o sqlite-olc.so \
		-I open-location-code/c/src/ \
		sqlite-olc.c open-location-code/c/src/olc.c

windows:
	x86_64-w64-mingw32-gcc -g -W -Wall -Wextra --pedantic -Wno-sign-compare -Wno-unused-parameter -fPIC -shared -o sqlite-olc.dll \
		-I open-location-code/c/src/ -I /usr/x86_64-w64-mingw32/ \
		sqlite-olc.c open-location-code/c/src/olc.c /usr/include/sqlite3ext.h
