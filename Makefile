CC = g++
CFLAGS = -Wall -O3 -fopenmp -Igzstream -Isrc -Isrc/models -std=c++0x
LDFLAGS = -lgomp -lgzstream -lz -lstdc++ -Lgzstream
OBJECTS = obj/common.o obj/corpus.o obj/model.o gzstream/gzstream.o
MODELOBJECTS = obj/models/POP.o obj/models/BPRMF.o obj/models/FPMC.o obj/models/SPMC.o obj/models/SBPR.o obj/models/GBPR.o
OUTPUTS = cwcai*

all: train

obj/common.o: src/common.hpp src/common.cpp Makefile
	$(CC) $(CFLAGS) -c src/common.cpp -o $@

obj/corpus.o: src/corpus.hpp src/corpus.cpp obj/common.o gzstream/gzstream.o Makefile
	$(CC) $(CFLAGS) -c src/corpus.cpp -o $@

obj/model.o: src/model.hpp src/model.cpp obj/corpus.o obj/common.o Makefile
	$(CC) $(CFLAGS) -c src/model.cpp -o $@

obj/models/POP.o: src/models/POP.cpp src/models/POP.hpp obj/model.o obj/corpus.o obj/common.o Makefile
	$(CC) $(CFLAGS) -c src/models/POP.cpp -o $@

obj/models/BPRMF.o: src/models/BPRMF.cpp src/models/BPRMF.hpp obj/model.o obj/corpus.o obj/common.o Makefile
	$(CC) $(CFLAGS) -c src/models/BPRMF.cpp -o $@

obj/models/FPMC.o: src/models/FPMC.cpp src/models/FPMC.hpp obj/model.o obj/corpus.o obj/common.o Makefile
	$(CC) $(CFLAGS) -c src/models/FPMC.cpp -o $@

obj/models/SPMC.o: src/models/SPMC.cpp src/models/SPMC.hpp obj/model.o obj/corpus.o obj/common.o Makefile
	$(CC) $(CFLAGS) -c src/models/SPMC.cpp -o $@

obj/models/SBPR.o: src/models/SBPR.cpp src/models/SBPR.hpp obj/model.o obj/corpus.o obj/common.o Makefile
	$(CC) $(CFLAGS) -c src/models/SBPR.cpp -o $@

obj/models/GBPR.o: src/models/GBPR.cpp src/models/GBPR.hpp obj/model.o obj/corpus.o obj/common.o Makefile
	$(CC) $(CFLAGS) -c src/models/GBPR.cpp -o $@

gzstream/gzstream.o:
	cd gzstream && make


train: src/main.cpp $(OBJECTS) $(MODELOBJECTS) Makefile
	$(CC) $(CFLAGS) -o train src/main.cpp $(OBJECTS) $(MODELOBJECTS) $(LDFLAGS)

clean:
	rm -rf $(OBJECTS) $(MODELOBJECTS) train
