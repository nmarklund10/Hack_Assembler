CXX = g++ -std=c++11
OPTS = -g -O2
WARN = -Wall -Werror
DEPS = -MMD -MF $*.d
INCL =

OBJS = assembler.exe

default: $(OBJS)

clean:
	rm -rf Dependencies $(OBJS)

%.exe: %.cpp
	$(CXX) $(OPTS) $(WARN) $(DEPS) $(INCL) $< -o $@
	cat $*.d >> Dependencies
	rm -f $*.d

-include Dependencies