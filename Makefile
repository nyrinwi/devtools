.PHONY: test_stdinc test_popt

CXXFLAGS=-Wall -ggdb
CC=g++

all:: mincore evict

test:: test_popt test_stdinc

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


test_mincore:
	./mincore mincore foo

mincore: mincore.o mapping.o

evict: evict.o mapping.o


PROGS=mincore \
	evict \
	stdinc \
	popt \
	$()

install::
	cp $(PROGS) $(HOME)/bin

uninstall::
	$(RM) $(addprefix $(HOME)/bin/,$(PROGS))


clean::
	$(RM) test_stdinc* *.pyc

