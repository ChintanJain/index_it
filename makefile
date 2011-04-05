CC= gcc
CXX= g++
CFLAGS=
SRCS= main.cpp parser.cpp
OBJS= main.o parser.o
HDRS= parser.h
BINS= index_it

all: $(BINS)

$(BINS): $(OBJS) $(HDRS)
	$(CXX) -o $@ $(CFLAGS) $(OBJS) -lz

$(OBJS): $(SRCS) $(HDRS)
	$(CXX) -c $(CFLAGS) $(SRCS)

.PHONY: clean clean_all run done

clean:
	rm *.o $(BINS)

clean_all:
	rm *.o $(BINS) structures/u* structures/l* structures/i* postings/s* data/*_data data/*_index

run:
	./index_it

done:
	echo "Done running"
