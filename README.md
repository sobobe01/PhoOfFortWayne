# Pho of Fort Wayne

A C++ mockup of ordering at a Vietnamese noodle house — now with an AI ordering
assistant, a Foodie Passport, saved cards, and a menu report for the owner.

| Run | What you get |
|---|---|
| `./pho` | The classic numbered menu in the terminal |
| `./pho --agent` | Chat with the ordering assistant in the terminal |
| `./pho --web` | The web app at http://localhost:8080 — chat, menu, passport, restaurant report |
| `./pho --report` | The owner's menu report; add `--insights` for AI recommendations |

The assistant and insights need an Anthropic API key in `ANTHROPIC_API_KEY`.
Everything else works without one.

## Build

```bash
brew install nlohmann-json cpp-httplib
make
```

libcurl ships with macOS. On Linux install `libcurl4-openssl-dev`, `nlohmann-json3-dev`
and cpp-httplib's single header.

## What it does

**Chat to order.** Tell the assistant what you're in the mood for. It suggests
dishes as cards you can add with one tap, places the order for you, offers your
usual if you've been before, and suggests one side or drink that other customers
pair with your main. It uses Claude (`claude-opus-5-5`) and acts only through
tools that call the C++ code — it cannot change your cart in any way the menu
can't, and it cannot invent dishes or prices.

**Foodie Passport.** Every paid order earns a stamp. Four stamps take 10% off the
food on the next order, applied automatically, and the card starts over. The
passport also tracks which dishes and sections you've tried. Both numbers are
constants in `src/pho.hpp` (`STAMPS_FOR_REWARD`, `REWARD_PERCENT`).

**Saved cards (up to 3).** A card is checked (Luhn check, expiry, security code
length) and then reduced to what a wallet shows: brand, last 4 digits, expiry
and a nickname. The full number and the security code are never written to disk,
and the wallet file is readable only by you. Card details are typed at the
program's own checkout — never in the chat, and a message that looks like a card
number is not sent to the AI. A real app would store a token from a payment
processor instead. **This is a demo: no money moves.**

**Menu report for the owner.** From the order history: spend per item, share of
revenue, which dishes are ordered together, and each dish's menu-engineering
position (Star, Plowhorse, Puzzle, Dog) with a positioning recommendation —
feature it, pair it, move it up, or replace it. The classic method ranks by
profit margin; the menu has no food costs, so this uses price against the
section average and says so. Under 30 orders, the report flags its numbers as an
early signal.

## Files

| Path | What |
|---|---|
| `src/core.cpp` | The menu, the cart, the bill (discount, tax, tip), console input |
| `src/wallet.cpp` | Saved cards and card checks |
| `src/payment.cpp` | Console checkout |
| `src/history.cpp` | Order log, passport, customer profile, pairings, menu report |
| `src/agent.cpp` | Claude client (raw HTTPS via libcurl) and the assistant's tool loop |
| `src/classic.cpp`, `src/console_agent.cpp`, `src/web.cpp` | The front ends |
| `web/` | The web page (plain HTML, CSS and JavaScript) |

Data lives in `~/.pho_wallet` and `~/.pho_orders`; override with `PHO_WALLET`,
`PHO_ORDERS`, and the customer name with `PHO_CUSTOMER`.

## Money

Prices are kept in whole cents. The passport discount comes off the food subtotal
before tax; tax is 6% of what remains; the tip is a percentage of the full,
undiscounted subtotal. (Indiana's sales tax is 7% — change `TAX_PERCENT`.)

## Test

```bash
make test
```

Runs scripted sessions (ordering, bad input, card checks, saving a card, the
three-card limit, reordering, earning and redeeming the passport reward, the
report) and checks that no card number or security code reached the disk.
