// Taking a payment: tip, card choice, new-card checks, and the saved-card list.

#include "pho.hpp"

#include <cctype>
#include <ctime>
#include <iostream>
#include <iterator>

bool validTip(int percent)
{
    if (percent == 0) {
        return true;
    }
    for (int choice : TIP_CHOICES) {
        if (choice == percent) {
            return true;
        }
    }
    return false;
}

bool checkNewCard(const std::string& number, const std::string& expiry, const std::string& cvv,
                  SavedCard& card, std::string& error)
{
    const std::string digits = digitsOnly(number);
    if (!passesLuhn(digits)) {
        error = "That card number isn't valid. Please check it and try again.";
        return false;
    }

    // Expiry as MM/YY or MM/YYYY.
    const auto slash = expiry.find('/');
    const std::string month = slash == std::string::npos ? "" : digitsOnly(expiry.substr(0, slash));
    const std::string year  = slash == std::string::npos ? "" : digitsOnly(expiry.substr(slash + 1));
    if (month.empty() || month.size() > 2 || (year.size() != 2 && year.size() != 4)) {
        error = "Please enter the expiry as MM/YY.";
        return false;
    }
    const int expMonth = std::stoi(month);
    const int expYear  = year.size() == 2 ? 2000 + std::stoi(year) : std::stoi(year);
    const std::time_t now = std::time(nullptr);
    const std::tm* today  = std::localtime(&now);
    const int thisYear = today->tm_year + 1900, thisMonth = today->tm_mon + 1;
    if (expMonth < 1 || expMonth > 12) {
        error = "Please enter the expiry as MM/YY.";
        return false;
    }
    if (expYear < thisYear || (expYear == thisYear && expMonth < thisMonth) || expYear > thisYear + 20) {
        error = "That card has expired, or the expiry date isn't valid.";
        return false;
    }

    const std::string brand = cardBrand(digits);
    const std::string code  = digitsOnly(cvv);
    const std::size_t wantedLength = brand == "Amex" ? 4 : 3;
    if (code.size() != wantedLength || code.size() != cvv.size()) {
        error = "The security code should be " + std::to_string(wantedLength) + " digits.";
        return false;
    }

    card = {"", brand, digits.substr(digits.size() - 4), expMonth, expYear};
    return true;
}

// Returns false at end of input; otherwise fills `card`.
static bool askForNewCard(SavedCard& card)
{
    std::cout << "\n  New card - it is checked here, and only the brand, last 4 digits and expiry\n"
              << "  can be saved. The full number and security code are never stored.\n\n";
    while (true) {
        std::string number, expiry, cvv, error;
        if (!readLine("Card number: ", number) || !readLine("Expiry (MM/YY): ", expiry) ||
            !readLine("Security code: ", cvv)) {
            return false;
        }
        if (checkNewCard(number, expiry, cvv, card, error)) {
            return true;
        }
        std::cout << error << "\n\n";
    }
}

PaymentResult takePayment(const Cart& cart, Wallet& wallet, OrderLog& log,
                          const std::string& customer, int tipPercent)
{
    PaymentResult result;
    const Passport passport = passportFor(log, customer);
    const int discount = discountFor(passport);
    const int tipCount = static_cast<int>(std::size(TIP_CHOICES));
    if (passport.rewardReady) {
        std::cout << "\n  " << stampLine(passport) << "\n";
    }

    if (tipPercent < 0) {
        std::cout << "\n  Add a tip?\n\n";
        for (int i = 0; i < tipCount; ++i) {
            const Bill bill = billFor(cart, discount, TIP_CHOICES[i]);
            std::cout << "    " << i + 1 << ". " << TIP_CHOICES[i] << "%  " << money(bill.tip)
                      << "  (total " << money(bill.total) << ")\n";
        }
        std::cout << "    0. No tip   (total " << money(billFor(cart, discount, 0).total) << ")\n\n";
        int choice;
        if (!readInt("Choose a tip: ", 0, tipCount, choice)) {
            result.outcome = PaymentOutcome::InputEnded;
            return result;
        }
        tipPercent = choice == 0 ? 0 : TIP_CHOICES[choice - 1];
    }
    const Bill bill  = billFor(cart, discount, tipPercent);
    const long total = bill.total;

    const auto& cards    = wallet.cards();
    const int savedCount = static_cast<int>(cards.size());
    std::cout << "\n  Pay " << money(total) << " with:\n\n";
    for (int i = 0; i < savedCount; ++i) {
        std::cout << "    " << i + 1 << ". " << describe(cards[i]) << "\n";
    }
    std::cout << "    " << savedCount + 1 << ". A new card\n"
              << "    0. Cancel\n\n";
    int choice;
    if (!readInt("Choose: ", 0, savedCount + 1, choice)) {
        result.outcome = PaymentOutcome::InputEnded;
        return result;
    }
    if (choice == 0) {
        return result;
    }

    SavedCard card;
    if (choice <= savedCount) {
        card = cards[choice - 1];
    } else {
        if (!askForNewCard(card)) {
            result.outcome = PaymentOutcome::InputEnded;
            return result;
        }
        card.nickname = card.brand;
        if (wallet.full()) {
            std::cout << "You already have " << Wallet::MAX_CARDS
                      << " saved cards, so this one is used for this order only.\n";
        } else {
            int save;
            if (!readInt("Save this card for next time? (1 = yes, 0 = no) ", 0, 1, save)) {
                result.outcome = PaymentOutcome::InputEnded;
                return result;
            }
            if (save == 1) {
                std::string nickname;
                if (!readLine("Nickname for this card (Enter for \"" + card.brand + "\"): ", nickname)) {
                    result.outcome = PaymentOutcome::InputEnded;
                    return result;
                }
                std::string clean;
                for (char c : nickname) {
                    if (std::isprint(static_cast<unsigned char>(c)) && clean.size() < 20) {
                        clean += c;
                    }
                }
                if (!clean.empty() && !containsCardNumber(clean)) {
                    card.nickname = clean;
                }
                wallet.add(card);
                std::cout << "Saved " << describe(card) << ".\n";
            }
        }
    }

    log.record(customer, cart, bill);
    result.outcome         = PaymentOutcome::Paid;
    result.chargedCents    = total;
    result.tipCents        = bill.tip;
    result.cardDescription = card.brand + " ending " + card.last4;
    std::cout << "\nThank you! Your payment of " << money(total) << " on " << result.cardDescription
              << " was accepted.\n";
    if (bill.discount > 0) {
        std::cout << "Your passport reward saved you " << money(bill.discount) << ".\n";
    }
    std::cout << stampLine(passportFor(log, customer)) << "\n";
    return result;
}

bool manageCards(Wallet& wallet)
{
    while (true) {
        const auto& cards = wallet.cards();
        const int count   = static_cast<int>(cards.size());
        std::cout << "\n  Saved cards (" << count << " of " << Wallet::MAX_CARDS << ")\n\n";
        if (count == 0) {
            std::cout << "    None yet. You can save a card when you pay.\n";
            return true;
        }
        for (int i = 0; i < count; ++i) {
            std::cout << "    " << i + 1 << ". " << describe(cards[i]) << "\n";
        }
        int choice;
        if (!readInt("\nRemove which card? (0 to go back) ", 0, count, choice)) {
            return false;
        }
        if (choice == 0) {
            return true;
        }
        std::cout << "Removed " << describe(cards[choice - 1]) << ".\n";
        wallet.remove(choice - 1);
    }
}
