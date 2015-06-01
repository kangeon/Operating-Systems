///////////////////////////////////////////////////
// Author: Geon Kang							 //
// NYUID: N17120399                              //
// Operating Systems 						     //
// Spring 2014									 //
// Lab 4: IO Scheduler   					     //
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
#include <deque>

using namespace std;

bool godown = false;
bool scanning_Q1 = false;
bool first = true;

struct request {
	int arrival_time;
	int track;
	int finish_time;
	int wait_time;
};

class Scheduler {
	public:
		virtual request pick_request (deque<request>& readyQ, int current_track) = 0;
};

class FIFO: public Scheduler {
	public:
		request pick_request (deque<request>& readyQ, int current_track) {
			request temp = readyQ.front();
			readyQ.pop_front();
			return temp;
		};
};

class SSTF: public Scheduler {
	public:
		request pick_request (deque<request>& readyQ, int current_track) {
			int min_seek = abs(readyQ.front().track - current_track);
			int min_index = 0;
			for(int i = 0; i < readyQ.size(); i++){
				if(abs(readyQ[i].track - current_track) < min_seek){
					min_seek = abs(readyQ[i].track-current_track);
					min_index = i;
				}
			}
			request temp = readyQ[min_index];
			readyQ.erase(readyQ.begin()+min_index);
			return temp;
		};
};

class SCAN: public Scheduler {
	public:
		request pick_request (deque<request>& readyQ, int current_track) {
			int min_seek = 0;
			bool found = false;
			int select_index = 0;
			
			
			if(godown == false){
				for(int i = 0; i < readyQ.size(); i++){
					if(readyQ[i].track - current_track >= 0){
						min_seek = abs(readyQ[i].track - current_track);
						select_index = i;
						found = true;
						godown = false;
						break;
					}
				}
			
				for(int i = 0; i < readyQ.size(); i++){
					if(readyQ[i].track - current_track >= 0){
						if(abs(readyQ[i].track - current_track) < min_seek){
							min_seek = abs(readyQ[i].track - current_track);
							select_index = i;
							found = true;
							godown = false;
						}
					}
				}
			
				if(found == false){
					for(int i = readyQ.size()-1; i >= 0; i--){
						if(readyQ[i].track - current_track < 0){
							min_seek = abs(readyQ[i].track - current_track);
							select_index = i;
							found = true;
							godown = true;
						}
					}
				
					for(int i = readyQ.size()-1; i >= 0; i--){
						if(readyQ[i].track - current_track <= 0){
							if(abs(readyQ[i].track - current_track) < min_seek){
								min_seek = abs(readyQ[i].track - current_track);
								select_index = i;
								found = true;
								godown = true;
							}	
						}
					}	
				}
			}
			
			else{
				for(int i = readyQ.size()-1; i >= 0; i--){
					if(readyQ[i].track - current_track <= 0){

						min_seek = abs(readyQ[i].track - current_track);
						select_index = i;
						found = true;
						godown = true;

					}
				}
				
				for(int i = readyQ.size()-1; i >= 0; i--){
					if(readyQ[i].track - current_track <= 0){
						if(abs(readyQ[i].track - current_track) < min_seek){
							min_seek = abs(readyQ[i].track - current_track);
							select_index = i;
							found = true;
							godown = true;
						}	
					}
				}	
				

				
				if(found == false){
					for(int i = 0; i < readyQ.size(); i++){
						if(readyQ[i].track - current_track >= 0){
							min_seek = abs(readyQ[i].track - current_track);
							select_index = i;
							found = true;
							godown = false;
							break;
						}
					}
			
					for(int i = 0; i < readyQ.size(); i++){
						if(readyQ[i].track - current_track > 0){
							if(abs(readyQ[i].track - current_track) < min_seek){
								min_seek = abs(readyQ[i].track - current_track);
								select_index = i;
								found = true;
								godown = false;
							}
						}
					}
				}
			
			}

			request temp = readyQ[select_index];
			readyQ.erase(readyQ.begin()+select_index);

			return temp;
		};
};

class CSCAN: public Scheduler {
	public:
		request pick_request (deque<request>& readyQ, int current_track) {
			int min_seek = 0;
			bool found = false;
			int select_index = 0;
			
			for(int i = 0; i < readyQ.size(); i++){
				if(readyQ[i].track - current_track >= 0){
					min_seek = abs(readyQ[i].track - current_track);
					select_index = i;
					found = true;
					break;
				}
			}
			
			for(int i = 0; i < readyQ.size(); i++){
				if(readyQ[i].track - current_track >= 0){
					if(abs(readyQ[i].track - current_track) < min_seek){
						min_seek = abs(readyQ[i].track - current_track);
						select_index = i;
						found = true;
					}
				}
			}
			
			if(found == false){
				current_track = 0;
				for(int i = 0; i < readyQ.size(); i++){
					if(readyQ[i].track - current_track >= 0){
						min_seek = abs(readyQ[i].track - current_track);
						select_index = i;
						found = true;
						break;
					}
				}	
				
				for(int i = 0; i < readyQ.size(); i++){
					if(readyQ[i].track - current_track >= 0){
						if(abs(readyQ[i].track - current_track) < min_seek){
							min_seek = abs(readyQ[i].track - current_track);
							select_index = i;
							found = true;
						}
					}
				}
			}
			
			request temp = readyQ[select_index];
			readyQ.erase(readyQ.begin()+select_index);

			return temp;
		};
};

class FSCAN: public Scheduler {
	public:
		request pick_request (deque<request>& readyQ, int current_track) {
			int min_seek = 0;
			bool found = false;
			int select_index = 0;
			
			if(godown == false){
				for(int i = 0; i < readyQ.size(); i++){
					if(readyQ[i].track - current_track >= 0){
						min_seek = abs(readyQ[i].track - current_track);
						select_index = i;
						found = true;
						godown = false;
						break;
					}
				}
			
				for(int i = 0; i < readyQ.size(); i++){
					if(readyQ[i].track - current_track >= 0){
						if(abs(readyQ[i].track - current_track) < min_seek){
							min_seek = abs(readyQ[i].track - current_track);
							select_index = i;
							found = true;
							godown = false;
						}
					}
				}
			
				if(found == false){
					for(int i = readyQ.size()-1; i >= 0; i--){
						if(readyQ[i].track - current_track < 0){
							min_seek = abs(readyQ[i].track - current_track);
							select_index = i;
							found = true;
							godown = true;
						}
					}
				
					for(int i = readyQ.size()-1; i >= 0; i--){
						if(readyQ[i].track - current_track <= 0){
							if(abs(readyQ[i].track - current_track) < min_seek){
								min_seek = abs(readyQ[i].track - current_track);
								select_index = i;
								found = true;
								godown = true;
							}	
						}
					}	
				}
			}
			
			else{
				for(int i = readyQ.size()-1; i >= 0; i--){
					if(readyQ[i].track - current_track <= 0){

						min_seek = abs(readyQ[i].track - current_track);
						select_index = i;
						found = true;
						godown = true;

					}
				}
				
				for(int i = readyQ.size()-1; i >= 0; i--){
					if(readyQ[i].track - current_track <= 0){
						if(abs(readyQ[i].track - current_track) < min_seek){
							min_seek = abs(readyQ[i].track - current_track);
							select_index = i;
							found = true;
							godown = true;
						}	
					}
				}	
				

				
				if(found == false){
					for(int i = 0; i < readyQ.size(); i++){
						if(readyQ[i].track - current_track >= 0){
							min_seek = abs(readyQ[i].track - current_track);
							select_index = i;
							found = true;
							godown = false;
							break;
						}
					}
			
					for(int i = 0; i < readyQ.size(); i++){
						if(readyQ[i].track - current_track > 0){
							if(abs(readyQ[i].track - current_track) < min_seek){
								min_seek = abs(readyQ[i].track - current_track);
								select_index = i;
								found = true;
								godown = false;
							}
						}
					}
				}
			
			}

			request temp = readyQ[select_index];
			readyQ.erase(readyQ.begin()+select_index);
			if(readyQ.empty()){
				godown = false;
			}
			return temp;
		};
};

deque<request> eventQ;
deque<request> readyQ;
deque<request> readyQ2;
deque<request> completeQ;

int main (int argc, char **argv) {
	
	char* sValue = NULL;
	int option = 0;
	bool sFlag = false;
	Scheduler * algo = new FIFO();
	
	int total_time = 0;
	int tot_movement = 0;
	int turnaround_sum = 0;
	int waittime_sum = 0;
	double avg_turnaround = 0.0;
	double avg_waittime = 0.0;
	int max_waittime = 0;
	bool f_flag = false;
	
    // Handle options	
    /////////////////////////////////////////////////////////////////////////////////////////////////////////	
	
	opterr = 0;
	
	while ((option = getopt (argc, argv, "s:")) != -1) {
		switch (option) {
			case 's':
				sValue = optarg;
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

	if(sValue != NULL){
		//store first char of sValue in char variable
		char algType = *sValue;
		switch(algType){
			case 'i':
			{
				delete algo;
				algo = new FIFO();
				break;
			}
			case 'j':
			{
				delete algo;
				algo = new SSTF();
				break;
			}
			case 's':
			{
				delete algo;
				algo = new SCAN();
				break;
			}
			case 'c':
			{
				delete algo;
				algo = new CSCAN();
				break;
			}	
			case 'f':
			{
				delete algo;
				algo = new FSCAN();
				f_flag = true;
				break;
			}
			default:
				break;
		}
	}

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////	

	char line[2048];
	char *token;
	
	FILE *inputfile = fopen(argv[optind], "r");
	
	if(inputfile == NULL) {
		printf("Error: inputfile not found.\n");
		return 1;
	}

	//reads requests from file and puts them in the event queue
	int request_count = 0;
	while (fgets(line, 2048, inputfile))
	{	
		request req;
		//skip comment lines
		if(*line == '#') continue;
		token = strtok(line, " \t\n");
		req.arrival_time = atoi(token);
		token = strtok(NULL, " \t\n");
		req.track = atoi(token);
		req.finish_time = 0;
		req.wait_time = 0;
		request_count++;
		eventQ.push_back(req);
	}
	
	int time = 0;
	int current_track = 0;
	
	while(!eventQ.empty() || !readyQ.empty() || !readyQ2.empty()){
		int first_arrival_time;
		if(!eventQ.empty()){
			first_arrival_time = eventQ.front().arrival_time;
		}
		
		if(readyQ.empty() && readyQ2.empty() && time < first_arrival_time){
			time = first_arrival_time;
		}
		
		if(f_flag == true){
			while(!eventQ.empty() && eventQ.front().arrival_time <= time){
				request ready = eventQ.front();
				eventQ.pop_front();
				
				if(scanning_Q1 == false){
					readyQ.push_back(ready);
				}
				
				else{
					readyQ2.push_back(ready);
				}
			}	
		}
		
		else{
			while(!eventQ.empty() && eventQ.front().arrival_time <= time){
				request ready = eventQ.front();
				eventQ.pop_front();
				readyQ.push_back(ready);
			}
		}
	
		if(f_flag == true){
			if(scanning_Q1 == true && readyQ.empty()){
				scanning_Q1 = false;
			}
			
			else if(scanning_Q1 == false && readyQ2.empty()){
				scanning_Q1 = true;
			}
		}
		
		if(f_flag == true){
			if(!readyQ.empty() && scanning_Q1 == true){
				request current = algo->pick_request(readyQ, current_track);
				current.wait_time = time - current.arrival_time;
				if(current.wait_time > max_waittime){
					max_waittime = current.wait_time;
				}
				tot_movement = tot_movement + abs(current.track - current_track);
				time = time + abs(current.track - current_track);
				current_track = current.track;
				current.finish_time = time;
				turnaround_sum = turnaround_sum + (current.finish_time - current.arrival_time);
				waittime_sum = waittime_sum + current.wait_time;
				completeQ.push_back(current);
				scanning_Q1 = true;
			}
		
			else if(!readyQ2.empty() && scanning_Q1 == false){
				request current = algo->pick_request(readyQ2, current_track);
				current.wait_time = time - current.arrival_time;
				if(current.wait_time > max_waittime){
					max_waittime = current.wait_time;
				}
				tot_movement = tot_movement + abs(current.track - current_track);
				time = time + abs(current.track - current_track);
				current_track = current.track;
				current.finish_time = time;
				turnaround_sum = turnaround_sum + (current.finish_time - current.arrival_time);
				waittime_sum = waittime_sum + current.wait_time;
				completeQ.push_back(current);
				scanning_Q1 = false;
			}
		}

		else{
			if(!readyQ.empty()){
				request current = algo->pick_request(readyQ, current_track);
				current.wait_time = time - current.arrival_time;
				if(current.wait_time > max_waittime){
					max_waittime = current.wait_time;
				}
				tot_movement = tot_movement + abs(current.track - current_track);
				time = time + abs(current.track - current_track);
				current_track = current.track;
				current.finish_time = time;
				turnaround_sum = turnaround_sum + (current.finish_time - current.arrival_time);
				waittime_sum = waittime_sum + current.wait_time;
				completeQ.push_back(current);
			}	
		}
		
		
	}
	
	total_time = time;
	avg_turnaround = ((double)turnaround_sum/(double)request_count);
	avg_waittime = ((double)waittime_sum/(double)request_count);
	
	printf("SUM: %d %d %.2lf %.2lf %d\n", total_time, tot_movement, avg_turnaround, avg_waittime, max_waittime); 
	
}



