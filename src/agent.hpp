// The ordering assistant: Claude, reached over HTTPS, acting only through
// tools that call the menu, cart, wallet and order-history code.
#pragma once

#include "analytics.hpp"

#include <functional>

// Raw HTTP client for the Claude Messages API (there is no official C++ SDK).
// Reads ANTHROPIC_API_KEY, and ANTHROPIC_BASE_URL if set.
class ClaudeClient {
public:
    ClaudeClient();
    bool configured() const { return !apiKey_.empty(); }
    // Returns the parsed response, or null with `error` set.
    json send(const json& body, std::string& error) const;

private:
    std::string apiKey_;
    std::string baseUrl_;
};

constexpr const char* CLAUDE_MODEL = "claude-opus-5-5";

// Called when the assistant asks to check out. The front end takes over —
// shows the total, lets the customer confirm and pick or enter a card — and
// returns a short status the assistant reads back ("paid ...", "cancelled",
// "opened on screen").
using CheckoutHandler = std::function<std::string(int tipPercent)>;

class Agent {
public:
    Agent(Session& session, const ClaudeClient& client, CheckoutHandler onCheckout);

    // A greeting built from the customer's history and what sells — no API call.
    json greeting() const;

    // One customer message in, the assistant's reply out:
    // {"reply": text, "cards": [{title, items:[...]}], "cart_changed": bool, "error": text|null}
    json turn(const std::string& customerText);

    // Tells the assistant about something that happened outside the chat
    // (e.g. the customer paid in the checkout panel).
    void note(const std::string& text);

private:
    json runTool(const std::string& name, const json& input, json& cards, bool& cartChanged);

    Session& session_;
    const ClaudeClient& client_;
    CheckoutHandler onCheckout_;
    json messages_ = json::array();
};

// The most-ordered dishes, best first. Empty when nothing has been ordered.
json popularItems(const OrderLog& log, int limit);

// Asks Claude to turn the menu report into advice for the owner.
std::string writeInsights(const ClaudeClient& client, const json& report, std::string& error);
