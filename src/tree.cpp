#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "tree.hpp"
using namespace std;
vector<Block> blockList;

IRNode::IRNode(string name, int childrenNum, ...)
{
	this->name = name;
	this->childrenNum = childrenNum;
	va_list valist;
	va_start(valist, childrenNum);
	if (childrenNum > 0)
	{
		for (int i = 0; i < childrenNum; i++)
		{
			const char* child = va_arg(valist, const char*);
			children.push_back(child);
		}
	}
}

void IRNode::insertChild(string child)
{
	children.push_back(child);
	childrenNum++;
}

void printInstruction(ofstream &os){
	for (int i = 0; i < blockList.size(); i++)
	{
		if (blockList[i].isfunc)
		{
			os << "FUNCTION "<<blockList[i].func->name << ":\n";
			for(int j = 0; j < blockList[i].func->paralist.size(); j++){
				os << "    PARAM " << blockList[i].func->paralist[j].second << "\n";
			}
		}
		for (int j = 0; j < blockList[i].IRList.size(); j++)
		{
			if(blockList[i].IRList[j].name == "ASSIGN"){
				os << "    "<< blockList[i].IRList[j].children[0] << " = ";
				for(int k = 1; k < blockList[i].IRList[j].children.size(); k++){
					os << blockList[i].IRList[j].children[k];
					if(k != blockList[i].IRList[j].children.size() - 1){
						os << " ";
					}
				}
				os << "\n";
			}
			else if(blockList[i].IRList[j].name == "LABEL"){
				os << "  LABEL "<<blockList[i].IRList[j].children[0] << ":\n";
			}
			else if(blockList[i].IRList[j].name == "GOTO"){
				os << "    GOTO " << blockList[i].IRList[j].children[0] << "\n";
			}
			else if(blockList[i].IRList[j].name == "IF"){
				os << "    IF " << blockList[i].IRList[j].children[0] << " " << blockList[i].IRList[j].children[1] << " " << blockList[i].IRList[j].children[2] << " GOTO " << blockList[i].IRList[j].children[3] << "\n";
			}
			else if(blockList[i].IRList[j].name == "RETURN"){
				if(blockList[i].IRList[j].childrenNum == 0){
					os << "    RETURN\n";
				}else{
					os << "    RETURN " << blockList[i].IRList[j].children[0] << "\n";
				}
					
			}
			else if(blockList[i].IRList[j].name == "DEC"){
				os << "    DEC " << blockList[i].IRList[j].children[0] << " " << blockList[i].IRList[j].children[1] << "\n";
			}
			else if(blockList[i].IRList[j].name == "CALL"){
				os << "    " << blockList[i].IRList[j].children[0] << " = CALL " << blockList[i].IRList[j].children[1] << "\n";
			}
			else if(blockList[i].IRList[j].name == "GLOBAL"){
				os << "GLOBAL " << blockList[i].IRList[j].children[0] << ":\n";
			}
			else if(blockList[i].IRList[j].name == "WORD"){
				os << "    .WORD " << blockList[i].IRList[j].children[0] << "\n";
			}
			else if(blockList[i].IRList[j].name == "FUNCALL"){
				for(int k = 2; k < blockList[i].IRList[j].children.size(); k++){
					os << "    ARG " << blockList[i].IRList[j].children[k] << "\n";
				}
				os << "    ";
				if(blockList[i].IRList[j].children[0]!= "-1"){
					os << blockList[i].IRList[j].children[0]<< " = ";
				}
				os << "CALL " << blockList[i].IRList[j].children[1] << "\n";
			}
		}
		os << "\n";
	}
}

treeNode::treeNode(string name, int childrenNum, ...) : name(name), childrenNum(childrenNum)
{
	va_list valist;
	va_start(valist, childrenNum);
	if (childrenNum > 0)
	{
		for (int i = 0; i < childrenNum; i++)
		{
			treeNode *child = va_arg(valist, treeNode *);
			children.push_back(child);
		}
		line = firstChild()->line;
		if (childrenNum == 1)
		{
			content = firstChild()->content;
		}
	}
	else if (childrenNum == 0)
	{
		int line = va_arg(valist, int);
		this->line = line;
		if (name == "CONSTANT_INT")
		{
			int value;
			if (strlen(yytext) > 1 && yytext[0] == '0' && yytext[1] != 'x')
			{
				sscanf(yytext, "%o", &value); // 8进制整数
			}
			else if (strlen(yytext) > 1 && yytext[1] == 'x')
			{
				sscanf(yytext, "%x", &value); // 16进制整数
			}
			else
				value = atoi(yytext); // 10进制整数
			content = to_string(value);
		}
		else
		{
			content = yytext;
		}
	}
}

treeNode *treeNode::firstChild()
{
	if (childrenNum == 0)
	{
		return NULL;
	}
	return children[0];
}

treeNode *treeNode::lastChild()
{
	if (childrenNum == 0)
	{
		return NULL;
	}
	return children.back();
}

treeNode *treeNode::getChild(int index)
{
	if (index >= childrenNum || index < 0)
	{
		return NULL;
	}
	return children[index];
}


void treeNode::printTree(int degree, ofstream &os)
{
	string tab = "";
	for (int i = 0; i < degree; i++)
	{
		tab += "  ";
	}
	tab += "|--";
	if (childrenNum == 0)
	{
		os << tab << name << endl;
	}
	else
	{
		os << tab << name << "  " << childrenNum << endl;
		for (int i = 0; i < childrenNum; i++)
		{
			if(children[i] == NULL) continue;
			children[i]->printTree(degree + 1, os);
		}
	}
}