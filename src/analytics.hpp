// What the order history says: one customer's habits, and the restaurant's
// per-item spend, pairings and menu positioning.
#pragma once

#include "pho.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Below this many orders the report says its numbers are an early signal.
// Chosen, not measured.
constexpr int MIN_ORDERS_FOR_CONFIDENCE = 30;

// Order count, total spent, last order and favourite dishes for one customer.
json customerProfile(const OrderLog& log, const std::string& customer);

// Dishes most often ordered together with `itemId`, across every customer.
json pairingsFor(const OrderLog& log, const std::string& itemId, int limit);

// Per-item spend, popularity, price band, menu-engineering quadrant and a
// positioning recommendation, plus the strongest pairings overall.
json menuReport(const OrderLog& log);

void printReport(const json& report);

json cartJson(const Cart& cart, int discountPercent);
json passportJson(const OrderLog& log, const std::string& customer);
json savedCardsJson(const Wallet& wallet);
json menuJson();
