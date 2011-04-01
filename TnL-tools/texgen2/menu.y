%{
#include "menutypes.h"
%}

%token MENU BUTTON SEPARATOR ON FIRE SHOW STRING ID

%union{ Menu * menu;
        MenuList *menus;
        IElem * elem;
        ElemList * elems;
        KeyValPair *keyval;
        KeyValMap *kvmap;
        IAction *action;
        EventActionPair *eventaction;
        EventActionMap *eamap;
        string *str;
      }
      
%token <str> ID STRING
%type <menu> menudecl
%type <menus> menus
%type <elems> elems
%type <elem> elemdecl buttondecl separatordecl
%type <keyval> option
%type <kvmap> options
%type <action> action
%type <eventaction> actiondecl
%type <eamap> actions

%%

menus:  /* nothing */   { $$ = new MenuList; }
      | menus menudecl  { $1->push_back( *$2 ); $$ = $1; }
      ;

menudecl:   MENU ID options actions '{' elems '}' {
                                    $$ = new Menu;
                                    $$->elems = *$6;
                                    $$->id = *$2;
                                    $$->options = *$3;
                                    $$->actions = *$4; }
      ;

elems:  /* nothing */   { $$ = new ElemList; }
      | elems elemdecl  { $1->push_back( $2 ); $$ = $1; }
      ;

elemdecl:
        buttondecl
      | separatordecl
      ;

buttondecl:   BUTTON ID options actions
                        { $$ = new ButtonElem;
                          $$->id = *$2;
                          $$->options = *$3; $$->actions = *$4; }
      ;

separatordecl: SEPARATOR { $$ = new SeparatorElem; }

options: /* nothing */  { $$ = new KeyValMap; }
      | options option  { $1->insert( *$2 ); $$ = $1; }
      ;

option: ID '=' STRING { $$ = new KeyValPair;
                        $$->first=*$1;
                        $$->second=*$3; }
      ;
      
actions: /* nothing */      { $$ = new EventActionMap; }
      | actions actiondecl  { $1->insert( *$2 ); $$ = $1; }
      ;
      
actiondecl: ON ID action { $$ = new EventActionPair;
                           $$->first=*$2;
                           $$->second=$3; }
                           
action: FIRE STRING { $$ = new FireAction(*$2); }
      | SHOW ID     { $$ = new ShowAction(*$2); }
      ;


%%
#include "lex.yy.cc"
static yyFlexLexer * lexer;
static int yylex() { return lexer->yylex(); }
static int yyparse();

int lineno;
int main (int argc, char **argv)
{
	/* In case you don't already have a main() */
    yyFlexLexer l;
    
    lexer = &l;
	int result=yyparse();
    lexer = 0;
    
    if (result == 0) {
        for(MenuList::iterator i=yyval.menus->begin();
            i!=yyval.menus->end(); i++)
            i->dump();
    }
	return 0;
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
