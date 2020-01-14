# vim:noet

CC=g++
CXXFLAGS=--std=c++11
export CXXFLAGS
GTESTHOME=~/googletest-release-1.10.0/googletest
LDLIBS+=-L $(GTESTHOME)/lib -lgtest_main -lgtest -lpthread
CXXFLAGS+=-I $(GTESTHOME)/include -I $(CURDIR)/..

vpath %.cpp $(CURDIR)/..

all test:: tester
	./tester

tester: mapping.o

clean::
	$(RM) *.o tester
