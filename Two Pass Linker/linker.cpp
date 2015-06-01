////////////////////////////////////////////////////////////
// Author: Geon Kang									  //
// NYUID: N17120399                                       //
// Operating Systems 									  //
// Spring 2014											  //
// Lab 1: 2-Pass Linker									  //
////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <iomanip>

using namespace std;


const int MAX_CHARS_PER_LINE = 4096;

//code to print parse errors
//provided in assignment description
void __parseerror(int errcode, int linenum, int lineoffset)    
{
    const char* errstr[] = 
    {
		"NUM_EXPECTED",
		"SYM_EXPECTED",
		"ADDR_EXPECTED",
		"SYM_TOLONG",
		"TO_MANY_DEF_IN_MODULE",
		"TO_MANY_USE_IN_MODULE",
		"TO_MANY_INSTR",
	};
	printf("Parse Error line %d offset %d: %s\n",linenum, lineoffset, errstr[errcode]);
}

int main(int argc, char *argv[]){
  char* tokenList[MAX_CHARS_PER_LINE];
  int tokenLine[MAX_CHARS_PER_LINE];
  int tokenOffset[MAX_CHARS_PER_LINE];
  char line[MAX_CHARS_PER_LINE];
  
  //open file to read
  FILE *myFile = fopen(argv[1], "r");

  if (myFile == NULL) {
      cout << "Error: File not found" << endl;
      return 1;
  }
  
  int tokenCount = 0;
  int lineCount = 1;
  int moduleCount = 1;
  string symbolList[512];
  int symbolValue[512];
  int symbolDefinedModule[512];
  int defined[512] = {0};
  int definedAndUsed[512] = {0};
  int symbolCount = 0;
  int lengthLastLine = 0;
  
  
  //First Pass
  //read file line by line and store tokens, along with their line number and offset into arrays  
  while (fgets(line, MAX_CHARS_PER_LINE, myFile))
  {
	lengthLastLine = strlen(line);
    tokenList[tokenCount] = strtok(line, " \t\n");
    while (tokenList[tokenCount])
    {
	  tokenLine[tokenCount] = lineCount;
	  tokenOffset[tokenCount] = tokenList[tokenCount] - line + 1;
	  tokenList[tokenCount] = strdup(tokenList[tokenCount]);
	  tokenCount++;
	  tokenList[tokenCount] = strtok(NULL, " \t\n");
    }
	lineCount++;
  }
  fclose(myFile);
  

  int baseaddress = 0;
  int parseCount = 0;
  
  while(tokenList[parseCount]){
	//def list
	if(!isdigit(*tokenList[parseCount])){
		__parseerror(0, tokenLine[parseCount], tokenOffset[parseCount]);
		return 1;
	}
	
	int defCount = atoi(tokenList[parseCount]);
	
	if(defCount > 16) {
		__parseerror(4, tokenLine[parseCount], tokenOffset[parseCount]);
		return 1;	
	}
	
	parseCount++;
	
	for(int i = 0; i < defCount; i++){
	
		if(!tokenList[parseCount]) {
			if(tokenLine[parseCount-1] == lineCount-1){
				__parseerror(1, tokenLine[parseCount-1], tokenOffset[parseCount-1]+strlen(tokenList[parseCount-1]));
			}
			else{
				__parseerror(1, lineCount-1, lengthLastLine);
			}
		return 1;
		}
	
		if(!isalpha(*tokenList[parseCount])){
		__parseerror(1, tokenLine[parseCount], tokenOffset[parseCount]);
		return 1;
		}
		
		if(strlen(tokenList[parseCount]) > 16) {
		__parseerror(3, tokenLine[parseCount], tokenOffset[parseCount]);
		return 1;
		}
		
		string symb = tokenList[parseCount];
		parseCount++;
		
		if(!tokenList[parseCount]) {
			if(tokenLine[parseCount-1] == lineCount-1){
				__parseerror(0, tokenLine[parseCount-1], tokenOffset[parseCount-1]+strlen(tokenList[parseCount-1]));
			}
			else{
				__parseerror(0, lineCount-1, lengthLastLine);
			}
		return 1;
		}
		
		if(!isdigit(*tokenList[parseCount])){
		__parseerror(0, tokenLine[parseCount], tokenOffset[parseCount]);
		return 1;		
		}
		
		int relative = atoi(tokenList[parseCount]);

		string* check = find(symbolList, symbolList+symbolCount, symb);
		if(check == symbolList+symbolCount){
			symbolList[symbolCount] = symb;
			symbolValue[symbolCount] = relative + baseaddress;
			symbolDefinedModule[symbolCount] = moduleCount;
			symbolCount++;
		}
		else{
		defined[symbolList-check] = defined[symbolList-check] + 1;
		}
			parseCount++;
	}
	
	//use list

	if(!tokenList[parseCount]) {
		if(tokenLine[parseCount-1] == lineCount-1){
			__parseerror(0, tokenLine[parseCount-1], tokenOffset[parseCount-1]+strlen(tokenList[parseCount-1]));
		}
		else{
			__parseerror(0, lineCount-1, lengthLastLine);
		}
		return 1;
	}
    
    if(!isdigit(*tokenList[parseCount])){
		__parseerror(0, tokenLine[parseCount], tokenOffset[parseCount]);
		return 1;		
	}
    
    int useCount = atoi(tokenList[parseCount]);
    
    if(useCount > 16) {
		__parseerror(5, tokenLine[parseCount], tokenOffset[parseCount]);
		return 1;	
	}
    
    parseCount++;

    for(int i = 0; i < useCount; i++){
        
		if(!tokenList[parseCount]) {
			if(tokenLine[parseCount-1] == lineCount-1){
				__parseerror(1, tokenLine[parseCount-1], tokenOffset[parseCount-1]+strlen(tokenList[parseCount-1]));
			}
			else{
				__parseerror(1, lineCount-1, lengthLastLine);
			}
		return 1;
		}
        
    	if(!isalpha(*tokenList[parseCount])){
		__parseerror(1, tokenLine[parseCount], tokenOffset[parseCount]);
		return 1;
		}
		
		if(strlen(tokenList[parseCount]) > 16) {
		__parseerror(3, tokenLine[parseCount], tokenOffset[parseCount]);
		return 1;
		}
        parseCount++;
    }
    
    //program text

	if(!tokenList[parseCount]) {
		if(tokenLine[parseCount-1] == lineCount-1){
			__parseerror(0, tokenLine[parseCount-1], tokenOffset[parseCount-1]+strlen(tokenList[parseCount-1]));
		}
		else{
			__parseerror(0, lineCount-1, lengthLastLine);
		}
		return 1;
	}
    
    if(!isdigit(*tokenList[parseCount])){
		__parseerror(0, tokenLine[parseCount], tokenOffset[parseCount]);
		return 1;		
	}
    
    int moduleSize = atoi(tokenList[parseCount]);
    
    baseaddress = baseaddress + moduleSize;
    
    if(baseaddress > 512) {
        __parseerror(6, tokenLine[parseCount], tokenOffset[parseCount]);
		return 1;
    }

    parseCount++;
    
    for(int i = 0; i < moduleSize; i++){
	
		if(!tokenList[parseCount]) {
			if(tokenLine[parseCount-1] == lineCount-1){
				__parseerror(2, tokenLine[parseCount-1], tokenOffset[parseCount-1]+strlen(tokenList[parseCount-1]));
			}
			else{
				__parseerror(2, lineCount-1, lengthLastLine);
			}
		return 1;
		}
	
        if(!(strcmp(tokenList[parseCount],"I") || strcmp(tokenList[parseCount],"A") || strcmp(tokenList[parseCount],"R") || strcmp(tokenList[parseCount],"E"))) {
        __parseerror(2, tokenLine[parseCount], tokenOffset[parseCount]);
    	return 1;            
        }
        
        parseCount++;
		if(!tokenList[parseCount]) {
			if(tokenLine[parseCount-1] == lineCount-1){
				__parseerror(0, tokenLine[parseCount-1], tokenOffset[parseCount-1]+strlen(tokenList[parseCount-1]));
			}
			else{
				__parseerror(0, lineCount-1, lengthLastLine);
			}
		return 1;
		}
    
        if(!isdigit(*tokenList[parseCount])){
		    __parseerror(0, tokenLine[parseCount], tokenOffset[parseCount]);
		    return 1;		
	    }
        
        parseCount++;
        
    }
	
	//check if symbol size is bigger than module size
	int moduleMax = 0;
	if(moduleSize-1 < 0){
	moduleMax = 0;
	}
	else{
	moduleMax = moduleSize-1;
	}
	
	for(int i = symbolCount-defCount; i < symbolCount; i++) {
		if(symbolValue[i]-(baseaddress-moduleSize) > moduleSize){
		printf("Warning: Module %d: %s to big %d (max=%d) assume zero relative \n", moduleCount, symbolList[i].c_str(), symbolValue[i], moduleMax);
		symbolValue[i] = 0;
		}
	}
	moduleCount++;
	
  }
  
  //print symboltable
  cout << "Symbol Table" << endl;
  for (int i = 0; i < symbolCount; i++){
	if(defined[i] == 1){
		cout << symbolList[i] << "=" << symbolValue[i];
		cout << " Error: This variable is multiple times defined; first value used" << endl;
	}
	else {
		cout << symbolList[i] << "=" << symbolValue[i] << endl;
	}
  }
  cout << endl;
  
  
  //second pass
  //reset parse counter
  parseCount = 0;
  moduleCount = 1;
  baseaddress = 0;
  int memoryCount = 0;
  tokenCount = 0;
  
  
  //variables for warnings
  bool warnFlag = false;
  int warnCount = 0;
  int moduleWarn[512];
  string symbolWarn[512];
  int warnType[512] = {0};  
  
  cout << "Memory Map" << endl;

  myFile = fopen(argv[1], "r");

  if (myFile == NULL) {
      cout << "Error: File not found" << endl;
      return 1;
  }
  
  while (fgets(line, MAX_CHARS_PER_LINE, myFile))
  {
	lengthLastLine = strlen(line);
    tokenList[tokenCount] = strtok(line, " \t\n");
    while (tokenList[tokenCount])
    {
	  tokenLine[tokenCount] = lineCount;
	  tokenOffset[tokenCount] = tokenList[tokenCount] - line + 1;
	  tokenList[tokenCount] = strdup(tokenList[tokenCount]);
	  tokenCount++;
	  tokenList[tokenCount] = strtok(NULL, " \t\n");
    }
	lineCount++;
  }
  fclose(myFile);
  
  
    while(tokenList[parseCount]){
	//def list
	int defCount = atoi(tokenList[parseCount]);
	
	parseCount++;
	
	for(int i = 0; i < defCount; i++){
		parseCount++;
		parseCount++;
	}
	
	//use list    
    int useCount = atoi(tokenList[parseCount]);
    string useList[useCount];
	int used[useCount];
	parseCount++;

	//initialize used flag checker
	for(int i = 0; i < useCount; i++){
	used[i] = 0;
	}
	
    for(int i = 0; i < useCount; i++){
		useList[i] = tokenList[parseCount];
        parseCount++;
    }
    
    //program text
    int moduleSize = atoi(tokenList[parseCount]);
    int instruction;
	parseCount++;
	
    for(int i = 0; i < moduleSize; i++){
        if((strcmp(tokenList[parseCount],"I")) == 0) {
			parseCount++;
			instruction = atoi(tokenList[parseCount]);
			if(instruction > 9999){
				instruction = 9999;
				cout << setw(3) << setfill('0') << memoryCount << ": " << setw(4) << setfill('0') << instruction;
				cout << " Error: Illegal immediate value; treated as 9999" << endl;
				memoryCount++;
			}
			
			else {
				cout << setw(3) << setfill('0') << memoryCount << ": " << setw(4) << setfill('0') << instruction << endl;
				memoryCount++;
			}
		}
			
        else if(strcmp(tokenList[parseCount],"A") == 0) {	
			parseCount++;
			instruction = atoi(tokenList[parseCount]);
			if(instruction > 9999){
				instruction = 9999;
				cout << setw(3) << setfill('0') << memoryCount << ": " << setw(4) << setfill('0') << instruction;
				cout << " Error: Illegal opcode; treated as 9999" << endl;
				memoryCount++;
			}
			
			else if((instruction%1000) > 512) {
				instruction = instruction-(instruction%1000);
				cout << setw(3) << setfill('0') << memoryCount << ": " << setw(4) << setfill('0') << instruction;
				cout << " Error: Absolute address exceeds machine size; zero used" << endl;
				memoryCount++;
			}			
			
			else {
				cout << setw(3) << setfill('0') << memoryCount << ": " << setw(4) << setfill('0') << instruction << endl;
				memoryCount++;
			}
		}
		else if((strcmp(tokenList[parseCount],"R")) == 0) {
			parseCount++;
			instruction = atoi(tokenList[parseCount]);
			if(instruction > 9999){
				instruction = 9999;
				cout << setw(3) << setfill('0') << memoryCount << ": " << setw(4) << setfill('0') << instruction;
				cout << " Error: Illegal opcode; treated as 9999" << endl;
				memoryCount++;
			}
			
			else if((instruction%1000) > moduleSize){
				instruction = instruction-(instruction%1000)+baseaddress;
				cout << setw(3) << setfill('0') << memoryCount << ": " << setw(4) << setfill('0') << instruction;
				cout << " Error: Relative address exceeds module size; zero used" << endl;
				memoryCount++;
			}
			
			else{
				instruction = instruction + baseaddress;
				cout << setw(3) << setfill('0') << memoryCount << ": " << setw(4) << setfill('0') << instruction << endl;
				memoryCount++;
			}
		}
		else if((strcmp(tokenList[parseCount],"E")) == 0) {
			parseCount++;
			instruction = atoi(tokenList[parseCount]);
			if(instruction > 9999){
				instruction = 9999;
				cout << setw(3) << setfill('0') << memoryCount << ": " << setw(4) << setfill('0') << instruction;
				cout << " Error: Illegal opcode; treated as 9999" << endl;
				memoryCount++;
			}
			
			else if((instruction%1000) > useCount-1){
				cout << setw(3) << setfill('0') << memoryCount << ": " << setw(4) << setfill('0') << instruction;
				cout << " Error: External address exceeds length of uselist; treated as immediate" << endl;
				memoryCount++;
			}
			
			else{
				int useIndex = (instruction%1000);
				string useSymbol = useList[useIndex];
				string* defineCheck = find(symbolList, symbolList+symbolCount, useSymbol);
				//not defined
				if(defineCheck == symbolList+symbolCount){
					used[useIndex] = 1;
					instruction = instruction - (instruction%1000);
					cout << setw(3) << setfill('0') << memoryCount << ": " << setw(4) << setfill('0') << instruction;
					cout << " Error: " << useSymbol << " not defined; zero used" << endl;
				}
				
				else{
					//mark symbol as used (for warning)
					used[useIndex] = 1;
					//find index of symbol in symbol list
					for(int i = 0; i < symbolCount; i++){
						if(symbolList[i] == useSymbol){
							useIndex = i;
							//mark as defined & used
							definedAndUsed[useIndex] = 1;
							break;
						}
					}
					instruction = instruction - (instruction%1000) + symbolValue[useIndex];
					cout << setw(3) << setfill('0') << memoryCount << ": " << setw(4) << setfill('0') << instruction << endl;
				}
				memoryCount++;
			}
		
		}
		parseCount++;
    }
	
	//check if appeared in uselist but was not used
	for(int i = 0; i < useCount; i++){
		if(used[i] == 0){
			warnFlag = true;
			moduleWarn[warnCount] = moduleCount;
			symbolWarn[warnCount] = useList[i];
			warnCount++;
		}
	}
	
	baseaddress = baseaddress + moduleSize;
	moduleCount++;
	
  }
  
  //check if any symbols were defined but not used
  for(int i = 0; i < symbolCount; i++){
	if(definedAndUsed[i] == 0){
		warnFlag = true;
		moduleWarn[warnCount] = symbolDefinedModule[i];
		symbolWarn[warnCount] = symbolList[i];
		warnType[warnCount] = 1;
		warnCount++;
	}	
  }
  
  //print warnings

  if(warnFlag == true){
    cout << endl;
	for(int i = 0; i < warnCount; i++){
		if(warnType[i] == 0){
			cout << "Warning: Module " << moduleWarn[i] << ": " << symbolWarn[i] << " appeared in the uselist but was not actually used" << endl;
		}
		else {
			cout << "Warning: Module " << moduleWarn[i] << ": " << symbolWarn[i] << " was defined but never used" << endl;
		}
	}
  }
}
