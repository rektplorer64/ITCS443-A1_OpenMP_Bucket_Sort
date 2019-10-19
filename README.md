# ITCS443 - Assignment 1 - OpenMP: Bucket Sort
A student project. Mahidol University, Faculty of ICT: `ITCS443 – Parallel and Distributed Systems`. 2019

This is a implementation of Bucket Sort with parallelism inside OpenMP and C programing language.


## 😀 Our Members
|#|Name|Surname|Nickname|Student ID|
|-|-|-|-|-|
|1|Anon|Kangpanich|Non|6088053|
|2|Krittin|Chatrinan|Tey|6088022|
|3|Tanawin|Wichit|Pooh|6088221|

## ℹ Algorithms
1. Randomly generate numbers.
2. Find Minimum and Maximum of the generated data
3. Each thread calculates which number range it should take care of.
4. Each thread counts the number in the range out of the generated data.
5. Each thread creates its own bucket (or an array) with the count from the step 4.
6. Each thread iterates through all data elements to get all numbers that is in its range.
7. Each thread performs a sequential sort (in this case Merge Sort) to its bucket.
8. Each thread waits for each other.
9. Each thread populates the shared result array using the accumulation of data count of the previous threads (if any)
