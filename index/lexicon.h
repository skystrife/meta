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
#include "document.h"
#include "util/invertible_map.h"
#include "io/parser.h"
#include "util/common.h"
#include "structs.h"

using std::ofstream;
using std::vector;
using std::istringstream;
using std::endl;
using std::cerr;
using std::pair;
using std::make_pair;
using std::unordered_map;
using std::string;

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
         * @return whether this Lexicon is empty
         */
        bool isEmpty() const;

        /**
         * @return all lexicon information about a specific term.
         */
        TermData getTermInfo(TermID termID) const;

        /**
         * Writes the lexicon to disk.
         * @param docLengthsFilename
         * @param termMapFilename
         * @param docMapFilename
         */
        void save(const string & docLengthsFilename, const string & termMapFilename, const string & docMapFilename) const;

        /**
         * Adds a new term to the lexicon.
         */
        void addTerm(TermID term, TermData termData);

        /**
         * @param docID - the id of the document to get the length of
         * @return the length of the parameter document
         */
        unsigned int getDocLength(DocID docID) const;

        /**
         * @return the number of documents in this collection
         */
        unsigned int getNumDocs() const;

        /**
         * @return the average document length in the collection
         */
        double getAvgDocLength() const;

        /**
         * @return the string term associated with the termID
         */
        string getTerm(TermID termID) const;

        /**
         * @return the TermID associated with the given term
         */
        TermID getTermID(string term) const;
        
        /**
         * @return whether this lexicon has information on the specified termID
         */
        bool containsTermID(TermID termID) const;

        /**
         * @return the string document name associated with the termID
         */
        string getDoc(DocID docID) const;
        
        /**
         * @return the DocID of the given document name
         */
        DocID getDocID(string docName) const;

        /**
         * Reads document lengths from disk into memory.
         * We don't use InvertibleMap::readMap because many docLengths
         *  can be duplicated.
         */
        void setDocLengths(const string & filename);

        /**
         * @return the TermID mapping for this lexicon
         */
        const InvertibleMap<TermID, string> & getTermIDMapping() const;

    private:
 
        string _lexiconFilename;
        double _avgDL;

        unordered_map<TermID, TermData> _entries;
        unordered_map<DocID, unsigned int> _docLengths;
        InvertibleMap<TermID, string> _termMap;
        InvertibleMap<DocID, string> _docMap;

        /**
         * Reads a lexicon from disk if it exists.
         * This function is called from the lexicon constructor as
         *  well as the InvertedIndex constructor.
         */
        void readLexicon();

        /**
         * Calculates the average document length of the collection
         *  and stores it in the member variable.
         */
        void setAvgDocLength();
};

#endif
