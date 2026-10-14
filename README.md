# Pho of Fort Wayne

A C++ mockup of ordering at a Vietnamese noodle house. It started in 2020 as a
console menu; it is now a small ordering system with an AI assistant you chat
with, a Foodie Passport loyalty card, saved cards, a menu report for the owner,
and a web app — all running on your own computer.

> **Demo only.** No money moves and nothing is sent to the restaurant. Card
> checks are real; payments are not.

## Quick start

```bash
brew install nlohmann-json cpp-httplib     # two header-only libraries
make                                       # builds ./pho
export ANTHROPIC_API_KEY=sk-ant-...        # only for the chat assistant and insights
./pho --web                                # open http://localhost:8080
```

| Command | What you get | Needs the API key? |
|---|---|---|
| `./pho` | The classic numbered menu in the terminal | No |
| `./pho --agent` | Chat with the ordering assistant in the terminal | **Yes** |
| `./pho --web [port]` | The web app: chat, menu, cart, checkout, passport, restaurant report | Only for the chat and the "Ask for recommendations" button |
| `./pho --report` | The owner's menu report in the terminal | No |
| `./pho --report --insights` | The report plus AI-written positioning advice | **Yes** |

## The Anthropic API key

The chat assistant and the owner's insights are powered by Claude
(`claude-opus-5-5`), Anthropic's model. To use them you need your own API key.

1. Create an account at [console.anthropic.com](https://console.anthropic.com),
   add billing, and create a key under **API Keys**. It starts with `sk-ant-`.
2. Make it available to the program in the same terminal you run it from:

   ```bash
   export ANTHROPIC_API_KEY=sk-ant-...
   ./pho --web
   ```

   To keep it across terminals, add the `export` line to `~/.zshrc`.
3. Treat the key like a password: never commit it, paste it into chat, or put it
   in the web page. The program reads it on your computer and sends it only to
   Anthropic's API.

**Without a key** everything else still works: the classic menu, the web app's
menu and cart, checkout, saved cards, the passport and the report. The web app
says the assistant is off and opens the menu instead.

**Cost.** Anthropic bills API use per token. A chat turn sends the menu and the
conversation so far; the request asks for low reasoning effort and uses prompt
caching to keep turns cheap. Check current prices on Anthropic's pricing page.

Optional: `ANTHROPIC_BASE_URL` points the program at a different API address
(for example a test server).

## What changed

**The 2020 program** printed a menu, took orders from five copy-pasted sections
and "accepted payment". It had bugs: the tip was multiplied by the tax rate (an
18% tip on $10 came to $10.11) and tax vanished once a tip was chosen; checking
out twice double-counted the total; the program could never be quit; typing a
letter looped forever; and a negative quantity became about 4 billion dishes.

**The rework (PR #1)** fixed all of those: one menu table instead of five copies,
a real cart that merges repeat dishes and can remove a line, money kept in whole
cents, correct tax and tip, input that re-prompts instead of looping, and tests.

**This version (PR #2)** adds:

- **Chat ordering** — tell the assistant what you're craving; it orders for you.
- **Foodie Passport** — a stamp per order; four stamps take 10% off the next one.
- **Saved cards** — up to three, stored safely.
- **Order history** — "order my last order again", favourites, your usual.
- **A menu report for the owner** — spend per dish, pairings, menu positioning.
- **A web app** — the whole thing in a browser, in mint and white.

## How it works

### The ordering assistant

```
 you ──"two pho tai and a drink"──▶ the assistant (Claude)
                                        │  decides which tools to call
                                        ▼
                   add_to_cart · show_items · get_popular_items · get_pairings
                   get_my_history · get_passport · view_cart · checkout ...
                                        │  each tool is C++ code in this program
                                        ▼
                   the real cart, menu, order history and passport
                                        │  results go back to the assistant
                                        ▼
 you ◀──"Done! Two Pho Tai and an iced coffee are in your bowl."
```

The assistant never touches the cart directly. It asks for a tool, the program
runs that tool's C++ code, and the result goes back — so it can only do what the
menu and cart allow, and it can't invent a dish or a price. The menu, tax rate and
passport rules are in its instructions; popularity and pairings come from your
real order history, and it is told never to call something popular unless the
data says so.

When you say you're ready to pay, it reads back the order, asks about a tip and
calls `checkout`. That hands over to the program's own checkout — a prompt in the
terminal, a panel in the web app — where **you** confirm and choose a card. The
assistant never sees or asks for card details; a chat message that looks like a
card number is stopped before it is sent.

Technically: there is no official C++ library for Claude, so `src/agent.cpp`
calls the Messages API over HTTPS with libcurl and runs the tool loop itself
(up to 12 steps per message). If Claude declines a request, the API's built-in
fallback retries it on another model.

### Foodie Passport

Every paid order earns a stamp. With **4 stamps** the next order is **10% off its
food** (before tax), applied automatically, and the card starts over. The order
that uses the reward earns no stamp. The passport also shows which dishes and
menu sections you've tried. Both numbers are constants in `src/pho.hpp`
(`STAMPS_FOR_REWARD`, `REWARD_PERCENT`). Nothing extra is stored: the passport is
worked out from the order history.

### Saved cards

When you type a new card it is checked — Luhn checksum, expiry not in the past,
3 or 4 digit security code — and then reduced to what a wallet shows: **brand,
last 4 digits, expiry and a nickname**. The full number and the security code are
never written anywhere. Up to **3** cards can be saved; with three saved, a new
card is used once and not kept. The wallet file is readable only by you. (A real
shop would store a token from a payment processor such as Stripe instead.)

### The owner's menu report

Built from the order history:

- **Spend per dish** — units sold, revenue and share of revenue.
- **Ordered together** — which dishes appear in the same orders.
- **Menu position** — each dish sorted into the classic menu-engineering
  quadrants, with advice:

  | Position | Means | Advice |
  |---|---|---|
  | Star | Popular, higher-priced | Feature it at the top of its section |
  | Plowhorse | Popular, lower-priced | Pair it with a higher-priced side; review its price |
  | Puzzle | Not chosen, higher-priced | Move it up and sharpen its description |
  | Dog | Not chosen, lower-priced | Replace it or fold it into a combo |

  The classic method uses profit margin; the menu has no food costs, so this
  uses price against the section's average — and the report says so. Below 30
  orders it warns that the numbers are an early signal.

`--insights` (or **Ask for recommendations** in the web app) sends the report to
Claude for a short, plain-language set of recommendations.

### Money

Prices are kept in whole cents so totals are exact. The passport discount comes
off the food subtotal; tax is **6%** of what remains; the tip (18%, 20%, 24% or
none) is a percentage of the full, undiscounted subtotal. Indiana's sales tax is
7% — change `TAX_PERCENT` in `src/pho.hpp` if you want it realistic.

## Where things are kept

| File | Holds | Override with |
|---|---|---|
| `~/.pho_orders` | Every paid order (dishes, totals, time, customer name) | `PHO_ORDERS` |
| `~/.pho_wallet` | Saved cards: nickname, brand, last 4, expiry | `PHO_WALLET` |

The customer name is your computer's user name; set `PHO_CUSTOMER` to order as
someone else.

## Code map

| Path | What |
|---|---|
| `src/pho.hpp` | Shared declarations and every tunable constant |
| `src/core.cpp` | The menu, the cart, the bill, console input |
| `src/wallet.cpp` | Saved cards and card checks |
| `src/payment.cpp` | Terminal checkout |
| `src/history.cpp` | Order log, passport, your profile, pairings, the menu report |
| `src/agent.cpp` | The Claude client and the assistant's tool loop |
| `src/classic.cpp` · `src/console_agent.cpp` · `src/web.cpp` | The three front ends |
| `web/` | The web page — plain HTML, CSS and JavaScript |
| `tests/` | Scripted sessions and the test runner |

## Build details

macOS: libcurl ships with the system; `brew install nlohmann-json cpp-httplib`.
Linux: install `libcurl4-openssl-dev` and `nlohmann-json3-dev`, and put
cpp-httplib's single `httplib.h` on the include path. Then `make`.

## Test

```bash
make test
```

Runs scripted sessions — ordering, bad input, card checks, saving a card, the
three-card limit, reordering, earning and redeeming the passport reward, managing
cards and the report — and checks that no card number or security code reached
the disk and that the wallet file is private. The tests do not need an API key.
