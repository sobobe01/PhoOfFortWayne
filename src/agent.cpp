// The Claude client, the ordering assistant's tool loop, and owner insights.

#include "agent.hpp"

#include <curl/curl.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <sstream>
#include <thread>

// ---------------------------------------------------------------- client ----

ClaudeClient::ClaudeClient()
{
    if (const char* key = std::getenv("ANTHROPIC_API_KEY")) {
        apiKey_ = key;
    }
    const char* base = std::getenv("ANTHROPIC_BASE_URL");
    baseUrl_ = base && *base ? base : "https://api.anthropic.com";
}

static size_t collect(char* data, size_t size, size_t count, void* out)
{
    static_cast<std::string*>(out)->append(data, size * count);
    return size * count;
}

json ClaudeClient::send(const json& body, std::string& error) const
{
    if (!configured()) {
        error = "ANTHROPIC_API_KEY is not set.";
        return nullptr;
    }
    const std::string payload = body.dump();
    const std::string url = baseUrl_ + "/v1/messages";

    // Rate limits, overload and server errors are retried twice with backoff.
    for (int attempt = 0; attempt < 3; ++attempt) {
        if (attempt > 0) {
            std::this_thread::sleep_for(std::chrono::seconds(attempt * 2));
        }
        CURL* curl = curl_easy_init();
        if (!curl) {
            error = "Could not start the HTTP client.";
            return nullptr;
        }
        std::string response;
        curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "content-type: application/json");
        headers = curl_slist_append(headers, ("x-api-key: " + apiKey_).c_str());
        headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");
        // Lets the API re-run a declined request on Anthropic's recommended fallback model.
        headers = curl_slist_append(headers, "anthropic-beta: server-side-fallback-2026-07-01");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, collect);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 600L);
        const CURLcode code = curl_easy_perform(curl);
        long status = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (code != CURLE_OK) {
            error = std::string("Could not reach Claude: ") + curl_easy_strerror(code);
            continue;
        }
        json parsed = json::parse(response, nullptr, false);
        if (status == 200 && !parsed.is_discarded()) {
            return parsed;
        }
        error = "Claude API error " + std::to_string(status);
        if (!parsed.is_discarded() && parsed.contains("error")) {
            error += ": " + parsed["error"].value("message", "");
        }
        if (status != 429 && status != 529 && status < 500) {
            return nullptr;  // not worth retrying
        }
    }
    return nullptr;
}

// ----------------------------------------------------------------- tools ----

json popularItems(const OrderLog& log, int limit)
{
    const json report = menuReport(log);
    json items = report["items"];
    std::sort(items.begin(), items.end(), [](const json& a, const json& b) { return a["units"] > b["units"]; });
    json popular = json::array();
    for (const json& item : items) {
        if (static_cast<int>(popular.size()) >= limit || item["units"] == 0) {
            break;
        }
        popular.push_back({{"id", item["id"]}, {"name", item["name"]}, {"price", item["price"]},
                           {"times_ordered", item["units"]}, {"featured", item["quadrant"] == "Star"}});
    }
    return popular;
}

static json itemCards(const std::vector<std::string>& ids)
{
    json items = json::array();
    for (const std::string& id : ids) {
        ItemRef ref;
        if (findItem(id, ref)) {
            const Item& item = itemAt(ref);
            items.push_back({{"id", item.id}, {"name", item.name}, {"description", item.description},
                             {"price", money(item.cents)}});
        }
    }
    return items;
}

static json tool(const std::string& name, const std::string& description, json properties,
                 std::vector<std::string> required)
{
    return {{"name", name},
            {"description", description},
            {"strict", true},
            {"input_schema", {{"type", "object"},
                              {"properties", properties.is_null() ? json::object() : properties},
                              {"required", required},
                              {"additionalProperties", false}}}};
}

static const json& toolDefinitions()
{
    static const json tools = json::array({
        tool("get_popular_items",
             "The restaurant's most-ordered dishes across all customers, best first, with how many times each "
             "was ordered and whether the restaurant features it. Empty when there is no order history yet.",
             {{"limit", {{"type", "integer"}, {"description", "How many to return, 1-10"}}}}, {"limit"}),
        tool("get_my_history",
             "This customer's past orders: how many, total spent, their last order and their favourite dishes.",
             nullptr, {}),
        tool("get_pairings",
             "Dishes other customers most often ordered together with the given dish.",
             {{"item_id", {{"type", "string"}, {"description", "Menu id, e.g. P2"}}}}, {"item_id"}),
        tool("show_items",
             "Show dishes to the customer as cards they can add with one tap. Use it whenever you recommend or "
             "list specific dishes.",
             {{"title", {{"type", "string"}, {"description", "Short heading, e.g. \"Popular right now\""}}},
              {"item_ids", {{"type", "array"}, {"items", {{"type", "string"}}}}}},
             {"title", "item_ids"}),
        tool("add_to_cart", "Add a dish to the customer's cart.",
             {{"item_id", {{"type", "string"}}}, {"quantity", {{"type", "integer"}, {"description", "1-50"}}}},
             {"item_id", "quantity"}),
        tool("remove_from_cart", "Remove some or all of a dish from the cart.",
             {{"item_id", {{"type", "string"}}},
              {"quantity", {{"type", "integer"}, {"description", "How many to remove; 0 removes the dish entirely"}}}},
             {"item_id", "quantity"}),
        tool("view_cart", "The cart's lines, subtotal, tax, total and tip options.", nullptr, {}),
        tool("get_passport",
             "The customer's Foodie Passport: stamps (one per paid order), whether their reward is ready, and "
             "which menu sections and dishes they have tried. A ready reward comes off this order automatically.",
             nullptr, {}),
        tool("list_saved_cards",
             "The customer's saved cards: brand, last 4 digits and nickname only. Full numbers are never stored.",
             nullptr, {}),
        tool("checkout",
             "Hand the order to the secure checkout, where the customer confirms the total and chooses or enters "
             "a card. Only call this once the customer says they are ready to pay and has chosen a tip.",
             {{"tip_percent", {{"type", "integer"}, {"enum", {0, 18, 20, 24}}}}}, {"tip_percent"}),
    });
    return tools;
}

static std::string systemPrompt()
{
    std::ostringstream prompt;
    prompt << "You are the ordering assistant for Pho of Fort Wayne, a Vietnamese restaurant in Fort Wayne, "
              "Indiana (a demo ordering app). You chat with one customer and place their order for them.\n\n"
              "How to help:\n"
              "- Ask what they are in the mood for, then suggest a few fitting dishes. Keep replies short and "
              "warm; plain text, no markdown.\n"
              "- When you recommend or list dishes, call show_items so they appear as cards.\n"
              "- Use get_my_history for returning customers (offer their usual), get_popular_items for what "
              "sells, and get_pairings to suggest one side or drink that goes with their main. Suggest at most "
              "one add-on per order and never pressure.\n"
              "- Only call something popular if get_popular_items says so. Never invent numbers, ratings or "
              "dishes. The menu has no allergen or spice information: say so and suggest asking staff.\n"
              "- Foodie Passport: every paid order earns a stamp; "
           << STAMPS_FOR_REWARD << " stamps take " << REWARD_PERCENT
           << "% off the food on the next order, applied automatically at checkout. Use get_passport to tell "
              "the customer where they stand, celebrate a full card, and suggest a dish from a section they "
              "haven't tried when it fits.\n"
              "- Change the cart only through add_to_cart and remove_from_cart, and only for what the customer "
              "asked for or agreed to. Confirm what you changed.\n"
              "- Before checkout, read back the order with view_cart and ask about a tip (18%, 20%, 24% or none). "
              "Then call checkout. Payment happens in the secure checkout, never in chat: never ask for or "
              "repeat a card number, expiry or security code.\n\n"
              "Prices include no tax; tax is "
           << TAX_PERCENT << "% of the subtotal, and a tip is a percentage of the subtotal.\n\nThe menu (id, name, price, description):\n";
    for (const Category& category : MENU) {
        prompt << "\n" << category.title << "\n";
        for (const Item& item : category.items) {
            prompt << "  " << item.id << "  " << item.name << "  " << money(item.cents);
            if (!item.description.empty()) {
                prompt << "  - " << item.description;
            }
            prompt << "\n";
        }
    }
    return prompt.str();
}

// ----------------------------------------------------------------- agent ----

Agent::Agent(Session& session, const ClaudeClient& client, CheckoutHandler onCheckout)
    : session_(session), client_(client), onCheckout_(std::move(onCheckout))
{
}

json Agent::greeting() const
{
    const json profile = customerProfile(session_.log, session_.customer);
    const json popular = popularItems(session_.log, 4);
    const json passport = passportJson(session_.log, session_.customer);
    json cards = json::array();
    std::string text;
    if (profile["order_count"] > 0) {
        std::vector<std::string> ids;
        std::string summary;
        for (const json& item : profile["last_order"]["items"]) {
            ids.push_back(item["id"]);
            summary += (summary.empty() ? "" : ", ") + std::to_string(item["quantity"].get<int>()) + " x " +
                       item["name"].get<std::string>();
        }
        text = "Welcome back, " + session_.customer + "! Last time you had " + summary +
               ". Want the same again, or are you in the mood for something different?";
        cards.push_back({{"title", "Your last order"}, {"items", itemCards(ids)}, {"reorder", true}});
    } else {
        text = "Welcome to Pho of Fort Wayne! Tell me what you're in the mood for - soup, something grilled, "
               "a quick sandwich - and I'll put the order together for you.";
    }
    if (!popular.empty()) {
        std::vector<std::string> ids;
        for (const json& item : popular) {
            ids.push_back(item["id"]);
        }
        cards.push_back({{"title", "Popular right now"}, {"items", itemCards(ids)}});
    }
    if (passport["reward_ready"]) {
        text += " Good news: your Foodie Passport is full, so " + std::to_string(REWARD_PERCENT) +
                "% comes off the food on this order.";
    } else if (profile["order_count"] > 0) {
        text += " Foodie Passport: " + std::to_string(passport["stamps"].get<int>()) + " of " +
                std::to_string(STAMPS_FOR_REWARD) + " stamps - " + passport["message"].get<std::string>();
    }
    return {{"reply", text}, {"cards", cards}, {"cart_changed", false}, {"error", nullptr}};
}

json Agent::runTool(const std::string& name, const json& input, json& cards, bool& cartChanged)
{
    auto itemFrom = [&](ItemRef& ref) {
        return input.contains("item_id") && input["item_id"].is_string() &&
               findItem(input["item_id"].get<std::string>(), ref);
    };
    ItemRef ref;
    if (name == "get_popular_items") {
        const int limit = std::clamp(input.value("limit", 5), 1, 10);
        const json popular = popularItems(session_.log, limit);
        return popular.empty() ? json{{"items", popular}, {"note", "No orders yet, so nothing is popular yet."}}
                               : json{{"items", popular}};
    }
    if (name == "get_my_history") {
        return customerProfile(session_.log, session_.customer);
    }
    if (name == "get_pairings") {
        if (!itemFrom(ref)) return {{"error", "No dish with that id."}};
        const json pairs = pairingsFor(session_.log, itemAt(ref).id, 3);
        return pairs.empty() ? json{{"pairs", pairs}, {"note", "Not enough orders with this dish yet."}}
                             : json{{"pairs", pairs}};
    }
    if (name == "show_items") {
        std::vector<std::string> ids;
        for (const json& id : input.value("item_ids", json::array())) {
            if (id.is_string()) ids.push_back(id);
        }
        const json items = itemCards(ids);
        if (items.empty()) return {{"error", "None of those ids are on the menu."}};
        cards.push_back({{"title", input.value("title", "Suggestions")}, {"items", items}});
        return {{"shown", items.size()}};
    }
    if (name == "add_to_cart") {
        const int quantity = input.value("quantity", 0);
        if (!itemFrom(ref)) return {{"error", "No dish with that id."}};
        if (quantity < 1 || quantity > MAX_QUANTITY) return {{"error", "Quantity must be 1-50."}};
        if (!session_.cart.add(ref, quantity)) return {{"error", "That would be more than 50 of one dish."}};
        cartChanged = true;
        return {{"added", itemAt(ref).name}, {"quantity", quantity}, {"cart", cartJson(session_.cart, discountFor(passportFor(session_.log, session_.customer)))}};
    }
    if (name == "remove_from_cart") {
        if (!itemFrom(ref)) return {{"error", "No dish with that id."}};
        if (!session_.cart.remove(ref, input.value("quantity", 0))) return {{"error", "That dish is not in the cart."}};
        cartChanged = true;
        return {{"removed", itemAt(ref).name}, {"cart", cartJson(session_.cart, discountFor(passportFor(session_.log, session_.customer)))}};
    }
    if (name == "view_cart") {
        return cartJson(session_.cart, discountFor(passportFor(session_.log, session_.customer)));
    }
    if (name == "get_passport") {
        return passportJson(session_.log, session_.customer);
    }
    if (name == "list_saved_cards") {
        return savedCardsJson(session_.wallet);
    }
    if (name == "checkout") {
        const int tip = input.value("tip_percent", -1);
        if (!validTip(tip)) return {{"error", "Tip must be 0, 18, 20 or 24."}};
        if (session_.cart.empty()) return {{"error", "The cart is empty."}};
        const std::string status = onCheckout_(tip);
        cartChanged = true;
        return {{"status", status}};
    }
    return {{"error", "Unknown tool " + name}};
}

// After a server-side fallback the API asks that thinking and tool_use blocks
// produced before the last switch point are not echoed back.
static json echoable(const json& content)
{
    int lastFallback = -1;
    for (int i = 0; i < static_cast<int>(content.size()); ++i) {
        if (content[i].value("type", "") == "fallback") lastFallback = i;
    }
    if (lastFallback < 0) return content;
    json kept = json::array();
    for (int i = 0; i < static_cast<int>(content.size()); ++i) {
        const std::string type = content[i].value("type", "");
        if (i < lastFallback && (type == "thinking" || type == "redacted_thinking" || type == "tool_use")) {
            continue;
        }
        kept.push_back(content[i]);
    }
    return kept;
}

void Agent::note(const std::string& text)
{
    messages_.push_back({{"role", "user"}, {"content", text}});
}

json Agent::turn(const std::string& customerText)
{
    json result = {{"reply", ""}, {"cards", json::array()}, {"cart_changed", false}, {"error", nullptr}};
    if (containsCardNumber(customerText)) {
        result["reply"] = "That looks like a card number, so I didn't send it. Please never type card details "
                          "in chat - you'll enter them in the secure checkout.";
        return result;
    }

    const std::size_t mark = messages_.size();
    messages_.push_back({{"role", "user"}, {"content", customerText}});
    static const std::string system = systemPrompt();
    std::string reply;
    json cards = json::array();
    bool cartChanged = false;

    auto fail = [&](const std::string& error) {
        // Roll the conversation back so the next message starts clean.
        messages_.erase(messages_.begin() + static_cast<long>(mark), messages_.end());
        result["error"] = error;
        result["cards"] = cards;
        result["cart_changed"] = cartChanged;
        return result;
    };

    for (int step = 0; step < 12; ++step) {
        const json body = {
            {"model", CLAUDE_MODEL},
            {"max_tokens", 16000},
            {"system", json::array({{{"type", "text"}, {"text", system}}})},
            {"tools", toolDefinitions()},
            {"messages", messages_},
            {"output_config", {{"effort", "low"}}},  // a chat: fast, short turns
            {"cache_control", {{"type", "ephemeral"}}},
            {"fallbacks", "default"},
        };
        std::string error;
        const json response = client_.send(body, error);
        if (response.is_null()) {
            return fail(error);
        }
        const std::string stop = response.value("stop_reason", "");
        if (stop == "refusal") {
            return fail("Sorry, I can't help with that one.");
        }
        if (stop == "max_tokens") {
            return fail("That reply ran too long. Please try again.");
        }

        const json content = response.value("content", json::array());
        for (const json& block : content) {
            if (block.value("type", "") == "text") {
                reply += (reply.empty() ? "" : "\n") + block.value("text", "");
            }
        }
        messages_.push_back({{"role", "assistant"}, {"content", echoable(content)}});
        if (stop != "tool_use") {
            result["reply"] = reply;
            result["cards"] = cards;
            result["cart_changed"] = cartChanged;
            return result;
        }

        // Every tool result goes back in one message.
        json results = json::array();
        for (const json& block : content) {
            if (block.value("type", "") != "tool_use") continue;
            const json output = runTool(block.value("name", ""), block.value("input", json::object()), cards, cartChanged);
            json entry = {{"type", "tool_result"}, {"tool_use_id", block.value("id", "")}, {"content", output.dump()}};
            if (output.is_object() && output.contains("error")) entry["is_error"] = true;
            results.push_back(entry);
        }
        messages_.push_back({{"role", "user"}, {"content", results}});
    }
    return fail("I took too many steps on that one. Could you say it another way?");
}

// --------------------------------------------------------------- insights ---

std::string writeInsights(const ClaudeClient& client, const json& report, std::string& error)
{
    const json body = {
        {"model", CLAUDE_MODEL},
        {"max_tokens", 16000},
        {"system", "You advise the owner of a small Vietnamese restaurant on menu positioning. Work only from the "
                   "report you are given; do not invent numbers. Give at most six concrete recommendations: what "
                   "to feature, move, pair into combos, reprice or drop, and what the assistant should suggest. "
                   "Say plainly when the order count is too small to rely on, and that quadrants use price, not "
                   "margin. Plain text, short paragraphs or a short list, no markdown headings."},
        {"messages", json::array({{{"role", "user"}, {"content", "Here is this period's menu report:\n" + report.dump(2)}}})},
        {"output_config", {{"effort", "medium"}}},
        {"fallbacks", "default"},
    };
    const json response = client.send(body, error);
    if (response.is_null()) return "";
    if (response.value("stop_reason", "") == "refusal") {
        error = "Claude declined to write insights for this report.";
        return "";
    }
    std::string text;
    for (const json& block : response.value("content", json::array())) {
        if (block.value("type", "") == "text") text += block.value("text", "");
    }
    return text;
}
