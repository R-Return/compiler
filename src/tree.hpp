#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
extern char *yytext;
extern int yylineno; // 提供当前行数信息
using namespace std;


class IRNode
{
public:
	IRNode() {}
	IRNode(string name, int childrenNum, ...);
	void insertChild(string child);
	string name;
	int childrenNum = 0;
	vector<string> children;
};
struct S_symbol_;      // 声明 S_symbol_ 结构体，使其可以在后续中被引用
typedef struct S_symbol_ *S_symbol;   // 声明S_symbol类型为S_symbol_结构体的指针。
// 函数节点
class funcNode
{
public:
	funcNode() {}
	funcNode(string name) : name(name) {}

	string name;	   // 函数名
	vector<pair<S_symbol, string>> paralist; // 记录形参列表
};

// block的内容
class Block
{
public:
	Block() {}
	Block(funcNode *func): func(func), isfunc(true) {}

	funcNode *func = NULL;			   // 如果是函数，记录函数名
	bool isfunc = false;			   // 记录是否是函数
	vector<IRNode> IRList;		   // 记录IR列表
	vector<pair<S_symbol, string>> symbolList;	   // 记录符号表
	// map<string, varNode *> varMap;	   // 变量的map
    // map<string, varNode *> arrayMap;
	// string breakLabel;
	// bool canBreak = false;

};

void printInstruction(ofstream &os);

class treeNode
{
public:
    string name;
	string content = "";
	int line = -1; // 所在代码行数
	int childrenNum = 0;
	vector<treeNode *> children; // sibling 是邻居的意思，这个vector用来存孩子

	treeNode(string name, int childrenNum, ...);
	void printTree(int degree, ofstream &os);

	treeNode *firstChild();
	treeNode *lastChild();
	treeNode *getChild(int index);
};
extern treeNode *root;
extern vector<Block> blockList;