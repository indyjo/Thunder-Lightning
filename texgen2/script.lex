nl          \n|\r\n|\n\r
comment     \#[^\n\r]*{nl}
integer     [0-9]+
string      \"([^\"]*\\\")*[^\"]*\"
id          [a-zA-Z_][a-zA-Z0-9_-]*
float       [0-9]+\.[0-9]+

%%
{comment}   { yylineno++; }
{nl}        { yylineno++; }
[ \t]       ;
\<=         { return LE; }
\>=         { return GE; }
==          { return EQ; }
!=          { return NE; }
\<\<        { return SHL; }
\>\>        { return SHR; }
\|\|        { return OR;  }
\&\&        { return AND; }
{integer}   { yylval.i = atoi(yytext); return INT_LIT; }
{float}     { yylval.f = atof(yytext); return FLOAT_LIT; }
true        { yylval.b = true; return BOOL_LIT; }
false       { yylval.b = false; return BOOL_LIT; }
{string}    { std::string s(yytext);
              s = s.substr(1, s.size()-2);
              yylval.s = new std::string(s);
              return STRING_LIT; }
calc        { return CALC; }
tex         { return TEX; }
color       { return COLOR; }
when        { return WHEN; }
dimensions  { return DIMS; }
{id}        { yylval.id = new std::string(yytext);
              return ID; }
.           { return yytext[0]; }



