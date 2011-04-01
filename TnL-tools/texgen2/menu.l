nl          \n|\r\n|\n\r
comment     \#[^\n\r]*{nl}
menu        menu
button      button
separator   -+
on          on
fire        fire
show        show
string      \"([^\"]*\\\")*[^\"]*\"
id          [a-zA-Z_][a-zA-Z0-9_-]*

%%
{comment}       { yylineno++; }
{nl}            { yylineno++; }
[ \t]           ; 
{menu}          { return MENU; }
{button}        { return BUTTON; }
{separator}     { return SEPARATOR; }
{on}            { return ON; }
{fire}          { return FIRE; }
{show}          { return SHOW; }
{string}        { yylval.str = new string(yytext);
                  *yylval.str = yylval.str->substr(1, yylval.str->size()-2);
                  return STRING; }
{id}            { yylval.str = new string(yytext); return ID; }
.               { return yytext[0]; }



