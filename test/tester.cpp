#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sys/file.h>
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

    void read(ssize_t nbytes=-1,size_t offset=0);

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
    if (size)
    {
        int r = ftruncate(m_fd,size);
        assert(r==0);
    }
}

void Tempfile::read(ssize_t nbytes,size_t offset)
{
    if (nbytes == -1)
    {
        nbytes = m_size;
    }
    char buf[getpagesize()];

    int nReads = (nbytes + getpagesize() - 1) / getpagesize();
    lseek(m_fd,offset,SEEK_SET);
    for (int i=0; i<nReads; i++)
    {
        ssize_t readSize = getpagesize();
        ssize_t readto = (i+1)*readSize;;
        if (readto > m_size)
        {
            readSize = m_size % getpagesize();
        }
        ssize_t r = ::read(m_fd,buf,readSize);
        if (r == -1)
        {
            perror("read");
        }
        if (r != readSize)
        {
            std::cerr << "readSize " << readSize << std::endl;
            std::cerr << "r " << r << std::endl;
        }
        assert(r == readSize);
    }
}

Tempfile::~Tempfile()
{
    unlink(filename());
}

TEST(mapping,mincore)
{
    const size_t numPages = 128;
    const size_t filesize = numPages*getpagesize();

    Tempfile tmp(filesize);
    const size_t lowSize = getpagesize()*(numPages/3);
    const size_t highSize = filesize - lowSize;

    std::shared_ptr<Mapping> mapAll,mapLow,mapHigh;
    ASSERT_NO_THROW(mapAll.reset(new Mapping(tmp.filename()))) << "mapAll";
    ASSERT_NO_THROW(mapLow.reset(new Mapping(tmp.filename(),0,lowSize))) << "mapLow:" << std::hex << lowSize;
    ASSERT_NO_THROW(mapHigh.reset(new Mapping(tmp.filename(),lowSize,highSize))) << "mapHigh:"
        << std::hex << lowSize
        << "," << highSize;
    EXPECT_EQ(mapAll->size(),filesize);
    EXPECT_EQ(mapLow->size(),lowSize);
    EXPECT_EQ(mapHigh->size(),highSize);

    EXPECT_EQ(mapAll->n_resident,0U);
    EXPECT_EQ(mapLow->n_resident,0U);
    EXPECT_EQ(mapHigh->n_resident,0U);

    // read two pages in mapHigh
    tmp.read(2*getpagesize(),lowSize);

    // Update the mapping info
    for (auto map : {mapAll,mapLow,mapHigh})
    {
        map->mincore();
    }

    // mincore should show 2 pages resident in mapAll and mapHigh. mapLow should be empty still
    EXPECT_GE(mapAll->n_resident,2U);
    EXPECT_EQ(mapLow->n_resident,0U);
    EXPECT_GE(mapHigh->n_resident,2U);
}

TEST(mapping,evict)
{
    const size_t numPages = 128;

    // To make things interesting, use a file size that's not a multiple of page size
    // The mapping will still be numPages long
    const size_t filesize = numPages*getpagesize() - getpagesize()/2;

    Tempfile tmp(filesize);
    Mapping* mapping = new Mapping(tmp.filename());

    // Read the whole file to make all pages resident
    tmp.read(filesize);
    mapping->mincore();
    EXPECT_EQ(mapping->n_resident,numPages);

    // Behavior is determined by madvise(). In this case less than one page eviction means no eviction
    mapping->evict_bytes(1);
    EXPECT_EQ(mapping->n_resident,numPages) << "expected no page evicitons with only one byte";

    mapping->evict_bytes(getpagesize()-1);
    EXPECT_EQ(mapping->n_resident,numPages) << "expected no page evicitons with less than a page";

    mapping->evict_bytes(getpagesize());
    EXPECT_EQ(mapping->n_resident,numPages-1);
}

TEST(mapping,getXYZ)
{
    // Create a file of 1+1/3 pages
    const std::string filename = "testfile.dat";
    std::vector<char> buf(getpagesize()+getpagesize()/3);
    std::ofstream ofp(filename);
    ofp.write(&buf[0],buf.size());
    ofp.close();

    auto numPages = Mapping::getNumPages(filename.c_str());
    ASSERT_EQ(2,numPages) << "page count should round up";

    auto size = Mapping::getFileSize(filename.c_str());
    ASSERT_EQ(buf.size(),size) << "file size incorrect";
}

