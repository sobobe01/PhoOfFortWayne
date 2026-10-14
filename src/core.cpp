// The menu, the cart, money formatting and console input.

#include "pho.hpp"

#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>

// The whole menu. Add a category or an item here and every screen, and the
// agent, picks it up. Ids are a category letter plus a number.
const std::vector<Category> MENU = {
    {"Appetizers", {
        {"A1",  "Vietnamese Egg Rolls (2)",    "Ground pork, shrimp, carrots and taro, with lettuce and a sweet, spicy sauce", 299},
        {"A2",  "Vietnamese Spring Rolls (2)", "Pork, shrimp, rice noodles and mint wrapped in rice paper, with peanut sauce", 499},
        {"A3",  "Nem Nuong Cuon (2)",          "Grilled pork sausage spring rolls", 499},
        {"A4",  "Bo Nuong",                    "Marinated, grilled short rib, with lettuce and cilantro", 799},
        {"A5",  "Muc Chien",                   "Lightly battered calamari stir-fried with garlic and chili, with lettuce and cilantro", 899},
        {"A6",  "Hen Nuong",                   "Marinated mussels grilled with green onions", 999},
        {"A7",  "Chim Cut Quay",               "Slow-roasted quail", 999},
        {"A8",  "Canh Ga Nuoc Mam (6)",        "Chicken wings stir-fried with fish sauce", 699},
        {"A9",  "Sesame Chicken Wings (6)",    "Fried chicken wings in sesame sauce", 699},
        {"A10", "House Chicken Wings (6)",     "Spicy fried wings stir-fried in our garlic seasoning", 699},
    }},
    {"Pho - Rice Noodle Soup", {
        {"P1", "Pho Dac Biet",     "Flank steak, brisket, tendon and meatballs", 999},
        {"P2", "Pho Tai",          "Sliced flank steak", 899},
        {"P3", "Pho Ga",           "Seasoned chicken breast", 899},
        {"P4", "Hu Tieu Dac Biet", "Shrimp, crab meat, scallops and fish balls", 999},
        {"P5", "Hu Tieu Ga",       "Sliced chicken breast", 899},
    }},
    {"Mi - Egg Noodles", {
        {"M1", "Mi Dac Biet",     "Egg noodle soup with shrimp, crab meat and pork", 999},
        {"M2", "Mi Hai San",      "Egg noodle soup with shrimp, crab meat, scallops and fish balls", 1099},
        {"M3", "Mi Ga",           "Egg noodle soup with sliced chicken breast", 899},
        {"M4", "Mi Kho Dac Biet", "Dry egg noodles with shrimp, crab meat, pork and meatballs, broth on the side", 999},
        {"M5", "Mi Xao",          "Stir-fried egg noodles with seafood and vegetables", 1499},
    }},
    {"Banh Mi - Vietnamese Sandwiches", {
        {"B1", "Banh Mi Dac Biet", "BBQ pork, Vietnamese ham and head cheese", 500},
        {"B2", "Banh Mi Ga",       "Chicken with pickled vegetables", 500},
        {"B3", "Banh Mi Bo",       "Beef with pickled vegetables", 500},
        {"B4", "Banh Mi Bi",       "Shredded pork with pickled vegetables", 500},
    }},
    {"Drinks", {
        {"D1", "Soy Bean Milk",          "", 300},
        {"D2", "Coconut Juice",          "", 300},
        {"D3", "Fresh Lemonade",         "", 450},
        {"D4", "Vietnamese Iced Coffee", "", 450},
        {"D5", "Lime Juice Lipton",      "", 499},
        {"D6", "Bubble Tea",             "Milk tea, Thai tea, taro, coconut or mocha", 499},
        {"D7", "Smoothie",               "Strawberry, mango, lychee, mocha or pina colada", 450},
        {"D8", "Soft Drink",             "Coke, Diet Coke, Sprite or root beer", 199},
        {"D9", "Beer",                   "Heineken, Corona, Budweiser, Bud Light or Miller Lite", 310},
    }},
};

std::string money(long cents)
{
    std::ostringstream out;
    out << '$' << cents / 100 << '.' << std::setw(2) << std::setfill('0') << cents % 100;
    return out.str();
}

long percentOf(long cents, int percent)
{
    return (cents * percent + 50) / 100;
}

bool findItem(const std::string& id, ItemRef& ref)
{
    std::string wanted;
    for (char c : id) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            wanted += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
    }
    for (int c = 0; c < static_cast<int>(MENU.size()); ++c) {
        for (int i = 0; i < static_cast<int>(MENU[c].items.size()); ++i) {
            if (MENU[c].items[i].id == wanted) {
                ref = {c, i};
                return true;
            }
        }
    }
    return false;
}

const Item& itemAt(ItemRef ref)
{
    return MENU[ref.category].items[ref.item];
}

bool Cart::add(ItemRef ref, int quantity)
{
    for (CartLine& line : lines_) {
        if (line.ref.category == ref.category && line.ref.item == ref.item) {
            if (line.quantity + quantity > MAX_QUANTITY) {
                return false;
            }
            line.quantity += quantity;
            return true;
        }
    }
    if (quantity > MAX_QUANTITY) {
        return false;
    }
    lines_.push_back({ref, quantity});
    return true;
}

bool Cart::remove(ItemRef ref, int quantity)
{
    for (std::size_t i = 0; i < lines_.size(); ++i) {
        CartLine& line = lines_[i];
        if (line.ref.category == ref.category && line.ref.item == ref.item) {
            if (quantity <= 0 || quantity >= line.quantity) {
                lines_.erase(lines_.begin() + static_cast<long>(i));
            } else {
                line.quantity -= quantity;
            }
            return true;
        }
    }
    return false;
}

void Cart::removeLine(int index)
{
    lines_.erase(lines_.begin() + index);
}

int Cart::count() const
{
    int count = 0;
    for (const CartLine& line : lines_) {
        count += line.quantity;
    }
    return count;
}

long Cart::subtotal() const
{
    long cents = 0;
    for (const CartLine& line : lines_) {
        cents += itemAt(line.ref).cents * line.quantity;
    }
    return cents;
}

Bill billFor(const Cart& cart, int discountPercent, int tipPercent)
{
    Bill bill;
    bill.discountPercent = discountPercent;
    bill.tipPercent      = tipPercent;
    bill.subtotal        = cart.subtotal();
    bill.discount        = percentOf(bill.subtotal, discountPercent);
    bill.tax             = percentOf(bill.subtotal - bill.discount, TAX_PERCENT);
    bill.tip             = percentOf(bill.subtotal, tipPercent);
    bill.total           = bill.subtotal - bill.discount + bill.tax + bill.tip;
    return bill;
}

void printCart(const Cart& cart, int discountPercent)
{
    const Bill bill = billFor(cart, discountPercent, 0);
    std::cout << "\n  Your order\n\n";
    const auto& lines = cart.lines();
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const Item& item = itemAt(lines[i].ref);
        std::cout << std::setw(4) << std::right << i + 1 << ". " << std::setw(30) << std::left << item.name
                  << std::setw(4) << std::right << lines[i].quantity << " x " << std::setw(7)
                  << money(item.cents) << std::setw(10) << money(item.cents * lines[i].quantity) << "\n";
    }
    std::cout << "\n" << std::setw(51) << std::right << "Subtotal " << std::setw(10) << money(bill.subtotal);
    if (bill.discount > 0) {
        std::cout << "\n" << std::setw(51) << ("Passport reward (" + std::to_string(discountPercent) + "% off) ")
                  << std::setw(10) << ("-" + money(bill.discount));
    }
    std::cout << "\n" << std::setw(51) << ("Tax (" + std::to_string(TAX_PERCENT) + "%) ") << std::setw(10)
              << money(bill.tax) << "\n" << std::setw(51) << "Total " << std::setw(10) << money(bill.total) << "\n";
}

bool readLine(const std::string& prompt, std::string& line)
{
    std::cout << prompt << std::flush;
    return static_cast<bool>(std::getline(std::cin, line));
}

bool readInt(const std::string& prompt, int low, int high, int& value)
{
    std::string line;
    while (true) {
        if (!readLine(prompt, line)) {
            return false;
        }
        std::istringstream in(line);
        char extra;
        if (in >> value && !(in >> extra) && value >= low && value <= high) {
            return true;
        }
        std::cout << "Please enter a whole number from " << low << " to " << high << ".\n";
    }
}
