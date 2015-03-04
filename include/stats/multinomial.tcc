/**
 * @file multinomial.tcc
 * @author Chase Geigle
 */

#include <random>
#include <unordered_map>
#include "stats/multinomial.h"
#include "util/identifiers.h"

namespace meta
{
namespace stats
{

template <class T>
multinomial<T>::multinomial()
    : total_counts_{0.0}, prior_{0.0, 0ul}
{
    // nothing
}

template <class T>
multinomial<T>::multinomial(dirichlet<T> prior)
    : total_counts_{0.0}, prior_{std::move(prior)}
{
    // nothing
}

template <class T>
void multinomial<T>::increment(const T& event, double count)
{
    counts_[event] += count;
    total_counts_ += count;
}

template <class T>
void multinomial<T>::decrement(const T& event, double count)
{
    counts_[event] -= count;
    total_counts_ -= count;
}

template <class T>
double multinomial<T>::counts(const T& event) const
{
    return counts_.at(event) + prior_.pseudo_counts(event);
}

template <class T>
double multinomial<T>::counts() const
{
    return total_counts_ + prior_.pseudo_counts();
}

template <class T>
template <class Fun>
void multinomial<T>::each_seen_event(Fun&& fun) const
{
    for (const auto& count : counts_)
        fun(count.first);
}

template <class T>
void multinomial<T>::clear()
{
    counts_.clear();
    total_counts_ = 0;
}

template <class T>
double multinomial<T>::probability(const T& event) const
{
    return counts(event) / counts();
}

template <class T>
const dirichlet<T>& multinomial<T>::prior() const
{
    return prior_;
}

template <class T>
template <class Generator>
const T& multinomial<T>::operator()(Generator&& gen) const
{
    std::uniform_real_distribution<> dist{0, 1};
    auto rnd = dist(gen);
    double sum = 0;
    for (const auto& p : counts_)
    {
        if ((sum += probability(p.first)) >= rnd)
            return p.first;
    }
    throw std::runtime_error{"failed to generate sample"};
}

template <class T>
multinomial<T>& multinomial<T>::operator+=(const multinomial<T>& rhs)
{
    for (const auto& p : rhs.counts_)
        counts_[p.first] += p.second;
    total_counts_ += rhs.total_counts_;
    return *this;
}

}
}
