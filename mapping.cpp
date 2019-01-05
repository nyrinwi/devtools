#include <cstring>
#include <cassert>
#include <iostream>
#include <vector>
#include <numeric>
#include <stdexcept>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/errno.h>

#include "mapping.h"

static void throwError(const std::string &msg)
{
    const std::string errstr = strerror(errno);
    std::string emsg = msg + " - " + errstr;
    throw std::runtime_error(emsg);
}

Mapping::Mapping( const char* filename)
: filename(filename)
{
    struct stat sbuf;
    int r = stat(filename, &sbuf);
    if ( r != 0 )
    {
        throwError("cannot stat file");
    }
    size = sbuf.st_size;

    fd = open(filename,O_RDONLY);
    if ( fd == -1 )
    {
        throwError("cannot open file");
    }

    try
    {
        ptr = mmap(NULL,size,PROT_READ,MAP_SHARED|MAP_NORESERVE,fd,0);
        if ( ptr == MAP_FAILED )
        {
            throwError("cannot mmap file");
        }
    }
    catch ( const std::runtime_error &e )
    {
        close(fd);
        throw;
    }
    n_pages = ( size + getpagesize() - 1 ) / getpagesize();

    std::vector<unsigned char> vec(n_pages);
    r = mincore(ptr,size,&vec[0]);
    if ( r != 0 )
    {
        close(fd);
        throwError("mincore fails");
    }
    n_mapped = std::accumulate(vec.begin(),vec.end(),0);
}

Mapping::~Mapping()
{
    close(fd);
}

void Mapping::evict ( double pct )
{
    if ( pct < 0.0 or pct > 100.0 )
    {
        throw std::runtime_error("invalid pct");
    }
    const size_t nbytes = size * (pct/100.0);
    assert(nbytes != 0);
    madvise(ptr,nbytes,MADV_DONTNEED);
    assert(nbytes != 0);
    int r = posix_fadvise(fd,0,nbytes,POSIX_FADV_DONTNEED);
    if ( r != 0 )
    {
        throwError("error from posix_fadvise");
    }
}

