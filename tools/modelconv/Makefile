all:
	gcc src/main.c -o modelconv -lassimp -lm -Iinclude

test:
	gcc src/test.c -o modelconv_test -lassimp -lm -Iinclude

clean:
	rm -rf modelconv modelconv_test *.mdl *.dat \
		*.blend1 compile_commands.json .cache/
