/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include "roq/json/datetime.hpp"
#include "roq/json/number.hpp"
#include "roq/json/string.hpp"

#include "roq/fix_bridge/fix/execution_report.hpp"

namespace simple {
namespace json {

// note! supports both rest and websocket

struct ExecutionReport final {
  explicit ExecutionReport(roq::fix_bridge::fix::ExecutionReport const &value) : value_{value} {}

  template <typename Context>
  auto format_to(Context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"("order_id":{},)"
        R"("cl_ord_id":{},)"
        R"("orig_cl_ord_id":{},)"
        R"("ord_status_req_id":{},)"
        R"("mass_status_req_id":{},)"
        R"("tot_num_reports":{},)"
        R"("last_rpt_requested":{},)"
        R"("exec_id":{},)"
        R"("exec_type":{},)"
        R"("ord_status":{},)"
        R"("working_indicator":{},)"
        R"("ord_rej_reason":{},)"
        R"("account":{},)"
        R"("account_type":{},)"
        R"("symbol":{},)"
        R"("security_exchange":{},)"
        R"("side":{},)"
        R"("ord_type":{},)"
        R"("order_qty":{},)"
        R"("price":{},)"
        R"("stop_px":{},)"
        R"("currency":{},)"
        R"("time_in_force":{},)"
        R"("exec_inst":{},)"
        R"("last_qty":{},)"
        R"("last_px":{},)"
        R"("trading_session_id":{},)"
        R"("leaves_qty":{},)"
        R"("cum_qty":{},)"
        R"("avg_px":{},)"
        R"("transact_time":{},)"
        R"("position_effect":{},)"
        R"("max_show":{},)"
        R"("text":{},)"
        R"("last_liquidity_ind":{})"
        R"(}})"sv,
        roq::json::String{value_.order_id},
        roq::json::String{value_.cl_ord_id},
        roq::json::String{value_.orig_cl_ord_id},
        roq::json::String{value_.ord_status_req_id},
        roq::json::String{value_.mass_status_req_id},
        value_.tot_num_reports,
        value_.last_rpt_requested,
        roq::json::String{value_.exec_id},
        roq::json::String{value_.exec_type},
        roq::json::String{value_.ord_status},
        value_.working_indicator,
        roq::json::String{value_.ord_rej_reason},
        roq::json::String{value_.account},
        roq::json::String{value_.account_type},
        roq::json::String{value_.symbol},
        roq::json::String{value_.security_exchange},
        roq::json::String{value_.side},
        roq::json::String{value_.ord_type},
        roq::json::Number{value_.order_qty},
        roq::json::Number{value_.price},
        roq::json::Number{value_.stop_px},
        roq::json::String{value_.currency},
        roq::json::String{value_.time_in_force},
        roq::json::String{value_.exec_inst},
        roq::json::Number{value_.last_qty},
        roq::json::Number{value_.last_px},
        roq::json::String{value_.trading_session_id},
        roq::json::Number{value_.leaves_qty},
        roq::json::Number{value_.cum_qty},
        roq::json::Number{value_.avg_px},
        roq::json::DateTime{value_.transact_time},
        roq::json::String{value_.position_effect},
        roq::json::Number{value_.max_show},
        roq::json::String{value_.text},
        roq::json::String{value_.last_liquidity_ind});
  }

 private:
  roq::fix_bridge::fix::ExecutionReport const &value_;
};

}  // namespace json
}  // namespace simple

template <>
struct fmt::formatter<simple::json::ExecutionReport> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(simple::json::ExecutionReport const &value, Context &context) const {
    return value.format_to(context);
  }
};
