# vim:noet

CC=g++
GTESTHOME=~/src/googletest-release-1.8.1/googletest
LDLIBS+=-L $(GTESTHOME) -lgtest_main -lgtest -lpthread
CXXFLAGS+=-I $(GTESTHOME)/include -I $(CURDIR)/..

vpath %.cpp $(CURDIR)/..

all test:: tester
	./tester

tester: mapping.o

clean::
	$(RM) *.o tester
