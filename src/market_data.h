#ifndef MARKET_DATA_H
#define MARKET_DATA_H

extern "C" 
{

typedef char   md_date_t[9];
typedef char   md_time_t[9];
typedef int    md_millisec_t;
typedef char   md_instrument_id_t[31];
typedef char   md_exchange_id_t[9];
typedef double md_price_t;
typedef double md_money_t;
typedef double md_volume_t;
typedef char   md_instrument_name_t[21];

#pragma pack(push, 8)
struct market_data
{
    md_date_t            trading_day;          // trading day
    md_instrument_name_t instrument_name;      // contract name
    md_instrument_id_t   instrument_id;        // contract code
    md_exchange_id_t     exchange_id;          // exchange
    md_price_t           last_price;           // last price
    md_price_t           pre_close_price;      // closing price on the preceding trading day
    md_price_t           open_price;           // opening price
    md_price_t           pre_settlement_price; // settlement price on the preceding trading day
    md_price_t           highest_price;        // high price
    md_price_t           lowest_price;         // low price
    md_price_t           close_price;          // closing price
    md_price_t           settlement_price;     // settlement price
    md_price_t           upper_limit_price;    // limit up price
    md_price_t           lower_limit_price;    // limit down price
    md_price_t           average_price;        // trading-volume-weighted average execution price
#ifdef USE_DEEP_DATA
    md_price_t           bid_price1;
    md_price_t           ask_price1;
    md_price_t           bid_price2;
    md_price_t           ask_price2;
    md_price_t           bid_price3;
    md_price_t           ask_price3;
    md_price_t           bid_price4;
    md_price_t           ask_price4;
    md_price_t           bid_price5;
    md_price_t           ask_price5;
    md_volume_t          bid_volume1;
    md_volume_t          ask_volume1;
    md_volume_t          bid_volume2;
    md_volume_t          ask_volume2;
    md_volume_t          bid_volume3;
    md_volume_t          ask_volume3;
    md_volume_t          bid_volume4;
    md_volume_t          ask_volume4;
    md_volume_t          bid_volume5;
    md_volume_t          ask_volume5;
#endif
    md_volume_t          volume;             // trading volume
    md_volume_t          pre_open_interest;  // open interest on the preceding trading day
    md_volume_t          open_interest;      // open interest on the current trading day
    md_money_t           turnover;           // notional value
    md_time_t            action_time;        // operation time
    md_millisec_t        action_ms;          // operation time milliseconds
};
#pragma pack(pop)

}

#endif
