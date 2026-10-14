// The order log, and everything computed from it.

#include "analytics.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>

// ------------------------------------------------------------ order log ----

std::string OrderLog::defaultPath()
{
    if (const char* path = std::getenv("PHO_ORDERS")) {
        return path;
    }
    const char* home = std::getenv("HOME");
    return std::string(home ? home : ".") + "/.pho_orders";
}

std::string OrderLog::defaultCustomer()
{
    if (const char* name = std::getenv("PHO_CUSTOMER")) {
        return name;
    }
    const char* user = std::getenv("USER");
    return user ? user : "guest";
}

// One order per line, tab-separated, amounts in cents:
// when, customer, subtotal, discount, tax, tip, total, items as "P1x2,D4x1".
OrderLog::OrderLog(std::string path) : path_(std::move(path))
{
    std::ifstream in(path_);
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream fields(line);
        PastOrder order;
        std::string subtotal, discount, tax, tip, total, items;
        if (!(std::getline(fields, order.when, '\t') && std::getline(fields, order.customer, '\t') &&
              std::getline(fields, subtotal, '\t') && std::getline(fields, discount, '\t') &&
              std::getline(fields, tax, '\t') &&
              std::getline(fields, tip, '\t') && std::getline(fields, total, '\t') &&
              std::getline(fields, items))) {
            continue;
        }
        order.subtotal = std::atol(subtotal.c_str());
        order.discount = std::atol(discount.c_str());
        order.tax      = std::atol(tax.c_str());
        order.tip      = std::atol(tip.c_str());
        order.total    = std::atol(total.c_str());
        std::istringstream entries(items);
        std::string entry;
        while (std::getline(entries, entry, ',')) {
            const auto x = entry.find('x');
            ItemRef ref;
            if (x != std::string::npos && findItem(entry.substr(0, x), ref)) {
                order.lines.push_back({ref, std::atoi(entry.c_str() + x + 1)});
            }
        }
        if (!order.lines.empty()) {
            orders_.push_back(order);
        }
    }
}

void OrderLog::record(const std::string& customer, const Cart& cart, const Bill& bill)
{
    PastOrder order;
    char when[32];
    const std::time_t now = std::time(nullptr);
    std::strftime(when, sizeof when, "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&now));
    order.when     = when;
    order.customer = customer;
    order.lines    = cart.lines();
    order.subtotal = bill.subtotal;
    order.discount = bill.discount;
    order.tax      = bill.tax;
    order.tip      = bill.tip;
    order.total    = bill.total;
    orders_.push_back(order);

    std::ofstream out(path_, std::ios::app);
    out << order.when << '\t' << order.customer << '\t' << order.subtotal << '\t' << order.discount << '\t'
        << order.tax << '\t'
        << order.tip << '\t' << order.total << '\t';
    for (std::size_t i = 0; i < order.lines.size(); ++i) {
        out << (i ? "," : "") << itemAt(order.lines[i].ref).id << 'x' << order.lines[i].quantity;
    }
    out << '\n';
}

std::vector<PastOrder> OrderLog::forCustomer(const std::string& customer) const
{
    std::vector<PastOrder> mine;
    for (const PastOrder& order : orders_) {
        if (order.customer == customer) {
            mine.push_back(order);
        }
    }
    return mine;
}

// ------------------------------------------------------ foodie passport ----

Passport passportFor(const OrderLog& log, const std::string& customer)
{
    Passport passport;
    std::set<std::string> dishes;
    std::set<int> sections;
    for (const PastOrder& order : log.forCustomer(customer)) {
        if (order.discount > 0) {
            passport.stamps = 0;  // the reward was spent on this order
            ++passport.rewardsUsed;
        } else if (passport.stamps < STAMPS_FOR_REWARD) {
            ++passport.stamps;
        }
        for (const CartLine& line : order.lines) {
            dishes.insert(itemAt(line.ref).id);
            sections.insert(line.ref.category);
        }
    }
    passport.rewardReady   = passport.stamps >= STAMPS_FOR_REWARD;
    passport.dishesTried   = static_cast<int>(dishes.size());
    passport.sectionsTried = static_cast<int>(sections.size());
    passport.sectionsTotal = static_cast<int>(MENU.size());
    for (const Category& category : MENU) {
        passport.dishesTotal += static_cast<int>(category.items.size());
    }
    return passport;
}

std::string stampLine(const Passport& passport)
{
    std::string line = "Foodie Passport: ";
    for (int i = 0; i < STAMPS_FOR_REWARD; ++i) {
        line += i < passport.stamps ? "[*]" : "[ ]";
    }
    line += " " + std::to_string(passport.stamps) + " of " + std::to_string(STAMPS_FOR_REWARD) + " stamps";
    if (passport.rewardReady) {
        line += " - " + std::to_string(REWARD_PERCENT) + "% off this order!";
    }
    return line;
}

json passportJson(const OrderLog& log, const std::string& customer)
{
    const Passport p = passportFor(log, customer);
    json sections = json::array();
    std::set<std::string> tried;
    for (const PastOrder& order : log.forCustomer(customer)) {
        for (const CartLine& line : order.lines) tried.insert(itemAt(line.ref).id);
    }
    for (const Category& category : MENU) {
        int count = 0;
        for (const Item& item : category.items) count += tried.count(item.id) ? 1 : 0;
        sections.push_back({{"title", category.title}, {"tried", count}, {"total", category.items.size()}});
    }
    const int toGo = STAMPS_FOR_REWARD - p.stamps;
    return {{"stamps", p.stamps},
            {"stamps_for_reward", STAMPS_FOR_REWARD},
            {"reward_percent", REWARD_PERCENT},
            {"reward_ready", p.rewardReady},
            {"rewards_used", p.rewardsUsed},
            {"dishes_tried", p.dishesTried},
            {"dishes_total", p.dishesTotal},
            {"sections", sections},
            {"message", p.rewardReady
                            ? "Your passport is full: " + std::to_string(REWARD_PERCENT) + "% off this order."
                            : std::to_string(toGo) + (toGo == 1 ? " more order" : " more orders") + " until " +
                                  std::to_string(REWARD_PERCENT) + "% off."}};
}

// --------------------------------------------------------------- shapes ----

static json itemJson(ItemRef ref)
{
    const Item& item = itemAt(ref);
    return {{"id", item.id}, {"name", item.name}, {"price", money(item.cents)}};
}

json menuJson()
{
    json categories = json::array();
    for (const Category& category : MENU) {
        json items = json::array();
        for (const Item& item : category.items) {
            items.push_back({{"id", item.id}, {"name", item.name}, {"description", item.description},
                             {"price", money(item.cents)}, {"cents", item.cents}});
        }
        categories.push_back({{"title", category.title}, {"items", items}});
    }
    return categories;
}

json cartJson(const Cart& cart, int discountPercent)
{
    const Bill bill = billFor(cart, discountPercent, 0);
    json lines = json::array();
    for (const CartLine& line : cart.lines()) {
        json entry = itemJson(line.ref);
        entry["quantity"]   = line.quantity;
        entry["line_total"] = money(itemAt(line.ref).cents * line.quantity);
        lines.push_back(entry);
    }
    json tips = json::array();
    for (int percent : TIP_CHOICES) {
        const Bill withTip = billFor(cart, discountPercent, percent);
        tips.push_back({{"percent", percent}, {"amount", money(withTip.tip)}, {"total", money(withTip.total)}});
    }
    return {{"lines", lines}, {"item_count", cart.count()}, {"subtotal", money(bill.subtotal)},
            {"discount", money(bill.discount)}, {"discount_percent", discountPercent},
            {"tax", money(bill.tax)}, {"tax_percent", TAX_PERCENT}, {"total", money(bill.total)},
            {"tip_options", tips}};
}

json savedCardsJson(const Wallet& wallet)
{
    json cards = json::array();
    for (std::size_t i = 0; i < wallet.cards().size(); ++i) {
        const SavedCard& card = wallet.cards()[i];
        cards.push_back({{"index", i + 1}, {"nickname", card.nickname}, {"brand", card.brand},
                         {"last4", card.last4}, {"label", describe(card)}});
    }
    return {{"cards", cards}, {"limit", Wallet::MAX_CARDS}};
}

// -------------------------------------------------------- the customer ----

json customerProfile(const OrderLog& log, const std::string& customer)
{
    const std::vector<PastOrder> orders = log.forCustomer(customer);
    json profile = {{"customer", customer}, {"order_count", orders.size()}};
    if (orders.empty()) {
        return profile;
    }

    long spent = 0;
    std::map<std::string, std::pair<int, int>> byItem;  // id -> (orders containing, quantity)
    for (const PastOrder& order : orders) {
        spent += order.total;
        for (const CartLine& line : order.lines) {
            auto& counts = byItem[itemAt(line.ref).id];
            counts.first += 1;
            counts.second += line.quantity;
        }
    }

    std::vector<std::pair<std::string, std::pair<int, int>>> ranked(byItem.begin(), byItem.end());
    std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
        return a.second.first != b.second.first ? a.second.first > b.second.first
                                                : a.second.second > b.second.second;
    });
    json favorites = json::array();
    for (std::size_t i = 0; i < ranked.size() && i < 5; ++i) {
        ItemRef ref;
        findItem(ranked[i].first, ref);
        json entry = itemJson(ref);
        entry["orders_containing"] = ranked[i].second.first;
        entry["total_quantity"]    = ranked[i].second.second;
        favorites.push_back(entry);
    }

    const PastOrder& last = orders.back();
    json lastItems = json::array();
    for (const CartLine& line : last.lines) {
        json entry = itemJson(line.ref);
        entry["quantity"] = line.quantity;
        lastItems.push_back(entry);
    }

    profile["total_spent"] = money(spent);
    profile["average_order"] = money(spent / static_cast<long>(orders.size()));
    profile["favorites"] = favorites;
    profile["last_order"] = {{"when", last.when}, {"items", lastItems}, {"total", money(last.total)}};
    return profile;
}

// ----------------------------------------------------------- pairings -----

static std::set<std::string> idsIn(const PastOrder& order)
{
    std::set<std::string> ids;
    for (const CartLine& line : order.lines) {
        ids.insert(itemAt(line.ref).id);
    }
    return ids;
}

json pairingsFor(const OrderLog& log, const std::string& itemId, int limit)
{
    ItemRef target;
    if (!findItem(itemId, target)) {
        return json::array();
    }
    const std::string id = itemAt(target).id;
    int withItem = 0;
    std::map<std::string, int> together;
    for (const PastOrder& order : log.all()) {
        const auto ids = idsIn(order);
        if (!ids.count(id)) {
            continue;
        }
        ++withItem;
        for (const std::string& other : ids) {
            if (other != id) {
                ++together[other];
            }
        }
    }
    std::vector<std::pair<std::string, int>> ranked(together.begin(), together.end());
    std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
    json pairs = json::array();
    for (std::size_t i = 0; i < ranked.size() && static_cast<int>(i) < limit; ++i) {
        ItemRef ref;
        findItem(ranked[i].first, ref);
        json entry = itemJson(ref);
        entry["orders_together"] = ranked[i].second;
        entry["share_of_orders_with_item"] = std::round(1000.0 * ranked[i].second / withItem) / 10.0;
        pairs.push_back(entry);
    }
    return pairs;
}

// ------------------------------------------------------ the restaurant ----

// Menu engineering (Kasavana & Smith) sorts each dish by popularity and
// profitability. The menu holds no food costs, so profitability here is the
// dish's PRICE against its category's average price — a stand-in, and the
// report says so. Popular = at least 70% of an even share of its category's
// units (the method's usual threshold).
json menuReport(const OrderLog& log)
{
    const auto& orders = log.all();
    struct Stats {
        int units = 0;
        int orders = 0;
        long revenue = 0;
    };
    std::map<std::string, Stats> stats;
    long totalRevenue = 0;
    for (const PastOrder& order : orders) {
        for (const CartLine& line : order.lines) {
            Stats& s = stats[itemAt(line.ref).id];
            s.units += line.quantity;
            s.orders += 1;
            s.revenue += itemAt(line.ref).cents * line.quantity;
            totalRevenue += itemAt(line.ref).cents * line.quantity;
        }
    }

    json items = json::array();
    json categories = json::array();
    for (const Category& category : MENU) {
        int categoryUnits = 0;
        long categoryRevenue = 0;
        long priceSum = 0;
        for (const Item& item : category.items) {
            categoryUnits += stats[item.id].units;
            categoryRevenue += stats[item.id].revenue;
            priceSum += item.cents;
        }
        const double evenShare   = 1.0 / static_cast<double>(category.items.size());
        const long averagePrice  = priceSum / static_cast<long>(category.items.size());
        categories.push_back({{"title", category.title}, {"units", categoryUnits},
                              {"revenue", money(categoryRevenue)},
                              {"revenue_share", totalRevenue ? std::round(1000.0 * categoryRevenue / totalRevenue) / 10.0 : 0.0}});

        for (const Item& item : category.items) {
            const Stats& s = stats[item.id];
            const double share  = categoryUnits ? static_cast<double>(s.units) / categoryUnits : 0.0;
            const bool popular  = categoryUnits > 0 && share >= 0.7 * evenShare;
            const bool premium  = item.cents >= averagePrice;
            std::string quadrant, advice;
            if (popular && premium) {
                quadrant = "Star";
                advice   = "Feature it: top of its section, first thing the assistant suggests.";
            } else if (popular) {
                quadrant = "Plowhorse";
                advice   = "Sells on its own. Pair it with a higher-priced side or drink, and review its price.";
            } else if (premium) {
                quadrant = "Puzzle";
                advice   = "Priced well but not chosen. Move it up the section and sharpen its description.";
            } else {
                quadrant = "Dog";
                advice   = "Low sales at a low price. Consider replacing it or folding it into a combo.";
            }
            ItemRef ref;
            findItem(item.id, ref);
            json pair = pairingsFor(log, item.id, 1);
            items.push_back({{"id", item.id}, {"name", item.name}, {"category", category.title},
                             {"price", money(item.cents)}, {"units", s.units}, {"orders", s.orders},
                             {"revenue", money(s.revenue)}, {"revenue_cents", s.revenue},
                             {"revenue_share", totalRevenue ? std::round(1000.0 * s.revenue / totalRevenue) / 10.0 : 0.0},
                             {"share_of_category_units", std::round(1000.0 * share) / 10.0},
                             {"popular", popular}, {"price_band", premium ? "higher" : "lower"},
                             {"quadrant", categoryUnits ? quadrant : "No sales yet"},
                             {"recommendation", categoryUnits ? advice : "No orders in this section yet."},
                             {"best_pair", pair.empty() ? json(nullptr) : pair[0]}});
        }
    }

    std::stable_sort(items.begin(), items.end(),
                     [](const json& a, const json& b) { return a["revenue_cents"] > b["revenue_cents"]; });

    // The strongest pairs overall, by orders containing both.
    std::map<std::pair<std::string, std::string>, int> pairCounts;
    for (const PastOrder& order : orders) {
        const auto ids = idsIn(order);
        for (auto a = ids.begin(); a != ids.end(); ++a) {
            for (auto b = std::next(a); b != ids.end(); ++b) {
                ++pairCounts[{*a, *b}];
            }
        }
    }
    std::vector<std::pair<std::pair<std::string, std::string>, int>> rankedPairs(pairCounts.begin(), pairCounts.end());
    std::sort(rankedPairs.begin(), rankedPairs.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
    json topPairs = json::array();
    for (std::size_t i = 0; i < rankedPairs.size() && i < 5; ++i) {
        ItemRef a, b;
        findItem(rankedPairs[i].first.first, a);
        findItem(rankedPairs[i].first.second, b);
        topPairs.push_back({{"items", {itemAt(a).name, itemAt(b).name}}, {"orders_together", rankedPairs[i].second}});
    }

    long spent = 0, discounts = 0;
    std::set<std::string> customers;
    for (const PastOrder& order : orders) {
        spent += order.total;
        discounts += order.discount;
        customers.insert(order.customer);
    }
    return {{"orders", orders.size()},
            {"customers", customers.size()},
            {"food_revenue", money(totalRevenue)},
            {"average_order", orders.empty() ? money(0) : money(spent / static_cast<long>(orders.size()))},
            {"passport_discounts", money(discounts)},
            {"early_signal", static_cast<int>(orders.size()) < MIN_ORDERS_FOR_CONFIDENCE},
            {"method_note", "Quadrants use price against the category average, not profit margin: the menu has no food costs."},
            {"categories", categories},
            {"items", items},
            {"top_pairs", topPairs}};
}

void printReport(const json& report)
{
    std::cout << "\n  Pho of Fort Wayne - menu report\n\n"
              << "  Orders: " << report["orders"] << "   Customers: " << report["customers"]
              << "   Food revenue: " << report["food_revenue"].get<std::string>()
              << "   Average order: " << report["average_order"].get<std::string>()
              << "\n  Passport discounts given: " << report["passport_discounts"].get<std::string>() << "\n";
    if (report["orders"] == 0) {
        std::cout << "\n  No orders yet. Place some orders first.\n";
        return;
    }
    if (report["early_signal"]) {
        std::cout << "  Fewer than " << MIN_ORDERS_FOR_CONFIDENCE
                  << " orders: treat these numbers as an early signal.\n";
    }
    std::cout << "  " << report["method_note"].get<std::string>() << "\n\n"
              << "  " << std::left << std::setw(28) << "Item" << std::right << std::setw(7) << "Price"
              << std::setw(7) << "Units" << std::setw(10) << "Revenue" << std::setw(8) << "Share"
              << "  " << std::left << "Position\n";
    for (const json& item : report["items"]) {
        if (item["units"] == 0) {
            continue;
        }
        std::ostringstream share;
        share << std::fixed << std::setprecision(1) << item["revenue_share"].get<double>() << '%';
        std::cout << "  " << std::left << std::setw(28) << item["name"].get<std::string>() << std::right
                  << std::setw(7) << item["price"].get<std::string>() << std::setw(7) << item["units"].get<int>()
                  << std::setw(10) << item["revenue"].get<std::string>() << std::setw(8) << share.str() << "  "
                  << item["quadrant"].get<std::string>() << "\n";
    }

    std::cout << "\n  Positioning\n";
    for (const json& item : report["items"]) {
        if (item["units"] == 0) {
            continue;
        }
        std::cout << "    " << item["name"].get<std::string>() << " (" << item["quadrant"].get<std::string>()
                  << "): " << item["recommendation"].get<std::string>();
        if (!item["best_pair"].is_null()) {
            std::cout << " Most often ordered with " << item["best_pair"]["name"].get<std::string>() << ".";
        }
        std::cout << "\n";
    }

    std::cout << "\n  Never ordered (menu order)\n";
    bool any = false;
    for (const Category& category : MENU) {
        std::string names;
        for (const Item& item : category.items) {
            for (const json& row : report["items"]) {
                if (row["id"] == item.id && row["units"] == 0) {
                    names += (names.empty() ? "" : ", ") + item.name;
                }
            }
        }
        if (!names.empty()) {
            std::cout << "    " << category.title << ": " << names << "\n";
            any = true;
        }
    }
    if (!any) std::cout << "    none\n";
    std::cout << "\n  Ordered together most\n";
    if (report["top_pairs"].empty()) std::cout << "    No orders with two or more dishes yet.\n";
    for (const json& pair : report["top_pairs"]) {
        std::cout << "    " << pair["items"][0].get<std::string>() << " + " << pair["items"][1].get<std::string>()
                  << ": " << pair["orders_together"] << " orders\n";
    }
}
