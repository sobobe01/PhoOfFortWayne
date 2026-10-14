# Pho of Fort Wayne

A C++ console mockup of online ordering for a Pho restaurant: browse the menu,
build a cart, review it, remove items, add a tip and pay.

## Build and run

```bash
make run
```

Or without `make`:

```bash
c++ -std=c++17 -Wall -Wextra -o pho main.cpp && ./pho
```

## How it is put together

- **`MENU`** — one table of categories and items. Every screen (main menu,
  category lists, valid choices) is built from it, so adding a dish is one line.
- **Cart** — a list of `{category, item, quantity}` lines. Ordering the same dish
  twice raises its quantity; the checkout screen can remove a line.
- **Money** — kept in whole cents and rounded to the nearest cent, so totals are
  exact. Tax is `TAX_PERCENT` of the subtotal; the tip is a percentage of the
  pre-tax subtotal.
- **`readInt`** — every prompt reads a whole line and accepts only a whole number
  in range. Letters, blanks and out-of-range numbers re-prompt; end of input
  (Ctrl-D) exits cleanly.

## Test

```bash
make test
```

Runs each scripted session in `tests/*.in` (including bad input) and compares the
output with the matching `.expected` file.
