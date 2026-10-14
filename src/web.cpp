// The web app: a small JSON API over the same engine, plus the page in web/.
// It listens on 127.0.0.1 only — the checkout form posts card details here.

#include "agent.hpp"

#include <httplib.h>

#include <cstdlib>
#include <iostream>
#include <mutex>

namespace {

void reply(httplib::Response& res, const json& body, int status = 200)
{
    res.status = status;
    res.set_content(body.dump(), "application/json");
}

json bodyOf(const httplib::Request& req)
{
    json body = json::parse(req.body, nullptr, false);
    return body.is_object() ? body : json::object();
}

}  // namespace

int runWeb(Session& session, int port)
{
    const ClaudeClient client;
    std::mutex lock;  // one session, many requests
    json pendingCheckout = nullptr;

    // In the browser, checkout opens a panel; the customer pays there.
    Agent agent(session, client, [&](int tipPercent) -> std::string {
        pendingCheckout = {{"tip_percent", tipPercent}};
        return "Checkout is open on the customer's screen with a " + std::to_string(tipPercent) +
               "% tip selected. They confirm and pay there. Do not say the payment is complete.";
    });

    auto discount = [&]() { return discountFor(passportFor(session.log, session.customer)); };
    auto state = [&]() {
        return json{{"customer", session.customer},
                    {"cart", cartJson(session.cart, discount())},
                    {"passport", passportJson(session.log, session.customer)},
                    {"wallet", savedCardsJson(session.wallet)},
                    {"has_history", !session.log.forCustomer(session.customer).empty()}};
    };

    httplib::Server server;
    const char* webDir = std::getenv("PHO_WEB_DIR");
    if (!server.set_mount_point("/", webDir ? webDir : "web")) {
        std::cerr << "Cannot find the web/ folder. Run from the repository root or set PHO_WEB_DIR.\n";
        return 1;
    }

    server.Get("/api/state", [&](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> guard(lock);
        json body = state();
        body["menu"] = menuJson();
        body["assistant_available"] = client.configured();
        body["greeting"] = agent.greeting();
        reply(res, body);
    });

    server.Post("/api/cart/add", [&](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> guard(lock);
        const json body = bodyOf(req);
        ItemRef ref;
        const int quantity = body.value("quantity", 1);
        if (!findItem(body.value("item_id", ""), ref) || quantity < 1 || !session.cart.add(ref, quantity)) {
            return reply(res, {{"error", "Couldn't add that."}}, 400);
        }
        reply(res, state());
    });

    server.Post("/api/cart/remove", [&](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> guard(lock);
        const json body = bodyOf(req);
        ItemRef ref;
        if (!findItem(body.value("item_id", ""), ref) || !session.cart.remove(ref, body.value("quantity", 0))) {
            return reply(res, {{"error", "That dish isn't in the cart."}}, 400);
        }
        reply(res, state());
    });

    server.Post("/api/cart/reorder", [&](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> guard(lock);
        const auto mine = session.log.forCustomer(session.customer);
        if (mine.empty()) {
            return reply(res, {{"error", "No past orders yet."}}, 400);
        }
        for (const CartLine& line : mine.back().lines) {
            session.cart.add(line.ref, line.quantity);
        }
        reply(res, state());
    });

    server.Post("/api/chat", [&](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> guard(lock);
        if (!client.configured()) {
            return reply(res, {{"error", "The assistant needs ANTHROPIC_API_KEY set on the server."}}, 503);
        }
        const std::string message = bodyOf(req).value("message", "");
        if (message.empty() || message.size() > 2000) {
            return reply(res, {{"error", "Please type a message (up to 2000 characters)."}}, 400);
        }
        pendingCheckout = nullptr;
        json body = agent.turn(message);
        body["checkout"] = pendingCheckout;
        body["state"] = state();
        reply(res, body);
    });

    // Card details arrive here, are checked, and are reduced to brand + last 4.
    // Nothing in this handler logs or stores the full number or the CVV.
    server.Post("/api/pay", [&](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> guard(lock);
        const json body = bodyOf(req);
        const int tipPercent = body.value("tip_percent", -1);
        if (session.cart.empty()) return reply(res, {{"error", "Your cart is empty."}}, 400);
        if (!validTip(tipPercent)) return reply(res, {{"error", "Please choose a tip."}}, 400);

        SavedCard card;
        const int savedIndex = body.value("saved_card", 0);
        if (savedIndex > 0) {
            if (savedIndex > static_cast<int>(session.wallet.cards().size())) {
                return reply(res, {{"error", "That saved card no longer exists."}}, 400);
            }
            card = session.wallet.cards()[savedIndex - 1];
        } else {
            const json fresh = body.value("new_card", json::object());
            std::string error;
            if (!checkNewCard(fresh.value("number", ""), fresh.value("expiry", ""), fresh.value("cvv", ""), card, error)) {
                return reply(res, {{"error", error}}, 400);
            }
            card.nickname = card.brand;
            std::string nickname = fresh.value("nickname", "");
            if (nickname.size() > 20) nickname.resize(20);
            if (!nickname.empty() && !containsCardNumber(nickname)) card.nickname = nickname;
            if (fresh.value("save", false) && !session.wallet.add(card)) {
                return reply(res, {{"error", "You already have 3 saved cards. Remove one, or pay without saving."}}, 400);
            }
        }

        const Bill bill = billFor(session.cart, discount(), tipPercent);
        json receipt = {{"lines", cartJson(session.cart, 0)["lines"]},
                        {"subtotal", money(bill.subtotal)},
                        {"discount", money(bill.discount)},
                        {"discount_percent", bill.discountPercent},
                        {"tax", money(bill.tax)},
                        {"tip", money(bill.tip)},
                        {"total", money(bill.total)},
                        {"card", card.brand + " ending " + card.last4}};
        session.log.record(session.customer, session.cart, bill);
        session.cart = Cart();
        const json passport = passportJson(session.log, session.customer);
        receipt["stamp_earned"] = bill.discount == 0;
        agent.note("[Checkout] The customer paid " + money(bill.total) + " on " + card.brand + " ending " +
                   card.last4 + ". The order is placed and the cart is empty. Passport now: " +
                   passport["message"].get<std::string>());
        reply(res, {{"receipt", receipt}, {"state", state()}});
    });

    server.Post("/api/cards/remove", [&](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> guard(lock);
        const int index = bodyOf(req).value("index", 0);
        if (index < 1 || index > static_cast<int>(session.wallet.cards().size())) {
            return reply(res, {{"error", "No such card."}}, 400);
        }
        session.wallet.remove(index - 1);
        reply(res, state());
    });

    server.Get("/api/report", [&](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> guard(lock);
        reply(res, menuReport(session.log));
    });

    server.Post("/api/insights", [&](const httplib::Request&, httplib::Response& res) {
        json report;
        {
            std::lock_guard<std::mutex> guard(lock);
            report = menuReport(session.log);
        }
        if (!client.configured()) {
            return reply(res, {{"error", "Insights need ANTHROPIC_API_KEY set on the server."}}, 503);
        }
        if (report["orders"] == 0) {
            return reply(res, {{"error", "No orders yet."}}, 400);
        }
        std::string error;
        const std::string advice = writeInsights(client, report, error);
        if (advice.empty()) return reply(res, {{"error", error}}, 502);
        reply(res, {{"insights", advice}});
    });

    std::cout << "Pho of Fort Wayne is open at http://localhost:" << port << "  (Ctrl-C to close)\n";
    if (!client.configured()) {
        std::cout << "The chat assistant is off: set ANTHROPIC_API_KEY to turn it on.\n";
    }
    if (!server.listen("127.0.0.1", port)) {
        std::cerr << "Could not listen on port " << port << ".\n";
        return 1;
    }
    return 0;
}
