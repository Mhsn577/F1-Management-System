Grand Prix Weekend Ticket Booking System

A console-based ticket booking system for a three-day Grand Prix weekend, written in C as a Programming Fundamentals (CCP) group project. Customer records are stored in dynamically allocated memory that grows automatically, and all bookings are saved to disk so they survive restarts.

Race Weekend
Day	Event
Friday	Practice Session
Saturday	Qualifying Race
Sunday	Grand Prix
Features
Book tickets for a single day (Friday, Saturday or Sunday) or a Full Weekend Pass covering all three days.
Choose an enclosure for each day: General Admission, Grandstand, Premium or VIP.
Live seat availability that is checked and updated on every booking.
Automatic billing with age-based discounts and a printed bill slip.
Unique booking IDs, starting from 1001.
Cancel a booking by ID. The seats are released back to the enclosure.
Search a booking by ID to see the name, tickets, days and enclosures.
Sort customers alphabetically by name (case-insensitive).
No fixed customer limit: the customer list starts small and grows automatically as more bookings are made.
Input validation: menu choices, enclosure and ticket quantities reject letters and invalid numbers instead of crashing.
Data saved between runs, so bookings and seat counts are still there after the program is closed.
Seats Available (per day)
Enclosure	Seats
General Admission	100
Grandstand	80
Premium	60
VIP	20
Pricing and Discounts

Ticket type (chosen at booking):

Type	Price per ticket
Normal (N)	275.00
VIP (V)	956.00

Discount per person, based on age:

Age	Discount
5 and under	100% (free)
6 to 18	50%
19 to 60	none
Over 60	30%

The bill slip shows the subtotal, total savings and grand total.

Cancellation Policy

A Full Weekend Pass is one package and cannot be partly cancelled. To keep only some of the days (for example Friday and Sunday), cancel the whole pass first and then register those days again as individual bookings. After a cancellation the program offers to take the customer straight back to the booking page.

How It Works (Concepts Used)
Structs: Customer and Booking store each customer's details and day-wise tickets.
Arrays: a 2D seats[3][4] table (day x enclosure) and a list of customers.
Dynamic memory allocation:
malloc allocates the seat table and a temporary buffer when loading saved data.
calloc allocates the customer list (zero-initialised), starting at 10 customers.
ensureCapacity doubles the list size when it is full: it callocs a bigger block, copies the old records with memcpy, then frees the old block.
free releases all allocated memory when the program exits.
Pointers and double pointers: struct Customer **records lets functions replace the customer list with a bigger one, and int *count lets them update the customer count.
Linear search: searchByID finds a booking by its ID.
Bubble sort: sorting orders customers A to Z by name using compareNames.
Recursion: startBooking calls itself after a cancellation so the customer can register more days.
File handling:
bills.txt keeps a log of every bill slip.
booking_id.txt stores the last booking ID so IDs never repeat.
bookings.dat (binary) stores seats and customers using saveData and loadData.
Input validation: getValidInt keeps asking until the user enters a real number and clears leftover input.
Requirements
Windows (the program uses windows.h, Sleep() and system("cls")).
A C compiler such as GCC (MinGW) or Code::Blocks.
Compile and Run
bash
gcc grand_prix_booking.c -o grand_prix_booking.exe
grand_prix_booking.exe
Files Created While Running

These are created automatically the first time the program runs, so they are not included in the repository:

File	Purpose
bills.txt	Log of all printed bill slips
booking_id.txt	Last used booking ID
bookings.dat	Saved bookings and seat counts

To start completely fresh, delete all three files.

Limitations
Windows only.
Names are limited to 49 characters.
Group Members
Syed Muhammad Mohsin Abbas Rizvi
Yumna Irfan
Yusra Farooqui

Course: Programming Fundamentals (CCP)
