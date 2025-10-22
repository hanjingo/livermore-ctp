#ifndef UTIL_H
#define UTIL_H

#include <string>
#include "market_data.h"

namespace livermore::ctp
{

namespace util
{
    static void copy_from(market_data* dst, const market_data* src);
    static void reset(market_data* md);
    static bool is_equal(const market_data* lhs, const market_data* rhs, std::string& memo);
};

}

#endif