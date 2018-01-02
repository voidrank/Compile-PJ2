// defines the syntax tree
#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Node {
public:
    virtual void print(int indent) = 0;
};

class Op : public Node {
    string op;
public:
    Op(const char* _op) : op(_op) {}

    void print (int indent) {
        cout << string(indent, ' ') << "operator " << op << endl;
    }
};

class Id : public Node {
    string id;
public:
    Id(const char* _id) : id(_id) {}

    void print (int indent) {
        cout << string(indent, ' ') << "identifier:"<<id<<endl;
    }
};

class Number : public Node {
    string repr;
public:
    Number(const char* _repr) : repr(_repr) {}

    void print (int indent) {
        cout << string(indent, ' ') << "number " << repr << endl;
    }
};

class String : public Node {
    string str;
public:
    String(const char* _str) : str(_str) {}

    void print (int indent) {
        cout << string(indent, ' ') << "string literal " << str << endl;
    }
};

template <class T> 
class Multi: public Node { 
    // note because of the way the parse works, the order of nodes is backward
    // of its syntactic order
    vector<T*> nodes;
public:
    void add (T* n) {
        nodes.push_back(n);
    }

    bool empty () {
        return nodes.empty();
    }

    void print(int indent) {
        for (typename vector<T*>::reverse_iterator it=nodes.rbegin(); it!=nodes.rend(); it++) {
            (*it)->print(indent);
        }
    }
};


class Lvalue: public Node {
};

class IdLvalue: public Lvalue {
    Id* id;
public:
    IdLvalue(Id* _id) : id(_id) {}

    void print (int indent) {
        cout << string(indent, ' ') << "id lvalue" << endl;
        id->print(indent+4);
    }
};

class Expr;

class ArrayLvalue : public Lvalue {
    Lvalue* lval;
    Expr* expr;
public:
    ArrayLvalue(Lvalue* _lval, Expr* _expr) : lval(_lval), expr(_expr) {}

    void print (int indent) {
        cout << string(indent, ' ') << "array lvalue" << endl;
        cout << string(indent+2, ' ') << "array" << endl;
        lval->print(indent+4);
        cout << string(indent+2, ' ') << "index" << endl;
        ((Node*)expr)->print(indent+4);
    }
};

class RecordLvalue: public Lvalue {
    Lvalue* lval;
    Id* id;
public:
    RecordLvalue(Lvalue* _lval, Id* _id) : lval(_lval), id(_id) {}

    void print (int indent) {
        cout << string(indent, ' ') << "record lvalue" << endl;
        cout << string(indent+2, ' ') << "record" << endl;
        lval->print(indent+4);
        cout << string(indent+2, ' ') << "member" << endl;
        id->print(indent+4);
    }
};

class CompValue: public Node {
    Id* id;
    Expr* expr;
public:
    CompValue(Id* _id, Expr* _expr) : id(_id), expr(_expr) {}

    void print (int indent) {
        cout << string(indent, ' ') << "member" << endl;
        id->print(indent+2);
        cout << string(indent, ' ') << "value" << endl;
        ((Node*)expr)->print(indent+4);
    }
};

class ArrayValue: public Node {
};

class SimpleArrayValue : public ArrayValue {
    Expr* expr;
public:
    SimpleArrayValue(Expr* _expr) : expr(_expr) {}
   
    void print (int indent) {
        ((Node*)expr)->print(indent);
    }
};

class OfArrayValue : public ArrayValue {
    Expr* left;
    Expr* right;
public:
    OfArrayValue(Expr* _left, Expr* _right): left(_left), right(_right) {}

    void print (int indent) {
        ((Node*)left)->print(indent+4);
        cout << string(indent+2, ' ') << "of" << endl;
        ((Node*)right)->print(indent+4);
    }
}; 

class Expr : public Node {
};

class NumberExpr: public Expr {
    Number* n;
public:
    NumberExpr(Number* _n) : n(_n) {}

    void print (int indent) {
        n->print(indent);
    }
};

class LvalueExpr : public Expr {
    Lvalue* lval;
public:
    LvalueExpr(Lvalue* _lval) : lval(_lval) {}

    void print (int indent) {
        lval->print(indent);
    }
};

class UnaryOpExpr : public Expr {
    Op* op;
    Expr* expr;
public:
    UnaryOpExpr(Op* _op, Expr* _expr) : op(_op), expr(_expr) {}

    void print (int indent) {
        cout << string(indent, ' ') << "unary operator expression" << endl;
        op->print(indent+2);
        expr->print(indent+2);
    }
};

class BinOpExpr : public Expr {
    Op* op;
    Expr* left;
    Expr* right;
public:
    BinOpExpr(Op* _op, Expr* _left, Expr* _right) 
        :op(_op), left(_left), right(_right) {}

    void print (int indent) {
        cout << string(indent, ' ') << "binary operator expression" << endl;
        left->print(indent+4);
        op->print(indent+2);
        right->print(indent+4);
    }

};

class CallExpr : public Expr {
    Id* id;
    Multi<Expr>* params; //nullable
public:
    CallExpr(Id* _id, Multi<Expr>* _params) : id(_id), params(_params) {}

    void print (int indent) {
        cout << string(indent, ' ') << "function call" << endl;
        cout << string(indent+2, ' ') << "function id" << endl;
        id->print(indent+4);
        if  (params) {
            cout << string(indent+2, ' ') << "parameters" << endl;
            params->print(indent+4);
        } else {
            cout << string(indent+2, ' ') << "no parameter" << endl;
        }
    }
};

class RecordExpr : public Expr {
    Id* id;
    Multi<CompValue>* vals;
public:
    RecordExpr(Id* _id, Multi<CompValue>* _vals) : id(_id), vals(_vals) {}

    void print (int indent) {
        cout << string(indent, ' ') << "record expression" << endl;
        cout << string(indent+2, ' ') << "record id" << endl;
        id->print(indent+4);
        cout << string(indent+2, ' ') << "values" << endl;
        vals->print(indent+4);
    }
};

class ArrayExpr : public Expr {
    Id* id;
    Multi<ArrayValue>* vals;
public:
    ArrayExpr(Id* _id, Multi<ArrayValue>* _vals) : id(_id), vals(_vals) {}

    void print (int indent) {
        cout << string(indent, ' ') << "array expression" << endl;
        cout << string(indent+2, ' ') << "array id" << endl;
        id->print(indent+4);
        cout << string(indent+2, ' ') << "values" << endl;
        vals->print(indent+4);
    }
};

class WriteExpr : public Node {
};

class StrWriteExpr : public WriteExpr {
    String* str;
public:
    StrWriteExpr(String* _str) : str(_str) {}

    void print (int indent) {
        str->print(indent);
    }
};

class ExprWriteExpr : public WriteExpr {
    Expr* expr;
public:
    ExprWriteExpr(Expr* _expr) : expr(_expr) {}

    void print (int indent) {
        expr->print(indent);
    }
};

class Stat: public Node {
};

class AssignStat: public Stat {
    Lvalue* lvalue;
    Expr* expr;
public:
    AssignStat(Lvalue* _lvalue, Expr* _expr) 
        : lvalue(_lvalue), expr(_expr) 
    {}

    void print (int indent) {
        cout << string(indent, ' ') << "assignment statement" << endl;
        lvalue->print(indent+2);
        cout << string(indent+2, ' ') << "value" << endl;
        expr->print(indent+4);
    }
};

class CallStat: public Stat {
    Id* id;
    Multi<Expr>* params; // nullable
public:
    CallStat(Id* _id, Multi<Expr>* _params) : id(_id), params(_params) {}

    void print (int indent) {
        cout << string(indent, ' ') << "function call statement" << endl;
        cout << string(indent+2, ' ') << "function name" << endl;
        id->print(indent+4);
        if (params) {
            cout << string(indent+2, ' ') << "parameters" << endl;
            params->print(indent+4);
        } else {
            cout << string(indent+2, ' ') << "no parameter" << endl;
        }
    }
};

class ReadStat: public Stat {
    Multi<Lvalue>* lvalues;
public:
    ReadStat(Multi<Lvalue>* _lvalues) : lvalues(_lvalues) {}

    void print (int indent) {
        cout << string(indent, ' ') << "read statement" << endl;
        lvalues->print(indent+2);
    }
};

class WriteStat: public Stat {
    Multi<WriteExpr>* write_params; // nullable
public:
    WriteStat(Multi<WriteExpr>* _write_params) : write_params(_write_params) {}

    void print (int indent) {
        cout << string(indent, ' ') << "write statement" << endl;
        write_params->print(indent+2);
    }
};

class ElseIf : public Node {
    Expr* cond;
    Multi<Stat>* then;
public:
    ElseIf(Expr* _cond, Multi<Stat>* _then) : cond(_cond), then(_then) {}

    void print (int indent) {
        cout << string(indent, ' ') << "condition" << endl;
        cond->print(indent+2);
        cout << string(indent, ' ') << "then" << endl;
        then->print(indent+2);
    }
};

class IfStat: public Stat {
    Expr* cond;
    Multi<Stat>* then;
    Multi<ElseIf>* elseif;
    Multi<Stat>* else_; // nullable
public:
    IfStat(Expr* _cond, Multi<Stat>* _then, Multi<ElseIf>* _elseif, Multi<Stat>* _else)
        : cond(_cond), then(_then), elseif(_elseif), else_(_else) 
    {}

    void print (int indent) {
        cout << string(indent, ' ')<< "if statement" << endl;
        cout << string(indent+2, ' ') <<"condition" << endl;
        cond->print(indent+4);
        cout << string(indent+2, ' ') <<"then" << endl;
        then->print(indent+4);
        if (elseif && !elseif->empty()) {
            cout << string(indent+2, ' ') <<"elseif" << endl;
            elseif->print(indent+4);
        }
        if (else_) {
            cout << string(indent+2, ' ') <<"else" << endl;
            else_->print(indent+4);
        }
    }
};

class WhileStat: public Stat {
    Expr* cond;
    Multi<Stat>* body;
public:
    WhileStat(Expr* _cond, Multi<Stat>* _body) : cond(_cond), body(_body) {};

    void print (int indent) {
        cout << string(indent, ' ') << "while loop" << endl;
        cout << string(indent+2, ' ') << "condition expression" << endl;
        cond->print(indent+4);
        cout << string(indent+2, ' ')  << "body" << endl;
        body->print(indent+4);
    }
};

class LoopStat: public Stat {
    Multi<Stat>* body;
public:
    LoopStat(Multi<Stat>* _body) : body(_body) {}

    void print (int indent) {
        cout << string(indent, ' ') << "loop" << endl;
        body->print(indent+2);
    }
};

class ForStat: public Stat {
    Id* id;
    Expr* from;
    Expr* to;
    Expr* by; // nullable
    Multi<Stat>* body;
public:
    ForStat(Id* _id, Expr* _from, Expr* _to, Expr* _by, Multi<Stat>* _body)
        : id(_id), from(_from), to(_to), by(_by), body(_body)
    {}
    
    void print (int indent) {
        cout << string(indent, ' ') << "for statement" << endl;

        cout << string(indent+2, ' ') << "for variable" << endl;
        id->print(indent+4);

        cout << string(indent+2, ' ') << "from" << endl;
        from->print(indent+4);

        cout << string(indent+2, ' ') << "to" << endl;
        to->print(indent+4);
        
        if (by) {
            cout << string(indent+2, ' ') << "by" << endl;
            by->print(indent+4);
        }

        cout << string(indent+2, ' ') << "for body" << endl;
        body->print(indent+4);
    }
};

class ExitStat: public Stat {
public:
    void print (int indent) {
        cout << string(indent, ' ') << "exit statement" << endl;
    }
};

class ReturnStat: public Stat {
    Expr* val; //nullable
public:
    ReturnStat(Expr* _val) : val(_val) {}
    void print (int indent) {
        if (val) {
            cout << string(indent, ' ') << "return value" << endl;
            val->print(indent+4);
        } else {
            cout << string(indent, ' ') << "return statement" << endl;
        }
    }
};

class Type;

class VarDecl: public Node {
    Multi<Id>* ids;
    Type* type; // may be NULL
    Expr* expr;
public:
    VarDecl(Multi<Id>* _ids, Type* _type, Expr* _expr)
        : ids(_ids), type(_type), expr(_expr) 
    {
    }

    void print (int indent) {
        cout << string(indent, ' ') << "variable declaration" << endl;
        cout << string(indent+2, ' ') << "variable names" << endl;
        ids->print(indent+4);
        if (type) {
            cout << string(indent+2, ' ') << "type" << endl;
            ((Node*)type)->print(indent+4);
        }
        cout << string(indent+2, ' ') << "initializer" << endl;
        expr->print(indent+4);
    }
};

class Type : public Node {
};

class UserType : public Type {
    Id* id;
public:
    UserType(Id* _id)  : id(_id) {}

    void print (int indent) {
        cout << string(indent, ' ') << "user defined type: " << endl;
        id->print(indent+2);
    }
};

class BuiltinType : public Type {
    string typename_;
public:
    BuiltinType(string _typename) : typename_(_typename) {}

    void print (int indent) {
        cout << string(indent, ' ') << "type: " << typename_ << endl;
    }
};

class ArrayType : public Type {
    Type* elem_type;
public:
    ArrayType(Type* _elem_type) : elem_type(_elem_type) {}

    void print(int indent) {
        cout << string(indent, ' ') << "array type of" << endl;
        elem_type->print(indent+2);
    }
};

class Component : public Node {
    Id* id;
    Type* type;
public:
    Component(Id* _id, Type* _type) : id(_id), type(_type) {}

    void print(int indent) {
        cout << string(indent, ' ') << "component id" <<endl;
        id->print(indent+2);
        cout << string(indent, ' ') << "component type" << endl;
        type->print(indent+2);
    }
};

class RecordType: public Type {
    Multi<Component>* components;
public:
    RecordType(Multi<Component>* _components) : components(_components) {}

    void print(int indent) {
        cout << string(indent, ' ') << "record type" << endl;
        components->print(indent+2);
    }
};

class TypeDecl: public Node {
    Id* id;
    Type* type;
public:
    TypeDecl(Id* _id, Type* _type) : id(_id), type(_type) {}
    void print (int indent) {
        cout << string(indent, ' ') << "type declaration" << endl;
        id->print(indent+2);
        type->print(indent+2);
    }
};

class FPSec: public Node {
    Multi<Id>* ids;
    Type* type_;
public:
    FPSec(Multi<Id>* _ids, Type* _type) 
        : ids(_ids), type_(_type) 
    {}

    void print (int indent) {
        cout << string(indent, ' ') << "identifiers" << endl;
        ids->print(indent+2);
        cout << string(indent, ' ') << "type" << endl;
        type_->print(indent+2);
    }
};

class Body;

class ProcDecl: public Node {
    Id* id;
    Multi<FPSec>* fpsecs; // may be NULL
    Type* type; // nullable
    Body* body;
public:
    ProcDecl(Id* _id, Multi<FPSec>* _fpsecs, Type* _type, Body* _body) 
        : id(_id), fpsecs(_fpsecs), type(_type), body(_body) 
    {}

    void print (int ident) {
        cout << string(ident, ' ') << "procedure declaration" << endl;
        cout << string(ident+2, ' ') << "id" << endl;
        id->print(ident+4);
        if (fpsecs) {
            cout << string(ident+2, ' ') << "formal parameters" << endl;
            fpsecs->print(ident+4);
        }
        if (type) {
            cout << string(ident+2, ' ') << "return type" << endl;
            type->print(ident+4);
        }
        ((Node*)body)->print(ident+4);
    }
};

class Decl: public Node {
    string type; // "var" "type" "procedure"
    Multi<Node>* block;
public:
    Decl(const char* _type, Multi<Node>* _block) : type(_type), block(_block) {}

    void print (int ident) {
        block->print(ident);
    }
};

class Body: public Node {
    Multi<Decl>* decls;
    Multi<Stat>* stats;
public:
    Body(Multi<Decl>* _decls, Multi<Stat>* _stats): decls(_decls), stats(_stats) {}
    void print (int ident) {
        cout << string(ident, ' ') << "body" << endl;
        cout << string(ident+2, ' ')<< "declarations" << endl;
        decls->print(ident+4);
        cout << string(ident+2, ' ') << "statements" << endl;
        stats->print(ident+4);
    }
};

class Program: public Node {
    Body* body;
public:
    Program(Body* _body) : body(_body) {}
    void print (int ident) {
        cout << string(ident, ' ') << "program" << endl;;
        body->print(ident+2);
    }
};


