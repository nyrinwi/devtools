.PHONY: test_stdinc test_popt

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


clean::
	$(RM) test_stdinc* *.pyc
