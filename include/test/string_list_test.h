/**
 * @file string_list_test.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_STRING_LIST_TEST_H_
#define _META_STRING_LIST_TEST_H_

#include <cstring>
#include <fstream>

#include "test/unit_test.h"
#include "io/binary.h"
#include "index/string_list.h"
#include "index/string_list_writer.h"
#include "util/filesystem.h"

#include "unit_test.h"

namespace meta
{
namespace testing
{

void assert_read(std::ifstream& file, const std::string& expect);

struct file_guard
{
    file_guard(const std::string& path) : path_{path}
    {
        filesystem::delete_file(path_);
    }

    ~file_guard()
    {
        filesystem::delete_file(path_);
    }

    const std::string& path_;
};

int string_list_tests();
}
}

#endif
