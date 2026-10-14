// The numbered-menu console front end.

#include "pho.hpp"

#include <iomanip>
#include <iostream>

namespace {

// Returns false at end of input.
bool browseCategory(int category, Cart& cart)
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
        if (!cart.add({category, choice - 1}, quantity)) {
            std::cout << "That would be more than " << MAX_QUANTITY << " of one dish.\n";
            continue;
        }
        std::cout << "Added " << quantity << " x " << item.name << ". Cart: " << cart.count()
                  << " item(s), " << money(cart.subtotal()) << "\n";
    }
}

// Returns false at end of input.
bool removeFromCart(Cart& cart)
{
    int line;
    if (!readInt("Remove which line? (0 to cancel) ", 0, static_cast<int>(cart.lines().size()), line)) {
        return false;
    }
    if (line > 0) {
        std::cout << "Removed " << itemAt(cart.lines()[line - 1].ref).name << ".\n";
        cart.removeLine(line - 1);
    }
    return true;
}

enum class Step { Back, Paid, InputEnded };

Step checkout(Session& session)
{
    while (true) {
        if (session.cart.empty()) {
            std::cout << "\nYour cart is empty.\n";
            return Step::Back;
        }
        printCart(session.cart, discountFor(passportFor(session.log, session.customer)));
        std::cout << "\n  1. Pay\n  2. Remove an item\n  3. Saved cards\n  0. Back to the main menu\n\n";
        int choice;
        if (!readInt("Choose: ", 0, 3, choice)) {
            return Step::InputEnded;
        }
        if (choice == 0) {
            return Step::Back;
        }
        if (choice == 2) {
            if (!removeFromCart(session.cart)) return Step::InputEnded;
            continue;
        }
        if (choice == 3) {
            if (!manageCards(session.wallet)) return Step::InputEnded;
            continue;
        }
        const PaymentResult paid =
            takePayment(session.cart, session.wallet, session.log, session.customer, -1);
        if (paid.outcome == PaymentOutcome::InputEnded) return Step::InputEnded;
        if (paid.outcome == PaymentOutcome::Paid) return Step::Paid;
    }
}

void printMainMenu(const Cart& cart, bool hasLastOrder, const Passport& passport)
{
    const int count = static_cast<int>(MENU.size());
    std::cout << "\n  " << stampLine(passport) << "\n"
              << "  Dishes tried: " << passport.dishesTried << " of " << passport.dishesTotal << "\n";
    std::cout << "\n  Main menu";
    if (!cart.empty()) {
        std::cout << "  (cart: " << cart.count() << " item(s), " << money(cart.subtotal()) << ")";
    }
    std::cout << "\n\n";
    for (int i = 0; i < count; ++i) {
        std::cout << "    " << i + 1 << ". " << MENU[i].title << "\n";
    }
    std::cout << "    " << count + 1 << ". Review order and check out\n";
    if (hasLastOrder) {
        std::cout << "    " << count + 2 << ". Order my last order again\n";
    }
    std::cout << "    0. Leave without ordering\n\n";
}

}  // namespace

int runClassic(Session& session)
{
    std::cout << "\n  Welcome to Pho of Fort Wayne!\n"
              << "  Serving authentic Vietnamese food in Fort Wayne, Indiana since 2009\n";

    const int checkoutChoice = static_cast<int>(MENU.size()) + 1;
    while (true) {
        const auto mine = session.log.forCustomer(session.customer);
        printMainMenu(session.cart, !mine.empty(), passportFor(session.log, session.customer));
        int choice;
        const int highest = checkoutChoice + (mine.empty() ? 0 : 1);
        if (!readInt("Choose: ", 0, highest, choice) || choice == 0) {
            break;
        }
        if (choice == checkoutChoice + 1) {
            for (const CartLine& line : mine.back().lines) {
                session.cart.add(line.ref, line.quantity);
            }
            std::cout << "Added your last order. Cart: " << session.cart.count() << " item(s), "
                      << money(session.cart.subtotal()) << "\n";
        } else if (choice == checkoutChoice) {
            const Step step = checkout(session);
            if (step == Step::Paid) return 0;
            if (step == Step::InputEnded) break;
        } else if (!browseCategory(choice - 1, session.cart)) {
            break;
        }
    }
    std::cout << "\nGoodbye!\n";
    return 0;
}
