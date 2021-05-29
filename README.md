# Cache-Emulator

### Cache Simulator Control Flow

The cache simulator program takes two arguments, a cache configuration file and an input file. The cache simulator then outputs an output file that holds the global number of cycles, hits, misses, evictions, and a detailed description of every cache change. 


The cache simulator first reads the configuration file into a cache configuration data structure. Then the cache controller initializes an array of address info structures and clears their valid bits to represent that the cache blocks have meaningless data values.  Next, the input file is read line by line and the cache controller manipulates the cache array according to the configuration data structure. Lastly, results of the cache accesses are printed to an output file.
### Cache Data Structures

The cache simulator used an array of cache address structures. A cache address structure contains: 1) A tag, 2) The index of the block where the data is to be stored, 3) A valid bit to determine if a cache entry has data in it, and 4) A byte number that is used for determining whether a request is split across multiple cache blocks. 

The cache data configuration structure has fields for the number of byte offset bits, number of index bits, number of sets, associativity, replacement policy, write policy, cache access cycles, memory access cycles, and block size. 

### Cache Simulator Important Functions
#### getAddressInfo Function
The getAddressInfo function returns an AddressInfo structure that holds data regarding the tag, index, and byte number. The function converts the integer address into a binary value which is then split into the tag, index, and byte number. The byte number represents which byte an address points to within the given offset. 
#### cacheAccess Function
The cacheAccess function defines the operations of a cache according to a set of instructions. The function loops through a set of addresses dependent on the number of bytes requested by the user. After fetching the address information the function then copies the appropriate cache set into a new array. The temporary cache is then compared with the desired tag with hits, misses, and evictions being marked. The temp cache is then copied back into memory and the number of cycles is then calculated from the number of hits, misses, and evictions.
