/**
 * @file lexicon.h
 */

#ifndef _LEXICON_H_
#define _LEXICON_H_

#include <fstream>
#include <iterator>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <string>
#include <utility>
#include <sstream>
#include "io/parser.h"

using std::ofstream;
using std::vector;
using std::istringstream;
using std::endl;
using std::cerr;
using std::pair;
using std::make_pair;
using std::unordered_map;
using std::string;

typedef unsigned int TermID;
typedef unsigned int DocID;

/**
 * Represents metadata for a specific term in the lexicon.
 */
struct TermData
{
    unsigned int idf;
    unsigned int totalFreq;
    unsigned int postingIndex;
    unsigned char postingBit;
};

/**
 * Represents the dictionary or lexicon of an inverted index.
 */
class Lexicon
{
    public:
        /**
         * Constructor to read an existing lexicon from disk.
         */
        Lexicon(const string & lexiconFile);

        /**
         * Constructor to create a new lexicon.
         */
        Lexicon();

        /**
         * Destructor.
         */
        ~Lexicon();

        /**
         * Copy constructor.
         */
        Lexicon(const Lexicon & other);

        /**
         * Assigns the content of one lexicon to another.
         * @param other - the lexicon to copy
         * @return a reference to the copied lexicon
         */
        const Lexicon & operator=(const Lexicon & other);

        /**
         * @return all lexicon information about a specific term.
         */
        TermData getTermInfo(TermID termID) const;

        /**
         * Writes the lexicon to disk.
         * @param filename - the name to save the file as
         */
        void save() const;

        /**
         * Adds a new term to the lexicon.
         */
        void addTerm(TermID term, TermData termData);

    private:
 
        string _lexiconFilename;
        unordered_map<TermID, TermData>* _entries;

        /**
         * Reads a lexicon from disk if it exists.
         * This function is called from the lexicon constructor as
         *  well as the InvertedIndex constructor.
         */
        void readLexicon();

        /**
         * @return the string representation of an object
         */
        template <class T>
        static string toString(T value);
};

#endif
