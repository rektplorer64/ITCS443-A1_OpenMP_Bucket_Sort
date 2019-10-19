#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>

/**
 * Merges two sub-arrays of dataArray[] into one such that
 * - First subarray is dataArray[positionLeft to positionMiddle]
 * - Second subarray is dataArray[positionMiddle + 1 to positionRight]
 *
 * @param dataArray         integer array
 * @param positionLeft      the position on the left of the array
 * @param positionMiddle    the position on the middle of the array
 * @param positionRight     the position on the right of the array
 */
void merge(int dataArray[], int positionLeft, int positionMiddle, int positionRight) {
    int i, j, k;

    // Calculate the range of both parts
    int firstPartSize = positionMiddle - positionLeft + 1;
    int secondPartSize = positionRight - positionMiddle;

    // Create temporary arrays for both parts
    int firstPart[firstPartSize], secondPart[secondPartSize];

    // Copy data to temporary arrays: firstPart[] and secondPart[]
    for (i = 0; i < firstPartSize; i++) {
        firstPart[i] = dataArray[positionLeft + i];
    }
    for (j = 0; j < secondPartSize; j++) {
        secondPart[j] = dataArray[positionMiddle + 1 + j];
    }

    // Merge the temp arrays back into dataArray[positionLeft to positionRight]
    i = 0;                  // Initial index of first subarray
    j = 0;                  // Initial index of second subarray
    k = positionLeft;       // Initial index of merged subarray
    while (i < firstPartSize && j < secondPartSize) {
        if (firstPart[i] <= secondPart[j]) {
            dataArray[k] = firstPart[i];
            i++;
        } else {
            dataArray[k] = secondPart[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of firstPart[], if there are any
    while (i < firstPartSize) {
        dataArray[k] = firstPart[i];
        i++;
        k++;
    }

    // Copy the remaining elements of secondPart[], if there are any
    while (j < secondPartSize) {
        dataArray[k] = secondPart[j];
        j++;
        k++;
    }
}

/**
 * Recursively merge data array using Merge Sort
 *
 * @param dataArray         data array of int to be merged
 * @param positionLeft      position of the array on the left
 * @param positionRight     position of the array on the right
 */
void mergeSort(int dataArray[], int positionLeft, int positionRight) {
    // If the position on the left is lower than the right, continue the recursive call.
    if (positionLeft < positionRight) {
        // Same as (positionLeft + positionRight) / 2, but avoids overflow for large positionLeft
        int positionMiddle = positionLeft + (positionRight - positionLeft) / 2;

        // Sort first and second parts
        mergeSort(dataArray, positionLeft, positionMiddle);
        mergeSort(dataArray, positionMiddle + 1, positionRight);

        // Merge them together
        merge(dataArray, positionLeft, positionMiddle, positionRight);
    }
}

/**
 * Generate random numbers into given array
 *
 * @param array output array for the generated numbers
 * @param size  size of the array
 */
void generateRandomNumberArray(int *array, int size) {
    srand(1432427398);
    for (int i = 0; i < size; i++) {
        // srand(time(NULL));
        *(array + i) = rand();
        // printf("%d\n", array[i]);
    }
}

/**
 * Sort integer by using Bucket Sort with Parallelism
 *
 * @param dataSize  the size of the data array
 * @return          the amount of time taken to sort the number
 */
long parallelBucketSort(int dataSize) {
    // DEBUG: Initialize the timer
    struct timespec timeStampStop, timeStampStart;
    int TOTAL_THREADS = 16;

    // Allocate loop counters and data heap
    int i, j;
    int DATA_SIZE = dataSize;
    int *data = malloc(sizeof(int) * DATA_SIZE);

    // Generate random data
    generateRandomNumberArray(data, DATA_SIZE);

    // Write the generated numbers to originalArray.txt
    FILE *originalArrayFile;
    originalArrayFile = fopen("originalArray.txt", "w");
    for (int m = 0; m < DATA_SIZE; ++m) {
        fprintf(originalArrayFile, "%d\n", data[m]);
    }
    fclose(originalArrayFile);

    // Populate counters of all threads with 0
    int threadDataCount[TOTAL_THREADS];
    for (i = 0; i < TOTAL_THREADS; i++) {
        threadDataCount[i] = 0;
    }

    // DEBUG: Mark the starting time
    clock_gettime(CLOCK_MONOTONIC_RAW, &timeStampStart);
    double start_time = omp_get_wtime();

    // Find the Minimum value of the data in parallel
    int minimumElement = 99999999;
    #pragma omp parallel for reduction(min:minimumElement) default(none) shared(DATA_SIZE, data) num_threads(TOTAL_THREADS)
    for (i = 0; i < DATA_SIZE; i++) {
        if (data[i] < minimumElement) {
            minimumElement = data[i];
        }
    }

    // Find the Maximum value of the data in parallel
    int maximumElement = -99999999;
    #pragma omp parallel for reduction(max:maximumElement) default(none) shared(DATA_SIZE, data) num_threads(TOTAL_THREADS)
    for (i = 0; i < DATA_SIZE; i++) {
        if (data[i] > maximumElement) {
            maximumElement = data[i];
        }
    }

    // DEBUG: Shows Min and Max of the data
    // printf("Minimum Element = %d\n", minimumElement);
    // printf("Maximum Element = %d\n", maximumElement);

    // Enters the Main Parallel block with shared data array and thread data counter array
    #pragma omp parallel default(none) private(i, j) shared(DATA_SIZE, data, threadDataCount, TOTAL_THREADS, minimumElement, maximumElement) num_threads(TOTAL_THREADS)
    {
        int threadId = omp_get_thread_num();
        int chunkSize = DATA_SIZE / TOTAL_THREADS;

        // DEBUG: Load Sharing Summary
        // int actualCoverage = 0, totalLeftover = 0;
        // if (threadId == TOTAL_THREADS - 1) {
        //     actualCoverage = chunkSize * TOTAL_THREADS;
        //     totalLeftover = DATA_SIZE - actualCoverage;
        // }

        // int start = threadId * chunkSize;
        // int end = ((threadId + 1) * chunkSize) + totalLeftover;

        // printf("\nThread #%d\tStart: %d\t End: %d\t Size: %d\n", threadId, start, end - 1, end - start);

        int bucketBinSize = (maximumElement - minimumElement + 1) / TOTAL_THREADS;
        int lowerBinCriteria = (threadId * bucketBinSize) + minimumElement;
        int upperBinCriteria = lowerBinCriteria + bucketBinSize - 1;

        if (threadId == TOTAL_THREADS - 1) {
            upperBinCriteria = maximumElement;
        }

        // printf("Thread #%d\tValue Domain: [%d, %d)\n", threadId, lowerBinCriteria, upperBinCriteria);

        // Counts the amount of data that the thread has to handle
        // To prepare an array with the size of number count
        for (j = 0; j < DATA_SIZE; j++) {
            // If the number is in the bin range, count it up.
            if (data[j] >= lowerBinCriteria && data[j] <= upperBinCriteria) {
                threadDataCount[threadId] += 1;
            }
        }

        // Add number to its own private array
        int slicingCount = 0;
        int sliced[threadDataCount[threadId]];
        for (j = 0; j < DATA_SIZE; j++) {
            if (data[j] >= lowerBinCriteria && data[j] <= upperBinCriteria) {
                // printf("%d: %d\t%d\n", threadId, j, data[j]);
                sliced[slicingCount++] = data[j];
            }
        }

        // Initialize sequential sorting algorithm
        mergeSort(sliced, 0, slicingCount - 1);

        // Wait for all threads to finish
        #pragma omp barrier
        int actualStart = 0;
        if (threadId != 0) {
            for (int k = 0; k < threadId; ++k) {
                actualStart += threadDataCount[k];
            }
        }

        // printf("\nThread #%d\tBin = %d\tActual Start = %d\n", threadId, threadDataCount[threadId], actualStart);

        // Populate sort data based on accumulated previous data counts
        slicingCount = 0;
        for (j = actualStart; j < actualStart + threadDataCount[threadId]; j++) {
            data[j] = sliced[slicingCount++];
        }
    }

    // DEBUG: Mark the ending time
    clock_gettime(CLOCK_MONOTONIC_RAW, &timeStampStop);
    double time = omp_get_wtime() - start_time;
    uint64_t delta_us = (timeStampStop.tv_sec - timeStampStart.tv_sec) * 1000000 +
                        (timeStampStop.tv_nsec - timeStampStart.tv_nsec) / 1000;

    // DEBUG: PRINT WORK LOAD SUMMARY
    printf("\n--- LOAD BALANCING SUMMARY ---\n");
    int count = 0;
    for (i = 0; i < TOTAL_THREADS; i++) {
        count += threadDataCount[i];
        printf("Thread %d\thas %d element(s)\n", i, threadDataCount[i]);
    }
    printf("\n\nTotal Data Processed = %d out of %d\n", count, DATA_SIZE);

    printf("\n-- PARALLEL BUCKET SORT DURATION --\n");
    printf("Time Elapsed = %lu Microseconds (%lf Seconds)\n", delta_us, time);

    // DEBUG: Write sorted array into sortedArray.txt
    FILE *fp;
    fp = fopen("sortedArray.txt", "w");
    // fputs("This is testing for fputs...\n", fp);

    // printf("\nFinal Sorted Array is written to sortedArray.txt\n");
    for (int l = 0; l < DATA_SIZE; l++) {
        // printf("%d\n", data[l]);
        fprintf(fp, "%d\n", data[l]);
    }
    fclose(fp);

    free(data);

    // Return the time elapsed in Microseconds
    return delta_us;
}

/**
 * Sort integer by using Bucket Sort without Parallelism
 *
 * @param dataSize  the size of the data array
 * @return          the amount of time taken to sort the number
 */
long sequentialBucketSort(int dataSize) {
    // DEBUG: Initialize the timer
    struct timespec timeStampStop, timeStampStart;

    int DATA_SIZE = dataSize;
    int *data = malloc(sizeof(int) * DATA_SIZE);

    int BUCKET_AMOUNT = 16;
    int BUCKET_SIZE = dataSize / BUCKET_AMOUNT;

    generateRandomNumberArray(data, DATA_SIZE);

    // DEBUG: Mark the starting time
    clock_gettime(CLOCK_MONOTONIC_RAW, &timeStampStart);
    double start_time = omp_get_wtime();

    // TODO: Implement Bucket Sort in Sequential
    // mergeSort(data, 0, DATA_SIZE - 1);

    // DEBUG: Mark the ending time
    clock_gettime(CLOCK_MONOTONIC_RAW, &timeStampStop);
    double time = omp_get_wtime() - start_time;
    uint64_t delta_us = (timeStampStop.tv_sec - timeStampStart.tv_sec) * 1000000 +
                        (timeStampStop.tv_nsec - timeStampStart.tv_nsec) / 1000;

    printf("\n-- SEQUENTIAL BUCKET SORT DURATION --\n");
    printf("Time Elapsed = %lu Microseconds (%lf Seconds)\n", delta_us, time);
    return delta_us;
}

int main() {
    // Maximum supported data size when not running sequential sorting
    // int DATA_SIZE = 15000000;

    // FIXME: Fix the bug where sequential sort could not be processed when data size is set to 15,000,000 elements
    int DATA_SIZE = 2000000;
    long parallelBucketSortTime = parallelBucketSort(DATA_SIZE);
    long sequentialBucketSortTime = sequentialBucketSort(DATA_SIZE);

    printf("\n----- Duration Summary ----- \n");
    printf("Parallel Time:\t\t%lu Microseconds\n", parallelBucketSortTime);
    printf("Sequential Time:\t%lu Microseconds\n", sequentialBucketSortTime);

    long difference = sequentialBucketSortTime - parallelBucketSortTime;
    printf("* Difference:\t\t%ld Microseconds\n", difference);

    return 0;
}