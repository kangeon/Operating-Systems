///////////////////////////////////////////////////
// Author: Geon Kang							 //
// NYUID: N17120399                              //
// Operating Systems 						     //
// Spring 2014									 //
// Lab 3: Memory Manager					     //
///////////////////////////////////////////////////

#include <iostream>
#include <cstring>
#include <string>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <algorithm>
#include <vector>


using namespace std;

vector<int> usedlist;
vector<unsigned int> phys_ages;
vector<unsigned int> virt_ages;

struct PTE {
	unsigned present:1;
	unsigned modified:1;
	unsigned referenced:1;
	unsigned pagedout:1;
	unsigned frameindex:28;
};

struct frame{
	bool mapped;
	int mappedPage;
};

//single level page table
struct PTE page_table[64];

vector<int> randvals;
int randSize;
int ofs = 0;

int myrandom(int size) { 
    int value = randvals[ofs]%size; 
    //loop around if run out of numbers in array 
    ofs = (ofs+1)%randSize; 
    return value; 
} 

class PageReplacement {
	protected:
		int numberOfVpages;
	public:
		PageReplacement(){
			numberOfVpages = 64;
		}
		virtual int replace (int framesize, frame frametable[], PTE page_table[]) = 0;
};

class FIFO: public PageReplacement {
	private:
		int current;
	public:
		FIFO(int head) {
			current = head;
		}
		int replace (int framesize, frame frametable[], PTE page_table[]) {
			int temp;
			temp = current;
			current = (current+1)%framesize;
			return temp;
		}
};

class NRU: public PageReplacement {
	private:
		int count;
	public:
		NRU(int init) {
			count = init;
		}
	int replace (int framesize, frame frametable[], PTE page_table[]) {
		vector<int> classzero;
		vector<int> classone;
		vector<int> classtwo;
		vector<int> classthree;
		int index = 0;
		int temp = 0;
	
		
		for(int i = 0; i < 64; i++){
			if(page_table[i].present == 1){
				if(page_table[i].referenced == 0 && page_table[i].modified == 0) classzero.push_back(i);
				if(page_table[i].referenced == 0 && page_table[i].modified == 1) classone.push_back(i);
				if(page_table[i].referenced == 1 && page_table[i].modified == 0) classtwo.push_back(i);
				if(page_table[i].referenced == 1 && page_table[i].modified == 1) classthree.push_back(i);
			}
		}
		
		if(!classzero.empty()){
			index = myrandom(classzero.size());
			temp = classzero[index];
		}
		
		else if(!classone.empty()){
			index = myrandom(classone.size());
			temp = classone[index];
		}
		
		else if(!classtwo.empty()){
			index = myrandom(classtwo.size());
			temp = classtwo[index];
		}
		
		else if(!classthree.empty()){
			index = myrandom(classthree.size());
			temp = classthree[index];
		}
		
		classzero.clear();
		classone.clear();
		classtwo.clear();
		classthree.clear();
		
		count=(count+1)%10;
		if(count == 0){
			for(int i = 0; i < 64; i++){
				page_table[i].referenced = 0;
			}
		}
	
		return page_table[temp].frameindex;
	};
};

class LRU: public PageReplacement {
	int replace (int framesize, frame frametable[], PTE page_table[]) {
		return usedlist[usedlist.size()-1];
	}
};

class RANDOM: public PageReplacement {
	int replace (int framesize, frame frametable[], PTE page_table[]) {
		int temp;
		temp = myrandom(framesize);
		return temp;
	}
};

class SECONDCHANCE: public PageReplacement {
	private:
		int current;
	public:
		SECONDCHANCE(int head){
			current = head;
		}
	int replace (int framesize, frame frametable[], PTE page_table[]) {

		int temp = 0;
		while(true){
			if(page_table[frametable[current].mappedPage].referenced == 0){

				temp = current;
				current = (current+1)%framesize;
				return temp;
			}
			else{
				page_table[frametable[current].mappedPage].referenced = 0;
				current = (current+1)%framesize;
			}
		}
	}	
};

class CLOCK_PHYS: public PageReplacement {
	private:
		int hand;
	public:
		CLOCK_PHYS(int head){
			hand = head;
		}
	int replace (int framesize, frame frametable[], PTE page_table[]) {
		int temp = 0;
		while(true){
			if(page_table[frametable[hand].mappedPage].referenced == 0){
				temp = hand;
				hand = (hand+1)%framesize;
				return temp;
			}
			else{
				page_table[frametable[hand].mappedPage].referenced = 0;
				hand = (hand+1)%framesize;
			}
		}
	}
};

class CLOCK_VIRT: public PageReplacement {
	private:
		int hand;
	public:
		CLOCK_VIRT(int head){
			hand = head;
		}
	int replace (int framesize, frame frametable[], PTE page_table[]) {
		int temp = 0;
		while(true){
			if(page_table[hand].present == 1 && page_table[hand].referenced == 0){
				temp = page_table[hand].frameindex;
				hand = (hand+1)%64;
				return temp;
			}
			else if(page_table[hand].present == 1 && page_table[hand].referenced == 1) {
				page_table[hand].referenced = 0;
				hand = (hand+1)%64;
			}
			else{
				hand = (hand+1)%64;
			}
		}
	}
};

class AGING_PHYS: public PageReplacement {
	private:
		vector<unsigned int> ages;
	public:
		AGING_PHYS(int framesize){
			ages.resize(framesize);
		}
	int replace (int framesize, frame frametable[], PTE page_table[]) {
		int minindex = 0;
		unsigned int intmask = 0x80000000;
		for(int i = 0; i < ages.size(); i++){
			if(page_table[frametable[i].mappedPage].referenced == 1){
				ages[i] = (ages[i] >> 1) | intmask;
				page_table[frametable[i].mappedPage].referenced = 0;
			}
			else{
				ages[i] = (ages[i] >> 1);
			}
		}
		
		for(int i = 0; i < ages.size(); i++){
			if(ages[i] < ages[minindex]){
				minindex = i;
			}
		}
		
		ages[minindex] = 0;
		
		return minindex;
		
	}
};

class AGING_VIRT: public PageReplacement {
	private:
		vector<unsigned int> ages;
	public:
		AGING_VIRT(int ptsize){
			ages.resize(ptsize);
		}
	int replace (int framesize, frame frametable[], PTE page_table[]) {
	
		int minindex = 0;
		unsigned int intmask = 0x80000000;
		for(int i = 0; i < ages.size(); i++){
			if(page_table[i].present == 1 && page_table[i].referenced == 1){
				ages[i] = (ages[i] >> 1) | intmask;
				page_table[i].referenced = 0;
			}
			else if(page_table[i].present == 1 && page_table[i].referenced == 0){
				ages[i] = (ages[i] >> 1);
			}
			else if(page_table[i].present == 0){
				ages[i] = 0;
			}
		}
		
		for(int i = 0; i < ages.size(); i++){
			if(page_table[i].present == 1){
				minindex = i;
				break;
			}
		}
		
		for(int i = 0; i < ages.size(); i++){
			if(page_table[i].present == 1 && ages[i] < ages[minindex]){
				minindex = i;
			}
		}
		
		ages[minindex] = 0;
		
		return page_table[minindex].frameindex;

	}
};

struct statistics {
	int unmaps;
	int maps;
	int ins;
	int outs;
	int zeros;
	int reads;
	int writes;
};

struct statistics stats = {};


int findFreeFrame(frame frametable[], int size){
	for(int i = 0; i < size; i++){
		if(frametable[i].mapped == false){
			return i;
		}
	}
	return -1;
}



int main (int argc, char **argv) {
	
	char* aValue = NULL;
	char* oValue = NULL;
	char* fValue = NULL;
	int option = 0;
	int numFrames = 32;
	int inst = 0;
	int vPage = 0;
	int inst_count = 0;
	unsigned long long int totalcost = 0ULL;
	bool oFlag = false;
	bool pFlag = false;
	bool fFlag= false;
	bool sFlag = false;
	PageReplacement * algo = new LRU();
	
// Handle options	
/////////////////////////////////////////////////////////////////////////////////////////////////////////	
	
	opterr = 0;
	
	while ((option = getopt (argc, argv, "a:o:f:")) != -1) {
		switch (option) {
			case 'a':
				aValue = optarg;
				break;
			case 'o':
				oValue = optarg;
				break;
			case 'f':
				fValue = optarg;
				break;
			case '?':
				if(optopt == 'a' || optopt == 'o' || optopt == 'f')
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint(optopt))
					fprintf (stderr, "Unknown option '-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
				return 1; 
			default:
				abort ();
		}
	}
	
	if(fValue != NULL){
		numFrames = atoi(fValue);
	}
	

	if(aValue != NULL){
		//store first char of aValue in char variable
		char algType = *aValue;
		switch(algType){
			case 'l':
			{
				delete algo;
				algo = new LRU();
				break;
			}
			case 'r':
			{
				delete algo;
				algo = new RANDOM();
				break;
			}
			case 'f':
			{
				delete algo;
				algo = new FIFO(0);
				break;
			}
			case 's':
			{
				delete algo;
				algo = new SECONDCHANCE(0);
				break;
			}	
			case 'c':
			{
				delete algo;
				algo = new CLOCK_PHYS(0);
				break;
			}
			case 'a':
			{
				delete algo;
				algo = new AGING_PHYS(numFrames);
				break;
			}
			case 'N':
			{
				delete algo;
				algo = new NRU(0);
				break;
			}
			case 'C':
			{
				delete algo;
				algo = new CLOCK_VIRT(0);
				break;
			}
			case 'A':
			{
				delete algo;
				algo = new AGING_VIRT(64);
				break;
			}
			default:
				break;
		}
	}
	
	if(oValue !=NULL){
		string s = string(oValue);
		for(int i = 0; i < s.length(); i++){
			char optType = s[i];
			switch(optType){
				case 'O':
					oFlag = true;
					break;
				case 'P':
					pFlag = true;
					break;
				case 'F':
					fFlag = true;
					break;
				case 'S':
					sFlag = true;
					break;
				default:
					printf("Unknown option. using none as default. \n");
					break;
			}
		}
	}	

/////////////////////////////////////////////////////////////////////////////////////////////////////////////	

	char line[2048];
	char countRand[2048];
	char *token;
	struct frame frametable[numFrames];

	for(int i = 0; i < numFrames; i++){
		frametable[i].mapped = false;
	}
	
	FILE *inputfile = fopen(argv[optind], "r");
	FILE *randfile = fopen(argv[optind+1], "r");
	
	if(inputfile == NULL) {
		printf("Error: inputfile not found.\n");
		return 1;
	}

	if(randfile == NULL) {
		printf("Error: randfile not found.\n");
		return 1;
	}
	
	fgets(countRand, 2048, randfile); 
    randSize = atoi(countRand); 
	
	//store rand numbers into array 
    for(int i = 0; i < randSize; i++) { 
        fgets(line, 2048, randfile); 
        randvals.push_back(atoi(line));  
    } 
	
	fclose(randfile);
	
	int tempframe = 0;
	while (fgets(line, 2048, inputfile))
	{	
		//skip comment lines
		if(*line == '#') continue;
		token = strtok(line, " \t\n");
		inst = atoi(token);
		token = strtok(NULL, " \t\n");	
		vPage = atoi(token);	
		
		if(oFlag){
			printf("==> inst: %d %d\n", inst, vPage);
		}
		
		if(page_table[vPage].present == 0){
			int index = findFreeFrame(frametable, numFrames);
			
			//there are no available free frames
			if(index == -1){
				index = algo->replace(numFrames, frametable, page_table);
				//was swapped out before but not modified since (no need to swap out since content is the same)
				if(page_table[frametable[index].mappedPage].pagedout == 1 && page_table[frametable[index].mappedPage].modified == 0) {
					
					page_table[frametable[index].mappedPage].present = 0;
					
					if(oFlag){
						printf("%d: UNMAP %3d %3d\n", inst_count, frametable[index].mappedPage, index);
					}
					stats.unmaps++;
				}
				
				//was modified (need to swap out)
				if(page_table[frametable[index].mappedPage].modified == 1) {
					
					page_table[frametable[index].mappedPage].present = 0;
					page_table[frametable[index].mappedPage].modified = 0;
					page_table[frametable[index].mappedPage].pagedout = 1;
					
					
					if(oFlag){
						printf("%d: UNMAP %3d %3d\n", inst_count, frametable[index].mappedPage, index);
						printf("%d: OUT %5d %3d\n", inst_count, frametable[index].mappedPage, index);
					}
					
					stats.unmaps++;
					stats.outs++;
					
				}
				
				//wasn't swapped out before and wasn't modified
				if(page_table[frametable[index].mappedPage].pagedout == 0 && page_table[frametable[index].mappedPage].modified == 0) {
					
					page_table[frametable[index].mappedPage].present = 0;
					
					if(oFlag){
						printf("%d: UNMAP %3d %3d\n", inst_count, frametable[index].mappedPage, index);
					}
					stats.unmaps++;
				}
			}
			
			//determine ZERO or IN
			//was paged out (need to IN)
			if(page_table[vPage].pagedout == 1){
				if(oFlag){
					printf("%d: IN %6d %3d\n", inst_count, vPage, index);
				}
				stats.ins++;
			}
				
			//was not paged out (need to ZERO)
			else{
				if(oFlag){
					printf("%d: ZERO %8d\n", inst_count, index);
				}
				stats.zeros++;
			}
			
			page_table[vPage].present = 1;
			page_table[vPage].referenced = 1;
			if(inst == 1) {
				page_table[vPage].modified = 1;
				stats.writes++;
			}
			else{
				stats.reads++;
			}
			page_table[vPage].frameindex = index;
			frametable[index].mapped = true;
			frametable[index].mappedPage = vPage;
			if(!usedlist.empty()){
				usedlist.erase(remove(usedlist.begin(),usedlist.end(),index), usedlist.end());
			}
			usedlist.insert(usedlist.begin(), index);
			
			stats.maps++;
			
			if(oFlag){
				printf("%d: MAP %5d %3d\n", inst_count, vPage, index);
			}
		}
		else{
			int index = page_table[vPage].frameindex;
			if(inst == 1){
				page_table[vPage].referenced = 1;
				page_table[vPage].modified = 1;
				stats.writes++;
			}
			else{
				page_table[vPage].referenced = 1;
				stats.reads++;
			}
			if(!usedlist.empty()){
				usedlist.erase(remove(usedlist.begin(),usedlist.end(),index), usedlist.end());
			}
			usedlist.insert(usedlist.begin(), index);
		}		
		inst_count++;
		
		/*
		for(int i = 0; i < 64; i++){
			if(page_table[i].present == 0 && page_table[i].pagedout == 1){
				printf("# ");
			}
		
			if(page_table[i].present == 0 && page_table[i].pagedout == 0){
				printf("* ");
			}
		
			if(page_table[i].present == 1){
				printf("%d:", i);
				if(page_table[i].referenced == 1){
					printf("R");
				}
				else{
					printf("-");
				}
			
				if(page_table[i].modified == 1){
					printf("M");
				}
				else{
					printf("-");
				}
			
				if(page_table[i].pagedout == 1){
					printf("S ");
				}
				else{
					printf("- ");
				}
			}	
		}
		printf("\n");
		for(int i = 0; i < numFrames; i++){
			if(frametable[i].mapped == true){
				printf("%d " , frametable[i].mappedPage);
			}
			else{
				printf("* ");
			}
		}
		printf("\n");
		*/
		
	}
	
	totalcost = (400ULL*(stats.unmaps + stats.maps)) + (3000ULL*(stats.ins + stats.outs)) + (150ULL*stats.zeros) + stats.reads + stats.writes;

	
	//P option (print state of pagetable)
	if(pFlag == true){
		for(int i = 0; i < 64; i++){
			if(page_table[i].present == 0 && page_table[i].pagedout == 1){
				printf("# ");
			}
		
			if(page_table[i].present == 0 && page_table[i].pagedout == 0){
				printf("* ");
			}
		
			if(page_table[i].present == 1){
				printf("%d:", i);
				if(page_table[i].referenced == 1){
					printf("R");
				}
				else{
					printf("-");
				}
			
				if(page_table[i].modified == 1){
					printf("M");
				}
				else{
					printf("-");
				}
			
				if(page_table[i].pagedout == 1){
					printf("S ");
				}
				else{
					printf("- ");
				}
			}	
		}
	}

	//F option (print frame table)
	if(fFlag == true){
		printf("\n");
		for(int i = 0; i < numFrames; i++){
			if(frametable[i].mapped == true){
				printf("%d " , frametable[i].mappedPage);
			}
			else{
				printf("* ");
			}
		}
	}
	
	if(sFlag == true){
		printf("\n");
		printf("SUM %d U=%d M=%d I=%d O=%d Z=%d ===> %llu\n", inst_count, stats.unmaps, stats.maps, stats.ins, stats.outs, stats.zeros, totalcost); 
	}
	
}



