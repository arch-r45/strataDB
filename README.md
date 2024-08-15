# Persistent, multithreaded key value log store written in c

![alt text][UnearthDB]

[UnearthDB]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/UnearthDB.png


## Table of Contents

[Overview](#overview)  
[Introduction](#introduction)  
[System Bootup](#System-Bootup)  
[Buffer Pool Theory](#Buffer-Pool-theory)  
[Buffer Pool Implementation](#buffer-pool-implementation)   
[Set API](#set-api)  
[Get API](#get-api)  
[Compaction](#compaction)  
[Concurrency Control and Non-Determinsm](#Concurrency-Control-and-Non-Determinsm)  
[Future Improvements and Summary](#Future-Improvements-and-Summary)   



## Overview

In this repository, I implement a persistent, multithreaded key value log store based on bitcask.  I implement this in C using the Linux Operating system and package it into a Docker container.  I use Linux DIRECT_IO to bypass the operating systems buffer cache and implement my own buffer pool manager that handles page replacement intelligently in user space.  This also avoids double caching that hinders systems like PostgreSQL who choose to implement their own buffer pool manager but not use DIRECT_IO.  My implementation is able to perform 15,000 persistent writes per second which is a big improvement on similar hardware to the original Bitcask paper of 6000 writes per second.  I also implement a thread safe, compaction algorithm that runs in a background thread.  The compaction algorithm saves 99 GB of space for every 100GB written by eliminating the vast majority of redundant data.  The system also handles race conditions eloquently and avoids deadlock entirely by never violating the hold and wait principle.   

![alt text][rantests]

[rantests]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/write_test.png

> Testing Results for 15,000 Writes

## Introduction

Most Database Management Systems follow a relational model, where each file on disk is an individual table of the database, and these files are brought into memory in 4 KB(sometimes 8KB) pages depending on what record is being read.  The relational model acts as a general purpose data model which is part of why it's been a mainstay for over 50 years and will continue to be for the foreseeable future. [1]  Popular implementations consist of Oracle DB, MYSQL, PostgreSQL and SQlite3.   RDBMS’s ensure ACID properties, allow for widespread querying of data with range queries and joins, and generally have scalable performance for most workloads.[2] [3]   The reliability and versatility of relational databases indicate why they far outpace other types of databases in terms of frequency of use among developers.  [10].  

However, specific applications are willing to make tradeoffs to forgo some of the benefits of relational databases in favor of different optimizations.  The tradeoffs they make depend on the specific loads the individual applications are trying to optimize for.  There are certain applications that want to prioritize fast reading of dating and certain applications that want to favor fast writing of data.  The general rule of thumb is that RDBMS’s are the optimal option for read-heavy workloads but LSM Trees(NOSQL model) are optimal for write-heavy workloads. [2]  Designing an index structure that optimizes for one of the two access methods(reads or writes), enforces a hard lower bound on the other access method [9], therefore strong-arming the application developer to make a choice in which access method to optimize for.   

An example of a read heavy workload would be a social media site like Twitter where the majority of the users are reading tweets at a much greater clip than writing tweets.  Therefore systems like Twitter prioritize making tweets quick to read, while focusing less on the speed of writing/posting a tweet. [2]  If you, as a user, post a tweet, it will take noticeably longer for that post to go through, whereas refreshing your timeline and viewing recent tweets feels instantaneous.  Twitter has orders of magnitude more users trying to view tweets at any given time than users posting tweets, so it makes sense for them to optimize for reads.[2]  In comparison, real time systems like financial trading companies need to be consistently performing writes to their databases to reflect micro changes in stocks and markets.  These writes are happening on the timescale of microseconds and major trading platforms like NYSE and HKEX can perform up to 200 billion trades per day.[13]  This requires a focus on ensuring high write throughput but also high write efficiency, because any lag in data changes can have serious real world financial implications for users.   The main difference in read/write throughput is due to the access methods of the data system and application developers need to choose the proper database that fits their applications needs.   
     
Whether a database optimizes for reads or writes comes down to the choice of index structure.  Both Relational Databases and LSM Trees rely on sorted tree structures for their indexes.[2] [3]  In a typical SQL relational database, a B tree data structure is used to find the specific page of the record, bring that page into memory, perform the insertion, then rewrite the whole page back to disk.  A B tree uses the sorted property to maintain that lookups are at worst O(LogH) time complexity [8], where H is the height of the tree, which is very efficient for random reads of records.  However, for writes it seems wasteful to have to keep rewriting full 4KB pages back to disk for just performing one insertion.  Also random access on disk is around 100 times slower than sequential access. [3]  


![alt text][disk]

[disk]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/B+tree.png

> B+ Tree Depiction (Source: *Database System Concepts: Seventh Edition* [3])



In contrast, LSM trees solve the problems of the above two limitations of B trees to provide a write heavy solution.  They use a red black tree as their in memory index [2] [3], which allows them to maintain the sorted order in memory.  When a write comes in, the key is added to the in memory structure even if the key already exists somewhere else on disk.  Once the segment reaches its byte limit, the whole segment is written to disk.  

This takes advantage of sequential I/O where the disk arm seeks are kept to a minimum.  In a random access pattern, the disk arm needs to seek a different location after every write and could have to travel to sectors on the disk that are far away.[3]  

![alt text][disk]

[disk]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/disk_diagram.png

> Diagram of Magnetic Disk (Source: *Database System Concepts: Seventh Edition* [3])

Also, by never rewriting existing pages, write amplification is reduced.  Write amplification never used to be of much concern for disk oriented storage, but the rise of fast SSD storage has made avoiding write amplification important.  This is because SSDs can only overwrite the same blocks a limited number of times before wearing out[2] in comparison to traditional disks which usually have a mean time to failure of over 50 years. [3]  There are obviously some disadvantages and tradeoffs you are making when you use this structure and like everything in computer science, there is no free lunch.  Because you are never updating old segments, you end up having a large amount of duplicate records in your file, and LSM trees can run a compaction algorithm in the background, but this can sometimes interfere with performance or in some cases lag behind and never catch up.  Also, transactional guarantees are more difficult to ensure in LSM Trees due to the data being written in multiple locations which leads to applications choosing relational Databases in applications where transactional guarantees matter, like banking.  Finally, reads aren't as efficient in LSM Trees as they are in B-Trees. The properties of B-Trees keep them short and fat, and since lookups are only O(log(height)), we never have to search for long.  For LSM trees, if the key does not exist, these structures could potentially have to look through all the log files in the worst case, making the time complexity 0(N/S) where S is the size of a page.  However, workarounds like Bloom Filters can solve this problem.[3]  Popular implementations of LSM trees are LevelDB from Google and a fork of LevelDB [11], RocksDB, from Facebook [12].

Both index structures allow for a large number of keys, because the sorted order allows you to not have to keep all the keys in memory.  In both cases, if you have a subset of the keys in memory and their subsequent byte offsets, you can easily determine where the location of your record is based on the sorted property.  However, as RAM has become cheaper, it's becoming more feasible to keep your entire database completely in memory.  Memory is not persistent, meaning if the power goes out, your database is lost.  Different implementations of main memory databases have come up with different solutions to this problem.  Memcached, which is a cache only database, does not care about persistence.[14]  The key value store Redis writes keys asynchronously where it's okay if some data is lost. [2]

This leads us to Bitcask, which pulls elements from both in memory databases and persistent LSM trees to offer very high write performance but also persistence.  The one constraint is that the keys to the database must be small enough to fit in available memory, but the values of the keys can be larger than available memory.  Bitcask is based on a paper implementation by Riak and was developed in 2013.   The authors intended the use case to focus on databases with a small number of keys, where the key values are constantly being written and updated. [4]  Popular examples of this would be applications like YouTube or Twitter, where the key might be a video ID or tweet ID, and the value is the number of views. The number of keys is relatively small compared to the number of writes occurring per second, as some videos or tweets receive millions of views.


![alt text][youtubeviews]

[youtubeviews]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/youtubeviews.png

> View Count of a Popular Youtube Video (Source: *youtube.com*)

The underlying index structure for Bitcask is a hashmap, and the storage structure consists of log files, similar to LSM trees.  Each log file has its own hashmap structure in memory.  Each time a read is needed, you simply check if the key is in the most recent log file's hashmap. The value associated with that key is the byte offset and size of the record on disk.  If the key doesn't exist in the most recent log file, you check the next hashmap and so on.  When you need to write a key, you simply append the record to the most recent log file and you place the key and byte offset in the associated hashmap.  Like LSM trees, there are no updates to existing records and once a log file is closed, it's considered immutable.  Because we could run out of disk space over time, a compaction process can run in a separate thread that can be used to discard old files and write the most recent updates to the current log file. [4] 

![alt text][compaction-diagram]

[compaction-diagram]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/compaction-diagram.png

> Compaction Algorithm on Sorted Keys (Source: *Designing Data-Intensive Applications*)

I implement my own version of Bitcask by following most of the design choices of the original authors and I extend their implementation with my own choices and optimizations.  Some differences arise due to me choosing to use C in order to have full control over the memory management of the process versus the Bitcask authors using the language, Erlang.  The biggest difference in architecture arose from me choosing to implement my own buffer pool manager and not rely on the operating system to manage the buffer cache.  This has been a highly contested debate within the database architecture community with the majority of experts and implementations siding with not relying on the OS to perform buffering. [5]  When people are designing their own database implementations it can be enticing to use the system call MMAP() which memory maps files to a logical address space.  You are getting the buffer management for free which can save a lot of development time because getting the buffer pool manager to work properly can be tricky. Additionally, kernel developers have spent a long time perfecting the OS’s buffer cache, so why reinvent it from scratch?  The argument against mmap() is that the operating system doesn't have insight into the database's operations, so it can't make as intelligent decisions as the database developer about which pages to keep in memory, bring into memory, or evict.  I do not choose to use MMAP() in my database and choose to use Direct_IO on Linux.  Direct_IO bypasses the operating systems buffer cache when the original call to open() is made and allows the user space to handle buffering. [6] Mac OS does not allow for Direct_IO and forces you to give hints, but the hints may or may not be received.  This leads to the double caching problem that systems like PostgreSQL run into.  At system boot up, PostgreSQL can only allocate half of the memory compared to other systems like MYSQL and RocksDB because of this double caching problem.  Double caching occurs when systems manage their own buffer pool but also read files through the operating system's filesystem, causing those files to be loaded into the OS's buffer cache as well.  This is redundant because we do not need to buffer data in two separate places.  RocksDB was a fork of LevelDB and the first change they made was to get rid of MMAP and implement their own buffer pool manager.[5]


```c
\\ Linux open system call with Direct_IO
open(path, O_RDWR|O_DIRECT, 0666);
```

![alt text][open-call]

[open-call]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/opencall.png

> Linux open() Flags (Source: *The Linux Programming Interface*[6])

## System Bootup

At system bootup, I first initialize a buffer separate from my buffer pool manager called directory_buffer.  At first, this seems to violate the sequential writes advantages of a log structured output file DB.  Yes, random access is required with a directory log, but the cost of writes is amortized over time because you are only performing random writes every 4096 bytes(size of each page).  This is because we only need to write the directory buffer when we add a new file number to it and this only occurs when we run out of space on the current page and move onto the next page.  The directory buffer is very simple but also very important.  It records the file number of each file and persists this on disk in case of system crash where we can then reboot and rebuild the subsequent hash maps.  We also need the directory buffer in memory to be able to loop through all the hash maps in order to read older keys, and compact the files together. 

When construct_hash_map_from_directory() is called, we try to read in the contents of db/dir, if nothing is read we know we are starting from a blank slate.  We make some more initializations and we can start writing to our first file.  If the contents are read from db/dir, we know there is already existing data in the database and we need to reconstruct the hashmaps in memory to be able to access some of this older data, and make sure the writer_thread is set to whatever the final byte offset is of the last record in our directory buffer.  

Some implementations use popular libraries for encoding binary objects into byte sequences.  Popular implementation that most people have heard of is Python’s library Pickle.   It's generally advised against using these “cheap” tricks because they bring in many issues related to security, performance and space utilization. [2]

![alt text][dir_buf]

[dir_buf]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/dir_buf.png

> My directory buffer:
>
> ```c
> int directory_buffer[1024];
> ```

For me, constructing the hashmaps consisted of simply reading all the files into memory and going through and constructing each hashmap in a linear scan fashion.  This can be slow and make boot-ups very time intensive if the number of files is very large.  Bitcask solves this problem by constructing a hint file during compaction of existing keys, which can greatly speed up boot up time.  If the hint file for a particular file exists, the database scans that one instead, which just contains the keys and the offsets, and the full metadata is in the normal file.  This is something that I neglected but the implementation would not be too hard to add at a later point. 

## Buffer Pool Theory
 

Virtual memory in operating systems is the idea that we need to be able to execute a process that is larger than available memory.  One reason is because your computer usually has numerous processes running simultaneously, so each process can’t have access to all available memory on a system. Another reason we want to do this is because we have way less RAM than disk space, and in the typical Von Neumann architecture of a computer, we need to bring programs and data into memory for them to be eligible to execute on a CPU.  The way the OS allows this to happen is through demand paging.  When we need a file from disk, we read it from disk, store it in a physical memory location, expose the process to a logical location of that physical memory and allow the process to access that memory.  Because reading from disk is orders of magnitude slower than reading from RAM, and processes need to reuse certain pieces of memory, it makes no sense to relinquish that memory after use. [7]


![alt text][memory]

[memory]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/memory.png

> Storage Device Hierarchy of a Modern Day Computer

Therefore the operating system's job is to try to give the illusion to the running process that every piece of data they access is in memory even if the amount of data is greater than the total available memory on the system.  The way they do this is through a buffer cache.  When a file is read by a process, first the OS checks if the file is in the buffer cache, if it is, it returns a pointer to that memory location.  If it's not, it checks a free frame list to see if there are any free frames available in the buffer cache.  If there are free frames, it then makes the call to disk and loads the file contents in the memory location that was free, then returns a pointer to that memory location.  If there are no free frames, it then needs to evict data that is not being used from the buffer cache, to free up space for the new file. [7] There have been many papers and different attempts on the best eviction protocols, but Least Recently Used(LRU) eviction policies are what most operating systems use.  The idea is that a doubly linked list is used with a *sentinel* (dummy) variable at the head and a *sentinel* variable at the tail to mark the insertion and deletion locations.  Everytime something is accessed from the buffer pool it is added to the front of the link list.  This is done even if the page is located in the middle of the linked list or its being read in for the first time.  This can be done in O(1) constant time [8] even if the value is already in the middle of the linked list because all you have to do is connect the previous nodes pointer to the next nodes pointer and vice versa.  When you need to evict, you simply connect the tail.prev pointer to current_node.prev.next pointer and vice versa. 

In database management systems, this becomes even more important due to the sheer size of some databases and the painfully slow speed it takes to read from disk.  Coupled with the proliferation of data that has emerged over the course of the current century, and the need for quick response times for web pages, minimizing the amount of block transfers between disk and memory is paramount.  Many popular DBMS’s started out with relying on the operating system for buffer caching, and almost all of them have eventually switched to using direct IO and relying entirely on their own buffer pool manager.  [5] This is because for general purpose programs that the operating system needs to support, it's not possible to accurately predict which blocks will be referenced.  In result, the operating system predicts future block accesses by the past pattern of block references.  [3].  A database system is able to accurately predict the pattern of future references for its application better than an operating system can for general purpose programs.  Database engineers have exploited this property in development and use it to boost query performance.  

Bitcask chose not to implement a buffer pool manager and their reasoning centered around the fact they were already getting so much for free from the operating system.[4]  It definitely makes more sense for relational database management systems to pay the costs of implementing their own because of the abundance of join and range queries that are not possible in a system like Bitcask but common in a relational DB. 

## Buffer Pool Implementation

![alt text][buffer]

[buffer]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/bufferpool.png

> My buffer pool in the midst of executing a read_from_buffer_pool() call. 

There is a boot up call to the buffer pool manager that initializes the buffer_pool struct which is the main struct for the buffer pool.  The main variables here are the pointer to block which points to the first memory address of the block of the big block of memory that is going to be added when we call 

```c
void boot_up_buffer_pool();
```

We will allocate a fixed amount of bytes when we make the call and also have control over the size of the page.  The only requirement is the pages need to be the same size throughout the block.  Without this requirement, pointer arithmetic would be harder and there lacks a valid reason to have different sizes when we are just reading and writing from log files.  Usually block sizes are 4 KB or 8 KB even in relational databases  and there's a couple main reasons for this. First, I/O system calls are expensive, we do not want to bring in one record to a block everytime we make an I/O system call; we would rather just bring in the whole block and perhaps use some of the other data at another time.  Also 4KB is the max upper limit the operating system guarantees that the write will happen atomically.  Anything more than 4KB and we risk the I/O scheduler reordering our write call and perhaps losing some of the data in a system crash which therefore makes it impossible to guarantee that the transaction was atomic.  Atomicity is obviously a huge deal in transactional guarantees, and we need this all or nothing property to hold in most databases.  


The read api that we expose to the rest of the application is the only entry point from the rest of our application into our buffer pool.  
```c
char * read_from_buffer_pool(char *path)
```
We accept, as input, a pointer to a string path that represents the log file path of the file we are trying to access and we output a pointer to the address of the beginning of the block in our buffer pool where this file is located.  This makes the API that we are exposing very similar to a read() system call,  therefore expressing a high degree of modularity by keeping the components from becoming too intertwined.  Once the call is made, we check if the path is already in our buffer pool, and if it is, we return a pointer to that location and we send the node representing the path to the front of our lru_cache.   We keep a hash map called a page table which can be used to efficiently check in constant time if our path is already in our buffer pool.  If the path is not already in our buffer pool, we try to see if there are any free frames in our free frame list.  If there are free frames, we read the file from disk into the free location and set the path to that location in our page_table while also adding the node to the front of the lru_cache.  If the free frame list is empty, we need to evict.  My eviction policy is an LRU cache which I described above but I want to try different variations of it.  I also want to experiment with a Least Frequently Used(LFU) Cache for this use case but it's hard to make this work well on modern hardware. [3]  

One of the most important parts of the buffer pool is making sure to pin certain blocks of memory in place when they are being accessed by some thread.  In a concurrent application with many workers fulfilling concurrent read operations, problems can arise without pinning certain memory blocks down.  This can occur if one thread accesses a block of memory, and in the middle of its operation, another thread forces the system to evict, and the memory location the first thread was accessing is in the same memory location, but the data inside that location changes.  In my application, I avoided having concurrent reads initially but it is something I am looking to add.  However, I still needed to implement a pinning mechanism because my compaction algorithm is running in parallel to my main thread.  The compaction algorithm also runs a greater risk of getting blocks evicted because it performs linear scans through older files while the main thread is quickly serving reads on more current files.

The biggest problem of pinning is the risk of thrashing.  Thrashing is another concept from operating systems, and its definition is when a process spends more time paging new files then executing.  This can happen if the number of concurrent processes(or threads in databases) is too high, and each time a process tries to access a file, it page-faults and needs to evict some other process from memory.  In most operating systems, the CPU then gets underutilized and therefore increases the degree of multiprogramming, resulting in more thrashing.  [7]


![alt text][thrashing]

[thrashing]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/thrashing.png

> Thrashing Depiction (Source: *Operating System Concepts:Tenth Edition* [7])

In database systems we are not immune to performance issues in regard to thrashing because we use pins.  Some operating systems use different but similar types of pins as well and they run into performance degradation because the new processes end up in queues waiting to be serviced longer than they spend executing, destroying performance.[7]  When we implement concurrent reads, we need to ensure the tradeoffs between parallelism and cache misses are balanced to avoid performance decline.  


## Set API

Our set call takes as input a pointer to a character array as the key and a pointer to a character array as the value and returns an integer of 0 for success and -1 for failure.    
```c
 int set(char * key, char * value)
```

Because we only have one writer thread to take advantage of sequential access being significantly faster than random access, we need to index into our directory buffer and receive the current file path.  In relational databases, they can have concurrent writes to the database and this is because each write needs to be random access because of the underlying B tree index structure.  It's important that right before we access our current file in our directory buffer, we set a mutex lock.  This is because our compaction thread is also writing new records concurrently and could cause the current file index to actually change,  leading us to an invalid memory location resulting in a segmentation fault.  This mutex lock ensures that over the course of our set call, no other thread competing for the same resource in our critical section can be touched.  This obviously slows a program down because the compaction thread will have to wait until the resource is available, but correctness of a program needs to be ensured before speed.  


![alt text][setkey]

[setkey]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/set_key.png

> Memory and Disk Layout of an Individual Record

How the data is laid out on disk is shown above.  We use two 32 bit integers to determine the size of the key and values respectively.  We then write the keys and values out in binary.  The reason for having the key and value sizing is to avoid the need for escape characters (‘\0’) or delimiters and to be able to quickly load the key and value into character arrays upon retrieval.  The authors of Bitcask choose to exclude any extra compression techniques and cited that compression benefits are normally very application specific.  [4]  

We make a call to write_to_buffer_pool() to return the buffer location and also check to make sure the number of new bytes being written to our current buffer does not exceed the capacity.  If it does exceed, the current page we are on becomes immutable, and we switch pages.  Checking minimizes the amount of internal fragmentation within our log files while still ensuring each record doesn’t occupy multiple log files.  Certain databases allow records to span multiple pages but generally these are reserved for multimedia databases that contain records like images and videos that contain more bytes than a single page size.  [3] We also check if the current number of files exceeds a certain amount, which would invoke our compaction process which we describe later on. 

Once we then perform the write() system call, we add the string key as our key to our current files hashmap and set the value to an integer pointer pointing to an array of size 2 * sizeof(int).  This integer array contains the offset within the file where the record is located and the size of the file in bytes.  This hashmap *map* resides in a larger hashmap *master_map* which maps the file number to the memory location of the individual hashmap.  The individual hashmaps allows us to keep a smaller amount of keys in memory than the amount that resides on disk because hashmaps overwrite non unique keys.  It also allows us to easily jump to the location within each buffer in constant time and grab the exact size of the record. 

![alt text][structure]

[structure]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/hash_map_structure.png

> In Memory Hash Map Indexing Structure


## Get API

Our Get API takes as input a pointer to the address space of some character *key* and returns a character pointer to the value of that key.  On success, it returns the pointer the value of the key inserted, it returns a pointer to a char array "Key Does Not Exist" if not.  

```c
char *get(char *key)
```

When we enter the get call, we immediately need to place a mutex lock to ensure our compaction thread does not delete the file and subsequent buffer block from disk before we have returned our value.  Failure to do this will result in segmentation faults.  

Then we need to loop through our directory buffer in reverse.  The reason for performing this loop in reverse is that our most up to date value of a given key comes from the most recent insertion.  If you think of our buffer pool like a queue, we add new files to the end and we pop files from the front.  The main idea is we are going from back to front, or from newest to oldest, whatever the best way to describe it is.  We go through our records in our directory buffer and check to see if the key is in each record's hashmap.  This is very efficient, because for each hashmap we check we have average of O(1) constant time lookups on each key, so our time complexity ends up being 0(N) * O(1) or just O(N) with N being the number of records in our directory buffer.  However, the compaction algorithm ensures that N does not grow without bound, and everytime our file number gets above a certain threshold, it compacts the data together,  reducing the size of our directory buffer to some constant.  This ends up making our time complexity closer to O(1) constant time depending on what kind of data we are dealing with.  Also, because of compaction, we are constantly shifting our unique keys to the end of our directory buffer, further reducing our average time lookup of most keys.  

If we finish our loop, and don’t find any key, we know the value does not exist so we can return.  If not, we grab our buffer memory location from our buffer pool manager, index into our array in our hashmap to grab our offset and byte size and perform our lookup.  We have to help out our buffer pool before we return our value by releasing the pin the buffer pool set.  We can do this by flipping the bit.  Then we can release our mutex lock and return the value.  

## Compaction

Our compaction call only gets invoked when our file number goes above a certain threshold.  In our call, we initialize a temporary hashmap that is just string keys and string values.  We loop through our directory buffer starting from the front and add the keys and values to our hashmap, overwriting values that contain the same keys.  In C, this was trickier than other languages to implement as we had to keep track of where in the offset we were and ensure we did not go out of bounds.  The key_size and value_size helped significantly to ensure we were staying in bounds.  

After populating our temp_map, each key and value that occupy our hashmap is up to date and we can then loop through our temp_map and set all these values.  The one bug that could leak in is once we are done looping through all our files, we cannot add any more keys.  It takes some time to then loop through our hashmap and re-set() all these keys on our normal thread.  During that time, if we encounter new set() calls, we cannot add these to our hashmap because our hashmap needs to be closed off from accepting new values to ensure we eventually can exit out of our compaction() call.  So in order to ensure the most up to date value is in being set, we would need to hold a long mutex lock during this loop.  This obviously has serious performance ramifications and Bitcask was not specific with how they handled this in their paper. [4]  

After we are done with setting all the new values, we need to ensure we hold a mutex lock over our directory buffer while we go and delete our files that we no longer need and change our important current_fd_buffer_index variable.  We cannot have another thread entering into their critical section on a read and trying to index into a variable that then gets deleted by our compaction call.   We need to replace our directory buffer in global memory with our new buffer and then we can release the mutex lock.  


Lastly, the file threshold number was never specified in Bitcask’s papers, and this could be something that could be researched further.  It probably is very application dependent, but there should be an optimal threshold number that can reach some equilibrium point between amount of data on disk, and speed of executing reads and writes.  


The compaction technique here saves a lot of storage space.  Obviously if the application developer uses it incorrectly and has a large amount of unique keys, compaction will be less effective.  But this is very similar in Relational Database Management systems and building query optimizers.  In the end, the best relational query optimization plan can be rendered obsolete by incorrect use by the developer.  So we are making an assumption that the developer uses our tool properly when we assess the storage use benchmarks.   If we follow the constraint that all keys need to be able to fit in available memory, then we are assuming this database is used by applications with a small number of keys relative to the total number of individual records.  The compaction technique therefore could be estimated at saving 99 percent of the total storage space because we assume same-key writes are prominent.  In assessing the space complexity, it should always be linear in respect to the number of unique keys we have, because compaction clears out any duplicates.  Again, part of the space complexity relies on what we set the file threshold number to, but assuming we pick an optimal threshold, the space complexity should still be linear.  



## Concurrency Control and Non-Determinsm 

Computers are by law deterministic[15] meaning that each time we run a program with a set of inputs we expect it to produce the same exact output every time.  This phenomenon holds especially true for anyone who has ever written a simple serial program that contains a bug.  No matter how many times you retry the same program(to maybe print some output to the terminal in order to debug), you will get the same error message every time.  The reason is because computers obey laws of nature, which are themselves deterministic.  Anytime we encounter a phenomenon in nature that appears to give random outcomes, the variables are either too unknown to us, or too intractable to take account of.  [16]  Appearing as random does not invoke non-determinism in this sense.  The impossibility of writing a non-pseudo random number generator program illustrates that computers must follow deterministic instructions.  If one could write a true random number generator that actually produces a different answer with the exact same inputs, then determinism would be disproven not only in computers, but in physics as well.  Yet, we are left with computer programs that can only simulate the *effect* of randomness, including, but not limited to, some extremely clever techniques that take advantage of hardware decay and clocks.  These programs are not non-deterministic, as if someone was able to input the exact variables that the program is using, it will always return the same result.  

However, when we introduce memory-sharing threads with concurrency,  we can *appear* to get a completely different phenomenon altogether, as we can run the same program multiple times with the same input and receive different answers.  


![alt text][concurrency]

[concurrency]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/concurrency.png

> Example of inconsistent results of the same exact program


In the above terminal output, this is me running my main.test.c output file, testfile in consecutive iterations and yielding appearingly random results.  The tests will pass at 100% and then only at 80% rate, meaning some programs inside my test cases experienced deviations in their results on different iterations.  There was no recompiling of the program, these were simply output files being run consecutively.  In shared memory concurrency, these inconsistencies are caused by race conditions.  Because two threads can manipulate the same variable, if they both arrive at a shared variable at the same time, it becomes dependent on the CPU scheduler of the Operating System to decide which low level assembly instructions to execute first.[]  Because the CPU scheduler is complex and below the level of abstraction of the individual programmer, it is pretty intractable to understand how exactly it's going to execute at a specific point of time, because its managing numerous other processes.  


I was running into problems trying to figure out why I was getting inconsistent results.  This was arising from race conditions on some variables inside my set() call.  I would get the file index at the start of my call from my directory buffer and write the key and value to disk.  However, once the write was successful, I would then add the byte size and offset in the files hashmap for the respective key.  However, if the compaction thread's low level execution was intertwined with my set() call at this point, and the compaction call changes the file index in between writing the metadata to disk and setting to the hashmap, we would end up setting the file index and offset to the wrong hashmap(the next files hashmap).   If we then called get() in our tests on that key, we would have mismatched keys and return the error that is in the picture above.  In some cases, this would produce the wrong result, and in some cases it wouldn’t, which is why it seems to be non-deterministic.  The computer though is behaving exactly according to how its instructed, however from the developers perspective, its acting random, but its closely following the complicated code of the operating system.  

In order to solve this problem, I implemented mutex locks which prevent other threads from manipulating variables when locks are set.  This is expensive as it makes other threads stall which effectively reduces the degree of multiprogramming as one less thread can make progress.  However, locks are essential for guaranteeing correctness.  Mutex locks can lead to problems like deadlock, where multiple threads are waiting for a resource to consume that only the other thread can release.  I never ran into deadlock problems with my application.  This is because deadlock can never occur if the hold and wait condition never happens.  I made sure to not allow a thread that was trying to request a resource, to hold onto another resource.  


## Future Improvements and Summary

I originally set out to implement the unique functionality of Bitcask with the Get() and Set() and compaction() calls.  I also added the buffer pool manager that Bitcask neglected to add but many other data systems employ and used all my own data structure implementations.  These are by no means bug free and optimal, and I will want to continue to make changes to some of my existing functionality to make it better.  I also set out to replicate Bitcask’s performance metrics, but I have not rigidly benchmarked everything.  I was able to achieve 15,000 writes in just over a second but the testing could have been performed better and definitely had some limitations.  So my existing functionality could be improved, but at the same time I am doing this to learn, first and foremost,  and there are some other areas from other papers and textbooks that pique my interest and I want to look to add, even if this implementation isn’t perfect.

There are other areas that are not unique to Bitcask, but other systems have unique ways of implementing them.  I wish to explore these other areas in the future and connect those components to this implementation.  One database I am particularly interested in reimplementing parts of is Yellowbrick. [17]  They do really interesting engineering work by bypassing different areas of the operating system, and reimplementing their own mini unikernel.  I outline some of the cool engineering work they do below as well as some other areas that I want to explore on top of this project.

* tcp/ip networking
  * Aiming to introduce my own TCP/IP network protocol for communication with my process.  Yellowbrick uses kernel bypass here and grabs raw packets from the hardware and reimplements their own networking stack which would be cool to explore

* Parser-> Lexical Analyzer (Mini Compiler)
  * Want to implement the parser and LA from scratch in c, this would revolve around simulating an NFA inside a DFA and writing my own mini compiler(without anything past intermediate code gen)

* LSM Tree Extension(Red Black Tree Index)
  * Fork this in to an LSM Tree implementation with a red black tree index structure 
    * Then extend the compiler to write a query optimizer that can take existing queries and execute the one with the lowest costs.  LSM Trees can perform range queries so this will give me more to optimize

* Make it Distributed
  * I do not mean connect this to AWS, but make it evenly distributed across different running processes and ports to get a better working understanding on Distributed Systems

* Improve my hashmap implementations
  * I implemented all my hash maps in c, and because the hash maps were never getting seriously large, optimizing them did not matter.  With query optimization, it becomes vital to have ultra high performing hash maps so I want to explore optimizing them.  

* Memory Allocator
  * Yellowbrick does not call malloc() after they boot up, all memory is allocated in user space from the already allocated chunk of memory.  This is what the operating system performs for the user and can offer some great performance measurements according to Yellowbrick[17]

* Add FPGA/Hardware Accelerators
  * An interesting paper uses FPGA's to speed up the compaction for LSM Trees [18], Writing a hardware driver that connected to the database would be something I am dying to explore.   






[1]: https://db.cs.cmu.edu/papers/2024/whatgoesaround-sigmodrec2024.pdf
[2]: https://dataintensive.net/
[3]: https://db-book.com/
[4]: https://riak.com/assets/bitcask-intro.pdf
[5]: https://db.cs.cmu.edu/papers/2022/cidr2022-p13-crotty.pdf
[6]: https://man7.org/tlpi/
[7]: https://os-book.com/OS10/index.html
[8]: https://mitpress.mit.edu/9780262046305/introduction-to-algorithms/
[9]: https://openproceedings.org/2016/conf/edbt/paper-12.pdf
[10]: https://survey.stackoverflow.co/2024/technology#1-databases
[11]: https://opensource.googleblog.com/2011/07/leveldb-fast-persistent-key-value-store.html

[12]: https://scontent-bos5-1.xx.fbcdn.net/v/t39.8562-6/260203686_676839503287444_5314493774276549632_n.pdf?_nc_cat=103&ccb=1-7&_nc_sid=e280be&_nc_ohc=IJTyN6ica4wQ7kNvgFrxlQ9&_nc_ht=scontent-bos5-1.xx&oh=00_AYCXWpweCFtT2tD6TrytSOQBfGkrh5nhK8hGV8YK3u_SvQ&oe=66C174FC

[13]: https://www.hkex.com.hk/Market-Data/Statistics/Consolidated-Reports/Securities-Statistics-Archive/Trading_Value_Volume_And_Number_Of_Deals?sc_lang=en#select1=0&selection=2020-2024-(up-to-the-end-of-previous-month)

[14]: https://www.linuxjournal.com/article/7451

[15]: https://www2.eecs.berkeley.edu/Pubs/TechRpts/2006/EECS-2006-1.pdf

[16]: https://www.thebeginningofinfinity.com/

[17]: https://15721.courses.cs.cmu.edu/spring2024/papers/21-yellowbrick/p2-cusack.pdf

[18]: https://conferences.computer.org/icde/2020/pdfs/ICDE2020-5acyuqhpJ6L9P042wmjY1p/290300b261/290300b261.pdf
