#include "../src/symbolTable.hpp"

using namespace std;
unordered_map<string, int> symbolRecord;
int Temp::count = -1;
int Temp1::count = -1;
int label::count = 0;

void print_Func(S_symbol exp){
    funcNode *fn = new funcNode(exp->name);
    S_symbolList& temp = exp->binding->formals;
    for(int i = 0; i < temp.size(); i++){
        fn->paralist.push_back(make_pair(temp[i], getVarTemp(temp[i]->name)));
        if(temp[i]->binding->ty->kind == Ty_ty_::Ty_array){
            arrayStack.push_back(getVarTemp(temp[i]->name));
        }
    }
    blockList.push_back(Block(fn));
    blockList.back().symbolList.insert(blockList.back().symbolList.end(), fn->paralist.begin(), fn->paralist.end());
    
}

//递归访问函数每个参数
void traverse_args(treeNode *tree, int& t_list){
    if(tree->childrenNum > 1){
        traverse_args(tree->children[0], t_list);
        traverse_args(tree->children[1], t_list);
    }else{
        if(t_list<8){
            Temp t(t_list);
            translate_exp(tree, t);
            t_list++;
        }else{
            blockList.back().IRList.push_back(IRNode("ASSEMBLE", 1, "    addi fp, fp, -4\n"));
            Temp t("t0", 0);
            translate_exp(tree, t);
            string s = "    sw " + t.printName + ", 0(fp)\n";
            blockList.back().IRList.push_back(IRNode("ASSEMBLE", 1, s.c_str()));
            t_list++;
        }
        
        
    }
}


void translate_exp(treeNode *exp, Temp& place){
    int flag = 0;
    if(place.index == -1){
        if(exp->name == "FuncCall" || exp->name == "Exp")
            flag = 1;
        else return;
    }
    if(exp->name == "Exp"){
        translate_exp(exp->children[0], place);
    }
    else if(exp->name == "ConstInt"){
        blockList.back().IRList.push_back(IRNode("ASSIGN", 2, place.name.c_str(),("#"+exp->children[0]->name).c_str()));
        return;
    }
    else if(exp->name == "UnaryExp"){
        if(exp->children[0]->children[0]->name == "-"){
            Temp t(place.name);
            translate_exp(exp->children[1], t);
            blockList.back().IRList.push_back(IRNode("ASSIGN", 4, place.name.c_str(), "-x0", "-",t.name.c_str()));
            return;
        }
        else {
            error("translate_exp UnaryExp no match");
        }
    }
    else if(exp->name == "LVal"){
        int isGlobal = is_global(exp->children[0]->name);
        if(exp->childrenNum == 1){
            if(is_single_array(exp->children[0]->name)){
                // //数组
                // if(isGlobal){
                //     Temp t;
                //     blockList.back().IRList.push_back(IRNode("ASSIGN", 2, place.name.c_str(), ("&"+exp->children[0]->name).c_str()));
                //     return;
                // }else{
                //     blockList.back().IRList.push_back(IRNode("ASSIGN", 2, place.name.c_str(), getVarTemp(exp->children[0]->name).c_str()));
                //     return;
                // }
                //若是参数数组
                for(int i = arrayStack.size()-1; i >= 0; i--){
                    if(arrayStack[i] == getVarTemp(exp->children[0]->name)){
                        blockList.back().IRList.push_back(IRNode("ASSIGN", 2, place.name.c_str(), getVarTemp(exp->children[0]->name).c_str()));
                        return;
                    }
                }
                //不是参数数组
                blockList.back().IRList.push_back(IRNode("ASSEMBLE", 1, ("    la "+place.printName+", "+getVarTemp(exp->children[0]->name)+"\n").c_str()));
                return;
            }
            if(isGlobal){
                blockList.back().IRList.push_back(IRNode("ASSIGN", 2, place.name.c_str(), ("&"+exp->children[0]->name).c_str()));
                blockList.back().IRList.push_back(IRNode("ASSIGN", 2, place.name.c_str(), ("*"+place.name).c_str()));
                return;
            }else{
                blockList.back().IRList.push_back(IRNode("ASSIGN", 2, place.name.c_str(), getVarTemp(exp->children[0]->name).c_str()));
                return;
            }

        }
        else{
            //数组
            string arrayName;
            // if(isGlobal){
            //     Temp t;
            //     blockList.back().IRList.push_back(IRNode("ASSIGN", 2, t.name.c_str(), ("&"+exp->children[0]->name).c_str()));
            //     arrayName = t.name;
            // }else{
            //     arrayName = getVarTemp(exp->children[0]->name);
            // }
            Temp t;
            t.lockTemp();
            blockList.back().IRList.push_back(IRNode("ASSIGN", 2, t.name.c_str(), ("&"+getVarTemp(exp->children[0]->name)).c_str()));
            arrayName = t.name;
            //获取数组的index
            int isArray = 0;
            get_array_address1(arrayName, exp, place, isArray);
            t.deLock();
            if(isArray){
                blockList.back().IRList.push_back(IRNode("ASSIGN", 2, place.name.c_str(), place.name.c_str()));
            }
            else{
                blockList.back().IRList.push_back(IRNode("ASSIGN", 2, place.name.c_str(), ("*"+place.name).c_str()));
            }
            return;
        }
        error("translate_exp LVal no match");
    }
    else if(exp->name == "MulExp" || exp->name == "AddExp"){
        Temp t1;
        t1.lockTemp();
        translate_exp(exp->children[0], t1);
        treeNode *tempTree = exp->children[1];
        Temp t2;
        t2.lockTemp();
        while(1){
            translate_exp(tempTree->children[1], t2);
            if(tempTree->childrenNum == 2){
                blockList.back().IRList.push_back(IRNode("ASSIGN", 4, place.name.c_str(), t1.name.c_str(), tempTree->children[0]->children[0]->name.c_str(), t2.name.c_str()));
                break;
            }
            blockList.back().IRList.push_back(IRNode("ASSIGN", 4, t1.name.c_str(), t1.name.c_str(), tempTree->children[0]->children[0]->name.c_str(), t2.name.c_str()));
            tempTree = tempTree->children[2];
        }
        t1.deLock();
        t2.deLock();
        return;
    }
    else if(exp->name == "FuncCall"){
        
        string placeName = place.name;
        if(flag){
            placeName = "-1";
        }
        if(exp->childrenNum > 1){
            //有参数
            int t_list=0;
            traverse_args(exp->children[1], t_list);
            
            // for(int i = 0; i < t_list.size(); i++){
            //     tempNode.insertChild(t_list[i]->name);
            // }
        }
        IRNode tempNode("FUNCALL",2,placeName.c_str(),exp->children[0]->name.c_str());
        blockList.back().IRList.push_back(tempNode);
        return;
    }
}

// string translate_stmt(treeNode* stmt){

// }
string get_relop(string op){
    if(op == "LE_OP")return "<=";
    else if(op == "GE_OP")return ">=";
    else if(op == "EQ_OP")return "==";
    else if(op == "NE_OP")return "!=";
    else return op;
}
void translate_cond(treeNode *Exp, label &label_true, label &label_false){
    if(Exp->name == "RelExp" || Exp->name == "EqExp"){
        Temp t1, t2;
        t1.lockTemp();
        t2.lockTemp();
        translate_exp(Exp->children[0], t1);
        translate_exp(Exp->children[2], t2);
        string op = get_relop(Exp->children[1]->name);
        blockList.back().IRList.push_back(IRNode("IF", 4, t1.name.c_str(), op.c_str(), t2.name.c_str(), label_true.name.c_str()));
        blockList.back().IRList.push_back(IRNode("GOTO", 1, label_false.name.c_str()));
        t1.deLock();
        t2.deLock();
        return;
    }
    else if(Exp->name == "UnaryExp" && Exp->children[0]->children[0]->name == "!"){
            translate_cond(Exp->children[1], label_false, label_true);
            return;
    }
    else if(Exp->name == "LAndExp"){
        label label1 = label();
        translate_cond(Exp->children[0], label1, label_false);
        treeNode *tempTree = Exp->children[1];
        label oldl = label1;
        while(1){
            blockList.back().IRList.push_back(IRNode("LABEL", 1, oldl.name.c_str()));
            if(tempTree->childrenNum == 2){
                translate_cond(tempTree->children[1], label_true, label_false);
                break;
            }
            label label2 = label();
            translate_cond(tempTree->children[1], label2, label_false);
            
            tempTree = tempTree->children[2];
            oldl = label2;
        }
        return;
    }else if(Exp->name == "LOrExp"){
        label label1 = label();
        translate_cond(Exp->children[0], label_true, label1);
        treeNode *tempTree = Exp->children[1];
        label oldl = label1;
        while(1){
            blockList.back().IRList.push_back(IRNode("LABEL", 1, oldl.name.c_str()));
            if(tempTree->childrenNum == 2){
                translate_cond(tempTree->children[1], label_true, label_false);
                break;
            }
            label label2 = label();
            translate_cond(tempTree->children[1], label_true, label2);
            tempTree = tempTree->children[2];
            oldl = label2;
        }
        return;
    }else if(Exp->name == "FuncCall" || Exp->name == "AddExp" || Exp->name == "MulExp" || Exp->name == "LVal" || Exp->name == "UnaryExp"){
        Temp t1;
        t1.lockTemp();
        translate_exp(Exp, t1);
        blockList.back().IRList.push_back(IRNode("IF", 4, t1.name.c_str(), "!=", "-x0", label_true.name.c_str()));
        t1.deLock();
        blockList.back().IRList.push_back(IRNode("GOTO", 1, label_false.name.c_str()));
        return;
    }
}

void translate_stmt(treeNode *Stmt){
    if(Stmt->name == "Exp"){
        Temp t(-1);
        translate_exp(Stmt->children[0], t);
        return;
    }else if(Stmt->name == "AssignStmt"){
        int isGlobal = is_global(Stmt->children[0]->children[0]->name);
        if(Stmt->children[0]->childrenNum == 1){
            if(isGlobal){
                //全局变量赋值
                Temp t;
                t.lockTemp();
                blockList.back().IRList.push_back(IRNode("ASSIGN", 2, t.name.c_str(), ("&"+Stmt->children[0]->children[0]->name).c_str()));
                Temp t1;
                t1.lockTemp();
                translate_exp(Stmt->children[1], t1);
                blockList.back().IRList.push_back(IRNode("ASSIGN", 2, ("*"+t.name).c_str(), t1.name.c_str()));
                t1.deLock();
                t.deLock();
                return;
            }
            //单个数赋值
            Temp t;
            t.lockTemp();
            translate_exp(Stmt->children[1], t);
            blockList.back().IRList.push_back(IRNode("ASSIGN", 2, getVarTemp(Stmt->children[0]->children[0]->name).c_str(), t.name.c_str()));
            t.deLock();
            return;
        }
        //数组
        string arrayName;
        // if(isGlobal){
        //     Temp t;
        //     blockList.back().IRList.push_back(IRNode("ASSIGN", 2, t.name.c_str(), ("&"+Stmt->children[0]->children[0]->name).c_str()));
        //     arrayName = t.name;
        // }else{
        //     arrayName = getVarTemp(Stmt->children[0]->children[0]->name);
        // }
        //若是数组参数

        Temp t;
        t.lockTemp();
        //函数参数
        int flag = 0;
        for(int i = arrayStack.size()-1; i >= 0; i--){
            if(arrayStack[i] == getVarTemp(Stmt->children[0]->children[0]->name)){
                blockList.back().IRList.push_back(IRNode("ASSIGN", 2, t.name.c_str(), getVarTemp(Stmt->children[0]->children[0]->name).c_str()));
                arrayName = t.name;
                flag = 1;
                break;
            }
        }
        //不是函数参数
        if(!flag){
            blockList.back().IRList.push_back(IRNode("ASSIGN", 2, t.name.c_str(), ("&"+getVarTemp(Stmt->children[0]->children[0]->name)).c_str()));
            arrayName = t.name;
        }
        
        //获取数组地址
        Temp t2;
        t2.lockTemp();
        int isArray;
        get_array_address1(arrayName,Stmt->children[0], t2, isArray);
        t.deLock();
        Temp t3;
        t3.lockTemp();
        translate_exp(Stmt->children[1], t3);
        blockList.back().IRList.push_back(IRNode("ASSEMBLE", 1, ("    sw "+t3.printName+", 0("+t2.printName+")\n").c_str()));
        t2.deLock();
        t3.deLock();
        return;
    }else if(Stmt->name == "ReturnStmt"){
        if(Stmt->childrenNum == 2){
            //有返回值
            Temp t;
            t.lockTemp();
            translate_exp(Stmt->children[1], t);
            blockList.back().IRList.push_back(IRNode("RETURN", 1, t.name.c_str()));
            t.deLock();
        }else{
            blockList.back().IRList.push_back(IRNode("RETURN", 0));
        }
        return;
    }
}

int get_array_int_index(vector<int> &a, vector<int>&index){
    int arraySize = 1;
    int place = 0;
    for(int i = index.size()-1; i >= 0; i--){
        place += index[i]*arraySize;
        arraySize *= a[i];
    }
    return place;
}
//数组初始化时
void get_array_index(vector<int> &a, vector<int>&index, Temp &place){
    int arraySize = 1;
    blockList.back().IRList.push_back(IRNode("ASSIGN", 2, place.name.c_str(), "#0"));
    for(int i = index.size()-1; i >= 0; i--){
        Temp t;
        t.lockTemp();
        blockList.back().IRList.push_back(IRNode("ASSIGN", 2, t.name.c_str(), ("#"+to_string(index[i])).c_str()));
        blockList.back().IRList.push_back(IRNode("ASSIGN", 4, t.name.c_str(), t.name.c_str(), "*", ("#"+to_string(arraySize)).c_str()));
        blockList.back().IRList.push_back(IRNode("ASSIGN", 4, place.name.c_str(), place.name.c_str(), "+", t.name.c_str()));
        t.deLock();
        arraySize *= a[i];
    }
    return;
}

// void get_array_index(string &arrayName, vector<string>&index, Temp &place, int &isArray){
//     //从stack中找到a数组
//     vector<int> a;
//     for(int i = symbolStack.size()-1; i >= 0; i--){
//         if(symbolStack[i].first->name == arrayName){
//             a = symbolStack[i].first->binding->ty->array;
//         }
//     }
//     int arraySize = 1;
//     blockList.back().IRList.push_back(IRNode("ASSIGN", 2, place.name.c_str(), "#0"));
//     for(int i = index.size(); i < a.size(); i++){
//         isArray = 1;
//         index.push_back("0");
//     }
//     Temp t1;
//     for(int i = index.size()-1; i >= 0; arraySize *= a[i], i--){
//         if(index[i] == "0")continue;
//         blockList.back().IRList.push_back(IRNode("ASSIGN", 4, t1.name.c_str(), index[i].c_str(), "*", ("#"+to_string(arraySize)).c_str()));
//         blockList.back().IRList.push_back(IRNode("ASSIGN", 4, place.name.c_str(), place.name.c_str(), "+", t1.name.c_str()));
//     }
//     return;
// }

//获取array的地址
void get_array_address(string &arrayName, string &arrayIndex, Temp& place){
    blockList.back().IRList.push_back(IRNode("ASSIGN", 2, place.name.c_str(), arrayIndex.c_str()));
    blockList.back().IRList.push_back(IRNode("ASSIGN", 4, place.name.c_str(), place.name.c_str(), "*", "#4"));
    blockList.back().IRList.push_back(IRNode("ASSIGN", 4, place.name.c_str(), arrayName.c_str(), "+", place.name.c_str()));
    return;
}
//获取对应地址, isArray标记是否为数组
void get_array_address1(string arrayName, treeNode *exp, Temp &place, int &isArray){
    treeNode *tempTree = exp->children[1];

    vector<int> a;
    for(int i = symbolStack.size()-1; i >= 0; i--){
        if(symbolStack[i].first->name == exp->children[0]->name){
            a = symbolStack[i].first->binding->ty->array;
        }
    }
    int arraySize = 1;
    for(int i = 1; i < a.size(); i++){
        arraySize *= a[i];
    }
    blockList.back().IRList.push_back(IRNode("ASSIGN", 2, place.name.c_str(), "#0"));
    vector<string> index;
    Temp t1;
    t1.lockTemp();
    int i = 0;
    while(1){
        translate_exp(tempTree->children[0], t1);
        blockList.back().IRList.push_back(IRNode("ASSIGN", 4, t1.name.c_str(), t1.name.c_str(), "*", ("#"+to_string(arraySize)).c_str()));
        blockList.back().IRList.push_back(IRNode("ASSIGN", 4, place.name.c_str(), place.name.c_str(), "+", t1.name.c_str()));
        if(tempTree->childrenNum == 1 || i == a.size()-1){
            break;
        }
        arraySize /= a[i+1];
        tempTree = tempTree->children[1];
        i++;
    }
    t1.deLock();
    if(arraySize != 1){
        isArray = 1;
    }
    //计算偏移量
    blockList.back().IRList.push_back(IRNode("ASSIGN", 4, place.name.c_str(), place.name.c_str(),"*","#4"));
    blockList.back().IRList.push_back(IRNode("ASSIGN", 4, place.name.c_str(), arrayName.c_str(), "+", place.name.c_str()));
    return;
}

int get_array_size(vector<int> &array){
    int size = 1;
    for(int i = 0; i < array.size(); i++){
        size *= array[i];
    }
    return size;
}

int is_global(string name){
    int globalEnd = 0, flag = 0;
    for(int i = 0; i < symbolStack.size(); i++){
        if(symbolStack[i].first == marksym){
            globalEnd = i;
            break;
        }
        else if(symbolStack[i].first->name == name){
            flag = 1;
        }
    }
    if(globalEnd == 0){
        //global部分尚未结束
        if(flag)return 1;
    }else{
        if(checkScopeVar(name) == 1)return 0;
        for(int i = globalEnd+1; i < symbolStack.size(); i++){
            if(symbolStack[i].first == marksym)continue;
            if(symbolStack[i].first->name == name){
                return 0;
            }
        }
        if(flag)return 1;
    }
    return 0;
}
//将变量名转换为代号名
string getVarTemp(string name){
    for(int i = symbolStack.size()-1; i >=0 ; i--){
        if(symbolStack[i].first == marksym)continue;
        if(symbolStack[i].first->name == name){
            return symbolStack[i].second;
        }
    }
    return "";
}

string insertSymbolRecord(string name){
    if(symbolRecord.find(name) == symbolRecord.end()){
        symbolRecord[name] = 1;
        return name + to_string(1);
    }
    else{
        symbolRecord[name]++;
        return name + to_string(symbolRecord[name]);
    }
}

int is_in_global_mode(){
    for(int i = 0; i < symbolStack.size(); i++){
        if(symbolStack[i].first == marksym){
            return 0;
        }
    }
    return 1;
}

int is_single_array(string name){
    for(int i = symbolStack.size()-1; i >= 0; i--){
        if(symbolStack[i].first->name == name){
            if(symbolStack[i].first->binding->ty->kind == Ty_ty_::Ty_array){
                return 1;
            }
        }
    }
    return 0;
}