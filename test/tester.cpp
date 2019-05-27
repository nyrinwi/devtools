#include <cstdio>
#include <cstring>
#include <iostream>
#include "mapping.h"

#include <gtest/gtest.h>

class Tempfile
{
    std::string m_filename;
    int m_fd;
public:
    const char* filename() const
        { return m_filename.c_str(); }

    int fd() const
        { return m_fd; }

    Tempfile(size_t size=0);
    ~Tempfile();
};

Tempfile::Tempfile(size_t size)
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

Tempfile::~Tempfile()
{
    unlink(filename());
}

TEST(mapping,basic)
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
    const size_t filesize = npages*getpagesize();

    Tempfile tmp(filesize);
    Mapping* mapping = new Mapping(tmp.filename());

    char* buf = new char[filesize];
    memcpy(buf,mapping->ptr(),filesize);
    mapping->mincore();
    EXPECT_EQ(mapping->n_resident,npages);

    mapping->evict_bytes(1);
    EXPECT_EQ(mapping->n_resident,npages);

    mapping->evict_bytes(getpagesize()-1);
    EXPECT_EQ(mapping->n_resident,npages);

    mapping->evict_bytes(getpagesize());
    EXPECT_EQ(mapping->n_resident,npages-1);
}

