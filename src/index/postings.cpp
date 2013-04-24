/**
 * @file postings.cpp
 */

#include "tokenizers/tokenizer.h"
#include "io/compressed_file_reader.h"
#include "io/compressed_file_writer.h"
#include "index/lexicon.h"
#include "index/structs.h"
#include "index/chunk_list.h"
#include "index/postings.h"
#include "index/index.h"

namespace meta {
namespace index {

using std::map;
using std::istringstream;
using std::unordered_map;
using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;
using std::cerr;
using std::endl;

using util::invertible_map;
using tokenizers::tokenizer;

Postings::Postings(const string & postingsFile):
    //_reader(postingsFile),
    _postingsFilename(postingsFile),
    _docMap(invertible_map<doc_id, string>()),
    _currentdoc_id(0)
{ /* nothing */ }

vector<PostingData> Postings::getDocs(const TermData & termData) const
{
    string line = getLine(termData.postingIndex);

    istringstream iss(line);
    vector<string> items;
    copy(std::istream_iterator<string>(iss),
         std::istream_iterator<string>(),
         std::back_inserter<vector<string>>(items));

    vector<PostingData> data;
    for(size_t i = 1; i < items.size(); i += 2)
    {
        PostingData postingData;
        istringstream(items[i]) >> postingData.docID;
        istringstream(items[i + 1]) >> postingData.freq;
        data.push_back(postingData);
    }

    return data;
}

vector<PostingData> Postings::getCompressedDocs(const TermData & termData) const
{
    vector<PostingData> data;
    return data;
}

string Postings::getLine(unsigned int lineNumber) const
{
    size_t currentLine = 0;
    ifstream infile(_postingsFilename, ifstream::in);
    string line;
    while(infile.good() && currentLine++ < lineNumber + 1)
        std::getline(infile, line);
    infile.close();
    return line;
}

size_t Postings::createChunks(vector<document> & documents, size_t chunkMBSize,
        std::shared_ptr<tokenizer> tokenizer)
{
    cerr << "[Postings]: creating chunks" << endl;

    size_t chunkNum = 0;
    size_t maxSize = chunkMBSize * 4 * 1024 * 1024;
    vector<string> chunkNames;
    map<term_id, vector<PostingData>> terms;

    // iterate over documents, writing data to disk when we reach chunkMBSize
    for(auto & doc: documents)
    {
        tokenizer->tokenize(doc);
        doc_id docID = getdoc_id(doc.path());
        cerr << " -> tokenizing " << doc.name() << " (docid " << docID << ")" << endl;
        unordered_map<term_id, unsigned int> freqs = doc.frequencies();
        for(auto & freq: freqs)
            terms[freq.first].push_back(PostingData(docID, freq.second));

        if(terms.size() * (sizeof(term_id) + sizeof(PostingData)) >= maxSize)
            writeChunk(terms, chunkNum++);
    }

    // write the last partial chunk
    if(terms.size())
        writeChunk(terms, chunkNum++);
    return chunkNum;
}

doc_id Postings::getdoc_id(const string & path)
{
    if(_docMap.contains_value(path))
        return _docMap.get_key(path);
    else
    {
        _docMap.insert(_currentdoc_id, path);
        return _currentdoc_id++;
    }
}

void Postings::writeChunk(map<term_id, vector<PostingData>> & terms, size_t chunkNum) const
{
    std::stringstream ss;
    ss << chunkNum;
    string fileNumber = ss.str();
    ofstream outfile(fileNumber + ".chunk");

    if(outfile.good())
    {
        for(auto & term: terms)
        {
            outfile << term.first;
            for(auto & pdata: term.second)
                outfile << " " << pdata.docID << " " << pdata.freq;
            outfile << "\n";
        }

        outfile.close();
        terms.clear();
    }
    else
        throw Index::index_exception("[Postings]: error creating chunk file");
}

void Postings::createPostingsFile(size_t numChunks, Lexicon & lexicon)
{
    cerr << "[Postings]: merging chunks to create postings file" << endl;
    ofstream postingsFile(_postingsFilename);
    if(!postingsFile.good())
        throw Index::index_exception("[Postings]: error creating postings file");

    size_t line = 0;
    ChunkList chunks(numChunks);
    while(chunks.hasNext())
    {
        IndexEntry entry = chunks.next();

        TermData termData;
        termData.idf = entry.data.size();
        termData.totalFreq = getTotalFreq(entry.data);
        termData.postingIndex = line++;
        termData.postingBit = 0; // uncompressed, always 0

        lexicon.addTerm(entry.termID, termData);
        postingsFile << entry.toString() << "\n";
    }

    postingsFile.close();
}

void Postings::savedoc_idMapping(const string & filename) const
{
    _docMap.save(filename);
}

unsigned int Postings::getTotalFreq(const vector<PostingData> & pdata) const
{
    unsigned int freq = 0;
    for(auto & d: pdata)
        freq += d.freq;
    return freq;
}

void Postings::saveDocLengths(const vector<document> & documents, const string & filename)
{
    ofstream outfile(filename);
    if(outfile.good())
    {
        for(auto & doc: documents)
            outfile << getdoc_id(doc.path()) << " " << doc.length() << endl;
    }
    else
        throw Index::index_exception("[Postings]: error saving document lengths");
}

}
}
