// Pho of Fort Wayne — the page. Talks to the C++ server's /api endpoints.
// Every piece of text from the server or the assistant goes in via
// textContent, never innerHTML.

const $ = (id) => document.getElementById(id);
let state = null;
let menu = [];
let assistantOn = false;
let reportLoaded = false;

// ---------------------------------------------------------------- helpers --

function el(tag, attrs = {}, ...children) {
  const node = document.createElement(tag);
  for (const [key, value] of Object.entries(attrs)) {
    if (key === "class") node.className = value;
    else if (key.startsWith("on")) node.addEventListener(key.slice(2), value);
    else if (value !== false && value != null) node.setAttribute(key, value === true ? "" : value);
  }
  for (const child of children.flat(Infinity)) {
    if (child == null || child === false) continue;
    node.append(child instanceof Node ? child : document.createTextNode(String(child)));
  }
  return node;
}

function icon(name) {
  const svg = document.createElementNS("http://www.w3.org/2000/svg", "svg");
  svg.setAttribute("aria-hidden", "true");
  const use = document.createElementNS("http://www.w3.org/2000/svg", "use");
  use.setAttribute("href", `#${name}`);
  svg.append(use);
  return svg;
}

async function api(path, body) {
  const response = await fetch(path, body === undefined ? {} : {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(body),
  });
  const data = await response.json().catch(() => ({}));
  if (!response.ok) throw new Error(data.error || `Something went wrong (${response.status}).`);
  return data;
}

function setState(next) {
  state = next;
  renderCart();
  renderPassport();
}

// ------------------------------------------------------------------- tabs --

document.querySelectorAll(".tabs button").forEach((button) => {
  button.addEventListener("click", () => {
    document.querySelectorAll(".tabs button").forEach((b) => b.setAttribute("aria-selected", b === button));
    document.querySelectorAll(".tab-panel").forEach((panel) => {
      panel.hidden = panel.id !== `tab-${button.dataset.tab}`;
    });
    if (button.dataset.tab === "restaurant") loadReport();
  });
});

document.querySelectorAll(".switch button").forEach((button) => {
  button.addEventListener("click", () => showView(button.dataset.view));
});

function showView(view) {
  document.querySelectorAll(".switch button").forEach((b) => b.setAttribute("aria-pressed", b.dataset.view === view));
  $("view-chat").hidden = view !== "chat";
  $("view-menu").hidden = view !== "menu";
}

// ------------------------------------------------------------------- menu --

function renderMenu() {
  const sections = $("menu-sections");
  const board = $("menu-board");
  sections.replaceChildren();
  board.replaceChildren();
  menu.forEach((category, index) => {
    const anchor = `section-${index}`;
    sections.append(el("button", { class: "chip", onclick: () => $(anchor).scrollIntoView({ behavior: "smooth" }) }, category.title));
    board.append(el("h3", { id: anchor }, category.title));
    for (const item of category.items) {
      board.append(el("div", { class: "menu-row" },
        el("div", {},
          el("div", { class: "name" }, item.name, el("small", {}, item.id)),
          item.description ? el("p", {}, item.description) : null),
        el("span", { class: "price" }, item.price),
        el("button", { class: "btn small", "aria-label": `Add ${item.name}`, onclick: () => addItem(item.id) }, "Add")));
    }
  });
}

async function addItem(id, quantity = 1) {
  try {
    setState(await api("/api/cart/add", { item_id: id, quantity }));
  } catch (error) {
    note(error.message);
  }
}

// ------------------------------------------------------------------- cart --

function renderCart() {
  const cart = state.cart;
  const lines = $("cart-lines");
  lines.replaceChildren();
  if (!cart.lines.length) {
    lines.append(el("p", { class: "empty" }, "Nothing yet. Tell us what you're craving, or browse the menu."));
  }
  for (const line of cart.lines) {
    lines.append(el("div", { class: "cart-line" },
      el("span", { class: "name" }, line.name),
      el("span", { class: "stepper" },
        el("button", { "aria-label": `One less ${line.name}`, onclick: () => changeLine(line.id, -1) }, "−"),
        el("span", {}, line.quantity),
        el("button", { "aria-label": `One more ${line.name}`, onclick: () => addItem(line.id) }, "+")),
      el("span", { class: "price" }, line.line_total)));
  }

  const totals = $("cart-totals");
  totals.replaceChildren(el("dt", {}, "Subtotal"), el("dd", {}, cart.subtotal));
  if (cart.discount_percent > 0 && cart.lines.length) {
    totals.append(el("dt", { class: "reward" }, `Passport reward (${cart.discount_percent}% off)`),
                  el("dd", { class: "reward" }, `−${cart.discount}`));
  }
  totals.append(el("dt", {}, `Tax (${cart.tax_percent}%)`), el("dd", {}, cart.tax),
                el("dt", { class: "grand" }, "Total"), el("dd", { class: "grand" }, cart.total));
  $("checkout-btn").disabled = !cart.lines.length;

  const passport = state.passport;
  $("mini-passport").replaceChildren(
    el("span", { class: "stamps" }, stampRow(passport, "stamp")),
    el("span", {}, passport.message));
}

async function changeLine(id, delta) {
  try {
    setState(await api("/api/cart/remove", { item_id: id, quantity: -delta }));
  } catch (error) {
    note(error.message);
  }
}

$("checkout-btn").addEventListener("click", () => openCheckout(state.cart.tip_options.length ? 18 : 0));

// ----------------------------------------------------------------- stamps --

function stampRow(passport, className, landLast = false) {
  const stamps = [];
  for (let i = 0; i < passport.stamps_for_reward; i++) {
    const on = i < passport.stamps;
    const stamp = el("span", { class: `${className}${on ? " on" : ""}${landLast && on && i === passport.stamps - 1 ? " land" : ""}` });
    if (on) stamp.append(icon("anise"));
    stamps.push(stamp);
  }
  return stamps;
}

function renderPassport() {
  const passport = state.passport;
  $("passport-owner").textContent = state.customer;
  $("stamp-grid").replaceChildren(...stampRow(passport, "stamp"));
  $("passport-message").textContent = passport.message;
  const sections = $("passport-sections");
  sections.replaceChildren(el("p", { class: "muted" }, `${passport.dishes_tried} of ${passport.dishes_total} dishes`));
  for (const section of passport.sections) {
    const percent = Math.round((100 * section.tried) / section.total);
    sections.append(el("div", { class: "section-bar" },
      el("div", { class: "label" }, el("span", {}, section.title), el("span", { class: "muted" }, `${section.tried}/${section.total}`)),
      el("div", { class: "bar" }, el("span", { style: `width:${percent}%` }))));
  }

  const saved = $("saved-cards");
  saved.replaceChildren();
  if (!state.wallet.cards.length) saved.append(el("p", { class: "empty" }, "No saved cards. You can save one at checkout."));
  for (const card of state.wallet.cards) {
    saved.append(el("div", { class: "saved" },
      el("span", {}, card.label),
      el("button", { class: "btn small", onclick: () => removeCard(card.index) }, "Remove")));
  }
}

async function removeCard(index) {
  try {
    setState(await api("/api/cards/remove", { index }));
  } catch (error) {
    alert(error.message);
  }
}

// ------------------------------------------------------------------- chat --

function addMessage(who, text) {
  const row = el("div", { class: `msg ${who}` },
    who === "me" ? null : el("span", { class: "avatar" }, icon("bowl")),
    el("div", { class: "bubble" }, text));
  $("messages").append(row);
  row.scrollIntoView({ block: "end" });
  return row;
}

function note(text) {
  addMessage("note", text);
}

function addCards(cards) {
  for (const card of cards || []) {
    const scroller = el("div", { class: "scroller" });
    for (const item of card.items) {
      scroller.append(el("div", { class: "dish" },
        el("strong", {}, item.name),
        el("p", {}, item.description || ""),
        el("div", { class: "foot" },
          el("span", { class: "price" }, item.price),
          el("button", { class: "btn small", "aria-label": `Add ${item.name}`, onclick: () => addItem(item.id) }, "Add"))));
    }
    const block = el("div", { class: "dish-cards" },
      el("div", { class: "row-between" },
        el("h4", {}, card.title),
        card.reorder ? el("button", { class: "btn small", onclick: reorder }, "Add all") : null),
      scroller);
    $("messages").append(block);
    block.scrollIntoView({ block: "end" });
  }
}

async function reorder() {
  try {
    setState(await api("/api/cart/reorder", {}));
    note("Your last order is in the bowl.");
  } catch (error) {
    note(error.message);
  }
}

function renderChips() {
  const chips = ["What's popular?", "Something spicy", "Something light", "Surprise me", "How many stamps do I have?", "I'm ready to pay"];
  if (state.has_history) chips.unshift("Same as last time");
  $("chips").replaceChildren(...chips.map((text) =>
    el("button", { class: "chip", type: "button", onclick: () => send(text) }, text)));
}

async function send(text) {
  if (!text.trim()) return;
  if (!assistantOn) {
    note("The chat assistant is off on this server. Browse the menu to order.");
    return;
  }
  addMessage("me", text);
  $("chat-input").value = "";
  const typing = addMessage("typing", "Ladling up a reply…");
  const sendButton = $("chat-form").querySelector("button");
  sendButton.disabled = true;
  try {
    const data = await api("/api/chat", { message: text });
    typing.remove();
    if (data.reply) addMessage("bot", data.reply);
    addCards(data.cards);
    if (data.error) note(data.error);
    if (data.state) setState(data.state);
    if (data.checkout) openCheckout(data.checkout.tip_percent);
  } catch (error) {
    typing.remove();
    note(error.message);
  } finally {
    sendButton.disabled = false;
    $("chat-input").focus();
  }
}

$("chat-form").addEventListener("submit", (event) => {
  event.preventDefault();
  send($("chat-input").value);
});

// --------------------------------------------------------------- checkout --

let tipPercent = 18;

function openCheckout(tip) {
  if (!state.cart.lines.length) return;
  tipPercent = tip;
  $("pay-form").hidden = false;
  $("receipt").hidden = true;
  $("pay-error").textContent = "";
  renderCheckout();
  $("checkout").showModal();
}

function renderCheckout() {
  const cart = state.cart;
  const option = cart.tip_options.find((o) => o.percent === tipPercent);
  const total = option ? option.total : cart.total;

  const summary = $("pay-summary");
  summary.replaceChildren(el("dt", {}, `${cart.item_count} item(s)`), el("dd", {}, cart.subtotal));
  if (cart.discount_percent > 0) {
    summary.append(el("dt", { class: "reward" }, `Passport reward (${cart.discount_percent}% off)`),
                   el("dd", { class: "reward" }, `−${cart.discount}`));
  }
  summary.append(el("dt", {}, "Tax"), el("dd", {}, cart.tax),
                 el("dt", {}, "Tip"), el("dd", {}, option ? option.amount : "$0.00"),
                 el("dt", { class: "grand" }, "Total"), el("dd", { class: "grand" }, total));

  const tips = $("tips");
  tips.replaceChildren();
  const choices = [{ percent: 0, amount: "No tip" }, ...cart.tip_options];
  for (const choice of choices) {
    tips.append(el("label", {},
      el("input", { type: "radio", name: "tip", value: choice.percent, checked: choice.percent === tipPercent,
                    onchange: () => { tipPercent = choice.percent; renderCheckout(); } }),
      choice.percent ? `${choice.percent}%` : "None",
      el("br"),
      el("small", { class: "muted" }, choice.percent ? choice.amount : "")));
  }

  const chosen = document.querySelector('input[name="card"]:checked')?.value ?? (state.wallet.cards.length ? "1" : "new");
  const cards = $("card-choices");
  cards.replaceChildren();
  for (const card of state.wallet.cards) {
    cards.append(el("label", {},
      el("input", { type: "radio", name: "card", value: card.index, checked: String(card.index) === chosen, onchange: renderCheckout }),
      card.label));
  }
  cards.append(el("label", {},
    el("input", { type: "radio", name: "card", value: "new", checked: chosen === "new", onchange: renderCheckout }),
    "A new card"));

  const isNew = (document.querySelector('input[name="card"]:checked')?.value ?? chosen) === "new";
  $("new-card").hidden = !isNew;
  const full = state.wallet.cards.length >= state.wallet.limit;
  $("cc-save").disabled = full;
  if (full) $("cc-save").checked = false;
  $("save-note").textContent = full ? `You have ${state.wallet.limit} saved cards. Remove one on the Passport tab to save another.` : "";
  $("nick-row").hidden = !$("cc-save").checked;
  $("pay-btn").textContent = `Pay ${total}`;
}

$("cc-save").addEventListener("change", () => { $("nick-row").hidden = !$("cc-save").checked; });
$("cc-number").addEventListener("input", (event) => {
  const digits = event.target.value.replace(/\D/g, "").slice(0, 19);
  event.target.value = digits.replace(/(.{4})/g, "$1 ").trim();
});
$("cc-exp").addEventListener("input", (event) => {
  const digits = event.target.value.replace(/\D/g, "").slice(0, 4);
  event.target.value = digits.length > 2 ? `${digits.slice(0, 2)}/${digits.slice(2)}` : digits;
});

$("pay-btn").addEventListener("click", async () => {
  const choice = document.querySelector('input[name="card"]:checked')?.value;
  const body = { tip_percent: tipPercent };
  if (choice === "new") {
    body.new_card = {
      number: $("cc-number").value, expiry: $("cc-exp").value, cvv: $("cc-cvv").value,
      save: $("cc-save").checked, nickname: $("cc-nick").value.trim(),
    };
  } else {
    body.saved_card = Number(choice);
  }
  $("pay-btn").disabled = true;
  $("pay-error").textContent = "";
  try {
    const data = await api("/api/pay", body);
    for (const id of ["cc-number", "cc-exp", "cc-cvv", "cc-nick"]) $(id).value = "";
    $("cc-save").checked = false;
    setState(data.state);
    showReceipt(data.receipt);
  } catch (error) {
    $("pay-error").textContent = error.message;
  } finally {
    $("pay-btn").disabled = false;
  }
});

function showReceipt(receipt) {
  const passport = state.passport;
  const box = $("receipt");
  const head = el("div", { class: "receipt-head" });
  if (receipt.stamp_earned) {
    const stamp = el("span", { class: "stamp on land" }, icon("anise"));
    head.append(stamp, el("h2", {}, "Stamped!"));
  } else {
    head.append(el("h2", {}, "Reward redeemed"));
  }
  head.append(el("p", {}, `Paid ${receipt.total} on ${receipt.card}. Your order is in the kitchen.`),
              el("p", { class: "muted" }, passport.message));
  const totals = el("dl", { class: "totals" },
    receipt.lines.map((line) => [el("dt", {}, `${line.quantity} × ${line.name}`), el("dd", {}, line.line_total)]),
    el("dt", {}, "Subtotal"), el("dd", {}, receipt.subtotal),
    receipt.discount_percent ? [el("dt", { class: "reward" }, `Passport reward (${receipt.discount_percent}% off)`), el("dd", { class: "reward" }, `−${receipt.discount}`)] : null,
    el("dt", {}, "Tax"), el("dd", {}, receipt.tax),
    el("dt", {}, "Tip"), el("dd", {}, receipt.tip),
    el("dt", { class: "grand" }, "Total"), el("dd", { class: "grand" }, receipt.total));
  box.replaceChildren(head, totals,
    el("button", { class: "btn primary wide", onclick: () => $("checkout").close() }, "Done"));
  $("pay-form").hidden = true;
  box.hidden = false;
  addMessage("bot", `Thanks! Paid ${receipt.total} on ${receipt.card}. ${passport.message}`);
  renderChips();
  reportLoaded = false;
}

// ------------------------------------------------------------- restaurant --

async function loadReport() {
  if (reportLoaded) return;
  const report = await api("/api/report");
  reportLoaded = true;
  $("kpis").replaceChildren(
    ...[["Orders", report.orders], ["Customers", report.customers], ["Food revenue", report.food_revenue],
        ["Average order", report.average_order], ["Passport discounts", report.passport_discounts]]
      .map(([label, value]) => el("div", { class: "kpi" }, el("b", {}, value), el("span", {}, label))));

  const notes = [report.method_note];
  if (report.orders === 0) notes.unshift("No orders yet. Place a few orders on the Order tab first.");
  else if (report.early_signal) notes.unshift("Fewer than 30 orders: treat these numbers as an early signal.");
  $("report-notes").textContent = notes.join(" ");

  const sold = report.items.filter((item) => item.units > 0);
  const table = $("report-table");
  table.replaceChildren(el("thead", {}, el("tr", {},
    el("th", {}, "Item"), el("th", { class: "num" }, "Price"), el("th", { class: "num" }, "Units"),
    el("th", { class: "num" }, "Revenue"), el("th", { class: "num" }, "Share"), el("th", {}, "Position"),
    el("th", {}, "Recommendation"))));
  const body = el("tbody");
  for (const item of sold) {
    body.append(el("tr", {},
      el("td", {}, item.name, el("div", { class: "muted small" }, item.category)),
      el("td", { class: "num" }, item.price), el("td", { class: "num" }, item.units),
      el("td", { class: "num" }, item.revenue), el("td", { class: "num" }, `${item.revenue_share}%`),
      el("td", {}, el("span", { class: `q ${item.quadrant}` }, item.quadrant)),
      el("td", {}, item.recommendation, item.best_pair ? el("div", { class: "muted small" }, `Most often with ${item.best_pair.name}`) : null)));
  }
  if (!sold.length) body.append(el("tr", {}, el("td", { colspan: 7, class: "empty" }, "No sales yet.")));
  table.append(body);

  $("top-pairs").replaceChildren(...(report.top_pairs.length
    ? report.top_pairs.map((pair) => el("li", {}, `${pair.items[0]} + ${pair.items[1]}`, el("span", { class: "muted" }, ` · ${pair.orders_together} orders`)))
    : [el("li", { class: "empty" }, "No orders with two or more dishes yet.")]));
  const never = report.items.filter((item) => item.units === 0);
  $("never").replaceChildren(...(never.length
    ? never.map((item) => el("li", {}, item.name, el("span", { class: "muted" }, ` · ${item.category}`)))
    : [el("li", { class: "empty" }, "Every dish has sold at least once.")]));
}

$("insights-btn").addEventListener("click", async () => {
  const box = $("insights");
  const button = $("insights-btn");
  box.hidden = false;
  box.textContent = "Reading the report…";
  button.disabled = true;
  try {
    box.textContent = (await api("/api/insights", {})).insights;
  } catch (error) {
    box.textContent = error.message;
  } finally {
    button.disabled = false;
  }
});

// ------------------------------------------------------------------ start --

(async function start() {
  try {
    const data = await api("/api/state");
    menu = data.menu;
    assistantOn = data.assistant_available;
    setState(data);
    renderMenu();
    renderChips();
    addMessage("bot", data.greeting.reply);
    addCards(data.greeting.cards);
    if (!assistantOn) {
      note("The chat assistant is off on this server (no ANTHROPIC_API_KEY), so the menu is open instead.");
      showView("menu");
      $("chat-input").disabled = true;
    }
  } catch (error) {
    document.querySelector("main").prepend(el("p", { class: "error" }, `Couldn't reach the kitchen: ${error.message}`));
  }
})();
