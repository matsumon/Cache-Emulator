/*
	Cache Simulator (Starter Code) by Justin Goins
	Oregon State University
	Spring Term 2021
*/

#include "CacheController.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <cmath>
#include <ctime>
#include <cstdlib>


using namespace std;
int randomNumber(int associativity){
	return rand()%associativity;
}
CacheController::CacheController(CacheInfo ci, string tracefile) {
	// store the configuration info
	this->ci = ci;
	this->inputFile = tracefile;
	this->outputFile = this->inputFile + ".out";
	// compute the other cache parameters
	this->ci.numByteOffsetBits = log2(ci.blockSize);
	this->ci.numSetIndexBits = log2(ci.numberSets);
	// initialize the counters
	this->globalCycles = 0;
	this->globalHits = 0;
	this->globalMisses = 0;
	this->globalEvictions = 0;
	
	// create your cache structure
	// ...
	this->cache = new AddressInfo[this->ci.numberSets * this->ci.associativity];
	for(int i = 0; i < this->ci.numberSets * this->ci.associativity; i++){
		this->cache[i].valid = 0;
	}
	// manual test code to see if the cache is behaving properly
	// will need to be changed slightly to match the function prototype
	/*
	cacheAccess(false, 0);
	cacheAccess(false, 128);
	cacheAccess(false, 256);

	cacheAccess(false, 0);
	cacheAccess(false, 128);
	cacheAccess(false, 256);
	*/
}

/*
	Starts reading the tracefile and processing memory operations.
*/
void CacheController::runTracefile() {
	srand(time(NULL)); //need to seed in order for random numbers

	cout << "Input tracefile: " << inputFile << endl;
	cout << "Output file name: " << outputFile << endl;
	
	// process each input line
	string line;
	// define regular expressions that are used to locate commands
	regex commentPattern("==.*");
	regex instructionPattern("I .*");
	regex loadPattern(" (L )(.*)(,)([[:digit:]]+)$");
	regex storePattern(" (S )(.*)(,)([[:digit:]]+)$");
	regex modifyPattern(" (M )(.*)(,)([[:digit:]]+)$");

	// open the output file
	ofstream outfile(outputFile);
	// open the output file
	ifstream infile(inputFile);
	// parse each line of the file and look for commands
	while (getline(infile, line)) {
		// these strings will be used in the file output
		string opString, activityString;
		smatch match; // will eventually hold the hexadecimal address string
		unsigned long int address;
		// create a struct to track cache responses
		CacheResponse response;

		// ignore comments
		if (std::regex_match(line, commentPattern) || std::regex_match(line, instructionPattern)) {
			// skip over comments and CPU instructions
			continue;
		} else if (std::regex_match(line, match, loadPattern)) {
			cout << "Found a load op!" << endl;
			istringstream hexStream(match.str(2));
			hexStream >> std::hex >> address;
			outfile << match.str(1) << match.str(2) << match.str(3) << match.str(4);
			cacheAccess(&response, false, address, stoi(match.str(4)));
			logEntry(outfile, &response);
			
		} else if (std::regex_match(line, match, storePattern)) {
			cout << "Found a store op!" << endl;
			istringstream hexStream(match.str(2));
			hexStream >> std::hex >> address;
			outfile << match.str(1) << match.str(2) << match.str(3) << match.str(4);
			cacheAccess(&response, true, address, stoi(match.str(4)));
			logEntry(outfile, &response);
		} else if (std::regex_match(line, match, modifyPattern)) {
			cout << "Found a modify op!" << endl;
			istringstream hexStream(match.str(2));
			// first process the read operation
			hexStream >> std::hex >> address;
			outfile << match.str(1) << match.str(2) << match.str(3) << match.str(4);
			cacheAccess(&response, false, address, stoi(match.str(4)));
			logEntry(outfile, &response);
			outfile << endl;
			// now process the write operation
			hexStream >> std::hex >> address;
			outfile << match.str(1) << match.str(2) << match.str(3) << match.str(4);
			cacheAccess(&response, true, address, stoi(match.str(4)));
			logEntry(outfile, &response);
		} else {
			throw runtime_error("Encountered unknown line format in tracefile.");
		}
		outfile << endl;
	}
	// add the final cache statistics
	outfile << "Hits: " << globalHits << " Misses: " << globalMisses << " Evictions: " << globalEvictions << endl;
	outfile << "Cycles: " << globalCycles << endl;

	infile.close();
	outfile.close();
}

/*
	Report the results of a memory access operation.
*/
void CacheController::logEntry(ofstream& outfile, CacheResponse* response) {
	outfile << " " << response->cycles <<" L1";
	if (response->hits > 0)
		outfile << " hit";
	if (response->misses > 0)
		outfile << " miss";
	if (response->evictions > 0)
		outfile << " eviction";
}

/*
	Calculate the block index and tag for a specified address.
*/
CacheController::AddressInfo CacheController::getAddressInfo(unsigned long int address) {
	AddressInfo ai;
	// this code should be changed to assign the proper index and tag
	bitset<64>binaryAddress(address); //converting int address to binary
	string binaryAddressString = binaryAddress.to_string();
	//splitting address into tag, index, and byte number componeents
	string tag = binaryAddressString.substr(0,binaryAddressString.size()-(this->ci.numByteOffsetBits + this->ci.numSetIndexBits));
	string lineSetIndex = binaryAddressString.substr(tag.length(),this->ci.numSetIndexBits);
	string byteNumber = binaryAddressString.substr(tag.length()+this->ci.numSetIndexBits,binaryAddressString.size());
	ai.tag = strtoul(tag.c_str(),nullptr,2); //string back into long int
	ai.setIndex = stoi(lineSetIndex,nullptr,2); //string back into int
	ai.byteNumber = stoi(byteNumber,nullptr,2); //string back into int
	return ai;
}

/*
	This function allows us to read or write to the cache.
	The read or write is indicated by isWrite.
	address is the initial memory address
	numByte is the number of bytes involved in the access
*/
void CacheController::cacheAccess(CacheResponse* response, bool isWrite, unsigned long int address, int numBytes) {
		response->cycles = 0;
		response->hits = 0;
		response->misses = 0;
		response->evictions = 0;
		int iterations = 0; //This is used to calculate the number of times that the user needs to check the cache
		CacheResponse localResponse;
	for(int i = 0; i<numBytes; i+= pow(2,this->ci.numByteOffsetBits)){
		localResponse.cycles = 0;
		localResponse.hits = 0;
		localResponse.misses = 0;
		localResponse.evictions = 0;
		AddressInfo ai = getAddressInfo(address + iterations * pow(2,this->ci.numByteOffsetBits));
		iterations++;
		//Modifies the for loop iterator to correctly advance through the cache
		if(i==0){
			i -= ai.byteNumber;
		}
		cout << "\tSet index: " << ai.setIndex << ", tag: " << ai.tag << endl;
		AddressInfo * tempCache = new AddressInfo[this->ci.associativity];
		//Copying the main cache to a temp cache for ease of modification
		for(int a = 0; a < this->ci.associativity; a++){
			tempCache[a] = this->cache[a + this->ci.associativity * ai.setIndex];
		}
		int foundIndex = 0; // 0 = false, 1 = already exists 2 - misses but valid place to put
		int entryIndex = -1;
		//Checking the cache tag against the desired tag
		for(int b = 0; b < this->ci.associativity; b++){
			if (tempCache[b].valid == 1 && tempCache[b].tag == ai.tag){
				foundIndex = 1;
				entryIndex = b;
				localResponse.hits++;
			}
			if (tempCache[b].valid == 0 && foundIndex == 0 ){
				foundIndex = 2;
				entryIndex = b;
			}
		}
		if(foundIndex == 2){
			localResponse.misses++;
		}else if(foundIndex == 0){
			localResponse.misses++;
			localResponse.evictions++;
		}
		//Editing the cache according to results of above lines and replacement policy
		if((foundIndex == 1 || foundIndex == 2) && entryIndex != -1){
			AddressInfo temp= tempCache[0];
			AddressInfo temp2 = tempCache[0];
			tempCache[0].valid = 1;
			tempCache[0].tag = ai.tag;
			tempCache[0].setIndex = ai.setIndex;
			for(int c = 1; c < entryIndex + 1; c++){
				temp = temp2;
				temp2 = tempCache[c];
				tempCache[c]=temp;
			}
		}
		else {
			if(this->ci.rp == ReplacementPolicy::LRU){
				AddressInfo temp= tempCache[0];
				AddressInfo temp2 = tempCache[0];
				tempCache[0].valid = 1;
				tempCache[0].tag = ai.tag;
				tempCache[0].setIndex = ai.setIndex;
				for(int c = 1; c < this->ci.associativity; c++){
					temp = temp2;
					temp2 = tempCache[c];
					tempCache[c]=temp;
				}
			} else if(this->ci.rp == ReplacementPolicy::Random){
				int indexReplace = randomNumber(this->ci.associativity);
				tempCache[indexReplace].valid = 1;
				tempCache[indexReplace].tag = ai.tag;
				tempCache[indexReplace].setIndex = ai.setIndex;
			}
		}
		//Copying temp cache back to main cache
		for(int a = 0; a < this->ci.associativity; a++){
			this->cache[a+this->ci.associativity*ai.setIndex] = tempCache[a];
		}
		// your code should also calculate the proper number of cycles that were used for the operation
		//Determing how many extra bytes for a RAM access
		int extraMemoryBlocks = (ceil((float)( (float)this->ci.blockSize) / 8.) - 1) ;
		extraMemoryBlocks = (extraMemoryBlocks > 0) ? extraMemoryBlocks : 0;
		if(isWrite == false){
			localResponse.cycles += 
				localResponse.hits * this->ci.cacheAccessCycles  +
				localResponse.misses * this->ci.cacheAccessCycles + 
				localResponse.misses * (this->ci.memoryAccessCycles + extraMemoryBlocks);
		}
		if(isWrite == true){
			localResponse.cycles += 
				localResponse.hits * this->ci.cacheAccessCycles  +
				localResponse.hits * (this->ci.memoryAccessCycles + extraMemoryBlocks) +
				2 * (localResponse.misses * this->ci.cacheAccessCycles + 
				localResponse.misses * (this->ci.memoryAccessCycles + extraMemoryBlocks));
		}
		//Updating global counters
		this->globalCycles += localResponse.cycles;
		this->globalHits += localResponse.hits;
		this->globalMisses += localResponse.misses;
		this->globalEvictions += localResponse.evictions;
		response->cycles += localResponse.cycles;
		response->hits += localResponse.hits;
		response->misses += localResponse.misses;
		response->evictions += localResponse.evictions;
		
	}
		// your code needs to update the global counters that track the number of hits, misses, and evictions
	if (response->hits > 0)
		cout << "Operation at address " << std::hex << address << " caused " << response->hits << " hit(s)." << std::dec << endl;
	if (response->misses > 0)
		cout << "Operation at address " << std::hex << address << " caused " << response->misses << " miss(es)." << std::dec << endl;

	cout << "-----------------------------------------" << endl;
	return;
}
