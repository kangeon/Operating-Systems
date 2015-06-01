////////////////////////////////////////////////////////////
// Author: Geon Kang							  //
// NYUID: N17120399                           //
// Operating Systems 						  //
// Spring 2014									  //
// Lab 2: Scheduling							  //
////////////////////////////////////////////////////////////

#include <iostream>
#include <cstring>
#include <string>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <algorithm>
#include <vector>
#include <deque>

using namespace std;

struct process {
	int pid;
    int AT, TC, CB, IO, FT, TT, IT, CW;
	//reinsert time in ready queue
	int RAT; 
	//remaining CPU to completion
	int RC;
	int LO;
	bool preempted;
};

void FCFS(process ready, deque<process>& readyQ) {
	//ready queue is empty
	if(readyQ.empty()){
		readyQ.push_back(ready);
		return;
	}

	//find insertion point 	
	deque<process>::iterator it = readyQ.begin();
	for(int i = 0; i < readyQ.size(); i++){
		if(ready.RAT < readyQ[i].RAT){
		readyQ.insert(it,ready);
		return;
		}
		else{
			if(it != readyQ.end()){
				it++;
			}
		}
	}
	
	//if reach here, ready belongs at end of queue
	readyQ.push_back(ready);
	return;
}

void LCFS(process ready, deque<process>& readyQ) {
	//ready queue is empty
	if(readyQ.empty()){
		readyQ.push_front(ready);
		return;
	}

	//find insertion point
	deque<process>::iterator it = readyQ.begin();
	for(int i = 0; i < readyQ.size(); i++){
		if(ready.RAT >= readyQ[i].RAT){
		readyQ.insert(it,ready);
		return;
		}
		else{
			if(it != readyQ.end()){
				it++;
			}
		}
	}
	
	//if reach here, ready belongs at front of queue
	readyQ.push_front(ready);
	return;
}

void SJF(process ready, deque<process>& readyQ) {
	//ready queue is empty
	if(readyQ.empty()){
		readyQ.push_back(ready);
		return;
	}

	//find insertion point (by remaining time)
	deque<process>::iterator it = readyQ.begin();
	for(int i = 0; i < readyQ.size(); i++){
		if(ready.RC < readyQ[i].RC){
		readyQ.insert(it,ready);
		return;
		}
		else{
			if(it != readyQ.end()){
				it++;
			}
		}
	}
	
	//if reach here, ready belongs at end of queue
	readyQ.push_back(ready);
	return;
}

void RR(process ready, deque<process>& readyQ) {
	//ready queue is empty
	if(readyQ.empty()){
		readyQ.push_back(ready);
		return;
	}

	//find insertion point (by arrival time)	
	deque<process>::iterator it = readyQ.begin();
	for(int i = 0; i < readyQ.size(); i++){
		if(ready.RAT < readyQ[i].RAT){
		readyQ.insert(it,ready);
		return;
		}
		else{
			if(it != readyQ.end()){
				it++;
			}
		}
	}
	
	//if reach here, ready belongs at end of queue
	readyQ.push_back(ready);
	return;
}

void putEventQ(process event, deque<process>& eventQ){
	//event queue is empty
	if(eventQ.empty()){
		eventQ.push_back(event);
		return;
	}

	//find insertion point (by arrival time)	
	deque<process>::iterator it = eventQ.begin();
	for(int i = 0; i < eventQ.size(); i++){
		if(event.RAT < eventQ[i].RAT){
		eventQ.insert(it,event);
		return;
		}
		else{
			if(it != eventQ.end()){
				it++;
			}
		}
	}
	
	//if reach here, ready belongs at end of queue
	eventQ.push_back(event);
	return;
}

deque<process> eventQ;
deque<process> readyQ;
deque<process> completeQ;

//counter var for random number function
int ofs = 0;
vector<int> randvals;
int randSize;

//uniform repeatable random number generating function for CB and IO
int myrandom(int burst) {
	int value = 1+(randvals[ofs]%burst);
	//loop around if run out of numbers in array
	ofs = (ofs+1)%randSize;
	return value;
}

void (*schedule[4])(process ready, deque<process>& readyQ);

process get_event(deque<process>& q) {
	if(!q.empty()){
		process a = q.front();
		q.pop_front();
		return a;
	}
	else {
		cout << "Error: Empty Event Queue" << endl;
		abort();
	}
}

bool completeQSortCriteria(const process& p1, const process& p2){
	return p1.pid<p2.pid;
}


int main (int argc, char **argv) {
	
	schedule[0] = FCFS;
	schedule[1] = LCFS;
	schedule[2] = SJF;
	schedule[3] = RR;
	char *svalue = NULL;
	int c;
	int algIndex=0;
	bool quantumFlag = false;
	int cBurst=0;
	int ioBurst=0;
	int quantum=0;
	
	//prevent error message
	opterr = 0;
	
	//get option. only allow -s option
	while ((c = getopt (argc, argv, "s:")) != -1) {
		switch (c) {
			case 's':
				svalue = optarg;
				break;
			case '?':
				if(optopt == 's')
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

	//to dodge segfault in case no options are passed)
	if(svalue != NULL){
		//store first char of svalue (F,L,S,R) in char variable
		char algType = *svalue;
		switch(algType){
			case 'F':
				algIndex = 0;
				break;
			case 'L':
				algIndex = 1;
				break;
			case 'S':
				algIndex = 2;
				break;
			case 'R':
				//extract number from svalue
				quantum = atoi(svalue+1);
				if(quantum <= 0) {
					printf("Cannot burst if quantum is less than or equal to zero.\n");
					abort();
				}
				quantumFlag = true;
				algIndex = 3;
				break;
			default:
				printf("Unknown algorithm type. Please use F,L,S,or R.\n");
				abort();
				break;
		}
	}
	
	char line[2048];
	char countRand[2048];
	char *token;
	
	FILE *inputfile = fopen(argv[optind], "r");
	FILE *randfile = fopen(argv[optind+1], "r");
	
	if(inputfile == NULL) {
		printf("Error: inputfile not found.\n");
		return 1;
	}

	//reads processes from file and puts them in the event queue
	int idCount = 0;
	while (fgets(line, 2048, inputfile))
	{	
		process proc;
		token = strtok(line, " \t\n");
		while(token){
			proc.pid = idCount;
			idCount++;
			proc.AT = atoi(token);
			proc.RAT = proc.AT;
			proc.CW = 0;
			proc.LO = 0;
			proc.FT = 0;
			proc.TT = 0;
			proc.IT = 0;
			token = strtok(NULL, " \t\n");
			proc.TC = atoi(token);
			proc.RC = proc.TC;
			token = strtok(NULL, " \t\n");
			proc.CB = atoi(token);
			token = strtok(NULL, " \t\n");
			proc.IO = atoi(token);
			proc.preempted = false;
			token = strtok(NULL, " \t\n");
		}
		eventQ.push_back(proc);
	}
	
	if(randfile == NULL) {
		printf("Error: randfile not found.\n");
		return 1;
	}
	
	//get count of random numbers
	fgets(countRand, 2048, randfile);
	randSize = atoi(countRand);
	
	//store rand numbers into array
	for(int i = 0; i < randSize; i++) {
		fgets(line, 2048, randfile);
		randvals.push_back(atoi(line));	
	}
	
	fclose(inputfile);
	fclose(randfile);
	
	int ioStart=0;
	int ioEnd=0;
	int tmpStart=0;
	int tmpEnd=0;
	int ioUsedSum = 0;

	//event simulation infrastructure
	int time = 0;
	
	while(!eventQ.empty() || !readyQ.empty()){
		int firstRAT;
		if(!eventQ.empty()){
			firstRAT = eventQ.front().RAT;
		}
	    //if there is nothing in the ready queue to execute increment time to the arrival time of the first event in the event queue.
		if(readyQ.empty() && time < firstRAT){
			time = firstRAT;
		}
		
		//get all newly arrived processes and schedule into ready queue because time may have updated
		while(!eventQ.empty() && eventQ.front().RAT <= time){
			process ready = get_event(eventQ);
			(*schedule[algIndex])(ready, readyQ);
		}
		
		if(!readyQ.empty()){
			process running = get_event(readyQ);
			if(running.preempted == false){
				cBurst = myrandom(running.CB);
			}
			else{
				cBurst = running.LO;
				running.preempted = false;
			}

			running.CW = running.CW + (time - running.RAT);
			
			if(quantumFlag == true && quantum < cBurst) {
				//remaining CPU required is less than quantum
				if(running.RC <= quantum){
					time = time + running.RC;
					running.FT = time;
					running.TT = time - running.AT;
					completeQ.push_back(running);				
					cBurst = cBurst - running.RC;
				}
				//remaining CPU requires is more than quantum
				//preempt process and reschedule (no IO burst required)
				else{
					running.preempted = true;
					time = time + quantum;
					running.RC = running.RC - quantum;
					running.LO = cBurst - quantum;
					running.RAT = time;
					putEventQ(running, eventQ);
				}	
			}
			
			else{
				//remaining CPU required is less than cpu burst
				//complete running process and subtract amount of cpu used from cpu burst (for next process)
				if(running.RC <= cBurst){
					time = time + running.RC;			
					running.FT = time;
					running.TT = time - running.AT;
					cBurst = cBurst - running.RC;		
					completeQ.push_back(running);
				}
				//remaining CPU required is more than cpu burst
				//subtract amount of cpu burst used (used all) from remaining CPU required
				//set cpu burst to 0 because used up cpu burst
				//update rearrival time to time + ioBurst
				//reschedule into ready queue
				else{
					running.RC = running.RC - cBurst;
					time = time + cBurst;
					cBurst = 0;
					ioBurst = myrandom(running.IO);
					running.IT = running.IT + ioBurst;
					tmpStart = time;
					tmpEnd = time+ioBurst;
					if(tmpStart <= ioEnd && tmpEnd > ioEnd){
						ioUsedSum = ioUsedSum + (tmpEnd-ioEnd);
						ioEnd = tmpEnd;
					}
					else if(tmpStart > ioEnd){
						ioUsedSum = ioUsedSum + (tmpEnd-tmpStart);
						ioStart = tmpStart;
						ioEnd = tmpEnd;
					}
					running.RAT = time + ioBurst;
					putEventQ(running, eventQ);				
				}
			}
		}
	}
	
	//print results--------------------------------------------------------------------------------------------------------------------------------------------
	switch(algIndex){
		case 0:
			printf("FCFS\n");
			break;
		case 1:
			printf("LCFS\n");
			break;
		case 2:
			printf("SJF\n");
			break;
		case 3:
			printf("RR %d\n", quantum);
			break;	
		default:
			printf("Error: unknown scheduling algorithm.\n");
			abort();
	}
	
	int lastEventFT=0;
	int cpuUsedSum=0;
	int waitSum=0;
	int processCount=0;
	int turnaroundSum=0;

	//order completeQ by process id
	sort(completeQ.begin(),completeQ.end(),completeQSortCriteria);
	
	//print process info and gather values for summary information
	for(int i = 0; i < completeQ.size(); i++){
		printf("%04d: %4d %4d %4d %4d | %4d %4d %4d %4d\n", completeQ[i].pid, completeQ[i].AT, completeQ[i].TC, completeQ[i].CB, completeQ[i].IO, 
				completeQ[i].FT, completeQ[i].TT, completeQ[i].IT, completeQ[i].CW);
		if(completeQ[i].FT > lastEventFT){
			lastEventFT = completeQ[i].FT;
		}
		cpuUsedSum = cpuUsedSum + completeQ[i].TC;
		waitSum = waitSum + completeQ[i].CW;
		turnaroundSum = turnaroundSum + completeQ[i].TT;
		processCount++;
	}
	
	//calculate summary information
	double cpuUtilPerc = ((double)cpuUsedSum/(double)lastEventFT) * 100.0;
	double ioUtilPerc = ((double)ioUsedSum/(double)lastEventFT) * 100.0;
	double turnaroundAvg = (double)turnaroundSum/(double)processCount;
	double waitAvg = ((double)waitSum)/((double)processCount);
	double throughput = ((double)processCount/(double)lastEventFT) * 100.0;
	
	//print summary information
	printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", lastEventFT, cpuUtilPerc, ioUtilPerc, turnaroundAvg, waitAvg, throughput); 
	return 0;
}

