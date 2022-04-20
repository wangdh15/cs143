#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "fb3-2.h"

static unsigned symhash(char *sym) {
    unsigned int hash = 0;
    unsigned c;
    while (c = *sym++) hash = hash * 9 ^ c;
    return hash;
}

// 线性探测法
struct symbol *
lookup(char* sym) {

    struct symbol *sp = &symtab[symhash(sym) % NHASH];
    int scount = NHASH;
    while (--scount >= 0) {
        if (sp->name && !strcmp(sp->name, sym)) { return sp; }
        if (!sp->name) {
            sp->name = strdup(sym);
            sp->value = 0;
            sp->func = NULL;
            sp->syms = NULL;
            return sp;
        }

        if (++sp >= symtab + NHASH) sp = symtab;
    }
    yyerror("symbol table overflow\n");
    abort();
}

struct ast *
newast(int nodetype, struct ast *l, struct ast *r) {
    struct ast *a = malloc(sizeof(struct ast));
    if (!a) {
        yyerror("out of space");
        exit(0);
    }
    a->nodetype = nodetype;
    a->l = l;
    a->r = r;
    return a;
}

struct ast *
newnum(double d) {
    struct numval *a = malloc(sizeof(struct numval));

    if (!a) {
        yyerror("out of space");
        exit(0);
    }

    a->nodetype = 'K';
    a->number = d;
    return (struct ast*)a;
}

struct ast *
newcmp(int cmptype, struct ast *l, struct ast *r) {
    struct ast *a = malloc(sizeof(struct ast));

    if (!a) {
        yyerror("out of space");
        exit(0);
    }

    a->nodetype = '0' + cmptype;
    a->l = l;
    a->r = r;
    return a;
}

struct ast *
newfunc(int functype, struct ast *l) {
    struct fncall *a = malloc(sizeof(struct fncall));

    if (!a) {
        yyerror("out of space");
        exit(0);
    }

    a->nodetype = 'F';
    a->l = l;
    a->functype = functype;
    return (struct ast *)a;
}

struct ast *
newcall(struct symbol *s, struct ast *l) {
    struct ufncall *a= malloc(sizeof(struct ufncall));

    if (!a) {
        yyerror("out of space");
        exit(0);
    }

    a->nodetype = 'C';
    a->l = l;
    a->s = s;
    return (struct ast*)a;
}

struct ast *
newref(struct symbol *s) {
    struct symref *a = malloc(sizeof(struct symref));

    if (!a) {
        yyerror("out of space");
        exit(0);
    }

    a->nodetype = 'N';
    a->s = s;
    return (struct ast *)a;
}

struct ast *
newasgn(struct symbol *s, struct ast *v) {
    struct symasgn *a = malloc(sizeof(struct symasgn));

    if (!a) {
        yyerror("out of space");
        exit(0);
    }

    a->nodetype = '=';
    a->s = s;
    a->v = v;
    return (struct ast*)a;
}

struct ast *
newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el) {
    struct flow *a = malloc(sizeof(struct flow));
    if (!a) {
        yyerror("out of space");
        exit(0);
    }

    a->nodetype = nodetype;
    a->cond = cond;
    a->tl = tl;
    a->el = el;
    return (struct ast*)a;
}

void treefree(struct ast *a) {

    switch (a->nodetype) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '1': case '2': case '3': case '4': case '5': case '6':
        case 'L':
            treefree(a->r);
        case '|':
        case 'M': case 'C': case 'F':
            treefree(a->l);
        case 'K': case 'N':
            break;

        case '=':
            free(((struct symasgn*)a)->v);
            break;
        case 'I': case 'W':
            free(((struct flow*)a)->cond);
            if (((struct flow*)a)->tl) treefree(((struct flow*)a)->tl);
            if (((struct flow*)a)->el) treefree(((struct flow*)a)->el);
            break;
        default:
            printf("internal error: free bad node %c\n", a->nodetype);
    }
    free(a);
}

struct symlist*
newsymlist(struct symbol *sym, struct symlist *next) {
    struct symlist *sl = malloc(sizeof(struct symlist));
    if (!sl) {
        yyerror("out of space");
        exit(0);
    }

    sl->sym = sym;
    sl->next = next;
   return sl;
}

void
symlistfree(struct symlist *sl) {
    struct symlist *nsl;
    while (sl) {
        nsl = sl->next;
        free(sl);
        sl = nsl;
    }
}

static double callbuiltin(struct fncall *);
static double calluser(struct ufncall*);

double eval(struct ast *a) {

    double v;
    if (!a) {
        yyerror("internal error, null eval");
        return 0.0;
    }

    switch (a->nodetype) {
    case 'K': v = ((struct numval*)a)->number; break;

    case 'N': v = ((struct symref*)a)->s->value; break;

    case '=': v = ((struct symasgn*)a)->s->value = eval(((struct symasgn*)a)->v); break;

    case '+': v = eval(a->l) + eval(a->r); break;
    case '-': v = eval(a->l) - eval(a->r); break;
    case '*': v = eval(a->l) * eval(a->r); break;
    case '/': v = eval(a->l) / eval(a->r); break;
    case '|': v = fabs(eval(a->l)); break;
    case 'M': v = -eval(a->l); break;

    case '1': v = (eval(a->l) > eval(a->r))? 1 : 0; break;
    case '2': v = (eval(a->l) < eval(a->r))? 1 : 0; break;
    case '3': v = (eval(a->l) != eval(a->r))? 1 : 0; break;
    case '4': v = (eval(a->l) == eval(a->r))? 1 : 0; break;
    case '5': v = (eval(a->l) >= eval(a->r))? 1 : 0; break;
    case '6': v = (eval(a->l) <= eval(a->r))? 1 : 0; break;

    case 'I':
        if (eval(((struct flow*)a)->cond) != 0) {
            if (((struct flow*)a)->tl) {
                v = eval(((struct flow*)a)->tl);
            } else {
                v = 0.0;
            }
        } else {
            if (((struct flow*)a)->el) {
                v = eval(((struct flow*)a)->el);
            } else {
                v = 0.0;
            }
        }
        break;

    case 'W':
        v = 0.0;
        if (((struct flow*)a)->tl) {

            while (eval(((struct flow*)a)->cond) != 0) {
                v = eval(((struct flow*)a)->tl);
            }
        }
        break;

    case 'L':
        eval(a->l); v = eval(a->r);
        break;
    case 'F':
        v = callbuiltin((struct fncall*)a);
        break;
    case 'C':
        v = calluser((struct ufncall*)a);
        break;

    default:
        printf("internal error: bad node %c\n", a->nodetype);

    }
    return v;
}


static double
callbuiltin(struct fncall* f) {

    enum bifs functype = f->functype;
    double v = eval(f->l);
    switch (functype) {
    case B_sqrt:
        return sqrt(v);
    case B_exp:
        return exp(v);
    case B_log:
        return log(v);
    case B_print:
        printf("= %4.4g\n", v);
        return v;
    default:
        yyerror("Unkonwn built-in function %d", functype);
        return 0.0;
    }
}

void
dodef(struct symbol *name, struct symlist *syms, struct ast *func) {
    if (name->syms) symlistfree(name->syms);
    if (name->func) treefree(name->func);
    name->syms = syms;
    name->func = func;
}

static double
calluser(struct ufncall *f) {
    struct symbol *fn = f->s;
    struct symlist *sl;
    struct ast *args = f->l;
    double *oldval, *newval;
    double v;
    int nargs;
    int i;

    if (!fn->func) {
        yyerror("call to undefined function", fn->name);
        return 0;
    }

    sl = fn->syms;
    for (nargs = 0; sl; sl = sl->next) {
        nargs++;
    }

    oldval = (double*)malloc(nargs * sizeof(double));
    newval = (double*)malloc(nargs * sizeof(double));

    if (!oldval || !newval) {
        yyerror("out of space in %s", fn->name);
        return 0.0;
    }

    for (i = 0; i < nargs; ++i) {
        if (!args) {

            yyerror("too few args in call to %s", fn->name);
            free(oldval);
            free(newval);
            return 0.0;
        }

        if (args->nodetype == 'L') {
            newval[i] = eval(args->l);
            args = args->r;
        } else {
            newval[i] = eval(args);
            args = NULL;
        }

    }


    sl = fn->syms;
    for (i = 0; i < nargs; ++i) {
        struct symbol *s = sl->sym;
        oldval[i] = s->value;
        s->value = newval[i];
        sl = sl->next;
    }

    free(newval);

    v = eval(fn->func);

    sl=fn->syms;
    for (int i = 0; i < nargs; ++i) {
        struct symbol *s = sl->sym;
        s->value = oldval[i];
        sl = sl->next;
    }
    free(oldval);
    return v;
}

void yyerror(char *s, ...) {

    va_list ap;
    va_start(ap, s);

    fprintf(stderr, "error: ");
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}

int main() {
    printf("> ");
    return yyparse();
}