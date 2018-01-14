%{
#include <cstring>
#include <cstdio>
#include <iostream>
using namespace std;

#include "syntax.h"

extern "C" int yylex();
extern "C" FILE *yyin;
// defined in the tokenizer
extern "C" int ln, col;
extern "C" char* yytext;

void yyerror(const char *s);

#define YYSTYPE Node*
%}

%token TYPES
%token INTEGER
%token REAL
%token OPERATOR
%token STRING
%token IDENTIFIER
%token-table // keywords and delimiters are literal strings

//%define parse.error verbose 
%%

program: "PROGRAM" "IS" body ";" 
 { Program* prog = new Program((Body*)$3);
   prog->print(0);
   $$ = prog;
 };

body: declaration_block "BEGIN" statement_block "END" 
{ 
  $$ = new Body((Multi<Decl>*)$1, (Multi<Stat>*)$3); 
};

declaration_block: declaration declaration_block 
{ 
  ((Multi<Decl>*)$2)->add((Decl*)$1); 
 $$=$2; 
}
| { $$ = new Multi<Decl>(); }  ;

statement_block: statement statement_block 
{
  ((Multi<Stat>*)$2)->add((Stat*)$1);
  $$ = $2;
}
| { $$ = new Multi<Stat>(); } ;

declaration: "VAR" var_decl_block 
{
  $$ = new Decl("var", (Multi<Node>*)$2);
}
| "TYPE" type_decl_block
{
  $$ = new Decl("type", (Multi<Node>*)$2);
}
| "PROCEDURE" proc_decl_block 
{
  $$ = new Decl("procedure", (Multi<Node>*)$2);
};

var_decl_block: var_decl var_decl_block 
{
  ((Multi<VarDecl>*)$2)->add((VarDecl*)$1);
  $$ = $2;
}
| var_decl 
{
  Multi<VarDecl>* v = new Multi<VarDecl>();
  v->add((VarDecl*)$1);
  $$ = v;
}
;

type_decl_block: type_decl type_decl_block 
{
  ((Multi<TypeDecl>*)$2)->add((TypeDecl*)$1);
  $$ = $2;
}
| type_decl
{
  Multi<TypeDecl>* v = new Multi<TypeDecl>();
  v->add((TypeDecl*)$1);
  $$ = v;
}
;

proc_decl_block: proc_decl proc_decl_block 
{
  ((Multi<ProcDecl>*)$2)->add((ProcDecl*)$1);
  $$ = $2;
}
| proc_decl
{
  Multi<ProcDecl>* v = new Multi<ProcDecl>();
  v->add((ProcDecl*)$1);
  $$ = v;
}
;

var_decl: id_block type_opt ":=" expr ";" 
{
  $$ = new VarDecl((Multi<Id>*)$1, (Type*)$2, (Expr*)$4);
};

id_block: IDENTIFIER "," id_block 
{
  ((Multi<Id>*)$3)->add((Id*)$1);
  $$ = $3;
}
| IDENTIFIER
{
  Multi<Id>* v = new Multi<Id>();
  v->add((Id*)$1);
  $$ = v;
}
;

type_opt: ":" type 
{
  $$ = $2;
}
| { $$ = NULL; };

type_decl: IDENTIFIER "IS" type ";" 
{
  $$ = new TypeDecl((Id*)$1, (Type*)$3);
}
;

proc_decl: IDENTIFIER formal_params type_opt "IS" body ";" 
{
  $$ = new ProcDecl((Id*)$1, (Multi<FPSec>*)$2, (Type*)$3, (Body*)$5);
};

type: IDENTIFIER 
{
  $$ = new UserType((Id*)$1);
}
| TYPES 
{
  $$ = $1;
}
| "ARRAY" "OF" type 
{
  $$ = new ArrayType((Type *)$3);
}
| "RECORD" component_block "END"
{
  $$ = new RecordType((Multi<Component>*)$2);
}
;

component_block: component component_block 
{
  ((Multi<Component>*)$2)->add((Component*)$1);
  $$ = $2;
}
| component 
{
  Multi<Component>* v = new Multi<Component>();
  v->add((Component*)$1);
  $$ = v;
}
;

component: IDENTIFIER ":" type ";"
{
  $$ = new Component((Id*)$1, (Type*)$3);
}
;

formal_params: "(" fp_section_block ")"  { $$ = $2; } 
| "(" ")"
{
  $$ = NULL;
}
;

fp_section_block: fp_section ";" fp_section_block 
{
  ((Multi<FPSec>*)$3)->add((FPSec*)$1);
  $$ = $3;
}
| fp_section
{
  Multi<FPSec>* v = new Multi<FPSec>;
  v->add((FPSec*)$1);
  $$ = v;
}
;

fp_section: id_block ":" type
{
  $$ = new FPSec((Multi<Id>*)$1, (Type*)$3);
};

statement: 
 lvalue ":=" expr ";" 
 {
   $$ = new AssignStat((Lvalue*)$1, (Expr*)$3);
 } 
|
 IDENTIFIER actual_params ";" 
{
  $$ = new CallStat((Id*)$1, (Multi<Expr>*)$2);
}
|
 "READ" "(" lvalue_block ")" ";" 
{ 
  $$ = new ReadStat((Multi<Lvalue>*)$3);
}
|
 "WRITE" write_params ";" 
{
  $$ = new WriteStat((Multi<WriteExpr>*)$2);
}
|
 "IF" expr "THEN" statement_block elseif_block else_opt "END" ";" 
{
  $$ = new IfStat((Expr*)$2, (Multi<Stat>*)$4, (Multi<ElseIf>*)$5, 
                  (Multi<Stat>*)$5);
}
|
 "WHILE" expr "DO" statement_block "END" ";"
{
  $$ = new WhileStat((Expr*)$2, (Multi<Stat>*)$4);
}
|
 "LOOP" statement_block "END" ";" 
{
  $$ = new LoopStat((Multi<Stat>*)$2);
}
|
 "FOR" IDENTIFIER ":=" expr "TO" expr by_opt "DO" statement_block "END" ";" 
{
  $$ = new ForStat((Id*)$2, (Expr*)$4, (Expr*)$6, (Expr*)$7, (Multi<Stat>*)$9);
}
|
 "EXIT" ";" 
{
  $$ = new ExitStat();
}
|
 "RETURN" expr_opt ";"
{
  $$ = new ReturnStat((Expr*)$2);
}
 ;

by_opt: "BY" expr
{ $$ = $2; }  
| { $$ = NULL; };

elseif_block: "ELSIF" expr "THEN" statement_block elseif_block 
{
  Multi<ElseIf>* v = (Multi<ElseIf>*)$5;
  ElseIf* s = new ElseIf((Expr*)$2, (Multi<Stat>*)$4);
  v->add(s);
  $$ = v;
}
|
{
  $$ = new Multi<ElseIf>();
}
;
else_opt: "ELSE" statement_block 
{
  { $$ = $2; }
}
| { $$ = NULL;  } 
;

lvalue_block: lvalue "," lvalue_block 
{
  Multi<Lvalue>* v = (Multi<Lvalue>*)$3;
  v->add((Lvalue*)$1);
  $$ = v;
}
| lvalue
{
  Multi<Lvalue>* v = new Multi<Lvalue>();
  v->add((Lvalue*)$1);
  $$ = $1;
}
;

write_params:  "(" write_expr_block ")" 
{
  $$ = $2;
}
| "(" ")"
{
  $$ = NULL;
}
;
write_expr_block: write_expr "," write_expr_block 
{
  Multi<WriteExpr>* v = (Multi<WriteExpr>*)$3;
  v->add((WriteExpr*)$1);
  $$ = v;
}
| write_expr
{
  Multi<WriteExpr>* v = new Multi<WriteExpr>();
  v->add((WriteExpr*)$1);
  $$ = v;
}
;

write_expr: STRING 
{
  $$ = new StrWriteExpr((String*)$1);
}                        
| expr 
{
  $$ = new ExprWriteExpr((Expr*)$1);
}
;

expr: number { $$ = $1; }| 
lvalue { $$ = new LvalueExpr((Lvalue*)$1); }| 
"(" expr ")" { $$ = $2; }| 
unary_op expr { $$ = new UnaryOpExpr((Op*)$1, (Expr*)$2); } |
expr binary_op expr { $$ = new BinOpExpr((Op*)$2, (Expr*)$1, (Expr*)$3); } |
IDENTIFIER actual_params { $$ = new CallExpr((Id*)$1, (Multi<Expr>*)$2); }|
IDENTIFIER comp_values { $$ = new RecordExpr((Id*)$1, (Multi<CompValue>*)$2); } |
IDENTIFIER array_values { $$ = new ArrayExpr((Id*)$1, (Multi<ArrayValue>*)$2); };

expr_block: expr "," expr_block 
{
  Multi<Expr>* v = (Multi<Expr>*)$3;
  v->add((Expr*)$1);
  $$ = v;
}
| expr 
{
  Multi<Expr>* v = new Multi<Expr>();
  v->add((Expr*)$1);
  $$ = v;
}
;
expr_opt: expr { $$=$1; }| {$$=NULL;};

lvalue: IDENTIFIER { $$ = new IdLvalue(*((Id*)$1)); } |
lvalue "[" expr "]" { $$ = new ArrayLvalue((Lvalue*)$1, (Expr*)$3); } |
lvalue "." IDENTIFIER { $$ = new RecordLvalue((Lvalue*)$1, (Id*)$3); } ;

actual_params: "(" expr_block ")" { $$ = $2; } 
| "(" ")" { $$ = NULL; } ;

comp_values: "{" comp_value_block "}" { $$ = $2; };
comp_value_block: IDENTIFIER ":=" expr ";" comp_value_block 
{
  Multi<CompValue>* v = (Multi<CompValue>*)$5;
  v->add(new CompValue((Id*)$1, (Expr*)$3));
  $$ = v;
}
| IDENTIFIER ":=" expr
{
   Multi<CompValue>* v = new Multi<CompValue>();
   v->add(new CompValue((Id*)$1, (Expr*)$3));
   $$ = v;
}
;

array_values: "[<" array_value_block ">]" { $$ = $2; };

array_value_block: array_value "," array_value_block 
{
  Multi<ArrayValue>* v = (Multi<ArrayValue>*)$3;
  v->add((ArrayValue*)$1);
  $$ = v;
}
| array_value
{
  Multi<ArrayValue>* v = new Multi<ArrayValue>();
  v->add((ArrayValue*)$1);
  $$ = v;
}
;

array_value: expr "OF" expr 
{ $$ = new OfArrayValue((Expr*)$1, (Expr*)$3); }
| expr { $$ = new SimpleArrayValue((Expr*)$1); };

number: REAL { $$ = new NumberExpr((Number*)$1, 0); } 
| INTEGER { $$ = new NumberExpr((Number*)$1, 1); } ;

unary_op:  OPERATOR { $$ = $1; } | "NOT" { $$ = new Op("NOT"); };

binary_op: OPERATOR { $$ = $1; } 
| "DIV" { $$ = new Op("DIV"); }
| "MOD" { $$ = new Op("MOD"); } 
| "OR"  { $$ = new Op("OR"); }
| "AND" { $$ = new Op("AND"); }
;


%%

int main(int argc, char** argv) {
  if (argc != 2) {
    cout << "Need a file" << endl;
    return -1;
  }

	FILE *myfile = fopen(argv[1], "r");
	// make sure it is valid:
	if (!myfile) {
		cout << "I can't open file!" << endl;
		return -1;
	}
	// set flex to read from it instead of defaulting to STDIN:
	yyin = myfile;
	
	// parse through the input until there is no more:
	do {
		yyparse();
	} while (!feof(yyin));
	
}

void yyerror(const char *s) {
	cout << "EEK, parse error! Position: " << ln << ":" << col 
         << " token: " << yytext 
         << "  Message: " << s << endl;
	// might as well halt now:
	exit(-1);
}

// http://www.gnu.org/software/bison/manual/html_node/Calling-Convention.html#Calling-Convention
int find_token_code (char* token_buffer) {
    int i;
    for (i = 0; i < YYNTOKENS; i++) {
        if (yytname[i] != 0
            && yytname[i][0] == '"'
            && ! strncmp (yytname[i] + 1, token_buffer,
                          strlen (token_buffer))
            && yytname[i][strlen (token_buffer) + 1] == '"'
            && yytname[i][strlen (token_buffer) + 2] == 0)
          break;
    }

    return  255+i;
}
