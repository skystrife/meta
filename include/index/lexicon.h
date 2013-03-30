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

#include "structs.h"
#include "util/invertible_map.h"

namespace meta {
namespace index {

/**
 * Represents the dictionary or lexicon of an inverted index.
 */
class Lexicon
{
    public:

        /**
         * Constructor to read an existing lexicon from disk.
         */
        Lexicon(const std::string & lexiconFile);

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
        void save(const std::string & docLengthsFilename, const std::string & termMapFilename, const std::string & docMapFilename) const;

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
        std::string getTerm(TermID termID) const;

        /**
         * @return the TermID associated with the given term
         */
        TermID getTermID(std::string term) const;
        
        /**
         * @return whether this lexicon has information on the specified termID
         */
        bool containsTermID(TermID termID) const;

        /**
         * @return the string document name associated with the termID
         */
        std::string getDoc(DocID docID) const;
        
        /**
         * @return the DocID of the given document name
         */
        DocID getDocID(std::string docName) const;

        /**
         * Reads document lengths from disk into memory.
         * We don't use InvertibleMap::readMap because many docLengths
         *  can be duplicated.
         */
        void setDocLengths(const std::string & filename);

        /**
         * @return the TermID mapping for this lexicon
         */
        const util::InvertibleMap<index::TermID, std::string> & getTermIDMapping() const;

    private:

        /** the name of the lexicon file */
        std::string _lexiconFilename;

        /** saves the average document length in this collection */
        double _avgDL;

        /** maps TermID (tokens) -> TermData (where to find in the postingsFile) */
        std::unordered_map<TermID, TermData> _entries;

        /** lengths for all documents in the index, used in ranking functions */
        std::unordered_map<DocID, unsigned int> _docLengths;

        /** maps TermIDs to the strings they represent */
        util::InvertibleMap<index::TermID, std::string> _termMap;

        /** maps DocIDs to the document paths they represent */
        util::InvertibleMap<index::DocID, std::string> _docMap;

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

}
}

#endif
