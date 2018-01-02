%{
#include <iostream>
#include <cstdio>
#include "tokenizer.h"
#include "syntax.h"
using namespace std;
#define YY_DECL extern "C" int yylex()
#define YYSTYPE Node*
int find_token_code (char* token_buffer);

#include "main.tab.h" // to get the token types that we return


int ln = 1, col = 1;
const int tab_width = 8;

%}

types	INTEGER|REAL|STRING
keyword	AND|ELSIF|LOOP|PROGRAM|VAR|ARRAY|END|MOD|READ|WHILE|BEGIN|EXIT|NOT|RECORD|WRITE|BY|FOR|OF|RETURN|DIV|IF|OR|THEN|DO|IN|OUT|TO|ELSE|IS|PROCEDURE|TYPE
letter	[A-Za-z]
digit	[0-9]
operator	\+|\-|\*|\/|<|<=|>|>=|=|<>
assign :=
delimiter	:|;|,|\.|\(|\)|\[|\]|\{|\}|\[<|>\]|\\

%%
\(\*.*\*\) {
	col += strlen(yytext);
}
[ ] {
	col += 1;
}
[\t] {
	col = ((col - 1) / tab_width + 1)  * tab_width + 1;
}
[\n] {
	col = 1;
	ln += 1;
}
{types} {
    //cout << "Line " << ln << ", Colomn " << col;
	//cout << ": TYPE " << yytext << endl;
	col += strlen(yytext);
    yylval = new BuiltinType(yytext);
    return TYPES;
}
{keyword} {
	//cout << "Line " << ln << ", Colomn " << col;
	//cout << ": KEYWORD " << yytext << endl;
	col += strlen(yytext);
    return find_token_code(yytext);
}
{digit}+ {
    //cout << "Line " << ln << ", Colomn " << col;
    //cout << ": INT " << yytext;
    //if (check_integer(yytext) == INT_OVERFLOW)
    //    cout << "INT INT_OVERFLOW";
    //cout << endl;
	col += strlen(yytext);
    yylval = new Number(yytext);
    return  INTEGER;
}
{digit}+\.{digit}* {
    //cout << "Line " << ln << ", Colomn " << col;
	//cout << ": REAL " << yytext << endl;
	col += strlen(yytext);
    yylval = new Number(yytext);
    return REAL;
}
\"[^\"]*\" {
	//cout << "Line " << ln << ", Colomn " << col;
	//cout << ": STRING " << yytext;
    //int err = check_string(yytext);
    //if (err == STR_OVER_LONG)
    //    cout << "STRING OVER LONG";
    //else if (err == UNEXPECTED_WORD)
    //   cout << "UNEXPECTED WORD";
    //cout << endl;
    yylval = new String(yytext);
	col += strlen(yytext);
    return STRING;
}
{operator} {
	//cout << "Line " << ln << ", Colomn " << col;
	//cout << ": OPERATOR " << yytext << endl;
	col += strlen(yytext);
    yylval = new Op(yytext);
    return OPERATOR;
}
{assign} {
   col += strlen(yytext);
   return find_token_code(yytext);
}
{delimiter} {
	//cout << "Line " << ln << ", Colomn " << col;
	//cout << ": DELIMITER " << yytext << endl;
	col += strlen(yytext);
    return find_token_code(yytext);
}
{letter}({letter}|{digit})* {
	//cout << "Line " << ln << ", Colomn " << col;
	//cout << ": IDENTIFIER " << yytext << endl;
	col += strlen(yytext);
    yylval = new Id(yytext);
    return IDENTIFIER;
}

%%

int check_string(char *str) {
  if (strlen(str) > 255)
    return STR_OVER_LONG;
  else {
    for (int i = 0; i < strlen(str); ++i)
      if (str[i] < 32)
        return UNEXPECTED_WORD;
    return 0;
  }
}


int check_integer(char* int_str) {
  if (strlen(int_str) > strlen(MAX_INT))
    return INT_OVERFLOW;
  else if (strlen(int_str) == strlen(MAX_INT)) {
    for (int i = 0; i < strlen(int_str); ++i)
      if (int_str[i] > MAX_INT[i])
        return INT_OVERFLOW;
      else if (int_str[i] < MAX_INT[i])
        return 0;
  }
  return 0;
}

