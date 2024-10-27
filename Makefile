main-dyn: src/main.c
	@cc -I./raylib/include -I./src -L ./raylib/lib -l raylib ./src/main.c

game: $(wildcard ./src/*.c ./src/*.h)
	# @cc -I./raylib/include -I./src -D 'HOST="139.59.91.170"' -o ./game ./src/main.c ./raylib/lib/libraylib.a -lm 
	@cc -I./raylib/include -I./src -o ./game ./src/main.c ./raylib/lib/libraylib.a -lm 

.PHONY: run
run: game
	@LD_LIBRARY_PATH="$$LD_LIBRARY_PATH:$$PWD/raylib/lib" ./game

.PHONY: clean
clean:
	@rm -rf ./game ./srv

srv: $(wildcard ./server/*.c ./server/*.h)
	@cc -I./server -o ./srv ./server/main.c
