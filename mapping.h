#include <string>

/**
 * Create a read-only shared mapping of a file.
 * The size of the mapping is based on the size of the file at the
 * time of construction.
 */
class Mapping
{
    int m_fd;
    const std::string m_filename;
    size_t m_size;
    size_t m_n_pages;
    void* m_ptr;

public:
    int fd() { return m_fd; };
    const char* filename() { return m_filename.c_str(); };

    // Call mincore and update n_pages and n_resident
    void mincore();

    // Size in bytes
    size_t size() const { return m_size; };

    // Pointer to the base of the mapping
    void* ptr() const { return m_ptr; };

    // The total number of pages in the mapping
    // This is based on the size of the file at the time of construction
    // and does not change over the life of this object
    size_t n_pages() const { return m_n_pages; };

    // The number of pages resident at the time of construction.
    // Updated with each call to mincore()
    mutable size_t n_resident;

    // Return a value between 0.0 and 100.0 representing the number of pages resident in memory
    // at the time of construction or the most recent call to mincore
    double pct_resident() const
        { return 100.0 * static_cast<double>(n_resident) / static_cast<double>(m_n_pages); }

    // Kick pages out of memory starting at the beginning of the mapping
    void evict_pct(double pct=100.0);
    void evict_bytes(size_t n_bytes);
    void evict_pages(size_t n_pages);

    Mapping(const char* filename,size_t offset=0,size_t length=0);
    ~Mapping();
};

