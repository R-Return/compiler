#include <stdio.h>
#include <fstream>
#include <string>
#include "../src/tree.hpp"
#include "../src/semant.cpp"
using namespace std;

extern FILE * yyin;
extern FILE * yyout;
treeNode* root = NULL;
extern int yyparse();

int main(int argc,char* argv[]) {
	yyin = fopen(argv[1],"r");
	int y = yyparse();
	printf("yyparse finished\n");
	if (root == NULL) {
		printf("root is NULL\n");
		return 1;
	}
	// ofstream before("printTreeBefore.txt", ios::trunc);
	// root->printTree(0, before);

	SemanticAnalyzer semant(root);
	semant.deleteNull(root);
	
	// ofstream treeOs2;
	// treeOs2.open("printTreeBefore.txt", ios::trunc);
	// printf("correctly open\n");
	// root->printTree(0, treeOs2);

	semant.clean(root);
	
	printf("complete clean\n");

	ofstream treeOs;
	treeOs.open("printTree.txt", ios::trunc);
	printf("correctly open\n");
	root->printTree(0, treeOs);
	
	semant.initialize();
	printf("initialize done\n");

	

	semant.visit(root);
	ofstream treeOs1;
	treeOs1.open(argv[2], ios::trunc);
	printf("correctly open output\n");
	printTargetInstruction(treeOs1);
	

    // if (argc == 4)
    // {
    //     ofstream treeOs;
    //     treeOs.open(argv[2]);
    //     root->printTree(0, treeOs);
        
    // 	ofstream innerCodeOs;
	//     innerCodeOs.open(argv[3]);
    // }

	/* freeGramTree(root); */

	fclose(yyin);
	return y;
}