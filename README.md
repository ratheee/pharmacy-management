# Pharmacy / Medical Store Management System

A console-based Pharmacy / Medical Store Management System built in modern
C++ (C++17).
It uses file handling for persistent storage and dynamic memory for flexible
data management.

## Features

- **Accounts and roles**: login and sign-up with Seller (store staff) and
  Buyer (customer) roles, each with its own menu. Passwords are masked with
  `*` while typing, and every login is recorded in a history
- **Medicine management**: add, update, search and delete medicines
- **Search**: by ID, by name (partial, case-insensitive), or by category
- **Stock tracking**: restock, low-stock (≤ 10 units) and out-of-stock flags,
  expiry detection, and a stock report with total inventory value
- **Billing**: build a cart, add or remove items, check stock live, 5% GST,
  print the bill, deduct stock automatically, and save every bill to a file
- **Bill history**: sellers see every bill. Buyers see only their own orders
- **Medicine requests**: buyers can request medicines the store doesn't stock,
  and sellers can review and clear the requests
- **Persistent storage**: all data is saved to text files and reloaded on start

## Roles

| Seller | Buyer |
|--------|-------|
| Add, update/restock, delete medicines | View and search medicines |
| Stock report | Place an order (billed under their username) |
| Generate bills, view all bills | View their own orders |
| View and clear medicine requests | Request a medicine |
| View login history, create accounts | |

The first time the program runs, it asks you to create the store owner's
Seller account. After that, public sign-up creates **Buyer** accounts only.
New Seller accounts can only be created by a logged-in seller, so a customer
can't give themselves seller access.

## Project structure

```
include/
  DynamicArray.h  Growable heap array template (new[] / delete[])
  Medicine.h    Medicine class + serialization to/from a file record
  Inventory.h   Medicines in a DynamicArray, file load/save, reports
  Billing.h     Bill (cart in a DynamicArray) and BillingSystem workflow
  Account.h     Accounts, roles, login and login history
  Requests.h    Buyers' medicine requests
  Utils.h       Validated console input, masked password, date/string helpers
src/
  One .cpp per header, plus main.cpp (menus)
PharmacyManagement.sln / .vcxproj   Visual Studio 2019+ project
```

### Dynamic memory

`DynamicArray<T>` manages its own heap memory with `new[]` / `delete[]`. When
it is full, it doubles its capacity, moves the elements across and frees the
old block. Copying is deleted (`= delete`), so the memory can't be freed twice.
It supports range-based `for` loops. `Inventory` stores medicines in one, and
each `Bill` stores its cart in another.

### C++17 features used

- `std::optional` for lookups and parsing that can fail (`Medicine::parse`,
  `AccountManager::login`)
- `std::string_view` for prompts and constants
- `std::filesystem` to replace the data file safely, and `std::from_chars`
  for strict number parsing
- `if` with initializer, structured bindings, `[[nodiscard]]`, `constexpr`,
  `enum class Role`, `#pragma once`, lambdas with a template `printWhere()`
  for searches and reports
- Thread-safe `localtime_s` / `localtime_r` with `<chrono>` for timestamps

### File handling

| File               | Contents                                                     |
|--------------------|--------------------------------------------------------------|
| `medicines.txt`    | One medicine per line: `id\|name\|company\|category\|price\|qty\|expiry` |
| `bills.txt`        | Every generated bill, appended in printable form             |
| `bill_counter.txt` | Last bill number used (bills start at 1001)                  |
| `accounts.txt`     | `username\|role\|passwordHash`                               |
| `login_history.txt`| `username\|role\|timestamp`                                  |
| `requests.txt`     | `timestamp\|username\|medicine\|qty`                         |

Passwords are stored as a 64-bit FNV-1a hash, never as plain text. This keeps
passwords out of the file, but it is not a cryptographic password hash, so
don't reuse a real password here.

Saving writes to `medicines.txt.tmp` first and then replaces the real file, so
a failed write can't wipe your existing data. These files are created in the
folder you run the program from.

## Build and run

**Visual Studio (2019 or later):** open `PharmacyManagement.sln` and press
F5. This needs the "Desktop development with C++" workload. When run from
Visual Studio, data files are created in the project folder.

**Git Bash on Windows (easiest):**

```sh
./build.sh          # finds g++ even if it isn't on PATH
winpty ./pharmacy.exe
```

`winpty` gives the program a real console, so passwords show as `*`. Without
it, Git Bash shows the password as you type it.

**g++ 9+ / clang++ 9+ (C++17):**

```sh
g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp -o pharmacy
./pharmacy          # Windows: pharmacy.exe
```

Masked password input uses `<conio.h>`, so it works on Windows only. On other
systems, the password is read as a normal line.

## Usage

```
============================================
   PHARMACY / MEDICAL STORE MANAGEMENT
============================================
 1. Login
 2. Sign up (new buyer)
 0. Exit
```

- Medicine IDs are assigned automatically, starting at 101.
- Enter expiry dates as `YYYY-MM-DD`.
- Expired medicines can't be added to a bill. Billing also won't let you sell
  more than the available stock.
- Stock is only reduced after you confirm a bill. Cancelling leaves stock unchanged.
