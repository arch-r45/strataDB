# Persistent, multithreaded key value log store written in c


![alt text][UnearthDB]

[UnearthDB]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/UnearthDB.png


[Overview](#overview)
[Introduction](#introduction)

## Overview

In this repository, I implement a persistent, multithreaded key value log store based on bitcask.  I implement this in C using the Linux Operating system and package it into a Docker container.  I use Linux DIRECT_IO to bypass the operating systems buffer cache and implement my own buffer pool manager that handles page replacement intelligently in user space.  This also avoids double caching that hinders systems like PostgresQL who choose to implement their own buffer pool manager but not use DIRECT_IO.  My implementation is able to perform 15,000 persistent writes per second which is a big improvement on similar hardware to the original Bitcask paper of 6000 writes per second.  I also implement a thread safe, compaction algorithm that runs in a background thread.  The compaction algorithm saves 99 GB of space for every 100GB written by eliminating the vast majority of redundant data.  The system also handles race conditions eloquently and avoids deadlock entirely by never violating the hold and wait principle.   

![alt text][rantests]

[rantests]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/write_test.png

## Introduction

Most Database Management Systems follow a relational model, where each file on disk is an individual table of the database, and these files are brought into memory usually in 4 KB pages depending on what record is trying to be read.  The relational model acts as a general purpose database which is part of why it's been a mainstay for over 50 years and will continue to be. [1]  Popular implementations consist of Oracle DB, MYSQL, PostgresQL and SQlite3.   RDBMS’s ensure ACID properties, allow for widespread querying of data with range queries and joins, and generally have scalable performance for most workloads.   The general rule of thumb, however, is that RDBMS’s are the optimal option for read-heavy workloads but LSM Trees(NOSQL model) are optimal for write-heavy workloads. [2]  This difference in write throughput is due to the architectural designs of the implementation of the systems.   

Both B trees and LSM Trees rely on sorted tree structures for their indexes.  In a typical SQL relational database, a B tree data structure is used to find the specific page of the record, and then needs to bring that page into memory, perform the insertion, then rewrite the whole page back to disk.  A B tree uses the sorted property to maintain that lookups are at worst O(LogH) time complexity, where H is the height of the tree, which is very efficient for random reads of records.  However, for writes it seems wasteful to have to keep rewriting full 4KB pages back to disk for just performing one insertion.  Also random access on disk is around 100 times slower than sequential access. [3]  In contrast, LSM trees solve the problems of the above two limitations of B trees to provide a write heavy solution.  They use a red black tree as their in memory index, which allows them to maintain the sorted order in memory.  When a write comes in, the key is added to the in memory structure even if the key already exists somewhere else on disk.  Once the segment reaches its byte limit, the whole segment is written to disk.  

This takes advantage of sequential I/O where the disk arm seeks are kept to a minimum.  In a random access pattern, the disk arm needs to seek a different location after every write and could have to travel to sectors on the disk that are far away.[3]  

![alt text][disk]

[disk]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/disk_diagram.png

Also, by never rewriting existing pages, write amplification is reduced.  Write amplification never used to be of much concern for disk oriented storage, but the rise of fast SSD storage has made avoiding write amplification important.  This is because SSDs can only overwrite the same blocks a limited number of times before wearing out in comparison to traditional disks which usually have a mean time to failure of over 50 years.  [2, 3]  There are obviously some disadvantages and tradeoffs you are making when you use this structure and like everything in computer science, there is no free lunch.  Because you are never updating old segments, you end up having a large amount of duplicate records in your file, and LSM trees can run a compaction algorithm in the background, but this can sometimes interfere with performance or in some cases lag behind and never catch up.  Also transactional guarantees are more difficult to ensure in LSM Trees due to the data being written possibly in multiple locations which leads to this being avoided in applications where transactional guarantees matter like banking.  Popular implementations of LSM trees are LevelDB from Google and a fork of LevelDB, RocksDB, from Facebook.  

Both index structures allow for a large number of keys, because the sorted order allows you to not have to keep all the keys in memory.  This is because if you have a subset of the keys in memory and their subsequent byte offsets, you can easily determine where the location is of your record based on the sorted property.  However, as RAM has become cheaper, it's becoming feasible to keep your entire database completely in memory.  Memory is not persistent, meaning if the power goes out, your database is lost.  Different implementations of this idea have came up with different solutions to this problem.  Memcached, which is a cache only database, does not care about persistence.  The key value store Redis writes keys asynchronously where it's okay if some data is lost.  

This leads us to Bitcask, which pulls elements from both in memory databases and persistent LSM trees to offer very high write performance but also persistence.  The one constraint is that the keys to the database must be small enough to fit in available memory, but the values of the keys can be larger than available memory.  Bitcask is based on a paper implementation by Riak and was developed in 2013.   The authors wanted the use case to center around databases that have a small amount of keys, but the keys values were constantly being written and updated. [4]  Popular examples of this would be applications like Youtube or Twitter, where they have a key to a video ID, and the value is the number of views of that video or tweet.  The keys are relatively small compared to the number of writes that are happening per second as some videos or tweets get viewed millions of times.  


![alt text][youtubeviews]

[youtubeviews]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/youtubeviews.png

The underlying index structure for Bitcask is a hashmap and the storage structure is log files like LSM trees.  Each log file has its own hashmap structure in memory.  Each time you need to make a read you simply check if the key is in the most recent log files hash map, and the value to that key is the byte offset and size of record in that file .  If the key doesn't exist in the most recent log file, you check the next hashmap and so on.  When you need to write a key, you simply append the record to the most recent log file and you place the key and byte offset in the associated hashmap.  Like LSM trees, there are no updates to records and once a log file is closed, it's considered immutable.  Because we could run out of disk space over time, a compaction process can run in a separate thread that can be used to discard old files and write the most recent updates to the current log file.  

![alt text][compaction-diagram]

[compaction-diagram]: https://github.com/arch-r45/unearthDB/blob/main/docs/pictures/compaction-diagram.png

I implement my own version of Bitcask by following most of the design choices of the original authors and I extend their implementation with my own choices and optimizations.  Some differences arise due to me choosing to use C in order to have full control over the memory management of the process versus the Bitcask authors using the language, Erlang.  The biggest difference in architecture arose from me choosing to implement my own buffer pool manager and not rely on the operating system to manage the buffer cache.  This has been a highly contested debate within database architecture with the majority of experts and implementations siding with not relying on the OS to perform buffering. [5]  When people are designing their own database implementations it can be enticing to use the system call MMAP() which memory maps files to a logical address space.  You are getting the buffer management for free which can save a lot of development time because getting the buffer pool manager to work properly can be tricky.  Also kernel developers have spent a long time perfecting the OS’s buffer cache so why make it from scratch?  The argument against MMAP() is that the operating system has no idea what is happening in your database so therefore can't make choices as intelligently as the database developer over which pages to keep in memory, which to bring into memory and which to evict.  RocksDB was a fork of LevelDB and the first change they made was to get rid of MMAP and implement their own buffer pool manager.  




[1]: https://db.cs.cmu.edu/papers/2024/whatgoesaround-sigmodrec2024.pdf
[2]: https://dataintensive.net/
[3]: https://db-book.com/
[4]: https://riak.com/assets/bitcask-intro.pdf
[5]: https://db.cs.cmu.edu/papers/2022/cidr2022-p13-crotty.pdf