#include <string>

class Mapping
{
public:
    const std::string filename;
    mutable size_t size;
    mutable int fd;
    mutable void* ptr;
    mutable size_t n_pages;
    mutable size_t n_mapped;

    Mapping( const char* filename);
    ~Mapping();

    double pct_mapped() const
        { return 100.0 * static_cast<double>(n_mapped) / static_cast<double>(n_pages); }

    void evict( double pct=100.0 );
};

