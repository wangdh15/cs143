%{
#include <stdio.h>
#include <stdlib.h>
#include "fb3-1.h"
%}

%union {
    struct ast *a;
    double d;
}

%token <d> NUMBER
%token EOL

%type <a> exp factor term


%%

calclist:
    | calclist exp EOL {
        printf("= %4.4g\n", eval($2));
        treefree($2);
        printf("> ");
    }
    | calclist EOL { printf("> "); }
    ;

exp: factor
    | exp '+' factor {$$ = newast('+', $1, $3);}
    | exp '-' factor {$$ = newast('-', $1, $3);}
    ;

factor: term
    | factor '*' term {$$ = newast('*', $1, $3);}
    | factor '/' term {$$ = newast('/', $1, $3);}
    ;

term: NUMBER {$$ = newnum($1);}
    | '|' term {$$ = newast('|', $2, NULL);}
    | '(' exp ')' {$$ = $2;}
    | '-' term {$$ = newast('M', $2, NULL);}
    ;

%%