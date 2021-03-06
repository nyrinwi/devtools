#!/usr/bin/python

from __future__ import print_function
import optparse
import logging

_logger = logging.getLogger(__name__)

def main():
    parser = optparse.OptionParser()
    parser.add_option("--debug",action="count",default=0)
    opts,args = parser.parse_args()

    logging.basicConfig( level=logging.INFO )
    if opts.debug:
        _logger.setLevel(logging.DEBUG)
        _logger.debug("debug level {0}".format(opts.debug))
        if opts.debug > 1:
            logging.getLogger("").setLevel(logging.DEBUG)

if __name__ == "__main__":
    main()
