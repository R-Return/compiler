#pragma once

#include <string>
#include <stack>
#include <unordered_map>
#include <iostream>
#include "../src/tree.hpp"
using namespace std;

#define SIZE 109



struct Ty_ty_;         // 声明 Ty_ty_ 结构体
typedef struct Ty_ty_ *Ty_ty;  // 声明Ty_ty 类型为Ty_ty_结构体的指针
struct Ty_ty_
{
    enum Kind
    {
        Ty_int,
        Ty_bool,
        Ty_array,
        Ty_void
    } kind;
    vector<int> array;
    Ty_ty_():array(){}
    Ty_ty_(Kind k):array(){
        kind = k;
    }
};
typedef struct E_enventry_ *E_enventry;
typedef vector<S_symbol> S_symbolList;
struct E_enventry_
{
    enum Kind
    {
        E_varEntry,
        E_funEntry
    } kind;
    Ty_ty ty;   //var值或返回值
    S_symbolList formals;  //形参
    E_enventry_():formals(){}
    E_enventry_(Kind k):formals(){
        kind = k;
        ty = NULL;
    }
    E_enventry_(Kind k, Ty_ty t):formals(){
        kind = k;
        ty = t;
    }
};


struct S_symbol_
{
    string name;
    E_enventry binding;
    S_symbol next;

    S_symbol_(){
        name = "";
        binding = NULL;
        next = NULL;
    
    }
    S_symbol_(string name){
        this->name = name;
        binding = NULL;
        next = NULL;
    }
    S_symbol_(string name, S_symbol next){
        this->name = name;
        binding = NULL;
        this->next = next;
    }
};
extern vector<S_symbol> symbolTable;
S_symbol findSymbol(string name);
void modifySymbol(string name, S_symbol s);
S_symbol getSymbol(string name, bool isVar);
S_symbol setSymbol(string name, bool isVar);
Ty_ty typeEx(string name);
int checkScopeVar(string name);
E_enventry getBinding(treeNode* treeNode);//获取binding

int compareTyKind(Ty_ty left, Ty_ty right);
void calcArrayInit(string arrayName, vector<int> &a, vector<int> &index, treeNode *t, int &oldIndexNum);
Ty_ty actual_ty(treeNode *t);

//打印
string transVarKind(E_enventry_::Kind kind);
string transTyKind(Ty_ty_::Kind kind);
// void printSymbol(S_symbol s);
void printStack();
//报错
void error(string msg);
//辅助栈
void S_beginScope();
void S_endScope();
static int lock[23] = {0};
//translate
class Temp{
    static int count;
public:
    int index;
    string name;
    string printName;

    Temp(){
        int runout = 0;
        do{
            count = (++count) % 23;
            runout++;
            if(runout > 25){
                error("Temp run out");
            }
        }while(lock[count]);
        this->index = count;
        if(count < 7){
            this->name = "-a" + to_string(this->index+1);
        }else if(count < 12){
            this->name = "-t" + to_string(count-5);
        }else{
            this->name = "-s" + to_string(count-11);
        }
        this->printName = this->name.substr(1);
        
    }
    Temp(int index){
        this->index = index;
        this->name = "-a" + to_string(index);
        this->printName = this->name.substr(1);
    }
    Temp(string name){
        this->index = -2;
        this->name = name;
        this->printName = name;
    }
    Temp(string name, int index){
        this->name = "-"+name;
        this->printName = name;
        this->index = index;
    }
    void lockTemp(){
        lock[index] = 1;
    }
    void deLock(){
        lock[index] = 0;
    }
};

class Temp1{
    static int count;
public:
    int index;
    string name;
    string printName;

    Temp1(){
        this->index = ++count;
        int tempIndex = index%2;
        this->name = "-t" + to_string(tempIndex);
        this->printName = this->name.substr(1);
    }
};

class label{
    static int count;
public:
    int index;
    string name;

    label(){
        this->index = ++count;
        this->name = "label" + to_string(index);
    }
    label(int index){
        this->index = index;
        this->name = "label" + to_string(index);
    }
};
void print_Func(S_symbol exp);
void translate_exp(treeNode *exp, Temp& place);
void translate_stmt(treeNode *Stmt);
void translate_cond(treeNode *Exp, label &label_true, label &label_false);
int get_array_int_index(vector<int> &a, vector<int>&index);
void get_array_index(vector<int> &a, vector<int>&index, Temp &place);
void get_array_index(string &arrayName, vector<string>&index, Temp &place, int &isArray);
void get_array_address(string &arrayName, string &arrayIndex, Temp& place);
void get_array_address1(string arrayName, treeNode *exp, Temp &place, int &isArray);
int get_array_size(vector<int> &array);
int is_global(string name);
string getVarTemp(string name);
string insertSymbolRecord(string name);
int is_in_global_mode();
int is_single_array(string name);

//辅助栈
extern vector<pair<S_symbol, string>> symbolStack;
extern S_symbol marksym;
extern unordered_map<string, int> symbolRecord;
extern vector<string> arrayStack;

//转换目标代码
void printTargetInstruction(ofstream &os);