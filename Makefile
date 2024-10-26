main-dyn: src/main.c
	@cc -I./raylib/include -I./src -L ./raylib/lib -l raylib ./src/main.c

game: src/main.c
	@cc -I./raylib/include -I./src -o ./game ./src/main.c ./raylib/lib/libraylib.a -lm 

.PHONY: run
run: game
	@LD_LIBRARY_PATH="$$LD_LIBRARY_PATH:$$PWD/raylib/lib" ./game

.PHONY: clean
clean:
	@rm -rf ./game ./server

server: server/main.c
	@cc- -I./server -o server server/main.c