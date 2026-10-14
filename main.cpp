// A console mockup of online ordering for Pho of Fort Wayne.

#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Item {
    std::string name;
    std::string description;
    long cents;  // money is kept in whole cents so totals are exact
};

struct Category {
    std::string title;
    std::vector<Item> items;
};

// The whole menu. Add a category or an item here and every screen picks it up.
const std::vector<Category> MENU = {
    {"Appetizers", {
        {"Vietnamese Egg Rolls (2)",    "Ground pork, shrimp, carrots and taro, with lettuce and a sweet, spicy sauce", 299},
        {"Vietnamese Spring Rolls (2)", "Pork, shrimp, rice noodles and mint wrapped in rice paper, with peanut sauce", 499},
        {"Nem Nuong Cuon (2)",          "Grilled pork sausage spring rolls", 499},
        {"Bo Nuong",                    "Marinated, grilled short rib, with lettuce and cilantro", 799},
        {"Muc Chien",                   "Lightly battered calamari stir-fried with garlic and chili, with lettuce and cilantro", 899},
        {"Hen Nuong",                   "Marinated mussels grilled with green onions", 999},
        {"Chim Cut Quay",               "Slow-roasted quail", 999},
        {"Canh Ga Nuoc Mam (6)",        "Chicken wings stir-fried with fish sauce", 699},
        {"Sesame Chicken Wings (6)",    "Fried chicken wings in sesame sauce", 699},
        {"House Chicken Wings (6)",     "Spicy fried wings stir-fried in our garlic seasoning", 699},
    }},
    {"Pho - Rice Noodle Soup", {
        {"Pho Dac Biet",     "Flank steak, brisket, tendon and meatballs", 999},
        {"Pho Tai",          "Sliced flank steak", 899},
        {"Pho Ga",           "Seasoned chicken breast", 899},
        {"Hu Tieu Dac Biet", "Shrimp, crab meat, scallops and fish balls", 999},
        {"Hu Tieu Ga",       "Sliced chicken breast", 899},
    }},
    {"Mi - Egg Noodles", {
        {"Mi Dac Biet",     "Egg noodle soup with shrimp, crab meat and pork", 999},
        {"Mi Hai San",      "Egg noodle soup with shrimp, crab meat, scallops and fish balls", 1099},
        {"Mi Ga",           "Egg noodle soup with sliced chicken breast", 899},
        {"Mi Kho Dac Biet", "Dry egg noodles with shrimp, crab meat, pork and meatballs, broth on the side", 999},
        {"Mi Xao",          "Stir-fried egg noodles with seafood and vegetables", 1499},
    }},
    {"Banh Mi - Vietnamese Sandwiches", {
        {"Banh Mi Dac Biet", "BBQ pork, Vietnamese ham and head cheese", 500},
        {"Banh Mi Ga",       "Chicken with pickled vegetables", 500},
        {"Banh Mi Bo",       "Beef with pickled vegetables", 500},
        {"Banh Mi Bi",       "Shredded pork with pickled vegetables", 500},
    }},
    {"Drinks", {
        {"Soy Bean Milk",          "", 300},
        {"Coconut Juice",          "", 300},
        {"Fresh Lemonade",         "", 450},
        {"Vietnamese Iced Coffee", "", 450},
        {"Lime Juice Lipton",      "", 499},
        {"Bubble Tea",             "Milk tea, Thai tea, taro, coconut or mocha", 499},
        {"Smoothie",               "Strawberry, mango, lychee, mocha or pina colada", 450},
        {"Soft Drink",             "Coke, Diet Coke, Sprite or root beer", 199},
        {"Beer",                   "Heineken, Corona, Budweiser, Bud Light or Miller Lite", 310},
    }},
};

constexpr int TAX_PERCENT  = 6;
constexpr int TIP_CHOICES[] = {18, 20, 24};
constexpr int MAX_QUANTITY = 50;

struct CartLine {
    int category;
    int item;
    int quantity;
};

const Item& itemFor(const CartLine& line)
{
    return MENU[line.category].items[line.item];
}

std::string money(long cents)
{
    std::ostringstream out;
    out << '$' << cents / 100 << '.' << std::setw(2) << std::setfill('0') << cents % 100;
    return out.str();
}

// Rounds to the nearest cent.
long percentOf(long cents, int percent)
{
    return (cents * percent + 50) / 100;
}

// Reads a whole line and accepts it only if it is an integer in [low, high].
// Returns false at end of input, so the caller can stop instead of looping.
bool readInt(const std::string& prompt, int low, int high, int& value)
{
    std::string line;
    while (true) {
        std::cout << prompt;
        if (!std::getline(std::cin, line)) {
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

int cartCount(const std::vector<CartLine>& cart)
{
    int count = 0;
    for (const CartLine& line : cart) {
        count += line.quantity;
    }
    return count;
}

long subtotalOf(const std::vector<CartLine>& cart)
{
    long cents = 0;
    for (const CartLine& line : cart) {
        cents += itemFor(line).cents * line.quantity;
    }
    return cents;
}

// Adding an item that is already in the cart raises its quantity.
void addToCart(std::vector<CartLine>& cart, int category, int item, int quantity)
{
    for (CartLine& line : cart) {
        if (line.category == category && line.item == item) {
            line.quantity += quantity;
            return;
        }
    }
    cart.push_back({category, item, quantity});
}

// Returns false at end of input.
bool browseCategory(int category, std::vector<CartLine>& cart)
{
    const Category& section = MENU[category];
    const int count = static_cast<int>(section.items.size());
    while (true) {
        std::cout << "\n  " << section.title << "\n\n";
        for (int i = 0; i < count; ++i) {
            const Item& item = section.items[i];
            std::cout << std::setw(4) << std::right << i + 1 << ". " << std::setw(30) << std::left
                      << item.name << std::setw(8) << std::right << money(item.cents) << "\n";
            if (!item.description.empty()) {
                std::cout << "        " << item.description << "\n";
            }
        }
        std::cout << "\n     0. Back to the main menu\n\n";

        int choice;
        if (!readInt("Add which item? ", 0, count, choice)) {
            return false;
        }
        if (choice == 0) {
            return true;
        }
        const Item& item = section.items[choice - 1];
        int quantity;
        if (!readInt("How many " + item.name + "? ", 1, MAX_QUANTITY, quantity)) {
            return false;
        }
        addToCart(cart, category, choice - 1, quantity);
        std::cout << "Added " << quantity << " x " << item.name << ". Cart: "
                  << cartCount(cart) << " item(s), " << money(subtotalOf(cart)) << "\n";
    }
}

void printCart(const std::vector<CartLine>& cart)
{
    std::cout << "\n  Your order\n\n";
    for (std::size_t i = 0; i < cart.size(); ++i) {
        const Item& item = itemFor(cart[i]);
        std::cout << std::setw(4) << std::right << i + 1 << ". " << std::setw(30) << std::left << item.name
                  << std::setw(4) << std::right << cart[i].quantity << " x " << std::setw(7)
                  << money(item.cents) << std::setw(10) << money(item.cents * cart[i].quantity) << "\n";
    }
    const long subtotal = subtotalOf(cart);
    const long tax      = percentOf(subtotal, TAX_PERCENT);
    std::cout << "\n" << std::setw(51) << std::right << "Subtotal " << std::setw(10) << money(subtotal)
              << "\n" << std::setw(51) << ("Tax (" + std::to_string(TAX_PERCENT) + "%) ")
              << std::setw(10) << money(tax)
              << "\n" << std::setw(51) << "Total " << std::setw(10) << money(subtotal + tax) << "\n";
}

// Returns false at end of input.
bool removeFromCart(std::vector<CartLine>& cart)
{
    int line;
    if (!readInt("Remove which line? (0 to cancel) ", 0, static_cast<int>(cart.size()), line)) {
        return false;
    }
    if (line > 0) {
        std::cout << "Removed " << itemFor(cart[line - 1]).name << ".\n";
        cart.erase(cart.begin() + (line - 1));
    }
    return true;
}

// Tip is calculated on the pre-tax subtotal. Returns false at end of input.
bool pay(const std::vector<CartLine>& cart)
{
    const long subtotal = subtotalOf(cart);
    const long total    = subtotal + percentOf(subtotal, TAX_PERCENT);
    const int tipCount  = static_cast<int>(std::size(TIP_CHOICES));

    std::cout << "\n  Add a tip?\n\n";
    for (int i = 0; i < tipCount; ++i) {
        const long tip = percentOf(subtotal, TIP_CHOICES[i]);
        std::cout << "    " << i + 1 << ". " << TIP_CHOICES[i] << "%  " << money(tip)
                  << "  (total " << money(total + tip) << ")\n";
    }
    std::cout << "    0. No tip   (total " << money(total) << ")\n\n";

    int choice;
    if (!readInt("Choose a tip: ", 0, tipCount, choice)) {
        return false;
    }
    const long tip = choice == 0 ? 0 : percentOf(subtotal, TIP_CHOICES[choice - 1]);
    std::cout << "\nThank you! Your payment of " << money(total + tip) << " was accepted.\n";
    return true;
}

// Returns true once the order is paid, false to go back (or at end of input).
bool checkout(std::vector<CartLine>& cart, bool& inputEnded)
{
    while (true) {
        if (cart.empty()) {
            std::cout << "\nYour cart is empty.\n";
            return false;
        }
        printCart(cart);
        std::cout << "\n  1. Pay\n  2. Remove an item\n  0. Back to the main menu\n\n";
        int choice;
        if (!readInt("Choose: ", 0, 2, choice)) {
            inputEnded = true;
            return false;
        }
        if (choice == 0) {
            return false;
        }
        if (choice == 2) {
            if (!removeFromCart(cart)) {
                inputEnded = true;
                return false;
            }
            continue;
        }
        if (!pay(cart)) {
            inputEnded = true;
            return false;
        }
        return true;
    }
}

void printMainMenu(const std::vector<CartLine>& cart)
{
    const int count = static_cast<int>(MENU.size());
    std::cout << "\n  Main menu";
    if (!cart.empty()) {
        std::cout << "  (cart: " << cartCount(cart) << " item(s), " << money(subtotalOf(cart)) << ")";
    }
    std::cout << "\n\n";
    for (int i = 0; i < count; ++i) {
        std::cout << "    " << i + 1 << ". " << MENU[i].title << "\n";
    }
    std::cout << "    " << count + 1 << ". Review order and check out\n"
              << "    0. Leave without ordering\n\n";
}

}  // namespace

int main()
{
    std::cout << "\n  Welcome to Pho of Fort Wayne!\n"
              << "  Serving authentic Vietnamese food in Fort Wayne, Indiana since 2009\n";

    const int checkoutChoice = static_cast<int>(MENU.size()) + 1;
    std::vector<CartLine> cart;
    while (true) {
        printMainMenu(cart);
        int choice;
        if (!readInt("Choose: ", 0, checkoutChoice, choice) || choice == 0) {
            break;
        }
        if (choice == checkoutChoice) {
            bool inputEnded = false;
            if (checkout(cart, inputEnded)) {
                return 0;
            }
            if (inputEnded) {
                break;
            }
        } else if (!browseCategory(choice - 1, cart)) {
            break;
        }
    }
    std::cout << "\nGoodbye!\n";
    return 0;
}
