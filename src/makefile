CXX=g++
LD=g++
#CXXFLAGS=-std=c++0x `pkg-config --cflags gtkmm-3.0`
#LDFLAGS=`pkg-config --libs gtkmm-3.0`
CXXFLAGS=-std=c++11 -Wall -Wconversion $(shell pkg-config --cflags gtkmm-2.4 gtkglext-1.0)
LDFLAGS=$(shell pkg-config --libs gtkmm-2.4 gtkglext-1.0) -lglut
PROGRAM=sample
SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)
DEPS=$(SRCS:.cpp=.d)

.SUFFIXES: .o .cpp
.PHONY: all clean default

default: $(PROGRAM)

.cpp.o:
	@echo Compiling $<
	@$(CXX) $(CXXFLAGS) -c $<

%.d: %.cpp
	@set -e; rm -f $@; \
	$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
        rm -f $@.$$$$

$(PROGRAM): $(OBJS) $(DEPS)
	@echo Linking $(OBJS)
	@$(LD) -o $(PROGRAM) $(OBJS) $(LDFLAGS)

clean:
	$(RM) $(TARGET) $(OBJS)

-include $(DEPS)
