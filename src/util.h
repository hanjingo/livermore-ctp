#ifndef UTIL_H
#define UTIL_H

#include <string>

#include <ThostFtdcMdApi.h>
#include <ThostFtdcUserApiStruct.h>
#include <ThostFtdcUserApiDataType.h>

#include "market_data.h"

namespace livermore::ctp
{
namespace util
{
void copy_from(market_data *dst, const market_data *src);
void reset(market_data *md);
bool is_equal(const market_data *lhs,
              const market_data *rhs,
              std::string       &memo);

bool convert(market_data *dst, const CThostFtdcDepthMarketDataField *src);

} // namespace util
} // namespace livermore::ctp

#endif