#include "symbolTable.hpp"

void printTargetInitText(ofstream &os){
    os << "    .text\n";
    os << "_minilib_start:\n";
    os << "    la sp,_stack_top\n";
    os << "    call main\n";
    os << "    mv a1,a0\n";
    os << "    li a0,17\n";
    os << "    ecall\n";
    os << "read:\n";
    os << "    li a0,6\n";
    os << "    ecall\n";
    os << "    ret\n";
    os << "write:\n";
    os << "    mv a1,a0\n";
    os << "    li a0,1\n";
    os << "    ecall\n";
    os << "    ret\n";
    return;
}

void printTargetInitData(ofstream &os){
    os << "    .data\n";
    os << "    .align 4\n";
    os << "_stack_start:\n";
    os << ".space 1145140\n";
    os << "_stack_top:\n";
    return;
}
//若为变量，返回地址，若为临时变量，返回“-1”
string getVarAddr(string name, vector<pair<S_symbol, string>> &symbolList){
    if(name[0] == '-'){
        return "-1";
    }
    for(int i = 0; i < symbolList.size(); i++){
        if(symbolList[i].second == name){
            return to_string(i*4) + "(sp)";
        }
    }
    
}

string getReg(string name, vector<pair<S_symbol, string>> &symbolList, ofstream &os){
    string VarAddr = getVarAddr(name, symbolList);
    if(VarAddr != "-1"){
        Temp1 t1;
        os << "    lw " << t1.printName << ", " <<VarAddr << "\n";
        return t1.printName;
    }
    return name.substr(1);
}

int getGlobal(string name){
    for(int i = 0; i < blockList[0].symbolList.size(); i++){
        if(blockList[0].symbolList[i].second == name){
            return 1;
        }
    }
    return 0;
}
//判断是否为参数
int isArrayParam(string name, vector<pair<S_symbol, string>> &paralist){
    for(int i = 0; i < paralist.size(); i++){
        if(paralist[i].second == name){
            if(paralist[i].first->binding->ty->kind == Ty_ty_::Ty_array){
                return 1;
            }else{
                return 0;
            }
        }
    }
    return 0;
}
void printTargetInstruction(ofstream &os){
    printTargetInitText(os);
    if(blockList.size()>2){
        //交换main位置
        int mainIndex = 1;
        for(int i = 1; i < blockList.size(); i++){
            if(blockList[i].isfunc && blockList[i].func->name == "main"){
                mainIndex = i;
                break;
            }
        }
        blockList.insert(blockList.begin() + 1, blockList[mainIndex]);
        blockList.erase(blockList.begin() + mainIndex + 1);
    }
	for (int i = 1; i < blockList.size(); i++)
	{
		if (blockList[i].isfunc)
		{
			os <<blockList[i].func->name << ":\n";
            
            os <<"    addi sp, sp, -"<<blockList[i].symbolList.size()*4<<"\n";
            //存参数
            int up = blockList[i].func->paralist.size()<=8 ? blockList[i].func->paralist.size() : 8;
            for(int j = 0; j < up; j++){
                os << "    sw a"<<j<<", "<<j*4<<"(sp)\n";
            }
            if(blockList[i].func->paralist.size()>8){
                //倒序读入
                for(int j = 8; j < blockList[i].func->paralist.size(); j++){
                    os << "    lw t0, "<< (blockList[i].func->paralist.size()-j-1)*4 <<"(fp)\n";
                    os << "    sw t0, "<<j*4<<"(sp)\n";
                }
                os << "    addi fp, fp, "<<(blockList[i].func->paralist.size()-8)*4<<"\n";
            }
            //存返回地址
            os <<"    addi fp, fp, -4\n";
            os <<"    sw ra, 0(fp)\n";
		}
		for (int j = 0; j < blockList[i].IRList.size(); j++)
		{
			if(blockList[i].IRList[j].name == "ASSIGN"){
                if(blockList[i].IRList[j].childrenNum == 2){
                    if(blockList[i].IRList[j].children[1][0] == '#'){
                        string s1 = getVarAddr(blockList[i].IRList[j].children[0], blockList[i].symbolList);
                        if(s1 == "-1"){
                            //临时变量
                            os <<"    li " <<getReg(blockList[i].IRList[j].children[0], blockList[i].symbolList, os)<< ", " << blockList[i].IRList[j].children[1].substr(1) << "\n";
                        }else{
                            Temp1 t;
                            os <<"    li " <<t.printName<< ", " << blockList[i].IRList[j].children[1].substr(1) << "\n";
                            os <<"    sw " <<t.printName<< ", " << s1 << "\n";
                        }
                    }
                    else if(blockList[i].IRList[j].children[1][0] == '*'){
                        //右侧
                        Temp1 t;
                        if(blockList[i].IRList[j].children[1][1] == '-'){
                            //临时变量
                            os << "    lw "<< t.printName<<", 0("<<blockList[i].IRList[j].children[1].substr(2)<<")\n";
                        }else if(isArrayParam(blockList[i].IRList[j].children[1].substr(1), blockList[i].func->paralist)){
                            string VarAddr = getVarAddr(blockList[i].IRList[j].children[1].substr(1), blockList[i].symbolList);
                            os << "    lw "<< t.printName<<", "<<VarAddr<<"\n";
                        }else{
                            Temp1 t1;
                            os << "    la "<<t1.printName<<", "<<blockList[i].IRList[j].children[1].substr(1)<<"\n";
                            os << "    lw "<< t.printName<<", 0("<<t1.printName<<")\n";
                        }
                        //左侧
                        string s1 = getReg(blockList[i].IRList[j].children[0], blockList[i].symbolList,os);
                        os <<"    mv " <<s1<<", "<<t.printName<<"\n";
                    }
                    else if(blockList[i].IRList[j].children[0][0] == '*'){
                        //右侧
                        string s1 = getReg(blockList[i].IRList[j].children[1], blockList[i].symbolList, os);
                        //左侧
                        if(blockList[i].IRList[j].children[0][1] == '-'){
                            //临时变量
                            os << "    sw "<< s1<<", 0("<<blockList[i].IRList[j].children[0].erase(0,2)<<")\n";
                        }else{
                            Temp1 t;
                            os << "    la "<<t.printName<<", "<<blockList[i].IRList[j].children[0].substr(1)<<"\n";
                            os << "    sw "<< s1<<", 0("<<t.printName<<")\n";
                        }
                    }
                    else if(blockList[i].IRList[j].children[1][0] == '&'){
                        if(blockList[i].IRList[j].children[1][1] == '-'){
                            printf("对临时变量取地址\n");
                        }
                        //左侧
                        string s1 = getReg(blockList[i].IRList[j].children[0], blockList[i].symbolList, os);
                        //右侧
                        if(isArrayParam(blockList[i].IRList[j].children[1].substr(1), blockList[i].func->paralist)){
                            string VarAddr = getVarAddr(blockList[i].IRList[j].children[1].substr(1), blockList[i].symbolList);
                            os << "    lw "<< s1<<", "<<VarAddr<<"\n";
                        }else{
                            os << "    la "<<s1<<", "<<blockList[i].IRList[j].children[1].substr(1)<<"\n";
                        }
                    }
                    else{
                        string s1 = getVarAddr(blockList[i].IRList[j].children[0], blockList[i].symbolList);
                        string s2 = getReg(blockList[i].IRList[j].children[1], blockList[i].symbolList, os);
                        if(s1 == "-1"){
                            //临时变量
                            os <<"    mv " <<blockList[i].IRList[j].children[0].substr(1)<< ", " << s2 << "\n";
                        }
                        else{
                            Temp1 t;
                            os <<"    mv " <<t.printName<< ", " << s2 << "\n";
                            os <<"    sw " <<t.printName<< ", " << s1 << "\n";
                        }
                    }
                }
                else{
                    if(blockList[i].IRList[j].children[2] == "+"){
                        if(blockList[i].IRList[j].children[3][0] == '#'){
                            string s1 = getVarAddr(blockList[i].IRList[j].children[0], blockList[i].symbolList);
                            string s2 = getReg(blockList[i].IRList[j].children[1], blockList[i].symbolList, os);
                            if(s1 == "-1"){
                                //临时变量
                                os <<"    addi " <<blockList[i].IRList[j].children[0].substr(1)<< ", " << s2 << ", " << blockList[i].IRList[j].children[3].substr(1) << "\n";
                            }
                            else{
                                Temp1 t;
                                os <<"    addi " <<t.printName<< ", " << s2 << ", " << blockList[i].IRList[j].children[3].substr(1) << "\n";
                                os <<"    sw " <<t.printName<< ", " << s1 << "\n";
                            }
                        }
                        else{
                            string s1 = getVarAddr(blockList[i].IRList[j].children[0], blockList[i].symbolList);
                            string s2 = getReg(blockList[i].IRList[j].children[1], blockList[i].symbolList, os);
                            string s3 = getReg(blockList[i].IRList[j].children[3], blockList[i].symbolList, os);
                            if(s1 == "-1"){
                                //临时变量
                                os <<"    add " <<blockList[i].IRList[j].children[0].substr(1)<< ", " << s2 << ", " << s3 << "\n";
                            }
                            else{
                                Temp1 t;
                                os <<"    add " <<t.printName<< ", " << s2 << ", " << s3 << "\n";
                                os <<"    sw " <<t.printName<< ", " << s1 << "\n";
                            }
                        }
                    }else if(blockList[i].IRList[j].children[2] == "-"){
                        if(blockList[i].IRList[j].children[3][0] == '#'){
                            string s1 = getVarAddr(blockList[i].IRList[j].children[0], blockList[i].symbolList);
                            string s2 = getReg(blockList[i].IRList[j].children[1], blockList[i].symbolList, os);
                            if(s1 == "-1"){
                                //临时变量
                                os <<"    addi " <<blockList[i].IRList[j].children[0].substr(1)<< "," << s2 << ", -" << blockList[i].IRList[j].children[3].substr(1) << "\n";
                            }
                            else{
                                Temp1 t;
                                os <<"    addi " <<t.printName<< ", " << s2 << ", -" << blockList[i].IRList[j].children[3].substr(1) << "\n";
                                os <<"    sw " <<t.printName<< ", " << s1 << "\n";
                            }
                        }
                        else{
                            string s1 = getVarAddr(blockList[i].IRList[j].children[0], blockList[i].symbolList);
                            string s2 = getReg(blockList[i].IRList[j].children[1], blockList[i].symbolList, os);
                            string s3 = getReg(blockList[i].IRList[j].children[3], blockList[i].symbolList, os);
                            if(s1 == "-1"){
                                //临时变量
                                os <<"    sub " <<blockList[i].IRList[j].children[0].substr(1)<< "," << s2 << "," << s3 << "\n";
                            }
                            else{
                                Temp1 t;
                                os <<"    sub " <<t.printName<< "," << s2 << "," << s3 << "\n";
                                os <<"    sw " <<t.printName<< "," << s1 << "\n";
                            }
                        }
                    }else if(blockList[i].IRList[j].children[2] == "*"|| blockList[i].IRList[j].children[2] == "/" || blockList[i].IRList[j].children[2] == "%"){
                        string s1 = getVarAddr(blockList[i].IRList[j].children[0], blockList[i].symbolList);
                        string s2 = getReg(blockList[i].IRList[j].children[1], blockList[i].symbolList, os);
                        string s3;
                        if(blockList[i].IRList[j].children[3][0] == '#'){
                            Temp1 t;
                            os <<"    li " <<t.printName<< ", " << blockList[i].IRList[j].children[3].substr(1) << "\n";
                            s3 = t.printName;
                        }
                        else{s3 = getReg(blockList[i].IRList[j].children[3], blockList[i].symbolList, os);}
                        string op = blockList[i].IRList[j].children[2] == "*"? "mul":"div";
                        if(blockList[i].IRList[j].children[2] == "%") op = "rem";
                        if(s1 == "-1"){
                            //临时变量
                            os <<"    "<<op<<" " <<blockList[i].IRList[j].children[0].substr(1)<< "," << s2 << "," << s3 << "\n";
                        }
                        else{
                            Temp1 t;
                            os <<"    "<<op<<" " <<t.printName<< "," << s2 << "," << s3 << "\n";
                            os <<"    sw " <<t.printName<< "," << s1 << "\n";
                        }
                    }
                }
			}
            // else if(blockList[i].IRList[j].name == "DEC"){
            //     os << "    DEC " << blockList[i].IRList[j].children[0] << " " << blockList[i].IRList[j].children[1] << "\n";
            // }
			else if(blockList[i].IRList[j].name == "LABEL"){
				os <<blockList[i].IRList[j].children[0] << ":\n";
			}
			else if(blockList[i].IRList[j].name == "GOTO"){
				os << "    j " << blockList[i].IRList[j].children[0] << "\n";
			}
			else if(blockList[i].IRList[j].name == "IF"){
                string s0 = getReg(blockList[i].IRList[j].children[0], blockList[i].symbolList, os);
                string s2 = getReg(blockList[i].IRList[j].children[2], blockList[i].symbolList, os);
                string s3 = blockList[i].IRList[j].children[3];
                Temp1 t;
                if(blockList[i].IRList[j].children[1] == ">"){
                    os << "    sgt "<<t.printName<<", "<<s0<<", "<<s2<<"\n";
                }else if(blockList[i].IRList[j].children[1] == "<"){
                    os << "    slt "<<t.printName<<", "<<s0<<", "<<s2<<"\n";
                }else if(blockList[i].IRList[j].children[1] == "<="){
                    Temp1 t1;
                    os << "    sgt "<<t1.printName<<", "<<s0<<", "<<s2<<"\n";
                    os << "    seqz "<<t.printName<<", "<<t1.printName<<"\n";
                }else if(blockList[i].IRList[j].children[1] == ">="){
                    Temp1 t1;
                    os << "    slt "<<t1.printName<<", "<<s0<<", "<<s2<<"\n";
                    os << "    seqz "<<t.printName<<", "<<t1.printName<<"\n";
                }else if(blockList[i].IRList[j].children[1] == "=="){
                    Temp1 t1;
                    os << "    sub "<<t1.printName<<", "<<s0<<", "<<s2<<"\n";
                    os << "    seqz "<<t.printName<<", "<<t1.printName<<"\n";
                }else if(blockList[i].IRList[j].children[1] == "!="){
                    os << "    sub "<<t.printName<<", "<<s0<<", "<<s2<<"\n";
                }
                os << "    bnez " << t.printName << ", " << s3 << "\n";
			}
			else if(blockList[i].IRList[j].name == "RETURN"){
                //恢复ra
                os <<"    lw ra, 0(fp)\n";
                os <<"    addi fp, fp, 4\n";
				if(blockList[i].IRList[j].childrenNum == 0){
                    //恢复堆栈
                    os <<"    addi sp, sp, "<<blockList[i].symbolList.size()*4<<"\n";
					os << "    ret\n";
				}else{
                    string s0 = getReg(blockList[i].IRList[j].children[0], blockList[i].symbolList, os);
					os << "    mv a0, " << s0 << "\n";
                    //恢复堆栈
                    os <<"    addi sp, sp, "<<blockList[i].symbolList.size()*4<<"\n";
                    os << "    ret\n";
				}
					
			}
			
			else if(blockList[i].IRList[j].name == "FUNCALL"){
                //保存寄存器
                os<<"    addi sp, sp, -92\n";
                for(int i = 1; i < 8; i++){
                    os << "    sw a"<<i<<", "<<(i-1)*4<<"(sp)\n";
                }
                for(int i = 2; i < 7; i++){
                    os << "    sw t" << i << ", " << (i+5)*4 << "(sp)\n";
                }
                for(int i = 1; i < 12; i++){
                    os << "    sw s" << i << ", " << (i+11)*4 << "(sp)\n";
                }
				os << "    call "<<blockList[i].IRList[j].children[1]<<"\n";
                for(int i = 1; i < 8; i++){
                    os << "    lw a"<<i<<", "<<(i-1)*4<<"(sp)\n";
                }
                for(int i = 2; i < 7; i++){
                    os << "    lw t" << i << ", " << (i+5)*4 << "(sp)\n";
                }
                for(int i = 1; i < 12; i++){
                    os << "    lw s" << i << ", " << (i+11)*4 << "(sp)\n";
                }
                os<<"    addi sp, sp, 92\n";
				if(blockList[i].IRList[j].children[0]!= "-1"){
                    
                    
                    string s0 = getVarAddr(blockList[i].IRList[j].children[0], blockList[i].symbolList);
                    if(s0 == "-1"){
                        //临时变量
                        os << "    mv " <<blockList[i].IRList[j].children[0].substr(1)<<", a0\n";
                    }else{
                        Temp1 t;
                        os << "    mv " <<t.printName<<", a0\n";
                        os << "    sw " <<t.printName<<", "<<s0<<"\n";
                    }
				}
			}
            else if(blockList[i].IRList[j].name == "ASSEMBLE"){
                os << blockList[i].IRList[j].children[0];
            }
		}
		os << "\n";
	}
    printTargetInitData(os);
    for (int j = 0; j < blockList[0].IRList.size(); j++){
        if(blockList[0].IRList[j].name == "GLOBAL"){
            os <<blockList[0].IRList[j].children[0] << ":\n";
        }
        else if(blockList[0].IRList[j].name == "WORD"){
            os << "    .word " << blockList[0].IRList[j].children[0].substr(1) << "\n";
        }
    }
}