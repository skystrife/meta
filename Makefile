SEARCH = search
SEARCHOBJS = tokenizers/parse_tree.o index/document.o index/ram_index.o \
    tokenizers/tree_tokenizer.o tokenizers/ngram_tokenizer.o io/textfile.o io/parser.o \
    index/lexicon.o index/inverted_index.o io/compressed_file_reader.o io/compressed_file_writer.o \
    index/postings.o tokenizers/tokenizer.o index/chunk_list.o index/structs.o stemmers/porter2_stemmer.o \
    tokenizers/fw_tokenizer.o classify/knn.o io/config_reader.o classify/confusion_matrix.o

TESTER = tester
TESTEROBJS = $(SEARCHOBJS)

PLOT = plot
PLOTOBJS = $(SEARCHOBJS)

LIBLINEAR = liblinear
LIBLINEAROBJS = tokenizers/parse_tree.o tokenizers/tree_tokenizer.o \
    tokenizers/ngram_tokenizer.o io/textfile.o io/parser.o tokenizers/tokenizer.o \
    index/chunk_list.o index/structs.o stemmers/porter2_stemmer.o \
    tokenizers/fw_tokenizer.o io/config_reader.o index/document.o

TESTS = test/porter2_stemmer_test.h test/parse_tree_test.h \
    test/compressed_file_test.h test/unit_test.h

TEMPLATES = util/invertible_map.h util/invertible_map.cpp util/common.h util/common.cpp

CC = g++ -std=c++0x -fopenmp -I.
#CCOPTS = -g -O0
CCOPTS = -O3
LINKER = g++ -std=c++0x -fopenmp -I.

CLEANDIRS = tokenizers io index util stemmers classify

all: $(SEARCH) $(TESTER) $(PLOT) $(LIBLINEAR)

$(SEARCH): $(SEARCHOBJS) main.cpp $(TEMPLATES) $(TESTS)
	$(LINKER) main.cpp -o $@ $(SEARCHOBJS)

$(TESTER): $(TESTEROBJS) tester.cpp $(TEMPLATES) $(TESTS)
	$(LINKER) tester.cpp -o $@ $(TESTEROBJS)

$(PLOT): $(PLOTOBJS) scatter.cpp $(TEMPLATES) $(TESTS)
	$(LINKER) scatter.cpp -o $@ $(PLOTOBJS)

$(LIBLINEAR): $(LIBLINEAROBJS) liblinear.cpp $(TEMPLATES)
	$(LINKER) liblinear.cpp -o $@ $(LIBLINEAROBJS)

%.o : %.cpp $(wildcard *.h)
	$(CC) $(CCOPTS) -c $(@:.o=.cpp) -o $@

clean:
	for dir in $(CLEANDIRS) ; do rm -rf $$dir/*.o ; done
	rm -f preprocessor/*.class
	rm -f $(SEARCH) $(TESTER) $(PLOT) $(LIBLINEAR) *.o

tidy:
	rm -rf ./doc *.chunk postingsFile lexiconFile termid.mapping docid.mapping docs.lengths *compressed.txt
