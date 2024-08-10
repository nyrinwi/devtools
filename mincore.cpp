#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <iomanip>
#include <future>
#include <numeric>
#include <sys/mman.h>

#include "mapping.h"

static const size_t OneGiB = 1024*1024*1024;

static unsigned getMaxThreads()
{
    static unsigned maxThreads=0;
    if (maxThreads != 0)
    {
        return maxThreads;
    }
    const char* env = getenv("MINCORE_MAX_THREADS");
    if (env != 0)
    {
        maxThreads = std::stoi(env);
    }

    if (maxThreads == 0)
    {
        maxThreads = 8;
    }
    return maxThreads;
}

// Returns number of pages total
unsigned getMaps(const char* filename, std::vector<Mapping>& mVec)
{
    const auto numPages = Mapping::getNumPages(filename);
    const auto adjustedFileSize = numPages * getpagesize();
    //std::cout << "file size " << Mapping::getFileSize(filename) << " padded to " << adjustedFileSize << std::endl;
    assert(numPages>0);

    // How many threads will we use??
    const size_t targetPagesPerThread = OneGiB/getpagesize();
    const unsigned maxThreads = getMaxThreads();
    unsigned numThreads;
    if (numPages > targetPagesPerThread)
    {
        // Above target, we need threads
        numThreads = (numPages+targetPagesPerThread-1)/targetPagesPerThread;
    }
    else
    {
        // Below taget only one thread
        numThreads = 1;
    }

    // Limit the number of threads
    if (numThreads > maxThreads)
    {
        numThreads=maxThreads;
    }

    // Add 1 to make sure the sum of these values exceeds the file size
    // We'll adjust the last one to be the right size
    const auto numPagesPerThread = 1+numPages/numThreads;
    const auto numBytesPerThread = numPagesPerThread*getpagesize();
    assert(numThreads>0);
    assert(numPagesPerThread >0);

    std::vector<size_t> sizes(numThreads,numBytesPerThread);
    sizes.back() = adjustedFileSize % numBytesPerThread;
    size_t byteOffset = 0;
    size_t pageOffset = 0;
    for(auto size : sizes)
    {
        Mapping mapping(filename,byteOffset,size);
        mVec.push_back(mapping);
        byteOffset += size;
        pageOffset += size/getpagesize();
    }
    return numPages;
}

int main( int argc, char *argv[] )
{
    std::vector<Mapping> mVec;

    for (int i=1; i<argc; i++)
    {
        const char* filename = argv[i];
        unsigned numPages=0;
        try
        {
            numPages = getMaps(filename,mVec);
            //std::cout << "mVec.size() = " << mVec.size() << std::endl;;
            //std::cout << "numPages = " << numPages << std::endl;;
        }
        catch(const std::exception & e)
        {
            std::cout << "exception - " << e.what() << std::endl;
            continue;
        }


        unsigned pageOffset = 0;
        std::vector<uint8_t> pageVec(numPages);
        std::vector<std::future<int>> futures;
        for(auto& mm : mVec)
        {
            uint8_t* pages = &pageVec[pageOffset];
            pageOffset += mm.n_pages();
            auto fut = std::async(std::launch::async,[=]{
                return (mincore(mm.ptr(),mm.size(),pages)==0)?0:errno;});

            futures.push_back(std::move(fut));
        }
        for(auto& fut : futures)
        {
            int r = fut.get();
            if (r != 0)
            {
                perror("mincore");
                exit(1);
            }
        }
        unsigned numResident = std::accumulate(pageVec.begin(),pageVec.end(),0);
        double pct = 100.0*numResident/numPages;
        std::cout << filename << ": " << numPages << " pages, "
            << numResident << " resident, "
            << std::setprecision(3) << pct << "%" << std::endl;
    }
    exit(0);
}
