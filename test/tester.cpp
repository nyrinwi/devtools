#include <cstdio>
#include <cstring>
#include <iostream>
#include "mapping.h"

#include <gtest/gtest.h>

class Tempfile
{
    std::string m_filename;
    const size_t m_size;
    int m_fd;
public:
    const char* filename() const
        { return m_filename.c_str(); }

    int fd() const
        { return m_fd; }

    void read(ssize_t nbytes=-1);

    Tempfile(size_t size=0);
    ~Tempfile();
};

Tempfile::Tempfile(size_t size)
: m_size(size)
{
    char name[] = "tempfile.XXXXXX";
    m_fd = mkstemp(name);
    assert(m_fd != -1);
    m_filename = name;
    if ( size )
    {
        int r = ftruncate(m_fd,size);
        assert(r==0);
    }
}

void Tempfile::read(ssize_t nbytes)
{
    if ( nbytes == -1 )
    {
        nbytes = m_size;
    }
    char buf[ getpagesize() ];

    int nwrites = (nbytes + getpagesize() - 1) / getpagesize();
    lseek(m_fd,0,SEEK_SET);
    for (int i=0; i<nwrites; i++)
    {
        ssize_t rsize = getpagesize();
        ssize_t readto = (i+1)*rsize;;
        if ( readto > m_size )
        {
            rsize = m_size % getpagesize();
        }
        ssize_t r = ::read(m_fd,buf,rsize);
        if ( r == -1 )
        {
            perror("read");
        }
        if ( r != rsize )
        {
            std::cerr << "rsize " << rsize << std::endl;
            std::cerr << "r " << r << std::endl;
        }
        assert(r == rsize);
    }
}

Tempfile::~Tempfile()
{
    unlink(filename());
}

TEST(mapping,mincore)
{
    const size_t npages = 128;
    const size_t filesize = npages*getpagesize();

    Tempfile tmp(filesize);
    Mapping* mapping = new Mapping(tmp.filename());
    EXPECT_EQ(mapping->size(),filesize);
    EXPECT_EQ(mapping->n_resident,0U);
}

TEST(mapping,evict)
{
    const size_t npages = 128;
    const size_t filesize = npages*getpagesize() - getpagesize()/2;

    Tempfile tmp(filesize);
    Mapping* mapping = new Mapping(tmp.filename());

    tmp.read(filesize);
    mapping->mincore();
    EXPECT_EQ(mapping->n_resident,npages);

    mapping->evict_bytes(1);
    EXPECT_EQ(mapping->n_resident,npages);

    mapping->evict_bytes(getpagesize()-1);
    EXPECT_EQ(mapping->n_resident,npages);

    mapping->evict_bytes(getpagesize());
    EXPECT_EQ(mapping->n_resident,npages-1);
}

