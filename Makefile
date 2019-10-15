.PHONY: test_stdinc test_popt

CXXFLAGS=-Wall -ggdb
CC=g++

all:: mincore evict subs

.PHONY: subs
subs:
	$(MAKE) -C test

test:: test_popt test_stdinc
	$(MAKE) -C test test

test_popt:
	./popt > test_popt.py
	python ./test_popt.py --help
	python ./test_popt.py --debug
	python ./test_popt.py --debug --debug
	@echo $@ PASS

test_stdinc:
	./stdinc -M > test_stdinc.cpp && g++ -o $@ test_stdinc.cpp
	./stdinc -MC > test_stdinc.c && gcc -o $@ test_stdinc.c
	$(MAKE) clean
	@echo $@ PASS

test_waitinfo:
	waitinfo -x -- bash -c \'exit 5\'

test_mincore:
	./mincore mincore foo

mincore: mincore.o mapping.o

evict: evict.o mapping.o


PROGS=mincore \
	evict \
	stdinc \
	popt \
	waitinfo \
	$()

install::
	cp $(PROGS) $(HOME)/bin

uninstall::
	$(RM) $(addprefix $(HOME)/bin/,$(PROGS))

clean::
	$(RM) test_stdinc* *.pyc
	make -C test clean

# DO NOT DELETE

evict.o: mapping.h
mapping.o: /usr/include/fcntl.h /usr/include/features.h
mapping.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
mapping.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
mapping.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
mapping.o: /usr/include/bits/fcntl.h /usr/include/bits/fcntl-linux.h
mapping.o: /usr/include/time.h /usr/include/bits/stat.h /usr/include/unistd.h
mapping.o: /usr/include/bits/posix_opt.h /usr/include/bits/environments.h
mapping.o: /usr/include/bits/confname.h /usr/include/getopt.h
mapping.o: /usr/include/sys/file.h /usr/include/sys/mman.h
mapping.o: /usr/include/bits/mman.h /usr/include/sys/stat.h
mapping.o: /usr/include/sys/errno.h /usr/include/errno.h
mapping.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
mapping.o: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
mapping.o: /usr/include/asm-generic/errno-base.h mapping.h
mincore.o: mapping.h
