%{
#include <list>
#include <fstream>
#include "texscript.h"
#include "texgen.h"
typedef std::pair<Type, Expression *> TypedExpr;
typedef std::list<TypedExpr*> ParamList;
TypedExpr * unary(const char *s, TypedExpr *a);
TypedExpr * binary(const char *s, TypedExpr *a, TypedExpr *b);
TypedExpr * nary(const char *s, ParamList *pl);
static int yylex();
static int yyerror (const char *cmd);
static int yywrap();

static ScriptHost * host;
static TexGen * texgen;

#define check(x) if (x->second == 0) YYERROR;

%}

%union {
    TypedExpr      *te;
    std::string    *id;
    std::string    *s;
    int             i;
    float           f;
    bool            b;
    ParamList      *pl;
}

%token CALC TEX COLOR WHEN DIMS
%token <id> ID
%token <b> BOOL_LIT
%token <i> INT_LIT
%token <f> FLOAT_LIT
%token <s> STRING_LIT
%left OR AND
%left NE EQ
%left '<' '>' LE GE
%left SHL SHR
%left '+' '-'
%left '*' '/' '%'
%left NEG '!'

%type <te> expr
%type <pl> params

%%

script: stmts
      ;

stmts:  /* nothing */
      | stmts stmt
      ;

stmt:   CALC expr           { for(int i=0; i<512*512; i++ ) $2->second->eval(); std::cout <<"result: "<<$2->second->eval()<<std::endl; }
      | tex_stmt            {   }
      | dims_stmt
      ;

tex_stmt: TEX INT_LIT COLOR INT_LIT INT_LIT INT_LIT WHEN expr
            {
                if ($8->first != BOOL) {
                    printf("Must give a boolean expression!");
                    YYERROR;
                } else texgen->addLayer($2, $4, $5, $6, $8->second);
            }
      ;
      
dims_stmt: DIMS FLOAT_LIT FLOAT_LIT FLOAT_LIT
    { texgen->setDims($2, $3, $4); }

expr:   INT_LIT             { $$ = new TypedExpr(std::make_pair(INT,    literal($1))); }
      | FLOAT_LIT           { $$ = new TypedExpr(std::make_pair(FLOAT,  literal($1))); }
      | BOOL_LIT            { $$ = new TypedExpr(std::make_pair(BOOL,   literal($1))); }
      | STRING_LIT          { $$ = new TypedExpr(std::make_pair(STRING, literal(*$1)));}
      | '(' expr ')'        { $$ = $2; check($$); }
      | expr '+' expr       { $$ = binary("+", $1, $3); check($$); }
      | expr '-' expr       { $$ = binary("-", $1, $3); check($$); }
      | expr '*' expr       { $$ = binary("*", $1, $3); check($$); }
      | expr '/' expr       { $$ = binary("/", $1, $3); check($$); }
      | expr '%' expr       { $$ = binary("%", $1, $3); check($$); }
      | expr SHL expr       { $$ = binary("<<", $1, $3); check($$); }
      | expr SHR expr       { $$ = binary(">>", $1, $3); check($$); }
      | expr '<' expr       { $$ = binary("<", $1, $3); check($$); }
      | expr '>' expr       { $$ = binary(">", $1, $3); check($$); }
      | expr LE expr        { $$ = binary("<=", $1, $3); check($$); }
      | expr GE expr        { $$ = binary(">=", $1, $3); check($$); }
      | expr EQ expr        { $$ = binary("==", $1, $3); check($$); }
      | expr NE expr        { $$ = binary("!=", $1, $3); check($$); }
      | expr OR expr        { $$ = binary("||", $1, $3); check($$); }
      | expr AND expr       { $$ = binary("&&", $1, $3); check($$); }
      | '-' expr %prec NEG  { $$ = unary("-", $2); check($$); }
      | '+' expr %prec NEG  { $$ = unary("+", $2); check($$); }
      | '!' expr %prec NEG  { $$ = unary("!", $2); check($$); }
      | ID '(' params ')'   { $$ = nary($1->c_str(), $3);check($$); delete $1;}
      ;
      
params: /* nothing */       { $$ = new ParamList; }
      | expr                { $$ = new ParamList; $$->push_back($1); }
      | params ',' expr     { $$ = $1; $$->push_back($3); }
      ;

%%
#include "lex.yy.cc"
static yyFlexLexer * lexer;
static int yylex() { return lexer->yylex(); }
int yyparse();

TypedExpr *query(const char *s, const Spec & spec) {
    TypedExpr *r = new TypedExpr(host->query(s, spec));
    if (!r->second) {
        std::cerr << "Function not found: " << s <<": " << spec << std::endl;
    }
    return r;
}

TypedExpr * binary(const char *s, TypedExpr *a, TypedExpr *b) {
    TypedExpr *r = query(s, Spec(a->first, b->first, ANY));
    if (r->second) r->second->param(a->second)->param(b->second);
    delete a;
    delete b;
    return r;
}

TypedExpr * unary(const char *s, TypedExpr *a) {
    TypedExpr *r = query(s, Spec(a->first, ANY));
    if (r->second) r->second->param(a->second);
    delete a;
    return r;
}

TypedExpr * nary(const char *s, ParamList *pl) {
    Spec spec(ANY);
    for(ParamList::iterator i=pl->begin(); i!=pl->end(); i++)
        spec.in.push_back((*i)->first);
    TypedExpr *r = query(s, spec);
    if (r->second) {
        for(ParamList::iterator i=pl->begin(); i!=pl->end(); i++) {
            r->second->param((*i)->second);
            delete *i;
        }
    }
    delete pl;
    return r;
}

/*
int main (int argc, char **argv)
{
    yyFlexLexer l;
    
    lexer = &l;
	yyparse();
    lexer = 0;
    
	return 0;
}
*/

bool parse(TexGen * tg, ScriptHost *sh, const char *file) {
    texgen = tg;
    host = sh;
    
    std::ifstream in(file);
    yyFlexLexer l(&in);
    lexer = &l;
    bool ok = yyparse() == 0;
    lexer = 0;
    texgen = 0;
    host = 0;
    return ok;
}
    

/* A very cheap error handler */
int yyerror (const char *cmd)
{
	fprintf (stderr, "%d: %s\n", lexer->lineno(), lexer->YYText());
    fprintf (stderr, "Error: %s\n", cmd);
    return 42;
}

/* yywrap: Called when EOF is reached on current input.
 * Have it return 1 if processing is finished, or
 * do something to fix the EOF condition (like open
 * another file and point to it) and return 0 to indicate
 * more input is available.
 */
int yywrap ()
{
	return 1;
}
