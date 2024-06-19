#include <cstring>
#include <cassert>
#include <iostream>
#include <sstream>
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

Mapping::Mapping(const char* filename,size_t offset,size_t length)
: m_filename(filename)
{
    struct stat sbuf;
    int r = stat(filename, &sbuf);
    if (r != 0)
    {
        throwError("cannot stat file");
    }

    if (length==0 and offset==0)
    {
        m_size = sbuf.st_size;
    }
    else
    {
        size_t fileSize = sbuf.st_size;
        if (offset+length > fileSize)
        {
            std::ostringstream oss;
            oss << "offset+length exceeds file size";
            throw std::runtime_error(oss.str());
        }
        if (length == 0)
        {
            m_size = sbuf.st_size;
        }
        else
        {
            m_size = length;
        }
    }

    m_fd = open(m_filename.c_str(),O_RDONLY);
    if (m_fd == -1)
    {
        throwError("cannot open file");
    }

    m_ptr = mmap(NULL,m_size,PROT_READ,MAP_SHARED|MAP_NORESERVE,m_fd,offset);
    if (m_ptr == MAP_FAILED)
    {
        close(m_fd);
        throwError("cannot mmap file");
    }
    m_n_pages = (m_size + getpagesize() - 1) / getpagesize();

    mincore();
}

void Mapping::mincore()
{
    std::vector<unsigned char> vec(m_n_pages);
    int r = ::mincore(m_ptr,m_size,&vec[0]);
    if (r != 0)
    {
        close(m_fd);
        throwError("mincore fails");
    }
    n_resident = std::accumulate(vec.begin(),vec.end(),0);
}

Mapping::~Mapping()
{
    close(m_fd);
}

void Mapping::evict_pct(double pct)
{
    if (pct < 0.0 or pct > 100.0)
    {
        throw std::runtime_error("invalid pct");
    }
    const size_t nbytes = m_size * (pct/100.0);
    evict_bytes(nbytes);
}

void Mapping::evict_bytes(size_t n_bytes)
{
    int r = madvise(m_ptr,n_bytes,MADV_DONTNEED);
    if (r != 0)
    {
        throwError("error from madvise");
    }

    r = posix_fadvise(m_fd,0,n_bytes,POSIX_FADV_DONTNEED);
    if (r != 0)
    {
        throwError("error from posix_fadvise");
    }
    mincore();
}

void Mapping::evict_pages(size_t n_pages)
{
    size_t nbytes = n_pages * getpagesize();
    evict_bytes(nbytes);
}

