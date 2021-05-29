# Cache-Emulator

### Introduction

In this final project you will implement a cache simulator. Your simulator will be configurable and will be able to handle caches with varying capacities, block sizes, levels of associativity, replacement policies, and write policies. The simulator will operate on trace files that indicate memory access properties. All input files to your simulator will follow a specific structure so that you can parse the contents and use the information to set the properties of your simulator.

After execution is finished, your simulator will generate an output file containing information on the number of cache misses, hits, and miss evictions (i.e. the number of block replacements). In addition, the file will also record the total number of (simulated) clock cycles used during the situation. Lastly, the file will indicate how many read and write operations were requested by the CPU.

It is important to note that your simulator is required to make several significant assumptions for the sake of simplicity.

    You do not have to simulate the actual data contents. We simply pretend that we copied data from main memory and keep track of the hypothetical time that would have elapsed.
    Accessing a sub-portion of a cache block takes the exact same time as it would require to access the entire block. Imagine that you are working with a cache that uses a 32 byte block size and has an access time of 15 clock cycles. Reading a 32 byte block from this cache will require 15 clock cycles. However, the same amount of time is required to read 1 byte from the cache.
    In this project assume that main memory RAM is always accessed in units of 8 bytes (i.e. 64 bits at a time).
    When accessing main memory, it's expensive to access the first unit. However, DDR memory typically includes a prefetch buffer which means that the RAM can provide access to the successive memory (in 8 byte chunks) with minimal overhead. In this project we assume an overhead of 1 additional clock cycle per contiguous unit.
    For example, suppose that it costs 255 clock cycles to access the first unit from main memory. Based on our assumption, it would only cost 257 clock cycles to access 24 bytes of memory.
    Assume that all caches utilize a "fetch-on-write" scheme if a miss occurs on a Store operation. This means that you must always fetch a block (i.e. load it) before you can store to that location (if that block is not already in the cache).


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
