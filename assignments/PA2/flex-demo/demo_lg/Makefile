fb3-2: fb3-2.h fb3-2.lex fb3-2.y fb3-2funcs.c
	bison -d fb3-2.y
	flex -o fb3-2.lex.c fb3-2.lex
	gcc -o $@ fb3-2funcs.c fb3-2.tab.c fb3-2.lex.c  -lm -lfl
clean:
	rm fb3-2.tab.* fb3-2.lex.c fb3-2
