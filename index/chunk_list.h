/**
 * @file chunk_list.h
 */

#ifndef _CHUNK_LIST_H_
#define _CHUNK_LIST_H_

#include <vector>

/**
 * Represents a collection of chunks that are waiting to be merged into
 *  a postings file.
 */
class ChunkList
{
    public:

        /**
         * Constructor.
         * @param numChunks - this shows how many chunks have been
         *  generated for this index. It also allows the ChunkList
         *  to access the correct files.
         */
        ChunkList(size_t numChunks);

        /**
         * @return whether there is another string to write to the postings file
         */
        bool hasNext() const;

        /**
         * @return the next string to write to the postings file
         */
        IndexEntry next();

    private:

        /** how many chunks need to be merged */
        size_t _numChunks;

        /** a parser for each chunk that needs to be merged */
        std::vector<Parser> _parsers;

        /**
         * Appends entries from the vector of IndexEntries into one IndexEntry.
         * When this function ends, the entries vector will be empty.
         * @param entries - the entries to combine together
         * @param combined - where to store the combined IndexEntries
         */
        void combinePostingData(std::vector<IndexEntry> & entries, IndexEntry & combined) const;
};

#endif
