#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define BUCKET_COUNT 20
uint32_t buckets[BUCKET_COUNT] = {0};

static int bucketEnumerateIndex = 0;
void NmeaTimeBucketEnumerateStart()
{
    bucketEnumerateIndex = 0;
}
bool NmeaTimeBucketEnumerate(int size, char *text) //returns false if there is no more information
{
    if (bucketEnumerateIndex >= BUCKET_COUNT) return false;
    snprintf(text, size, "%lu\r\n", buckets[bucketEnumerateIndex]); //snprintf always teminates with a null character after n-1 characters
    bucketEnumerateIndex++;
    return true;
}
int NmeaTimeBucketGetSize()
{
    return BUCKET_COUNT;
}
uint32_t NmeaTimeBucketGetItem(int item)
{
    return buckets[item];
}
void NmeaTimeBucketAdd(int nmeaStartMs)
{
    int bucket = nmeaStartMs * BUCKET_COUNT / 1000;
    if (bucket < BUCKET_COUNT && bucket >= 0) buckets[bucket]++;
}
