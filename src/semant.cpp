#include "../src/tree.hpp"
#include "../src/symbolTable.hpp"
#include <iostream>
// 定义字符表
class SemanticAnalyzer
{
public:
    treeNode *root;
    SemanticAnalyzer(treeNode *root)
    {
        root = root;
    }

    // int check() {
    //     //清理语法树
    //     deleteNull(root);
    //     clean(root);
    //     //...进行语义分析
    //     // visit(root);
    // }

    void initialize()
    {
        // 加入int read(), void write(int) 函数

        S_symbol read = getSymbol("read", false);
        if (!read)
        {
            read = new S_symbol_("read", findSymbol("read"));
            modifySymbol("read", read);
        }
        read->binding = new E_enventry_(E_enventry_::E_funEntry);
        read->binding->ty = new Ty_ty_(Ty_ty_::Ty_int);

        S_symbol write = getSymbol("write", false);
        if (!write)
        {
            write = new S_symbol_("write", findSymbol("write"));
            modifySymbol("write", write);
        }
        write->binding = new E_enventry_(E_enventry_::E_funEntry);
        write->binding->ty = new Ty_ty_(Ty_ty_::Ty_void);
        S_symbol temp = new S_symbol_();
        temp->binding = new E_enventry_(E_enventry_::E_funEntry, new Ty_ty_(Ty_ty_::Ty_int));
        write->binding->formals.push_back(temp);
        
        blockList.push_back(Block());
    }
    void visit(treeNode *node)
    {
        printf("%s\n",node->name.c_str());
        string parentName = node->name;
        // printf("visit %s\n", parentName.c_str());
        if (parentName == "FuncDef")
        {
            // 查找是否存在重复定义
            string name = node->children[1]->name;
            S_symbol temp = setSymbol(name, false);
            if (temp->binding != NULL)
            {
                // 重复定义
                string s = "Error: function ";
                s = s + name + " has been defined\n";
                error(s);
            }
            //插入体开头
            S_beginScope();
            // 插入函数表，并处理参数
            temp->binding = getBinding(node);
            print_Func(temp);
            // printSymbol(temp);
            // 检查函数体
            visit(node->children[node->childrenNum - 1]->children[0]);
            S_endScope();
        }
        else if (parentName == "ReturnStmt")
        {
            // 检查返回值类型是否正确
            Ty_ty returnTy;
            if (node->childrenNum == 2)
            {
                returnTy = actual_ty(node->children[1]);
            }
            else
            {
                returnTy = typeEx("VOID");
            }
            for (int i = symbolStack.size() - 1; i >= 0; i--)
            {
                if(symbolStack[i].first == marksym)continue;
                if (symbolStack[i].first->binding->kind == E_enventry_::E_funEntry)
                {
                    Ty_ty returnFun = symbolStack[i].first->binding->ty;
                    // printStack();
                    // printf("name: %s, returnTy: %s\n",symbolStack[i - 1].first->name.c_str(), transTyKind(returnFun->kind).c_str());
                    if (compareTyKind(returnTy, returnFun))
                        error("Error: invalid return type\n");
                }
            }
            translate_stmt(node);
            arrayStack.clear();
        }
        else if (parentName == "VarDecl")
        {
            // 查找是否存在重复定义
            string type = node->children[0]->name;
            treeNode *parentNode = node;
            for (int i = 1; i < parentNode->childrenNum; i++)
            {
                if(parentNode->children[i]->name == "multiVar"){
                    parentNode = parentNode->children[i];
                    i = -1;
                    continue;
                }
                string name = parentNode->children[i]->children[0]->name;
                if (checkScopeVar(name) == 1)
                {
                    string s = "ERROR: already defined";
                    s = s + " " + name + "\n";
                    error(s);
                }
                S_symbol temp = setSymbol(name, true);
                // 插入变量表
                E_enventry e = new E_enventry_(E_enventry_::E_varEntry, typeEx(type));
                temp->binding = e;
                //检查是否为全局变量
                int isGlobal = is_global(temp->name);
                if(isGlobal)
                    blockList[0].IRList.push_back(IRNode("GLOBAL", 1, temp->name.c_str()));
                if (parentNode->children[i]->childrenNum > 1)
                {
                    // 检查是不是数组
                    if (parentNode->children[i]->children[1]->name == "array")
                    {
                        //是数组
                        e->ty->kind = Ty_ty_::Ty_array;
                        treeNode *t = parentNode->children[i]->children[1];
                        while (1)
                        {
                            e->ty->array.push_back(atoi(t->children[0]->name.c_str()));
                            if (t->childrenNum == 1)
                                break;
                            t = t->children[1];
                        }
                        int size = 1;
                        for(int i = 0; i < e->ty->array.size(); i++){
                            size = size * e->ty->array[i];
                        }
                        if(!isGlobal){
                            blockList[0].IRList.push_back(IRNode("GLOBAL", 1, getVarTemp(temp->name).c_str()));
                        }
                        // if(!isGlobal){
                        //     blockList.back().IRList.push_back(IRNode("DEC", 2, getVarTemp(temp->name).c_str(), ("#"+to_string(size * 4)).c_str()));
                        //     //向数组填充0
                        //     label label1 = label();
                        //     label label2 = label();
                        //     label label3 = label();
                        //     Temp t1, t2;
                        //     blockList.back().IRList.push_back(IRNode("ASSIGN", 2, t1.name.c_str(), "#0"));
                        //     blockList.back().IRList.push_back(IRNode("ASSIGN", 2, t2.name.c_str(), ("#"+to_string(size*4)).c_str()));
                        //     blockList.back().IRList.push_back(IRNode("LABEL", 1, label1.name.c_str()));
                        //     blockList.back().IRList.push_back(IRNode("IF", 4, t1.name.c_str(), "<", t2.name.c_str(), label2.name.c_str()));
                        //     blockList.back().IRList.push_back(IRNode("GOTO", 1, label3.name.c_str()));
                        //     blockList.back().IRList.push_back(IRNode("LABEL", 1, label2.name.c_str()));
                        //     //循环体
                        //     Temp t4;
                        //     blockList.back().IRList.push_back(IRNode("ASSIGN", 4, t1.name.c_str(), t1.name.c_str(), "+", "#4"));
                        //     blockList.back().IRList.push_back(IRNode("ASSIGN", 4, t4.name.c_str(), getVarTemp(temp->name).c_str(), "+", t1.name.c_str()));
                        //     blockList.back().IRList.push_back(IRNode("ASSIGN", 2, ("*"+t4.name).c_str(), "x0"));
                        //     blockList.back().IRList.push_back(IRNode("GOTO", 1, label1.name.c_str()));
                        //     blockList.back().IRList.push_back(IRNode("LABEL", 1, label3.name.c_str()));
                        // }
                            
                        if (parentNode->children[i]->childrenNum > 2)
                        {
                            //有数组初始化
                            if (parentNode->children[i]->children[2]->name == "initArrayList")
                            {
                                vector<int> index(e->ty->array.size(), 0);
                                index[index.size() - 1] = -1;
                                int oldIndexNum = 0;
                                calcArrayInit(getVarTemp(temp->name),e->ty->array, index, parentNode->children[i]->children[2], oldIndexNum);
                                int size = get_array_size(e->ty->array);
                                // if(isGlobal && oldIndexNum < size){
                                //     for(int i = oldIndexNum; i < size; i++){
                                //         blockList.back().IRList.push_back(IRNode("WORD", 1, "#0"));
                                //     }
                                // }
                                if(oldIndexNum < size){
                                    for(int i = oldIndexNum; i < size; i++){
                                        blockList[0].IRList.push_back(IRNode("WORD", 1, "#0"));
                                    }
                                }
                            }
                            else if (parentNode->children[i]->children[2]->name == "Exp")
                            {
                                error("ERROR: array initialize should be initializer list\n");
                            }
                        }else{
                            //无数组初始化
                            // if(isGlobal){
                            //     //给所有位置0
                            //     int size = get_array_size(e->ty->array);
                            //     for(int i = 0; i < size; i++){
                            //         blockList.back().IRList.push_back(IRNode("WORD", 1, "#0"));
                            //     }
                            // }
                            //给所有位置0
                            int size = get_array_size(e->ty->array);
                                for(int i = 0; i < size; i++){
                                    blockList[0].IRList.push_back(IRNode("WORD", 1, "#0"));
                                }
                        }
                    }
                    else
                    {
                        // 不是数组
                        // 变量初始化
                        if (actual_ty(parentNode->children[i]->children[1])->kind != e->ty->kind)
                        {
                            string s = "ERROR: wrong definition" + parentNode->children[i]->children[0]->name + "\n";
                            error(s);
                        }
                        if(!isGlobal){
                            Temp t;
                            t.lockTemp();
                            translate_exp(parentNode->children[i]->children[1], t);
                            blockList.back().IRList.push_back(IRNode("ASSIGN", 2, getVarTemp(parentNode->children[i]->children[0]->name).c_str(), t.name.c_str()));
                            t.deLock();
                        }else{
                            blockList.back().IRList.push_back(IRNode("WORD", 1, ("#"+to_string(atoi(parentNode->children[i]->children[1]->children[0]->children[0]->name.c_str()))).c_str()));
                        }
                        
                    }
                    
                }else if(isGlobal){
                    blockList[0].IRList.push_back(IRNode("WORD", 1, "#0"));
                }

                temp->binding = e;
            }
        }
        else if (parentName == "AssignStmt")
        {
            // 查找是否已经定义
            if (!checkScopeVar(node->children[0]->children[0]->name))
            {
                string s = "ERROR: assign not defined: " + node->children[0]->children[0]->name + "\n";
                error(s);
            }
            S_symbol temp = getSymbol(node->children[0]->children[0]->name, true);
            // 检查左右类型是否匹配
            Ty_ty left = actual_ty(node->children[0]);
            Ty_ty right = actual_ty(node->children[1]);
            if (left->kind != right->kind || left->kind == Ty_ty_::Ty_array)
            {
                error("Error: type not match\n");
            }
            translate_stmt(node);
        }
        else if (parentName == "FuncCall")
        {
            // 查找函数是否存在
            // 检查参数是否匹配
            actual_ty(node);
            Temp t(-1);
            translate_exp(node, t);
        }
        else if (parentName == "MulExp" || parentName == "AddExp")
        {
            actual_ty(node);
            Temp t;
            translate_exp(node, t);
        }
        else if (parentName == "IfStmt")
        {
            if ((actual_ty(node->children[0])->kind != Ty_ty_::Ty_bool) && (actual_ty(node->children[0])->kind != Ty_ty_::Ty_int))
            {
                error("ERROR: if condition not bool\n");
            }
            label end;
            translate_else(end, node);
            blockList.back().IRList.push_back(IRNode("LABEL", 1, end.name.c_str()));
        }
        else if (parentName == "WhileStmt")
        {
            if (actual_ty(node->children[0])->kind != Ty_ty_::Ty_bool)
            {
                error("ERROR: while condition not bool\n");
            }
            label label1 = label();
            label label2 = label();
            label label3 = label();
            blockList.back().IRList.push_back(IRNode("LABEL", 1, label1.name.c_str()));
            translate_cond(node->children[0], label2, label3);
            blockList.back().IRList.push_back(IRNode("LABEL", 1, label2.name.c_str()));
            visit(node->children[1]);
            blockList.back().IRList.push_back(IRNode("GOTO", 1, label1.name.c_str()));
            blockList.back().IRList.push_back(IRNode("LABEL", 1, label3.name.c_str()));
        }
        else if (parentName == "enterBlock")
        {
            S_beginScope();
            for (int i = 0; i < node->childrenNum; i++)
            {
                visit(node->children[i]);
            }
            S_endScope();
        }
        else if (parentName == "Exp")
        {
            actual_ty(node);
            Temp t(-1);
            translate_exp(node, t);
        }
        else
        {
            cout << "not proccessed node: " << parentName << endl;
            for (int i = 0; i < node->childrenNum; i++)
            {
                visit(node->children[i]);
            }
        }
    }

    void deleteNull(treeNode *node)
    {
        for (int i = 0; i < node->childrenNum; i++)
        {
            if (node->children[i] == NULL)
            {
                for (int j = i; j < node->childrenNum - 1; j++)
                {
                    node->children[j] = node->children[j + 1];
                }
                node->childrenNum--;
                i--;
                continue;
            }
            deleteNull(node->children[i]);
        }
    }
    void clean(treeNode *node)
    {
        for (int i = 0; i < node->childrenNum; i++)
        {
            // 删除无用中间节点
            if (node->children[i]->name != "enterBlock" && node->children[i]->name != "initArrayList" && node->children[i]->name != "Exp" && node->children[i]->childrenNum == 1 && node->children[i]->children[0]->childrenNum != 0)
            {
                treeNode *temp = node->children[i]->firstChild();
                treeNode *oldTemp;
                while (temp->childrenNum == 1 && temp->firstChild()->childrenNum != 0 && temp->name != "Exp" && temp->name != "initArrayList" && temp->name != "enterBlock")
                {
                    oldTemp = temp;
                    temp = temp->firstChild();
                    free(oldTemp);
                }
                oldTemp = node->children[i];
                node->children[i] = temp;
                free(oldTemp);
            }
            else if (node->children[i]->childrenNum == 0)
            {
                continue;
            }
            clean(node->children[i]);
        }
    }

    void translate_else(label &end, treeNode *stmt){
        label label1;
        string labelName;
        if(stmt->childrenNum>2){
            //含else
            label label2;
            labelName = label2.name;
            translate_cond(stmt->children[0], label1, label2);
        }else{
            //不含else
            translate_cond(stmt->children[0], label1, end);
        }
        blockList.back().IRList.push_back(IRNode("LABEL", 1, label1.name.c_str()));
        visit(stmt->children[1]);
        blockList.back().IRList.push_back(IRNode("GOTO", 1, end.name.c_str()));
        if(stmt->childrenNum>2){
            //有else
            blockList.back().IRList.push_back(IRNode("LABEL", 1, labelName.c_str()));
            if(stmt->children[3]->name == "IfStmt"){
                translate_else(end, stmt->children[3]);
            }else{
                visit(stmt->children[3]);
                blockList.back().IRList.push_back(IRNode("GOTO", 1, end.name.c_str()));
            }
        }
    }
};