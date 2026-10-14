// Saved cards (at most three) and the checks run on a newly typed card.

#include "pho.hpp"

#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>

std::string describe(const SavedCard& card)
{
    std::ostringstream out;
    out << card.brand << " ending " << card.last4 << " (" << card.nickname << "), expires "
        << (card.expMonth < 10 ? "0" : "") << card.expMonth << "/" << card.expYear % 100;
    return out.str();
}

std::string Wallet::defaultPath()
{
    if (const char* path = std::getenv("PHO_WALLET")) {
        return path;
    }
    const char* home = std::getenv("HOME");
    return std::string(home ? home : ".") + "/.pho_wallet";
}

Wallet::Wallet(std::string path) : path_(std::move(path))
{
    load();
}

bool Wallet::add(const SavedCard& card)
{
    if (full()) {
        return false;
    }
    cards_.push_back(card);
    save();
    return true;
}

void Wallet::remove(int index)
{
    cards_.erase(cards_.begin() + index);
    save();
}

// One card per line: nickname, brand, last4, month, year — tab-separated.
void Wallet::load()
{
    std::ifstream in(path_);
    std::string line;
    while (std::getline(in, line) && !full()) {
        std::istringstream fields(line);
        SavedCard card;
        std::string month, year;
        if (std::getline(fields, card.nickname, '\t') && std::getline(fields, card.brand, '\t') &&
            std::getline(fields, card.last4, '\t') && std::getline(fields, month, '\t') &&
            std::getline(fields, year, '\t')) {
            card.expMonth = std::atoi(month.c_str());
            card.expYear  = std::atoi(year.c_str());
            cards_.push_back(card);
        }
    }
}

void Wallet::save() const
{
    {
        std::ofstream out(path_, std::ios::trunc);
        for (const SavedCard& card : cards_) {
            out << card.nickname << '\t' << card.brand << '\t' << card.last4 << '\t'
                << card.expMonth << '\t' << card.expYear << '\n';
        }
    }
    // Readable by this user only.
    std::error_code ignored;
    std::filesystem::permissions(path_,
                                 std::filesystem::perms::owner_read | std::filesystem::perms::owner_write,
                                 std::filesystem::perm_options::replace, ignored);
}

std::string digitsOnly(const std::string& text)
{
    std::string digits;
    for (char c : text) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            digits += c;
        }
    }
    return digits;
}

bool passesLuhn(const std::string& digits)
{
    if (digits.size() < 13 || digits.size() > 19) {
        return false;
    }
    int sum = 0;
    bool doubleIt = false;
    for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
        int d = *it - '0';
        if (doubleIt) {
            d *= 2;
            if (d > 9) {
                d -= 9;
            }
        }
        sum += d;
        doubleIt = !doubleIt;
    }
    return sum % 10 == 0;
}

std::string cardBrand(const std::string& digits)
{
    auto starts = [&](const std::string& prefix) { return digits.rfind(prefix, 0) == 0; };
    const int two  = digits.size() >= 2 ? std::stoi(digits.substr(0, 2)) : 0;
    const int four = digits.size() >= 4 ? std::stoi(digits.substr(0, 4)) : 0;
    if (starts("4")) return "Visa";
    if ((two >= 51 && two <= 55) || (four >= 2221 && four <= 2720)) return "Mastercard";
    if (starts("34") || starts("37")) return "Amex";
    if (starts("6011") || starts("65")) return "Discover";
    return "Card";
}

bool containsCardNumber(const std::string& text)
{
    std::string run;
    auto check = [&]() { return passesLuhn(run); };
    for (char c : text) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            run += c;
        } else if (c == ' ' || c == '-') {
            continue;
        } else {
            if (check()) return true;
            run.clear();
        }
        if (run.size() > 19) {
            run.erase(0, 1);
        }
        if (check()) return true;
    }
    return check();
}
