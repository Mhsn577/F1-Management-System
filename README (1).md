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


│   └── output3.png
├── Project_Report.pdf
└── README.md
