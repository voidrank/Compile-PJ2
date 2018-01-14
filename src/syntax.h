// defines the syntax tree
#ifndef SYNTAX
#define SYNTAX
#include <iostream>
#include <vector>
#include <string>
#include <map>

using namespace std;

const int BASE = 0x00100000;
static int accum_base = BASE;
static int stack_top = 0x10000000;

class Node {
public:
    virtual void print(int indent) = 0;
    virtual void print() = 0;
};


class Expr : public Node {
public:
  virtual int eval_print(int indent) = 0;
  virtual void print_raw() = 0;
};

class Type;

static map<string, int> var_map;
static map<string, Type*> type_map;
static map<string, int> func_map;

static bool used_temp_reg[16];

static int jump_accum = 0;

static int find_unused_reg() {
  for (int i = 0; i < 16; ++i) 
    if (!used_temp_reg[i]) {
      used_temp_reg[i] = true;
      return i;
    }
  return -1;
}

static void release_reg(int reg_id) {
  used_temp_reg[reg_id] = false;
}

const int REAL_WIDTH = 8, INTEGER_WIDTH = 4;

class Op : public Node {
public:
    string op;
    Op(const char* _op) : op(_op) {}

    void print (int indent) {
        cout << string(indent, ' ') << "operator " << op << endl;
    }

    string print_raw() {
      if (op.compare("+") == 0)
        return "add";
      else if (op.compare("-") == 0)
        return "minus";
      else if (op.compare("*") == 0)
        return "mult";
      else if (op.compare("/") == 0)
        return "div";
      else if (op.compare("==") == 0)
        return "eq";
      else if (op.compare("<>") == 0)
        return "neq";
      return op;
    }

    void print() {}
};

class Id : public Node {
public:
    string id;
    Id(const char* _id) : id(_id) {
    }

    void print (int indent) {
        cout << string(indent, ' ') << "indentifier:"<<id<<endl;
    }
    void print() {
      cout << id ;
    }
};

class Number : public Node {
public:
    string repr;
    int type;
    Number(const char* _repr) : repr(_repr) {}

    void print (int indent) {
        cout << string(indent, ' ') << "number " << repr << endl;
    }

    void print() {
      if (type == 0) {
        double d = stod(repr);
        cout << "0x";
        for (int *i = (int*)&d, _ = 0; _ < 2; ++_, ++i)
          printf("%08x", *i);
      }
      else {
        int i = stoi(repr);
        printf("%08x", i);
      }
    }
};

class String : public Node {
public:
    string str;
    String(const char* _str) : str(_str) {}

    void print (int indent) {
        cout << string(indent, ' ') << str << endl;
        //cout << string(indent, ' ') << "string literal " << str << endl;
    }

    void print() {
    }
};

template <class T> 
class Multi: public Node { 
    // note because of the way the parse works, the order of nodes is backward
    // of its syntactic order
public:
    vector<T*> nodes;
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

    void print() {
    }
};


class Lvalue: public Node {
};

class IdLvalue: public Lvalue {
public:
    Id* id;
    IdLvalue(const Id& _id) {
      id = new Id(_id);
    }

    int cal_address() {
      return var_map[id->id];
    }

    void print (int indent) {
        cout << string(indent, ' ') << "id lvalue" << endl;
        id->print(indent+4);
    }

    void print() {
      id->print();
    }
};

class Expr;

class ArrayLvalue : public Lvalue {
public:
    Lvalue* lval;
    Expr* expr;
    ArrayLvalue(Lvalue* _lval, Expr* _expr) : lval(_lval), expr(_expr) {}

    void print (int indent) {
        cout << string(indent, ' ') << "array lvalue" << endl;
        cout << string(indent+2, ' ') << "array" << endl;
        lval->print(indent+4);
        cout << string(indent+2, ' ') << "index" << endl;
        ((Node*)expr)->print(indent+4);
    }

    void print() {
    }
};

class RecordLvalue: public Lvalue {
public:
    Lvalue* lval;
    Id* id;
    RecordLvalue(Lvalue* _lval, Id* _id) : lval(_lval), id(_id) {}

    void print (int indent) {
        cout << string(indent, ' ') << "record lvalue" << endl;
        cout << string(indent+2, ' ') << "record" << endl;
        lval->print(indent+4);
        cout << string(indent+2, ' ') << "member" << endl;
        id->print(indent+4);
    }

    void print() {
        //for (int i = 0; i <
    }
};

class CompValue: public Node {
public:
    Id* id;
    Expr* expr;
    CompValue(Id* _id, Expr* _expr) : id(_id), expr(_expr) {}

    void print (int indent) {
        cout << string(indent, ' ') << "member" << endl;
        id->print(indent+2);
        cout << string(indent, ' ') << "value" << endl;
        ((Node*)expr)->print(indent+4);
    }

    void print_raw() {
      expr->print_raw();
    }

    void print() {
    }
};

class ArrayValue: public Node {
public:
  virtual void print_raw() = 0;
};

class SimpleArrayValue : public ArrayValue {
public:
    Expr* expr;
    SimpleArrayValue(Expr* _expr) : expr(_expr) {}
   
    void print (int indent) {
        ((Node*)expr)->print(indent);
    }

    void print_raw() {
      expr->print_raw();
    }

    void print() {
    }
};

class OfArrayValue : public ArrayValue {
public:
    Expr* left;
    Expr* right;
    OfArrayValue(Expr* _left, Expr* _right): left(_left), right(_right) {}

    void print (int indent) {
        ((Node*)left)->print(indent+4);
        cout << string(indent+2, ' ') << "of" << endl;
        ((Node*)right)->print(indent+4);
    }

    void print_raw() {
    }

    void print() {
    }
}; 

class NumberExpr: public Expr {
public:
    Number* n;
    int type;
    NumberExpr(Number* _n, int _type) : n(_n), type(_type) {
      n->type = type;
    }

    void print (int indent) {
      n->print(indent);
    }

    void print() {
      n->print();
    }

    void print_raw() {
      print();
    }

    int eval_print(int indent) {
      int reg_id = find_unused_reg();
      cout << string(indent, ' ') << "load $t" << reg_id;
      cout << " #";
      n->print();
      cout << endl;
      return reg_id;
    }
};

class LvalueExpr : public Expr {
public:
    Lvalue* lval;
    LvalueExpr(Lvalue* _lval) : lval(_lval) {}

    void print (int indent) {
        lval->print(indent);
    }

    void print () {
        lval->print();
    }

    void print_raw() {
        print();
    }

    int eval_print(int indent) {
      int reg_id = find_unused_reg();
      cout << string(indent, ' ') << "load $t" << reg_id << " ";
      cout <<  ((IdLvalue*)this->lval)->cal_address() << endl;
      return reg_id;
    }
};

class UnaryOpExpr : public Expr {
public:
    Op* op;
    Expr* expr;
    UnaryOpExpr(Op* _op, Expr* _expr) : op(_op), expr(_expr) {}

    void print (int indent) {
        cout << string(indent, ' ') << "unary operator expression" << endl;
        op->print(indent+2);
        expr->print(indent+2);
    }

    void print() {
    }

    void print_raw() {
      cout << "error" << endl;
    }

    int eval_print(int indent) {
      int r_reg_id = expr->eval_print(indent);
      int l_reg_id = find_unused_reg();
      cout << string(indent, ' ') << op->print_raw() << " #0 $t" << r_reg_id << " $t" << l_reg_id << endl;
      release_reg(r_reg_id);
      return l_reg_id;
    }
};

class BinOpExpr : public Expr {
public:
    Op* op;
    Expr* left;
    Expr* right;
    BinOpExpr(Op* _op, Expr* _left, Expr* _right) 
        :op(_op), left(_left), right(_right) {}

    void print (int indent) {
        cout << string(indent, ' ') << "binary operator expression" << endl;
        left->print(indent+4);
        op->print(indent+2);
        right->print(indent+4);
    }

    void print_raw() {
      cout << "error" << endl;
    }

    int eval_print(int indent) {
      int r_reg_id0 = left->eval_print(indent), r_reg_id1 = right->eval_print(indent);
      int l_reg_id = find_unused_reg();
      cout << string(indent, ' ') << op->print_raw() << " $t" << r_reg_id0 << " $t" << r_reg_id1 << " $t" << l_reg_id << endl;
      release_reg(r_reg_id0);
      release_reg(r_reg_id1);
      return l_reg_id;
    }

    void print() {
    }
};

class CallExpr : public Expr {
public:
    Id* id;
    Multi<Expr>* params; //nullable
    CallExpr(Id* _id, Multi<Expr>* _params) : id(_id), params(_params) {}

    void print (int indent) {
      cout << params << endl;
      /*
      for (int i = 0; i < params->nodes.size(); ++i) {
        cout << "push "; 
        //params->print();
        cout << endl;
      }
      */
      // call
      cout << id << endl;
      //cout << "j l" << func_map[id->id] << endl;
      /*
        cout << string(indent, ' ') << "function call" << endl;
        cout << string(indent+2, ' ') << "function id" << endl;
        id->print(indent+4);
        if  (params) {
            cout << string(indent+2, ' ') << "parameters" << endl;
            params->print(indent+4);
        } else {
            cout << string(indent+2, ' ') << "no parameter" << endl;
        }
        */
    }

    void print_raw() {
        cout << "error" << endl;
    }

    void print() {
    }

    int eval_print(int indent) {
      return -1;
    }
};

class RecordExpr : public Expr {
public:
    Id* id;
    Multi<CompValue>* vals;
    RecordExpr(Id* _id, Multi<CompValue>* _vals) : id(_id), vals(_vals) {}

    void print (int indent) {
        cout << string(indent, ' ') << "record expression" << endl;
        cout << string(indent+2, ' ') << "record id" << endl;
        id->print(indent+4);
        cout << string(indent+2, ' ') << "values" << endl;
        vals->print(indent+4);
    }

    void print() {
    }

    void print_raw() {
      for (int i = 0; i < vals->nodes.size(); ++i) {
        vals->nodes[i]->print_raw();
        cout << " ";
      }
    }

    int eval_print(int indent) {
      return -1;
    }
};

class ArrayExpr : public Expr {
public:
    Id* id;
    Multi<ArrayValue>* vals;
    ArrayExpr(Id* _id, Multi<ArrayValue>* _vals) : id(_id), vals(_vals) {}

    void print (int indent) {
        cout << string(indent, ' ') << "array expression" << endl;
        cout << string(indent+2, ' ') << "array id" << endl;
        id->print(indent+4);
        cout << string(indent+2, ' ') << "values" << endl;
        vals->print(indent+4);
    }

    void print_raw() {
      for (int i = 0; i < vals->nodes.size(); ++i) {
        vals->nodes[i]->print_raw();
      }
    }

    void print() {
    }

    int eval_print(int indent) {
      return -1;
    }
};

class WriteExpr : public Node {
};

class StrWriteExpr : public WriteExpr {
public:
    String* str;
    StrWriteExpr(String* _str) : str(_str) {}

    void print (int indent) {
        str->print(indent);
    }

    void print() {
    }
};

class ExprWriteExpr : public WriteExpr {
public:
    Expr* expr;
    ExprWriteExpr(Expr* _expr) : expr(_expr) {}

    void print (int indent) {
        int reg_id = expr->eval_print(indent);
        release_reg(reg_id);
    }

    void print() {
    }
};

class Stat: public Node {
};

class AssignStat: public Stat {
public:
    Lvalue* lvalue;
    Expr* expr;
    AssignStat(Lvalue* _lvalue, Expr* _expr) 
        : lvalue(_lvalue), expr(_expr) 
    {}

    void print (int indent) {
        eval_print(indent);
        /*
        cout << string(indent, ' ') << "assignment statement" << endl;
        lvalue->print(indent+2);
        cout << string(indent+2, ' ') << "value" << endl;
        expr->print(indent+4);
        */
    }

    void print() {
    }

    int eval_print(int indent) {
      int reg_id = expr->eval_print(indent);
      cout << string(indent, ' ') << "store $t" << reg_id << " " << ((IdLvalue*)lvalue)->cal_address() << endl;
      release_reg(reg_id);
      return -1;
    }
};

class CallStat: public Stat {
public:
    Id* id;
    Multi<Expr>* params; // nullable
    CallStat(Id* _id, Multi<Expr>* _params) : id(_id), params(_params) {}

    void print (int indent) {
      if (params)
        for (int i = 0; i < params->nodes.size(); ++i) {
          cout << "push "; 
          params->nodes[i]->print();
          cout << endl;
        }
      cout << string(indent, ' ') << "push current_line" << endl;
      // call
      cout << string(indent, ' ') << "j L" << func_map[id->id] << endl;
      
      /*
        cout << string(indent, ' ') << "function call statement" << endl;
        cout << string(indent+2, ' ') << "function name" << endl;
        id->print(indent+4);
        if (params) {
            cout << string(indent+2, ' ') << "parameters" << endl;
            params->print(indent+4);
        } else {
            cout << string(indent+2, ' ') << "no parameter" << endl;
        }
        */
    }

    void print() {
    }
};

class ReadStat: public Stat {
public:
    Multi<Lvalue>* lvalues;
    ReadStat(Multi<Lvalue>* _lvalues) : lvalues(_lvalues) {}

    void print (int indent) {
        cout << string(indent, ' ') << "read statement" << endl;
        lvalues->print(indent+2);
    }

    void print() {
    }
};

class WriteStat: public Stat {
public:
    Multi<WriteExpr>* write_params; // nullable
    WriteStat(Multi<WriteExpr>* _write_params) : write_params(_write_params) {}

    void print (int indent) {
        cout << string(indent, ' ') << "PRINT ";
        write_params->print(indent);
        //cout << string(indent, ' ') << "PRINT $t0" << endl;
    }

    void print() {
    }
};

class ElseIf : public Node {
public:
    Expr* cond;
    Multi<Stat>* then;
    ElseIf(Expr* _cond, Multi<Stat>* _then) : cond(_cond), then(_then) {}

    void print (int indent) {
        int jump_id = jump_accum;
        jump_accum += 1;
        int reg_id = cond->eval_print(indent);
        cout << string(indent, ' ') << "jne $t" << reg_id << " L" << jump_id << endl;
        for (int i = 0; i < then->nodes.size(); ++i)
            then->nodes[i]->print(indent);
        cout << string(indent, ' ') << "L" << jump_id << ": " << endl;
    }

    void print() {
    }
};

class IfStat: public Stat {
public:
    Expr* cond;
    Multi<Stat>* then;
    Multi<ElseIf>* elseif;
    Multi<Stat>* else_; // nullable
    IfStat(Expr* _cond, Multi<Stat>* _then, Multi<ElseIf>* _elseif, Multi<Stat>* _else)
        : cond(_cond), then(_then), elseif(_elseif), else_(_else) 
    {}

    void print (int indent) {
        int jump_id = jump_accum;
        jump_accum += 1;
        int reg_id = cond->eval_print(indent);
        cout << string(indent, ' ') << "jne " << reg_id << " L" << jump_id << endl;
        for (int i = 0; i < then->nodes.size(); ++i)
            then->nodes[i]->print(indent);
        cout << string(indent, ' ') << "L" << jump_id << ": " << endl;
        if (elseif) {
          elseif->print(indent);
        }
        if (else_) {
          else_->print(indent);
        }
 
        /*
        cout << string(indent, ' ')<< "if statement" << endl;
        cout << string(indent+2, ' ') <<"condition" << endl;
        cond->print(indent+4);
        cout << string(indent+2, ' ') <<"then" << endl;
        then->print(indent+4);
        if (elseif && !elseif->empty()) {
            //cout << string(indent+2, ' ') <<"elseif" << endl;
            elseif->print(indent+4);
        }
        if (else_) {
            cout << string(indent+2, ' ') <<"else" << endl;
            else_->print(indent+4);
        }
        */
    }

    void print() {
    }
};

class WhileStat: public Stat {
public:
    Expr* cond;
    Multi<Stat>* body;
    WhileStat(Expr* _cond, Multi<Stat>* _body) : cond(_cond), body(_body) {};

    void print (int indent) {
        int jump_id0 = jump_accum, jump_id1 = jump_accum + 1;
        jump_accum += 2;
        cout << string(indent - 2, ' ') << "L" << jump_id0 << ": " << endl;
        cond->eval_print(indent);
        body->print(indent);
        cout << string(indent - 2, ' ') << "L" << jump_id1 << ": " << endl;
    }

    void print() {
    }
};

class LoopStat: public Stat {
public:
    Multi<Stat>* body;
    LoopStat(Multi<Stat>* _body) : body(_body) {}

    void print (int indent) {
        cout << string(indent, ' ') << "loop" << endl;
        body->print(indent+2);
    }

    void print() {
    }
};

class ForStat: public Stat {
public:
    Id* id;
    Expr* from;
    Expr* to;
    Expr* by; // nullable
    Multi<Stat>* body;
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

    void print() {
    }
};

class ExitStat: public Stat {
public:
    void print (int indent) {
        cout << string(indent, ' ') << "exit statement" << endl;
    }

    void print() {
    }
};

class ReturnStat: public Stat {
    Expr* val; //nullable
public:
    ReturnStat(Expr* _val) : val(_val) {}
    void print (int indent) {
      int reg_id = val->eval_print(indent);
      cout << string(indent, ' ') << "load $s0 " << "$t" << reg_id << endl;
      /*
        if (val) {
            cout << string(indent, ' ') << "return value" << endl;
            val->print(indent+4);
        } else {
            cout << string(indent, ' ') << "return statement" << endl;
        }
        */
    }

    void print() {
    }
};

class Type;

class Type : public Node {
  public:
  virtual int cal_size() = 0;
};

class UserType : public Type {
public:
    Id* id;
    UserType(Id* _id)  : id(_id) {
    }

    void print (int indent) {
        //cout << string(indent, ' ') << "user defined type: " << endl;
        //id->print(indent+2);
    }

    void print() {
    }

    int cal_size() {
      Type* usertype = type_map[id->id];
      return usertype->cal_size();
    }
};

class BuiltinType : public Type {
public:
    string typename_;
    BuiltinType(string _typename) : typename_(_typename) {}

    int cal_size() {
      if (typename_.compare("REAL") == 0)
        return REAL_WIDTH;
      else if (typename_.compare("INTEGER") == 0)
        return INTEGER_WIDTH;
      else
        return 0;
    }

    void print (int indent) {
        cout << string(indent, ' ') << "type: " << typename_ << endl;
    }

    void print() {
    }
};

class ArrayType : public Type {
public:
    Type* elem_type;
    ArrayType(Type* _elem_type) : elem_type(_elem_type) {}

    void print(int indent) {
        cout << string(indent, ' ') << "array type of" << endl;
        elem_type->print(indent+2);
    }

    void print() {
    }

    int cal_size() {return 0;}
};

class Component : public Node {
public:
    Id* id;
    Type* type;
    Component(Id* _id, Type* _type) : id(_id), type(_type) {}

    void print(int indent) {
        cout << string(indent, ' ') << "component id" <<endl;
        id->print(indent+2);
        cout << string(indent, ' ') << "component type" << endl;
        type->print(indent+2);
    }

    int cal_size() {
      return type->cal_size();
    }

    void print() {
    }

};

class RecordType: public Type {
public:
    Multi<Component>* components;
    RecordType(Multi<Component>* _components) : components(_components) {}

    void print(int indent) {
        cout << string(indent, ' ') << "record type" << endl;
        components->print(indent+2);
    }

    void print() {

    }
    
    int get_offset_by_component(Id* id) {
      int size = 0;
      for (int i = 0; i < components->nodes.size(); ++i)
          if (components->nodes[i]->id->id.compare(id->id) == 0)
              return size;
          else
              size += components->nodes[i]->cal_size();
      return -1;
    }

    int cal_size() {
      int size = 0;
      for (int i = 0; i < components->nodes.size(); ++i)
        size += components->nodes[i]->cal_size();
      return size;
    }
};



class VarDecl: public Node {
public:
    Multi<Id>* ids;
    Type* type; // may be NULL
    Expr* expr;
    VarDecl(Multi<Id>* _ids, Type* _type, Expr* _expr)
        : ids(_ids), type(_type), expr(_expr) 
    {
      for (int i = 0; i < ids->nodes.size(); ++i) {
        var_map[ids->nodes[i]->id] = accum_base;
        accum_base += type->cal_size();
      }
    }

    void print (int indent) {
        for (int i = 0; i < ids->nodes.size(); ++i) {
          cout << ids->nodes[i]->id << "  " << type->cal_size() << " ";
          expr->print_raw();
          cout << endl;
        }
        /*
        ids->print(indent+4);
        if (type) {
            cout << string(indent+2, ' ') << "type" << endl;
            ((Node*)type)->print(indent+4);
        }
        cout << string(indent+2, ' ') << "initializer" << endl;
        expr->print(indent+4);
        */
    }

    void print() {
    }
};

class TypeDecl: public Node {
    Id* id;
    Type* type;
public:
    TypeDecl(Id* _id, Type* _type) : id(_id), type(_type) {
      type_map[id->id] = type;
    }
    void print (int indent) {
        //cout << string(indent, ' ') << "type declaration" << endl;
        //id->print(indent+2);
        //type->print(indent+2);
    }

    void print() {
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
        cout << string(indent, ' ') << "indentifiers" << endl;
        ids->print(indent+2);
        cout << string(indent, ' ') << "type" << endl;
        type_->print(indent+2);
    }

    void print() {
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

    void print (int indent) {
        cout << ".sub" << endl;
        cout << "L" << jump_accum << ": " << endl;
        func_map[id->id] = jump_accum;
        jump_accum += 1;
        /*
        id->print(indent+4);
        if (fpsecs) {
            cout << string(indent+2, ' ') << "formal parameters" << endl;
            fpsecs->print(indent+4);
        }
        if (type) {
            cout << string(indent+2, ' ') << "return type" << endl;
            type->print(indent+4);
        }
        */
        ((Node*)body)->print(indent);
        int reg_id0 = find_unused_reg();
        cout << string(indent, ' ') << "pop $t" << reg_id0 << endl;
        int reg_id1 = find_unused_reg();
        if (fpsecs)
          for (int i = 0; i < fpsecs->nodes.size(); ++i)
            cout << string(indent, ' ') << "pop $t" << reg_id1 << endl;
        release_reg(reg_id0);
        release_reg(reg_id1);
        cout << string(indent, ' ') << "jr $t" << reg_id0 << endl;
    }

    void print() {
    }
};

class Decl: public Node {
    string type; // "var" "type" "procedure"
    Multi<Node>* block;
public:
    Decl(const char* _type, Multi<Node>* _block) : type(_type), block(_block) {}

    void print (int indent) {
        block->print(indent);
    }

    void print() {
    }
};

class Body: public Node {
    Multi<Decl>* decls;
    Multi<Stat>* stats;
public:
    Body(Multi<Decl>* _decls, Multi<Stat>* _stats): decls(_decls), stats(_stats) {}
    void print (int indent) {
        cout << string(indent, ' ')<< ".data" << endl;
        decls->print(indent+2);
        cout << string(indent, ' ') << ".text" << endl;
        stats->print(indent+2);
    }

    void print() {
    }
};

class Program: public Node {
    Body* body;
public:
    Program(Body* _body) : body(_body) {}
    void print (int indent) {
        body->print(indent);
    }

    void print() {
    }
};

#endif
