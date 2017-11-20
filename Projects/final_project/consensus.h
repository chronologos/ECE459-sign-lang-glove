#ifndef CONSENSUS_H
#define CONSENSUS_H

#include "mbed.h"                    
#include <string>
#include <vector>

namespace Consensus {
	
const int CONSENSUS_N = 10;
std::vector<string> consensus_queue;

void emptyQueue(){
	while (consensus_queue.size() > 0){
		consensus_queue.pop_back();
	}
	}
	
/* Check if last CONSENSUS_N readings constitute a consensus on what character was 
 * read. IMPT: Clears vector if no consensus. 
 */ 
bool hasConsensus(){
	if (consensus_queue.size() < CONSENSUS_N){
		return false;
	}
	string candidate = consensus_queue[0];
	for (int i =1; i<consensus_queue.size(); ++i){
		if (consensus_queue[i] != candidate){
			emptyQueue();
			return false;
		}
	}
	return true;
}

/* Return consensus value and clears vector, only safe to call if hasConsensus! */
std::string getConsensus(){
		string consensus = consensus_queue[0];
		emptyQueue();
		return consensus;
}
	
} /* namespace Consensus */

#endif /* CONSENSUS_H */
