# Persistent, multithreaded key value log store written in c


![alt text][UnearthDB]

[UnearthDB]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/UnearthDB.png


[Overview](#overview)  
[Introduction](#introduction)  
[System Bootup](#System-Bootup)  
[Buffer Pool Manager](#Buffer-Pool-Manager)  

## Overview

In this repository, I implement a persistent, multithreaded key value log store based on bitcask.  I implement this in C using the Linux Operating system and package it into a Docker container.  I use Linux DIRECT_IO to bypass the operating systems buffer cache and implement my own buffer pool manager that handles page replacement intelligently in user space.  This also avoids double caching that hinders systems like PostgresQL who choose to implement their own buffer pool manager but not use DIRECT_IO.  My implementation is able to perform 15,000 persistent writes per second which is a big improvement on similar hardware to the original Bitcask paper of 6000 writes per second.  I also implement a thread safe, compaction algorithm that runs in a background thread.  The compaction algorithm saves 99 GB of space for every 100GB written by eliminating the vast majority of redundant data.  The system also handles race conditions eloquently and avoids deadlock entirely by never violating the hold and wait principle.   

![alt text][rantests]

[rantests]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/write_test.png

## Introduction

Most Database Management Systems follow a relational model, where each file on disk is an individual table of the database, and these files are brought into memory usually in 4 KB pages depending on what record is trying to be read.  The relational model acts as a general purpose data model which is part of why it's been a mainstay for over 50 years and will continue to be for the foreseeable future. [1]  Popular implementations consist of Oracle DB, MYSQL, PostgresQL and SQlite3.   RDBMS’s ensure ACID properties, allow for widespread querying of data with range queries and joins, and generally have scalable performance for most workloads.   The general rule of thumb, however, is that RDBMS’s are the optimal option for read-heavy workloads but LSM Trees(NOSQL model) are optimal for write-heavy workloads. [2]  This difference in write throughput is due to the architectural designs of the implementation of the systems.   

Both B trees and LSM Trees rely on sorted tree structures for their indexes.  In a typical SQL relational database, a B tree data structure is used to find the specific page of the record, and then needs to bring that page into memory, perform the insertion, then rewrite the whole page back to disk.  A B tree uses the sorted property to maintain that lookups are at worst O(LogH) time complexity, where H is the height of the tree, which is very efficient for random reads of records.  However, for writes it seems wasteful to have to keep rewriting full 4KB pages back to disk for just performing one insertion.  Also random access on disk is around 100 times slower than sequential access. [3]  In contrast, LSM trees solve the problems of the above two limitations of B trees to provide a write heavy solution.  They use a red black tree as their in memory index, which allows them to maintain the sorted order in memory.  When a write comes in, the key is added to the in memory structure even if the key already exists somewhere else on disk.  Once the segment reaches its byte limit, the whole segment is written to disk.  

This takes advantage of sequential I/O where the disk arm seeks are kept to a minimum.  In a random access pattern, the disk arm needs to seek a different location after every write and could have to travel to sectors on the disk that are far away.[3]  

![alt text][disk]

[disk]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/disk_diagram.png

Also, by never rewriting existing pages, write amplification is reduced.  Write amplification never used to be of much concern for disk oriented storage, but the rise of fast SSD storage has made avoiding write amplification important.  This is because SSDs can only overwrite the same blocks a limited number of times before wearing out in comparison to traditional disks which usually have a mean time to failure of over 50 years. [2] [3]  There are obviously some disadvantages and tradeoffs you are making when you use this structure and like everything in computer science, there is no free lunch.  Because you are never updating old segments, you end up having a large amount of duplicate records in your file, and LSM trees can run a compaction algorithm in the background, but this can sometimes interfere with performance or in some cases lag behind and never catch up.  Also transactional guarantees are more difficult to ensure in LSM Trees due to the data being written possibly in multiple locations which leads to applications choosing B tree Databases in applications where transactional guarantees matter, like banking.  Popular implementations of LSM trees are LevelDB from Google and a fork of LevelDB, RocksDB, from Facebook.   

Both index structures allow for a large number of keys, because the sorted order allows you to not have to keep all the keys in memory.  This is because if you have a subset of the keys in memory and their subsequent byte offsets, you can easily determine where the location of your record is based on the sorted property.  However, as RAM has become cheaper, it's becoming more feasible to keep your entire database completely in memory.  Memory is not persistent, meaning if the power goes out, your database is lost.  Different implementations of main memory databases have come up with different solutions to this problem.  Memcached, which is a cache only database, does not care about persistence.  The key value store Redis writes keys asynchronously where it's okay if some data is lost.  

This leads us to Bitcask, which pulls elements from both in memory databases and persistent LSM trees to offer very high write performance but also persistence.  The one constraint is that the keys to the database must be small enough to fit in available memory, but the values of the keys can be larger than available memory.  Bitcask is based on a paper implementation by Riak and was developed in 2013.   The authors wanted the use case to center around databases that have a small amount of keys, but the keys values were constantly being written and updated. [4]  Popular examples of this would be applications like Youtube or Twitter, where they have a key to a video ID, and the value is the number of views of that video or tweet.  The keys are relatively small compared to the number of writes that are happening per second as some videos or tweets get viewed millions of times.  


![alt text][youtubeviews]

[youtubeviews]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/youtubeviews.png

The underlying index structure for Bitcask is a hashmap and the storage structure is log files like LSM trees.  Each log file has its own hashmap structure in memory.  Each time you need to make a read you simply check if the key is in the most recent log files hash map, and the value to that key is the byte offset and size of record in that file .  If the key doesn't exist in the most recent log file, you check the next hashmap and so on.  When you need to write a key, you simply append the record to the most recent log file and you place the key and byte offset in the associated hashmap.  Like LSM trees, there are no updates to records and once a log file is closed, it's considered immutable.  Because we could run out of disk space over time, a compaction process can run in a separate thread that can be used to discard old files and write the most recent updates to the current log file.  

![alt text][compaction-diagram]

[compaction-diagram]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/compaction-diagram.png

I implement my own version of Bitcask by following most of the design choices of the original authors and I extend their implementation with my own choices and optimizations.  Some differences arise due to me choosing to use C in order to have full control over the memory management of the process versus the Bitcask authors using the language, Erlang.  The biggest difference in architecture arose from me choosing to implement my own buffer pool manager and not rely on the operating system to manage the buffer cache.  This has been a highly contested debate within the database architecture community with the majority of experts and implementations siding with not relying on the OS to perform buffering. [5]  When people are designing their own database implementations it can be enticing to use the system call MMAP() which memory maps files to a logical address space.  You are getting the buffer management for free which can save a lot of development time because getting the buffer pool manager to work properly can be tricky.  Also kernel developers have spent a long time perfecting the OS’s buffer cache so why make it from scratch?  The argument against MMAP() is that the operating system has no idea what is happening in your database so therefore can't make choices as intelligently as the database developer over which pages to keep in memory, which to bring into memory and which to evict.  I do not choose to use MMAP() in my database and choose to use Direct_IO on Linux.  Direct_IO bypasses the operating systems buffer cache when the original call to open() is made and allows the user space to handle buffering. [6] Mac OS does not allow for Direct_IO and forces you to give hints, but the hints may or may not be received.  This leads to the double caching problem that systems like PostgresQL run into.  At system boot up, PostgresQL can only allocate half of the memory compared to other systems like MYSQL and RocksDB because of this double caching problem.  Double Caching means that the systems themselves are managing their own buffer pool but they are reading files through the operating system which means these files are being loaded into the operating systems buffer cache as well.  This is redundant because we do not need to buffer data in two separate places.  RocksDB was a fork of LevelDB and the first change they made was to get rid of MMAP and implement their own buffer pool manager.   


```c
\\ Linux open system call with Direct_IO
open(path, O_RDWR|O_DIRECT, 0666);
```

![alt text][open-call]

[open-call]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/opencall.png



## System Bootup

At system bootup, I first initialize a buffer separate from my buffer pool manager called directory_buffer.  At first, this seems to violate the sequential writes advantages of a log structured output file DB.  Yes, random access is required with a directory log, but the cost of writes is amortized over time because you are only performing random writes every 4096 bytes(size of each page).  This is because we only need to write the directory buffer when we add a new file number to it and this only occurs when we run out of space on the current page and move onto the next page.  The directory buffer is very simple but also very important.  It records the file number of each file and persists this on disk in case of system crash where we can then reboot and rebuild the subsequent hash maps.  We also need the directory buffer in memory to be able to loop through all the hash maps in order to read older keys, and compact the files together. 

When construct_hash_map_from_directory() is called, we try to read in the contents of db/dir, if nothing is read we know we are starting from a blank slate.  We make some more initializations and we can start writing to our first file.  If the contents are read from db/dir, we know there is already existing data in the database and we need to reconstruct the hashmaps in memory to be able to access some of this older data, and make sure the writer_thread is set to whatever byte offset is left off of the last in our directory buffer.  Some implementations use popular libraries for encoding binary objects into byte sequences.  Popular implementation that most people have heard of is Python’s library Pickle.   It's generally advised against using these “cheap” tricks because they bring in many issues related to security, performance and space. [2]

![alt text][dir_buf]

[dir_buf]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/dir_buf.png

For me, constructing the hashmaps consisted of simply reading all the files into memory and going through and constructing each hashmap in a linear scan fashion.  This can be slow and make boot-ups very time intensive if the number of files is very large.  Bitcask solves this problem by constructing a hint file during compaction of existing keys, which can greatly speed up boot up time.  If the hint file for a particular file exists, they scan that one ahead which just contains the keys and the offsets, and the full information is in the normal file.  This is something that I neglected but the implementation would not be too hard to add at a later point. 

## Buffer Pool Manager

![alt text][buffer]

[buffer]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/bufferpool.png

Virtual memory in operating systems is the idea that we need to be able to execute a process that is larger than available memory.  One reason is because your computer usually has numerous processes running simultaneously, so each process can’t have access to all available memory on a system. Another reason we want to do this is because we have way less RAM than disk space, and in the typical Von Neumann architecture of a computer, we need to bring programs and data into memory for them to be eligible to execute on a CPU.  The way the OS allows this to happen is through demand paging.  When we need a file from disk, we read it from disk, store it in a physical memory location, expose the process to a logical location of that physical memory and allow the process to access that memory.  Because reading from disk is orders of magnitude slower than reading from RAM, and processes need to reuse certain pieces of memory, it makes no sense to relinquish that memory after use. [7]


![alt text][memory]

[memory]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/memory.png

Therefore the operating system's job is to try to give the illusion to the running process that every piece of data they access is in memory even if the amount of data is greater than the total available memory on the system.  The way they do this is through a buffer cache.  When a file is read by a process, first the OS checks if the file is in the buffer cache, if it is, it returns a pointer to that memory location.  If it's not, it checks a free frame list to see if there are any free frames available in the buffer cache.  If there are free frames, it then makes the call to disk and loads the file contents in the memory location that was free, then returns a pointer to that memory location.  If there are no free frames, it then needs to evict data that is not being used from the buffer cache, to free up space for the new file. [7] There have been many papers and different attempts on the best eviction protocols, but Least Recently Used(LRU) eviction policies are what most operating systems use.  The idea is that a doubly linked list is used with a sentinel (dummy) variable at the head and a sentinel variable at the tail to mark the insertion and deletion locations.  Everytime something is accessed from the buffer pool it is added to the front of the link list.  This is done even if the page is located in the middle of the linked list or its being read in for the first time.  This can be done in O(1) constant time even if the value is already in the middle of the linked list because all you have to do is connect the previous nodes pointer to the next nodes pointer and vice versa.  When you need to evict, you simply connect the tail.prev pointer to current_node.prev.next pointer and vice versa. [8]


There is a boot up call to the buffer pool manager that initializes the buffer_pool struct which is the main struct for the buffer pool.  The main variables here are the pointer to block which points to the first memory address of the block of the big block of memory that is going to be added when we call 

```c
void boot_up_buffer_pool();
```

We will allocate a fixed amount of bytes when we make the call and also have control over the size of the page.  The only requirement is the pages need to be the same size throughout the block.  Without this requirement, pointer arithmetic would be harder and there lacks a valid reason to have different sizes when we are just reading and writing from log files.  Usually block sizes are 4 KB or 8 KB even in relational databases  and there's a couple main reasons for this. First, I/O system calls are expensive, we do not want to bring in one record to a block everytime we make an I/O system call; we would rather just bring in the whole block and perhaps use some of the other data at another time.  Also 4KB is the max upper limit the operating system guarantees that the write will happen atomically.  Anything more than 4KB and we risk the I/O scheduler reordering our write call and perhaps losing some of the data in a system crash which therefore makes it impossible to guarantee that the transaction was atomic.  Atomicity is obviously a huge deal in transactional guarantees, and we need this all or nothing property to hold in most databases.  




[1]: https://db.cs.cmu.edu/papers/2024/whatgoesaround-sigmodrec2024.pdf
[2]: https://dataintensive.net/
[3]: https://db-book.com/
[4]: https://riak.com/assets/bitcask-intro.pdf
[5]: https://db.cs.cmu.edu/papers/2022/cidr2022-p13-crotty.pdf
[6]: https://man7.org/tlpi/
[7]: https://os-book.com/OS10/index.html
[8]: https://mitpress.mit.edu/9780262046305/introduction-to-algorithms/