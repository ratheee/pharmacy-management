# Pharmacy Management System

[![C++ CI](https://github.com/ratheee/pharmacy-management/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/ratheee/pharmacy-management/actions/workflows/c-cpp.yml)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey)

A console-based pharmacy and medical store management system written in C++17.
It manages medicine stock, tracks expiry and low stock, and generates bills
with GST. All data is stored in plain text files, and records are kept in
memory in a custom dynamic array.

## Features

- **Medicines:** add, update, restock, search (by ID, name or category) and delete
- **Stock tracking:** each medicine is flagged as OK, LOW (10 units or fewer),
  OUT or EXPIRED, and a stock report shows total stock value
- **Billing:** a cart that checks stock as items are added, refuses expired
  medicines, adds 5% GST, and updates stock only after the bill is confirmed
- **Bill history:** every bill is saved, and buyers can see their own orders
- **Accounts:** Seller (store staff) and Buyer (customer) roles with separate
  menus, masked password entry, and a login history
- **Medicine requests:** buyers can request medicines the store doesn't stock
- **Input validation:** every number, date and menu choice is checked, and the
  program asks again until the input is valid

## Sample output

```
====================================================
                 CITY MEDICAL STORE
====================================================
Bill No : 1001
Date    : 2026-11-02 18:53:52
Customer: Ravi Kumar
----------------------------------------------------
No  Medicine                  Qty     Rate    Amount
----------------------------------------------------
1   Paracetamol 500mg           2    25.50     51.00
2   Amoxicillin 250mg           3    89.50    268.50
----------------------------------------------------
                               Subtotal :     319.50
                               GST (5%) :      15.98
                            Total (Rs.) :     335.48
====================================================
             Thank you! Get well soon.
====================================================
```

```
----------------------------------------------------------------------------------------------------
ID    Name                  Company         Category           Price    Qty  Expiry      Status
----------------------------------------------------------------------------------------------------
101   Paracetamol 500mg     Cipla           Tablet             25.50    127  2027-08-31  OK
102   Amoxicillin 250mg     Sun Pharma      Antibiotic         89.50      5  2027-02-28  LOW
103   Cetirizine 10mg       Dr Reddys       Antiallergic       32.00     60  2026-09-30  EXPIRED
----------------------------------------------------------------------------------------------------
```

## Getting started

### Requirements

A C++17 compiler: g++ 9 or later, clang++ 9 or later, or Visual Studio 2019 or later.

On Windows, g++ can be installed with:

```powershell
winget install -e --id BrechtSanders.WinLibs.POSIX.UCRT
```

### Build and run

| Environment | Steps |
|---|---|
| Windows, quickest | Double-click `run.bat`. It builds the program and starts it. |
| Git Bash | `./build.sh`, then `winpty ./pharmacy.exe` |
| Visual Studio | Open `PharmacyManagement.sln` and press F5 |
| Any terminal | `g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp -o pharmacy`, then `./pharmacy` |

The first time it runs, the program asks you to create the store owner's
Seller account.

## Roles

| Seller | Buyer |
|---|---|
| Add, update, restock and delete medicines | View and search medicines |
| Stock report | Place an order |
| Generate bills, view all bills | View their own orders |
| View and clear medicine requests | Request a medicine |
| View login history, create accounts | |

Public sign-up creates Buyer accounts only. New Seller accounts can only be
created by a logged-in seller.

## Project structure

```
include/
  DynamicArray.h   Growable array template that manages its own heap memory
  Medicine.h       Medicine record and its file format
  Inventory.h      Medicine storage, file load/save, search and reports
  Billing.h        Cart, bill formatting and the billing workflow
  Account.h        Accounts, roles, login and login history
  Requests.h       Medicine requests from buyers
  Utils.h          Validated console input, dates and string helpers
src/
  One .cpp file per header, plus main.cpp for the menus
.github/workflows/c-cpp.yml   CI: builds and tests on Linux and Windows
```

## Design notes

### Dynamic memory

`DynamicArray<T>` stores its elements in a heap block allocated with `new[]`.
When the block is full, it allocates one twice the size, moves the elements
across and frees the old block with `delete[]`. Copying is disabled, so the
same memory can never be freed twice. The inventory and each bill's cart are
built on it.

### File storage

| File | Format |
|---|---|
| `medicines.txt` | `id\|name\|company\|category\|price\|qty\|expiry` |
| `bills.txt` | Each bill appended in printable form |
| `bill_counter.txt` | Last bill number used (bills start at 1001) |
| `accounts.txt` | `username\|role\|passwordHash` |
| `login_history.txt` | `username\|role\|timestamp` |
| `requests.txt` | `timestamp\|username\|medicine\|qty` |

The data files are created in the folder the program runs from. Saving writes
to a temporary file first and then replaces `medicines.txt`, so a failed save
can't corrupt existing data. Invalid lines are skipped with a warning when the
file is loaded.

Passwords are stored as a 64-bit FNV-1a hash, never as plain text. It keeps
passwords out of the file, but it is not a cryptographic password hash.

### C++17 features used

`std::optional`, `std::string_view`, `std::filesystem`, `std::from_chars`,
structured bindings, `if` with initializer, `[[nodiscard]]`, `constexpr`,
`enum class`, and lambdas with a template `printWhere()` that serves search
and reports.

## Continuous integration

Every push and pull request is built on Ubuntu and Windows with `-Wall -Wextra
-Wpedantic -Werror`, so any warning fails the build. A smoke test then runs the
program, creates an account and checks that it was saved.

## Author

[@ratheee](https://github.com/ratheee)
