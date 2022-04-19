extern int yylinen;
void yyerror(char *s, ...);

struct ast {
    int nodetype;
    struct ast *l, *r;
};

struct numval{
    int nodetype;
    double number;
};

struct ast* newast(int nodetype, struct ast *l, struct ast *r);
struct ast* newnum(double d);

double eval(struct ast *);

void treefree(struct ast *);
