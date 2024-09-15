#include "symbolTable.hpp"
#define SIZE 109
vector<S_symbol> symbolTable(SIZE, NULL);
vector<pair<S_symbol, string>> symbolStack;
vector<string> arrayStack;

unsigned int hashIndex(string &s)
{
    unsigned int h = 0;
    for (int i = 0; i < s.size(); i++)
        h = h * 65599 + s[i];
    return h;
}

S_symbol findSymbol(string name)
{
    int index = hashIndex(name) % SIZE;
    return symbolTable[index];
}

void modifySymbol(string name, S_symbol s)
{
    int index = hashIndex(name) % SIZE;
    symbolTable[index] = s;
}
// 查找Symbol
S_symbol getSymbol(string name, bool isVar)
{
    S_symbol syms = findSymbol(name), sym;
    if (syms)
    {
        for (sym = syms; sym; sym = sym->next)
        {
            if (sym->name == name && isVar ? sym->binding->kind == E_enventry_::E_varEntry : sym->binding->kind == E_enventry_::E_funEntry){
                return sym;
            }
                
            if (!sym->next)
                break;
        }
    }
    return NULL;
}

void insertNewSymbol(S_symbol sym,string name){
    symbolStack.push_back(make_pair(sym,name));
    blockList.back().symbolList.push_back(symbolStack.back());
}
// 获得name对应symbol，如果没有，则创建
S_symbol setSymbol(string name, bool isVar)
{
    S_symbol syms = findSymbol(name), sym = NULL;
    if (syms)
    {
        for (sym = syms; sym; sym = sym->next)
        {
            if (sym->name == name && ((!isVar && sym->binding->kind == E_enventry_::E_funEntry) || (isVar && sym->binding->kind == E_enventry_::E_varEntry && checkScopeVar(name) == 1))){
                break;
            }
            if (!sym->next){
                sym = NULL;
                break;
            }
        }
    }
    if (!sym)
    {
        sym = new S_symbol_(name, findSymbol(name));
        modifySymbol(name, sym);
        if(isVar){
            if(is_in_global_mode())insertNewSymbol(sym,name);
            else insertNewSymbol(sym,insertSymbolRecord(name));
        }
        
    }
    return sym;
}

// 类型转换
Ty_ty typeEx(string name)
{
    Ty_ty result = new Ty_ty_; // Create a new object dynamically
    if (name == "INT")
    {
        *result = Ty_ty_(Ty_ty_::Ty_int);
    }
    else if (name == "VOID")
    {
        *result = Ty_ty_(Ty_ty_::Ty_void);
    }
    else
    {
        error("Error: typeEx invalid type\n");
    }
    return result;
}

// 检查变量在范围内是否被定义，0 没找到，1 在范围内，2 在范围外
int checkScopeVar(string name)
{
    int flag = 0;
    for (int i = symbolStack.size() - 1; i >= 0; i--)
    {
        if (symbolStack[i].first == marksym)
        {
            flag = 1;
            continue;
        }
        else if (symbolStack[i].first->name == name && symbolStack[i].first->binding->kind == E_enventry_::E_varEntry)
        {
            if (flag)
                return 2;
            else
                return 1;
        }
    }
    return 0;
}

int checkZero(vector<int> &a, int num)
{
    for (int i = 0; i < num; i++)
    {
        if (a[i] != 0)
        {
            return 0;
        }
    }
    return 1;
}

void arrayPlus(vector<int> &a, vector<int> &index, int degree)
{
    index[degree]++;
    if (index[degree] == a[degree])
    {
        if (degree > 0)
        {
            arrayPlus(a, index, degree - 1);
            index[degree] = 0;
        }
        else
        {
            error("ERROR: array out of bound\n");
        }
    }
}
void printIndex(vector<int> &index)
{
    for (int i = 0; i < index.size(); i++)
    {
        printf("%d ", index[i]);
    }
    printf("\n");
}

// 检查数组初始化个数是否正确
void calcArrayInit(string arrayName, vector<int> &a, vector<int> &index, treeNode *t, int &oldIndexNum)
{
    if (t->name == "initArrayList")
    {
        if (index[index.size() - 1] == -1)
        {
            // 还在最初的位置
            for (int i = 0; i < t->childrenNum; i++)
            {
                calcArrayInit(arrayName, a, index, t->children[i], oldIndexNum);
            }
        }
        else
        {
            // 不在最开始但遇到了initArrayList
            if (checkZero(index, index.size() - 1))
            {
                // 还在第一行
                if (index[index.size() - 1] != a[index.size() - 1] - 1)
                {
                    error("ERROR: array init not align\n");
                }
            }
            vector<int> oldIndex;
            index[index.size() - 1] = a[index.size() - 1] - 1;
            oldIndex = index;
            for (int i = 0; i < t->childrenNum; i++)
            {
                calcArrayInit(arrayName, a, index, t->children[i], oldIndexNum);
            }
            for (int i = oldIndex.size() - 1; i >= 0; i--)
            {
                if (oldIndex[i] == a[i] - 1)
                {
                    continue;
                }
                else
                {
                    arrayPlus(a, oldIndex, i);
                    break;
                }
            }
            if (oldIndex >= index)
            {
                index = oldIndex;
            }
            else
            {
                error("ERROR: initArray over range\n");
            }
        }
    }
    else if (t->name == "ArrayList")
    {
        for (int i = 0; i < t->childrenNum; i++)
        {
            calcArrayInit(arrayName, a, index, t->children[i], oldIndexNum);
        }
    }
    else if (t->name == "Exp")
    {
        if (actual_ty(t->children[0])->kind != Ty_ty_::Ty_int)
            error("ERROR: unknown array type\n");
        arrayPlus(a, index, index.size() - 1);
        // if(!isGlobal){
        //     Temp t3;
        //     get_array_index(a, index, t3);
        //     Temp t1;
        //     get_array_address(arrayName, t3.name, t1);
        //     Temp t2;
        //     translate_exp(t->children[0], t2);
        //     blockList.back().IRList.push_back(IRNode("ASSIGN", 2, ("*"+t1.name).c_str(), t2.name.c_str()));
        // }else{
        //     int currentIndex = get_array_int_index(a, index);
        //     for(int i = oldIndexNum; i< currentIndex; i++){
        //         blockList.back().IRList.push_back(IRNode("WORD", 1, "#0"));
        //     }
        //     blockList.back().IRList.push_back(IRNode("WORD", 1, ("#"+t->children[0]->children[0]->name).c_str()));
        //     oldIndexNum = currentIndex + 1;
        // }
        int currentIndex = get_array_int_index(a, index);
        for(int i = oldIndexNum; i< currentIndex; i++){
            blockList[0].IRList.push_back(IRNode("WORD", 1, "#0"));
        }
        blockList[0].IRList.push_back(IRNode("WORD", 1, ("#"+t->children[0]->children[0]->name).c_str()));
        oldIndexNum = currentIndex + 1;
    }
}
// 插入int型参数
S_symbol pushSymbol(treeNode *t)
{
    Ty_ty temp = typeEx(t->children[0]->name);
    if (checkScopeVar(t->children[1]->name) == 1)
    {
        string s = "ERROR: already defined";
        s = s + " " + t->children[1]->name + "\n";
        error(s);
    }
    S_symbol stemp = setSymbol(t->children[1]->name, true);
    stemp->binding = new E_enventry_(E_enventry_::E_varEntry, temp);
    return stemp;
}

// 插入array型参数
S_symbol pushSymbolArray(treeNode *t)
{
    Ty_ty temp = new Ty_ty_(Ty_ty_::Ty_array);
    temp->array.push_back(0); // 第一维
    if (checkScopeVar(t->children[1]->name) == 1)
    {
        string s = "ERROR: already defined";
        s = s + " " + t->children[1]->name + "\n";
        error(s);
    }
    if (t->childrenNum == 3)
    {
        treeNode *t1 = t->children[2];
        while (1)
        {
            temp->array.push_back(atoi(t1->children[0]->name.c_str()));
            if (t1->childrenNum == 1)
                break;
            t1 = t1->children[1];
        }
    }

    S_symbol stemp = setSymbol(t->children[1]->name, true);
    stemp->binding = new E_enventry_(E_enventry_::E_varEntry, temp);
    return stemp;
}
// travel params
void travelFuncParams(treeNode *t, E_enventry e)
{
    if (t->name == "FuncParams")
    {
        travelFuncParams(t->children[0], e);
        travelFuncParams(t->children[1], e);
    }
    else if (t->name == "FuncFParam")
    {
        e->formals.push_back(pushSymbol(t));
    }
    else if (t->name == "FuncFParamArray")
    {
        e->formals.push_back(pushSymbolArray(t));
    }
}
// 获得函数binding
E_enventry getBinding(treeNode *treeNode)
{
    E_enventry e = new E_enventry_(E_enventry_::E_funEntry);
    e->ty = typeEx(treeNode->children[0]->name); // 返回类型
    if (treeNode->childrenNum == 4)
    {
        // 有参数
        if (treeNode->children[2]->name == "FuncParams" || treeNode->children[2]->name == "FuncFParamArray")
        {
            travelFuncParams(treeNode->children[2], e);
        }
        else
        {
            e->formals.push_back(pushSymbol(treeNode->children[2]));
        }
    }
    return e;
}

int compareTyKind(Ty_ty left, Ty_ty right)
{
    if (left->kind != right->kind)
        return 1;
    if (left->kind == Ty_ty_::Ty_array)
    {
        if (left->array.size() != right->array.size())
            return 1;
        for (int i = 0; i < left->array.size(); i++){
            if(i == 0 && left->array[i] == 0)
                continue;
            if (left->array[i] != right->array[i])
                return 1;
        }
            
    }
    return 0;
}
void printTyVec(vector<Ty_ty> &list);
// 检查函数参数
void followParams(vector<S_symbol> &params, treeNode *t)
{
    if (t->name == "FuncRParams" || t->name == "ExpFunList")
        for (int i = 0; i < t->childrenNum; i++)
            followParams(params, t->children[i]);
    else
    {
        if (params.size() > 0)
        {
            Ty_ty temp = actual_ty(t);
            if (compareTyKind(params[0]->binding->ty, temp))
            {
                error("ERROR: wrong function param type\n");
            }
            // else if (params[0]->binding->ty->kind == Ty_ty_::Ty_array && temp->array.size() > 0)
            // {
            //     error("ERROR: Array dimensions not matched\n");
            // }
        }
        else
        {
            error("ERROR: empty funcRParams");
        }
        params.erase(params.begin(), params.begin() + 1);
    }
}

void findArrayDegree(treeNode *t, int &degree)
{
    if (t->name == "LValArray")
    {
        findArrayDegree(t->children[0], degree);
        findArrayDegree(t->children[1], degree);
    }
    else if (t->name == "Exp")
    {
        if (actual_ty(t)->kind == Ty_ty_::Ty_int)
        {
            degree++;
            return;
        }
        else
        {
            error("ERROR: array index not int\n");
        }
    }
    else
    {
        error("ERROR: findArrayDegree unknown type\n");
    }
}
Ty_ty actual_ty(treeNode *t)
{
    if (t->name == "Exp")
    {
        return actual_ty(t->children[0]);
    }
    else if (t->name == "UnaryExp")
    {
        return actual_ty(t->children[1]);
    }
    else if (t->name == "MulExp" || t->name == "AddExp")
    {
        Ty_ty left = actual_ty(t->children[0]);
        if (left->kind != Ty_ty_::Ty_int)
        {
            error("Error: Mul left invalid type\n");
        }
        Ty_ty right = actual_ty(t->children[1]);
        if (right->kind != Ty_ty_::Ty_int)
        {
            error("Error: Mul right invalid type\n");
        }
        return new Ty_ty_(Ty_ty_::Ty_int);
    }
    else if (t->name == "MulMore" || t->name == "AddMore")
    {
        Ty_ty left = actual_ty(t->children[1]);
        if (left->kind != Ty_ty_::Ty_int)
        {
            error("Error: MulMore invalid type\n");
        }
        if (t->childrenNum == 3)
        {
            Ty_ty right = actual_ty(t->children[2]);
            if (right->kind != Ty_ty_::Ty_int)
            {
                error("Error: MulMore more invalid type\n");
            }
        }
        return new Ty_ty_(Ty_ty_::Ty_int);
    }
    else if (t->name == "RelExp" || t->name == "EqExp")
    {
        Ty_ty left = actual_ty(t->children[0]);
        Ty_ty right = actual_ty(t->children[2]);
        if (compareTyKind(left, right))
        {
            error("ERROR: wrong relationExp type\n");
        }
        return new Ty_ty_(Ty_ty_::Ty_bool);
    }
    else if (t->name == "LAndExp" || t->name == "LOrExp")
    {
        Ty_ty left = actual_ty(t->children[0]);
        Ty_ty right = actual_ty(t->children[1]);
        if ((left->kind != Ty_ty_::Ty_bool && left->kind != Ty_ty_::Ty_int) || (right->kind != Ty_ty_::Ty_bool && right->kind != Ty_ty_::Ty_int))
        {
            error("Error: LAnd invalid type\n");
        }
        return new Ty_ty_(Ty_ty_::Ty_bool);
    }
    else if (t->name == "LAndMore" || t->name == "LOrMore")
    {
        Ty_ty left = actual_ty(t->children[1]);
        if (left->kind != Ty_ty_::Ty_bool && left->kind != Ty_ty_::Ty_int)
        {
            error("Error: LAndMore invalid type\n");
        }
        if (t->childrenNum == 3)
        {
            Ty_ty right = actual_ty(t->children[2]);
            if (right->kind != Ty_ty_::Ty_bool && right->kind != Ty_ty_::Ty_int)
            {
                error("Error: LAndMore more invalid type\n");
            }
        }
        return new Ty_ty_(Ty_ty_::Ty_bool);
    }
    else if (t->name == "LVal")
    {
        if (!checkScopeVar(t->children[0]->name))
        {
            string s = "ERROR: assign not defined: " + t->children[0]->name + "\n";
            error(s);
        }
        
        S_symbol temp = getSymbol(t->children[0]->name, true);
        if (t->childrenNum > 1)
        {
            if (temp->binding->ty->kind == Ty_ty_::Ty_array)
            {
                int ArrayDegree = 0;
                findArrayDegree(t->children[1], ArrayDegree);
                if (temp->binding->ty->array.size() == ArrayDegree)
                {
                    return new Ty_ty_(Ty_ty_::Ty_int);
                }
                else if (temp->binding->ty->array.size() < ArrayDegree)
                {
                    error("ERROR: array out of bound\n");
                }
                else
                {
                    int degree = temp->binding->ty->array.size() - ArrayDegree;
                    Ty_ty tempTy = new Ty_ty_(Ty_ty_::Ty_array);
                    for (int i = degree; i > 0; i--)
                    {
                        tempTy->array.push_back(temp->binding->ty->array[temp->binding->ty->array.size() - i]);
                    }
                    return tempTy;
                }
            }
            else
            {
                error("ERROR: Subscripting a non-array\n");
            }
        }
        return temp->binding->ty;
    }
    else if (t->name == "ConstInt")
    {
        return typeEx("INT");
    }
    else if (t->name == "FuncCall")
    {
        S_symbol temp = getSymbol(t->children[0]->name, false);
        if (temp == NULL)
        {
            error("Error: function has not been defined\n");
        }
        if (t->childrenNum == 2)
        {
            S_symbolList params = temp->binding->formals;
            followParams(params, t->children[1]);
        }
        return temp->binding->ty;
    }
    else
    {
        for (int i = 0; i < t->childrenNum; i++)
        {
            actual_ty(t->children[i]);
        }
    }
}

string transVarKind(E_enventry_::Kind kind)
{
    switch (kind)
    {
    case 0:
        return string("E_varEntry  ");
        break;
    case 1:
        return string("E_funEntry  ");
        break;
    }
}

string transTyKind(Ty_ty_::Kind kind)
{
    switch (kind)
    {
    case 0:
        return string("Ty_int  ");
        break;
    case 1:
        return string("Ty_bool  ");
        break;
    case 2:
        return string("Ty_array  ");
        break;
    case 3:
        return string("Ty_name  ");
        break;
    case 4:
        return string("Ty_void  ");
        break;
    }
}
void printTyVec(vector<Ty_ty> &list)
{
    cout << "printing Ty vector.....";
    for (int i = 0; i < list.size(); i++)
    {
        cout << transTyKind(list[i]->kind) << " ";
    }
    cout << endl;
}
// void printSymbol(S_symbol s)
// {
//     cout << s->name << "  " << transVarKind(s->binding->kind) << transTyKind(s->binding->ty->kind);
//     if (s->binding->formals.size() > 0)
//     {
//         cout << "param num:" << s->binding->formals.size() << "  ";
//         for (int i = 0; i < s->binding->formals.size(); i++)
//         {
//             cout << transTyKind(s->binding->formals[i]->binding->kind);
//         }
//     }
//     if (s->binding->ty->array.size() > 0)
//     {
//         cout << "array num:" << s->binding->ty->array.size() << "  ";
//         for (int i = 0; i < s->binding->ty->array.size(); i++)
//         {
//             cout << s->binding->ty->array[i] << "  ";
//         }
//     }
//     cout << endl;
// }

void printStack()
{
    printf("\ncurrent stack number: %d\n", symbolStack.size());
    for (int i = 0; i < symbolStack.size(); i++)
    {
        if (symbolStack[i].first == marksym)
        {
            printf("marksym ");
            continue;
        }
        printf("%s ", symbolStack[i].first->name.c_str());
    }
    printf("\n");
    cout << endl;
}
void error(string msg)
{
    printf("%s\n", msg.c_str());
    exit(1);
}
// 辅助栈
S_symbol marksym = new S_symbol_("marksym");

void S_beginScope()
{
    symbolStack.push_back(make_pair(marksym, ""));
}
void tableDel(S_symbol s)
{
    S_symbol old;
    S_symbol syms = findSymbol(s->name), sym;
    if (syms->name == s->name)
    {
        modifySymbol(s->name, syms->next);
        return;
    }
    else
    {
        if (syms)
        {
            old = syms;
            for (sym = syms->next; sym; sym = sym->next)
            {
                if (sym->name == s->name && sym->binding->kind == s->binding->kind)
                {
                    old->next = sym->next;
                    return;
                }
                old = sym;
            }
        }
    }

    error("ERROR: stack not equal to table!\n");
}
void S_endScope()
{
    S_symbol s = symbolStack.back().first;
    while (s != marksym)
    {
        tableDel(s);
        symbolStack.pop_back();
        s = symbolStack.back().first;
    }
    // pop marksym
    symbolStack.pop_back();
    // pop函数
    if(!symbolStack.empty()){
        s = symbolStack.back().first;
        if (s != marksym && s->binding->kind == E_enventry_::E_funEntry)
        {
            symbolStack.pop_back();
        }
    }
}
