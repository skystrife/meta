/**
 * @file length_filter.cpp
 * @author Chase Geigle
 */

#include "analyzers/filters/length_filter.h"
#include "util/utf.h"

namespace meta
{
namespace analyzers
{

length_filter::length_filter(std::unique_ptr<token_stream> source, uint64_t min,
                             uint64_t max)
    : source_{std::move(source)}, min_length_{min}, max_length_{max}
{
    next_token();
}

length_filter::length_filter(const length_filter& other)
    : source_{other.source_->clone()},
      token_{other.token_},
      min_length_{other.min_length_},
      max_length_{other.max_length_}
{
    // nothing
}

void length_filter::set_content(const std::string& content)
{
    token_ = util::nullopt;
    source_->set_content(content);
    next_token();
}

std::string length_filter::next()
{
    auto tok = *token_;
    next_token();
    return tok;
}

length_filter::operator bool() const
{
    return token_ || *source_;
}

void length_filter::next_token()
{
    if (!*source_)
    {
        token_ = util::nullopt;
        return;
    }

    while (*source_)
    {
        auto tok = source_->next();
        if (tok == "<s>" || tok == "</s>")
        {
            token_ = tok;
            break;
        }
        auto len = utf::length(tok);
        if (len >= min_length_ && len <= max_length_)
        {
            token_ = tok;
            break;
        }
    }
}

}
}
