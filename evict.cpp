#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <iostream>

#include "mapping.h"

int main( int argc, char *argv[] )
{
    for (int i=1; i<argc; i++)
    {
        const char* filename = argv[i];
        try
        {
            Mapping m(filename);
            m.evict_pct(100.0);
            printf("%s %lu pages, %.1f%% resident\n",
                filename, m.n_pages(), m.pct_resident());

            Mapping m_after(filename);
            printf("%s %lu pages resident\n",filename,m_after.n_resident);
        }
        catch ( const std::runtime_error &e )
        {
            std::cerr << filename << " " << e.what() << std::endl;
        }
    }
    exit(0);
}
