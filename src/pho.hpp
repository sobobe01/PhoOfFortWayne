// Shared declarations for Pho of Fort Wayne: the menu, the cart, the card
// wallet, console input, payment, and the two front ends (classic menu, agent).
#pragma once

#include <string>
#include <vector>

// ---------------------------------------------------------------- money ----

// Money is kept in whole cents so totals are exact.
std::string money(long cents);
// Rounds to the nearest cent.
long percentOf(long cents, int percent);

constexpr int TAX_PERCENT  = 6;
constexpr int TIP_CHOICES[] = {18, 20, 24};
constexpr int MAX_QUANTITY = 50;  // per dish

// ----------------------------------------------------------------- menu ----

struct Item {
    std::string id;  // stable short id, e.g. "P1" — what the agent refers to
    std::string name;
    std::string description;
    long cents;
};

struct Category {
    std::string title;
    std::vector<Item> items;
};

extern const std::vector<Category> MENU;

struct ItemRef {
    int category;
    int item;
};

// Finds an item by id (case-insensitive). Returns false if there is none.
bool findItem(const std::string& id, ItemRef& ref);
const Item& itemAt(ItemRef ref);

// ----------------------------------------------------------------- cart ----

struct CartLine {
    ItemRef ref;
    int quantity;
};

class Cart {
public:
    // Adding a dish already in the cart raises its quantity. Returns false
    // (and changes nothing) if the dish would go above MAX_QUANTITY.
    bool add(ItemRef ref, int quantity);
    // Removes `quantity` of a dish, or all of it when quantity <= 0.
    // Returns false if the dish is not in the cart.
    bool remove(ItemRef ref, int quantity);
    void removeLine(int index);

    const std::vector<CartLine>& lines() const { return lines_; }
    bool empty() const { return lines_.empty(); }
    int count() const;
    long subtotal() const;

private:
    std::vector<CartLine> lines_;
};

// What an order costs. The passport discount comes off the food subtotal
// before tax; the tip is a percentage of the full, undiscounted subtotal.
struct Bill {
    long subtotal = 0, discount = 0, tax = 0, tip = 0, total = 0;
    int discountPercent = 0, tipPercent = 0;
};
Bill billFor(const Cart& cart, int discountPercent, int tipPercent);

void printCart(const Cart& cart, int discountPercent);

// --------------------------------------------------------------- wallet ----

// A saved card holds only what a wallet shows: never the full number, never
// the CVV. A real app would keep a token from its payment processor instead.
struct SavedCard {
    std::string nickname;
    std::string brand;
    std::string last4;
    int expMonth;
    int expYear;  // four digits
};

std::string describe(const SavedCard& card);

class Wallet {
public:
    static constexpr int MAX_CARDS = 3;

    // The file is PHO_WALLET if set, otherwise ~/.pho_wallet.
    static std::string defaultPath();
    explicit Wallet(std::string path);

    const std::vector<SavedCard>& cards() const { return cards_; }
    bool full() const { return static_cast<int>(cards_.size()) >= MAX_CARDS; }
    bool add(const SavedCard& card);  // false when full
    void remove(int index);

private:
    void load();
    void save() const;

    std::string path_;
    std::vector<SavedCard> cards_;
};

// Card checks used when a customer types a new card.
std::string digitsOnly(const std::string& text);
bool passesLuhn(const std::string& digits);
std::string cardBrand(const std::string& digits);
// True if any run of 13–19 digits (spaces and dashes allowed) passes Luhn.
bool containsCardNumber(const std::string& text);

// ---------------------------------------------------------------- input ----

// Reads a whole line. Returns false at end of input.
bool readLine(const std::string& prompt, std::string& line);
// Accepts only an integer in [low, high]; re-prompts otherwise.
bool readInt(const std::string& prompt, int low, int high, int& value);

// Checks a newly typed card. On success fills `card` (last 4 only, nickname
// empty) and returns true; otherwise returns false with a customer-facing
// `error`. The full number and CVV are never stored or returned.
bool checkNewCard(const std::string& number, const std::string& expiry, const std::string& cvv,
                  SavedCard& card, std::string& error);
bool validTip(int percent);  // 0 or one of TIP_CHOICES

// -------------------------------------------------------- order history ----

struct PastOrder {
    std::string when;      // ISO-8601, UTC
    std::string customer;
    std::vector<CartLine> lines;
    long subtotal = 0, discount = 0, tax = 0, tip = 0, total = 0;
};

class OrderLog {
public:
    // The file is PHO_ORDERS if set, otherwise ~/.pho_orders.
    static std::string defaultPath();
    // The customer is PHO_CUSTOMER if set, otherwise $USER.
    static std::string defaultCustomer();
    explicit OrderLog(std::string path);

    void record(const std::string& customer, const Cart& cart, const Bill& bill);
    const std::vector<PastOrder>& all() const { return orders_; }
    std::vector<PastOrder> forCustomer(const std::string& customer) const;

private:
    std::string path_;
    std::vector<PastOrder> orders_;
};

// -------------------------------------------------------- foodie passport ----

// Every paid order earns a stamp. With STAMPS_FOR_REWARD stamps the next order
// is REWARD_PERCENT off its food, and the card starts over. The order that
// redeems the reward earns no stamp. Both numbers are the restaurant's choice.
constexpr int STAMPS_FOR_REWARD = 4;
constexpr int REWARD_PERCENT    = 10;

struct Passport {
    int stamps = 0;          // 0..STAMPS_FOR_REWARD
    bool rewardReady = false;
    int rewardsUsed = 0;
    int dishesTried = 0, dishesTotal = 0;
    int sectionsTried = 0, sectionsTotal = 0;
};

// Worked out from the order log: nothing else is stored.
Passport passportFor(const OrderLog& log, const std::string& customer);
inline int discountFor(const Passport& passport) { return passport.rewardReady ? REWARD_PERCENT : 0; }
std::string stampLine(const Passport& passport);  // e.g. "Foodie Passport: [*][*][ ][ ] 2 of 4 stamps"

// ------------------------------------------------------------- payment ----

enum class PaymentOutcome { Paid, Cancelled, InputEnded };

struct PaymentResult {
    PaymentOutcome outcome = PaymentOutcome::Cancelled;
    long chargedCents = 0;
    long tipCents = 0;
    std::string cardDescription;
};

// Console checkout: asks for a tip (unless tipPercent >= 0) and a card, then
// takes the payment and records the order. Card details are typed here, at
// the program's own prompt — never in the AI chat.
PaymentResult takePayment(const Cart& cart, Wallet& wallet, OrderLog& log,
                          const std::string& customer, int tipPercent);

// Lists saved cards and lets the customer remove one. Returns false at end of input.
bool manageCards(Wallet& wallet);

// ---------------------------------------------------------------- app -----

// Everything one customer session works on.
struct Session {
    Cart cart;
    Wallet wallet;
    OrderLog log;
    std::string customer;
};

int runClassic(Session& session);
int runConsoleAgent(Session& session);
int runReport(const OrderLog& log, bool withInsights);
int runWeb(Session& session, int port);
