
PROGRAM_NAME = prefetcher

CC=gcc
CXX=g++ -std=c++0x
RM= rm -f
CPPFLAGS= -g  -Wall


SRCS=prefetcher.cc
OBJS=$(subst .cc, .o, $(SRCS))

all: prefetcher.cc
	$(CXX) $(CPPFLAGS) -o $(PROGRAM_NAME) $(PROGRAM_NAME).cc

clean:
	$(RM) $(PROGRAM_NAME)
	$(RM) $(OBJS)
	$(RM) *~ *.bak

run: $(PROGRAM_NAME)
	./${PROGRAM_NAME}
