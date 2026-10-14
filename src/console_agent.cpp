// The chat assistant in the terminal, and the owner's report.

#include "agent.hpp"

#include <iostream>

static void printCards(const json& cards)
{
    for (const json& card : cards) {
        std::cout << "\n  " << card["title"].get<std::string>() << "\n";
        for (const json& item : card["items"]) {
            std::cout << "    [" << item["id"].get<std::string>() << "] " << item["name"].get<std::string>()
                      << "  " << item["price"].get<std::string>() << "\n";
        }
    }
}

int runConsoleAgent(Session& session)
{
    const ClaudeClient client;
    if (!client.configured()) {
        std::cout << "The assistant needs ANTHROPIC_API_KEY set. Run ./pho for the classic menu instead.\n";
        return 1;
    }

    bool paid = false;
    // Checkout runs at the program's own prompt, so card details never reach the chat.
    Agent agent(session, client, [&](int tipPercent) -> std::string {
        printCart(session.cart, discountFor(passportFor(session.log, session.customer)));
        std::cout << "\n  -- Secure checkout --\n";
        const PaymentResult result =
            takePayment(session.cart, session.wallet, session.log, session.customer, tipPercent);
        std::cout << "  -- Back to the assistant --\n";
        if (result.outcome == PaymentOutcome::Paid) {
            paid = true;
            const std::string status = "Paid " + money(result.chargedCents) + " (tip " + money(result.tipCents) +
                                       ") on " + result.cardDescription + ".";
            session.cart = Cart();
            return status;
        }
        return "The customer cancelled checkout; the cart is unchanged.";
    });

    const json hello = agent.greeting();
    std::cout << "\n  Pho of Fort Wayne - ordering assistant (type 'quit' to leave)\n\n"
              << "Assistant: " << hello["reply"].get<std::string>() << "\n";
    printCards(hello["cards"]);

    std::string line;
    while (!paid) {
        std::cout << "\nYou: " << std::flush;
        if (!std::getline(std::cin, line) || line == "quit" || line == "exit") {
            break;
        }
        if (line.find_first_not_of(" \t") == std::string::npos) {
            continue;
        }
        const json reply = agent.turn(line);
        if (!reply["reply"].get<std::string>().empty()) {
            std::cout << "\nAssistant: " << reply["reply"].get<std::string>() << "\n";
        }
        printCards(reply["cards"]);
        if (!reply["error"].is_null()) {
            std::cout << "\n(" << reply["error"].get<std::string>() << ")\n";
        }
    }
    std::cout << "\nGoodbye!\n";
    return 0;
}

int runReport(const OrderLog& log, bool withInsights)
{
    const json report = menuReport(log);
    printReport(report);
    if (!withInsights || report["orders"] == 0) {
        return 0;
    }
    const ClaudeClient client;
    if (!client.configured()) {
        std::cout << "\n(Insights need ANTHROPIC_API_KEY set.)\n";
        return 1;
    }
    std::string error;
    const std::string advice = writeInsights(client, report, error);
    if (advice.empty()) {
        std::cout << "\n(" << error << ")\n";
        return 1;
    }
    std::cout << "\n  Recommendations\n\n" << advice << "\n";
    return 0;
}
