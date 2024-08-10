# vim:noet

CC=g++
CXXFLAGS=--std=c++11
export CXXFLAGS
GTESTHOME=/usr/src/googletest/googletest
LDLIBS+=-L $(GTESTHOME)/lib -lgtest_main -lgtest -lpthread
CXXFLAGS+=-I $(GTESTHOME)/include -I $(CURDIR)/..

vpath %.cpp $(CURDIR)/..

all test:: tester
	./tester

tester: mapping.o

TEST_SIZE=1G
test_mincore: ../mincore
	@truncate -s $(TEST_SIZE)  test.dat
	@/usr/bin/time -f '$(TEST_SIZE) %E' ../mincore test.dat

clean::
	$(RM) *.o tester test.dat
