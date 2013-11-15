/**
 * @file sqlite_map.h
 * @author Sean Massung
 * 
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _SQLITE_MAP_H_
#define _SQLITE_MAP_H_

#include <mutex>
#include <type_traits>
#include <string>
#include "meta.h"
#include "sqlite3.h"

namespace meta {
namespace util {

/**
 * Wrapper for a sqlite3 database as a simple dictionary. Keys and values can be
 * integral types, floating point, or strings. Note that the strings currently
 * don't support containing spaces.
 */
template <class Key, class Value>
class sqlite_map 
{
    static_assert(
        std::is_integral<Key>::value ||
        std::is_floating_point<Key>::value ||
        std::is_base_of<util::numeric, Key>::value ||
        std::is_same<std::string, Key>::value,
        "sqlite_map templated types must be integral types or string types"
    );

    static_assert(
        std::is_integral<Value>::value ||
        std::is_floating_point<Value>::value ||
        std::is_base_of<util::numeric, Value>::value ||
        std::is_same<std::string, Value>::value,
        "sqlite_map templated types must be integral types or string types"
    );

    public:
        /**
         * @param filename The file where the map is stored on disk
         */
        sqlite_map(const std::string & filename);

        /**
         * Destructor.
         */
        ~sqlite_map();

        /**
         * If the key already exists, the map is unchanged.
         * @param key The key to insert
         * @param value The value to insert
         */
        void insert(const Key & key, const Value & value);

        /**
         * @param key The key of the item to search for
         * @return the value corresponding to this key
         */
        Value find(const Key & key);

        /**
         * Increments the key's value by the specified amount. If the key does
         * not exist yet, a new one is inserted with the amount as the value.
         * @param key
         * @param amount
         */
        void increment(const Key & key, const Value & amount);

        /**
         * @return the number of elements (rows) in this map
         */
        uint64_t size();

    private:
        /** pointer to the database; sqlite3 handles memory for this object */
        sqlite3* _db;

        /** contains the type that Key is stored as in the database */
        std::string _sql_key_type;

        /** contains the type that Value is stored as in the database */
        std::string _sql_value_type;

        /** where callback functions store results */
        Value _return_value;

        /** the number of elements in this map (calculated by sqlite) */
        uint64_t _size;

        /** a mutex for synchronization */
        std::mutex _mutex;

        /** buffer of insert commands */
        std::string _commands;

        /** indicates whether the _size variable may need to be updated */
        bool _size_dirty;

        /** the current number of insert commands cached */
        uint64_t _num_cached;

        /** after this number is reached, commit inserts and updates */
        const static uint64_t _max_cached = 100000;

        /**
         * @return the type that the C++ object would be stored in a SQL
         * database as
         */
        template <class T>
        std::string sql_type() const;

        /**
         * @param elem The element to convert to a sql string
         * @return a sql string of elem
         */
        template <class T>
        std::string sql_text(const T & elem) const;

        /**
         * Writes a series of inserts to the database.
         */
        void commit();

        /**
         * The find callback for an sqlite3_exec function.
         */
        static int find_callback(void* state, int num_cols,
                char** fields, char** cols);
        /**
         * The find callback for an sqlite3_exec function.
         */
        static int size_callback(void* state, int num_cols,
                char** fields, char** cols);

    public:
        /**
         * Basic exception for sqlite_map.
         */
        class sqlite_map_exception: public std::exception
        {
            public:
                sqlite_map_exception(const std::string & error):
                    _error(error) { /* nothing */ }

                const char* what () const throw ()
                {
                    return _error.c_str();
                }
           
            private:
                std::string _error;
        };
};

}
}

#include "sqlite_map.tcc"
#endif
