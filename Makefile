# vim: noet
.PHONY: test_stdinc test_popt

CXXFLAGS=-Wall -ggdb

CC=g++
PYTHON=python3

CXXPROGS=mincore evict

all:: $(CXXPROGS)

ifndef NOTEST
all:: subs
endif

.PHONY: subs
subs:
	$(MAKE) -C test

test:: test_popt test_stdinc test_waitinfo
	$(MAKE) -C test test

test_popt:
	./popt > test_popt.py
	$(PYTHON) ./test_popt.py --help
	$(PYTHON) ./test_popt.py --debug x
	$(PYTHON) ./test_popt.py --debug --debug x
	@echo $@ PASS

test_stdinc:
	./stdinc -M > test_stdinc.cpp && g++ -o $@ test_stdinc.cpp
	./stdinc -MC > test_stdinc.c && gcc -o $@ test_stdinc.c
	$(MAKE) clean
	@echo $@ PASS

test_waitinfo:
	./waitinfo -x -- bash -c \'exit 5\' | grep 'exited 5'
	./waitinfo -x -- bash -c \'exit 6\' | grep 'exited 6'

test_mincore:
	./mincore mincore foo

mincore: mincore.o mapping.o

evict: evict.o mapping.o

mapping.o: mapping.cpp mapping.h


PROGS=$(CXXPROGS) \
	cb \
	gencpp \
	stdinc \
	mkchroot \
	perror \
	pexpr \
	popt \
	repeat \
	waitinfo \
	$()

install::
	cp $(PROGS) $(HOME)/bin

uninstall::
	$(RM) $(addprefix $(HOME)/bin/,$(PROGS))

clean::
	$(RM) test_stdinc* *.pyc *.o $(CXXPROGS)
	make -C test clean

