#!/bin/sh
# Runs each scripted session in tests/cases against ./pho and diffs the output.
# Cases 1-3 share one wallet and order log, so they also test that saved cards
# and "order again" survive between runs. Every run uses fresh temp files.
set -u
cd "$(dirname "$0")/.."
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
export PHO_WALLET="$work/wallet" PHO_ORDERS="$work/orders" PHO_CUSTOMER=tester
failed=0

check() {  # name, then the command's input file
    name=$1
    if ! ./pho < "tests/cases/$name.in" | diff -u "tests/cases/$name.expected" - > "$work/diff"; then
        echo "FAIL $name"; cat "$work/diff"; failed=1
    fi
}

check 1-first-order-saves-card
check 2-reorder-with-saved-card
check 3-new-card-not-saved
check 8-fourth-stamp
check 9-reward-redeemed

# Card numbers and security codes must never reach the disk.
if grep -qE '4242424242424242|5555555555554444|\b123\b|\b321\b' "$PHO_WALLET" "$PHO_ORDERS"; then
    echo "FAIL a card number or CVV was written to disk"; failed=1
fi
if [ "$(stat -f %Lp "$PHO_WALLET" 2>/dev/null || stat -c %a "$PHO_WALLET")" != 600 ]; then
    echo "FAIL the wallet file is readable by others"; failed=1
fi

./pho --report | diff -u tests/cases/7-report.expected - > "$work/diff" || { echo "FAIL 7-report"; cat "$work/diff"; failed=1; }

export PHO_WALLET="$work/wallet2" PHO_ORDERS="$work/orders2"
check 4-leave-at-end-of-input
cp tests/fixtures/three-cards.wallet "$PHO_WALLET"
check 5-wallet-full
check 6-manage-cards

[ $failed = 0 ] && echo "tests passed"
exit $failed
