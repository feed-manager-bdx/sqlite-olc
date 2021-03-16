all:
	gcc -g -W -Wall -Wextra --pedantic -Wno-sign-compare -Wno-unused-parameter -fPIC -shared -o sqlite-olc.so \
		-I open-location-code/c/src/ \
		sqlite-olc.c open-location-code/c/src/olc.c
