# Grand Prix Weekend Ticket Booking System

A console-based ticket booking system for a three-day Grand Prix weekend, written in C as a Programming Fundamentals (CT-175) CCA group project. Customer records are stored in dynamically allocated memory that grows automatically, and all bookings are saved to disk so they survive program restarts.

## Group Members
- **Syed Muhammad Mohsin Abbas Rizvi** — Roll No: CT-26274
- **Yumna Irfan** — Roll No: CT-26267
- **Yusra Farooqui** — Roll No: CT-26254

**Course:** Programming Fundamentals (CT-175)

---

## Race Weekend Schedule
| Day | Event |
| :--- | :--- |
| **Friday** | Practice Session |
| **Saturday** | Qualifying Race |
| **Sunday** | Grand Prix |

## Features
- **Single-Day & Full Weekend Pass Booking:** Book tickets for Friday, Saturday, Sunday, or a Full Weekend Pass covering all three days.
- **Enclosure Selection:** Choose between General Admission, Grandstand, Premium, or VIP for each booked day.
- **Live Seat Tracking:** Checks and deducts available seats in real time upon every booking, and restores them upon cancellation.
- **Automated Billing & Discounts:** Calculates subtotals, age-based discounts, and grand totals, generating both an on-screen receipt and a persistent log (`bills.txt`).
- **Unique Sequential IDs:** Generates unique booking IDs starting from `1001` tracked via `booking_id.txt`.
- **Search & Sort:** Search any booking by ID (linear search) or view all customers sorted alphabetically by name (case-insensitive bubble sort).
- **Cancellation & Recursive Re-booking:** Cancels bookings by ID, releases seats back into inventory, enforces the Full Weekend Pass cancellation policy, and recursively prompts the user if they wish to re-book individual days.
- **Dynamic Memory Scaling:** Starts with an initial capacity of 10 customers and automatically doubles in size as new bookings are added—no fixed customer cap.
- **Input Validation:** Rejects non-numeric characters and out-of-range inputs cleanly without crashing.
- **Persistent Storage:** Saves seat availability and customer structs in binary format (`bookings.dat`) across sessions.

---

## Seats Available (Per Day)
| Enclosure | Seats |
| :--- | :--- |
| General Admission | 100 |
| Grandstand | 80 |
| Premium | 60 |
| VIP | 20 |

## Pricing & Age-Based Discounts
**Ticket Category:**
| Type | Price per Ticket |
| :--- | :--- |
| Normal (`N`) | 275.00 |
| VIP (`V`) | 956.00 |

**Discount per Person (by Age):**
| Age Group | Discount |
| :--- | :--- |
| 5 and under | 100% (Free) |
| 6 to 18 | 50% |
| 19 to 60 | 0% (Full Price) |
| Over 60 | 30% |

## Cancellation Policy
A Full Weekend Pass is treated as a single package and cannot be partially cancelled. To keep only specific days (e.g., Friday and Sunday), the user must cancel the full pass first and then re-register for the desired individual days when prompted.

---

## PF Concepts Used
- **Structs:** `Customer` and `Booking` store customer details, nested day-wise enclosure choices, ticket counts, and cancellation flags.
- **2D & 1D Arrays:** A `3x4` seat matrix (`days x enclosures`) and a dynamic array of `Customer` structs.
- **Dynamic Memory Allocation:**
  - `malloc`: Allocates the `3x4` seat matrix in `main()` and a temporary staging buffer in `loadData()`.
  - `calloc`: Allocates the zero-initialized `Customer` array starting at `INITIAL_CAPACITY` (10).
  - `ensureCapacity()` (`calloc` + `memcpy` + `free`): Doubles the customer array capacity dynamically whenever it reaches full capacity.
  - `free`: Releases all allocated heap memory cleanly before program exit.
- **Pointers & Double Pointers:** `struct Customer **records` allows functions (`startBooking`, `cancelBooking`, `loadData`) to resize and reassign the customer array pointer directly, while `int *count` and `int *capacity` update state across functions.
- **Searching & Sorting:** Linear search (`searchByID`) to locate bookings by ID, and Bubble Sort (`sorting` with `compareNames`) to order customers A–Z case-insensitively.
- **Recursion:** `startBooking()` calls itself when `fromCancel == 1` so users can register multiple individual days consecutively after cancelling a pass.
- **File Handling (Text & Binary):**
  - `bookings.dat` (Binary: `"wb"` / `"rb"`): Saves and loads raw seat matrices and customer struct blocks using `fwrite` and `fread`.
  - `bills.txt` (Text: `"a"`): Appends formatted customer receipts.
  - `booking_id.txt` (Text: `"r"` / `"w"`): Tracks the last issued booking ID across runs.


## Sample Input and Output

### 1. Booking a Single-Day Ticket (with Age Discounts)
**User Input:**
```text
Main Menu Choice: 1
Day Choice: 3 (Sunday)
Enclosure Choice: 2 (Grandstand)
Number of Tickets: 2
Ticket Type: N
Primary Customer Name: Ali Raza
Age for Person 1: 28
Age for Person 2: 12
```

**Program Output:**
```text
+----------------------------------------------------+
|                 COMBINED BILL SLIP                 |
+----------------------------------------------------+
| BOOKING ID      : 1001                             |
| PRIMARY ACCOUNT : Ali Raza                         |
| TICKET CATEGORY : N (Normal)                       |
| TOTAL QUANTITY  : 2                                |
| BASE UNIT PRICE : 275.00                           |
| SUBTOTAL COST   : 550.00                           |
| TOTAL SAVINGS   : -137.50                          |
+----------------------------------------------------+
| GRAND TOTAL     : 412.50                           |
+----------------------------------------------------+
      == WELCOME TO RACE WEEKEND ==

Bill processing finished! Record successfully logged to file.
```

---

### 2. Booking a Full Weekend Pass
**User Input:**
```text
Main Menu Choice: 1
Day Choice: 4 (Full Weekend Pass)
Friday Enclosure: 1 (General Admission)
Saturday Enclosure: 3 (Premium)
Sunday Enclosure: 4 (VIP)
Friday Tickets: 1
Saturday Tickets: 1
Sunday Tickets: 1
Ticket Type: V
Primary Customer Name: Sara Ahmed
Age for Person 1: 65
Age for Person 2: 30
Age for Person 3: 4
```

**Program Output:**
```text
+----------------------------------------------------+
|                 COMBINED BILL SLIP                 |
+----------------------------------------------------+
| BOOKING ID      : 1002                             |
| PRIMARY ACCOUNT : Sara Ahmed                       |
| TICKET CATEGORY : V (VIP)                          |
| TOTAL QUANTITY  : 3                                |
| BASE UNIT PRICE : 956.00                           |
| SUBTOTAL COST   : 2868.00                          |
| TOTAL SAVINGS   : -1242.80                         |
+----------------------------------------------------+
| GRAND TOTAL     : 1625.20                          |
+----------------------------------------------------+
      == WELCOME TO RACE WEEKEND ==

Bill processing finished! Record successfully logged to file.
```

---

### 3. Searching for a Booking by ID
**User Input:**
```text
Main Menu Choice: 3
Enter Booking ID: 1001
```

**Program Output:**
```text
Booking found!
+----------------------------------------------------+
| BOOKING ID      : 1001                             |
| NAME            : Ali Raza                         |
| TICKETS         : 2                                |
| Sunday          : 2 ticket(s) - Grandstand         |
+----------------------------------------------------+
```

---

### 4. Sorting Customers Alphabetically
**User Input:**
```text
Main Menu Choice: 4
```

**Program Output:**
```text
+----------------------------------------------------+
|              CUSTOMERS SORTED BY NAME              |
+----------------------------------------------------+
| BOOKING ID | NAME                        | TICKETS |
+----------------------------------------------------+
| 1001       | Ali Raza                    | 2       |
| 1002       | Sara Ahmed                  | 3       |
+----------------------------------------------------+
```

---

### 5. Cancelling a Booking
**User Input:**
```text
Main Menu Choice: 2
Enter Booking ID: 1001
Confirm Cancellation (Y/N): Y
Register for another day now? (Y/N): N
```

**Program Output:**
```text
This is a single day booking.

  > Do you really want to cancel booking 1001 (Ali Raza)? (Y/N): Y

Booking 1001 cancelled. Seats have been released.
```

---

### 6. Invalid Input Handling
**User Input:**
```text
Main Menu Choice: abc
```

**Program Output:**
```text
  > Enter your choice: abc
  Invalid input! Please enter a valid number.
```
