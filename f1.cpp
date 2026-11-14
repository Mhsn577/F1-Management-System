#include <stdio.h>      // printf, scanf, file functions (fopen, fprintf ...)
#include <stdlib.h>     // system("cls")
#include <string.h>     // strlen, strcpy, memcpy
#include <ctype.h>      // tolower (used when comparing names)
#include <windows.h>    // Sleep() and console colours (Windows only)

#define MAX_CUSTOMERS 100   // the most customers the program can store

///////////////////// STYLE SETTINGS (new) /////////////////////
// Width of every box / table on screen (in characters)
#define WIDTH 54

// Console colour codes used with SetConsoleTextAttribute (Windows)
#define COL_DEFAULT 7     // normal light-grey text
#define COL_TITLE   11    // light aqua  -> banners and headings
#define COL_OK      10    // light green -> success messages
#define COL_ERR     12    // light red   -> errors / invalid input
#define COL_WARN    14    // light yellow-> warnings, policy notes, prices
#define COL_PROMPT  15    // bright white-> questions asked to the user
#define COL_DIM     8     // grey        -> small details
//////////////////////////////////////////////////////////////////

///////////////////// Structs

// Details of ONE booking. Each array has 3 slots = 3 days:
// index 0 = Friday, 1 = Saturday, 2 = Sunday
struct Booking {
    int booking_id;       // unique ID of this booking
    int enclosure[3];     // enclosure chosen for each day (1-4), 0 = day not booked
    int tickets[3];       // number of tickets for each day, 0 = day not booked
    int cancelled[3];     // 1 if that day has been cancelled, otherwise 0
};

// One entry per confirmed customer. Used for SORTING (by name) and SEARCHING (by ID).
// The Booking struct is kept inside so a cancellation knows which seats to give back.
struct Customer {
    int id;
    char name[50];
    int tickets;              // total tickets across all days
    struct Booking booking;   // day-wise details (needed for cancellation)
};

// Lookup tables used only for printing
// (array index -> text, e.g. dayNames[0] is "Friday")
const char *dayNames[3] = {"Friday", "Saturday", "Sunday"};
const char *enclosureNames[4] = {"General Admission", "Grandstand", "Premium", "VIP"};

// Function declarations
int selectEnclosure(void);
void Welcome_scr(void);
int ticket_booking(void);
int getTicketQuantity(void);
int checkAvailibility(int seats[3][4], int choice, int enclosure, int ticket);
int FullPass(int seats[3][4], struct Booking *booking, struct Customer *cust);
int Bill(int ticket_quantity, char type, struct Customer *cust);

// New functions
void startBooking(int seats[3][4], struct Customer records[], int *count, int fromCancel);
void cancelBooking(int seats[3][4], struct Customer records[], int *count);
int  searchByID(struct Customer list[], int count, int id);
void searchBooking(struct Customer list[], int count);
void sorting(struct Customer list[], int count);
void showMenu(void);

// Data saving / loading (new) - keeps bookings and seats after the program is closed
void saveData(int seats[3][4], struct Customer records[], int count);
int  loadData(int seats[3][4], struct Customer records[], int *count);

// STYLE & INPUT helper declarations
void setColor(int color);
void printBorder(int color);
void printBanner(const char *title, int color);
void printRow(const char *label, const char *value, int color);
void printPrompt(const char *text);
void printMessage(const char *text, int color);



int  getValidInt(const char *prompt);


///////////////////////////////////////////////////////////////////////////
//                    STYLE & INPUT HELPER FUNCTIONS
///////////////////////////////////////////////////////////////////////////

// STYLE: changes the colour of everything printed after this call
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// STYLE: prints a horizontal border like  +--------------------+
void printBorder(int color) {
    int i;
    setColor(color);
    printf("+");
    for (i = 0; i < WIDTH - 2; i++) {
        printf("-");
    }
    printf("+\n");
    setColor(COL_DEFAULT);
}

// STYLE: prints a centred title inside a box
void printBanner(const char *title, int color) {
    int len = (int)strlen(title);
    int left = (WIDTH - 2 - len) / 2;          // spaces on the left of the title
    int right = WIDTH - 2 - len - left;        // spaces on the right of the title

    printf("\n");
    printBorder(color);
    setColor(color);
    printf("|%*s%s%*s|\n", left, "", title, right, "");
    setColor(COL_DEFAULT);
    printBorder(color);
}

// STYLE: prints one aligned "label : value" line inside a box
void printRow(const char *label, const char *value, int color) {
    setColor(color);
    printf("| %-16s: %-*.*s |\n", label, WIDTH - 22, WIDTH - 22, value);
    setColor(COL_DEFAULT);
}

// STYLE: prints a question / input prompt in bright white with an arrow
void printPrompt(const char *text) {
    setColor(COL_PROMPT);
    printf("  > %s", text);
    setColor(COL_DEFAULT);
}

// STYLE: prints a one-line coloured message (green = ok, red = error ...)
void printMessage(const char *text, int color) {
    setColor(color);
    printf("%s\n", text);
    setColor(COL_DEFAULT);
}

// INPUT VALIDATION: Safely reads an integer and flushes leftover characters
int getValidInt(const char *prompt) {
    int value;
    int status;
    while (1) {
        printPrompt(prompt);
        status = scanf("%d", &value);

        // Clear all leftover characters (including '\n' and invalid letters)
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);

        if (status == 1) {
            return value; // Valid integer read
        }
        printMessage("  Invalid input! Please enter a valid number.", COL_ERR);
    }
}


///////////////////////////////////////////////////////////////////////////
//                           CORE SYSTEM FUNCTIONS
///////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// selectEnclosure : shows the 4 enclosures and keeps asking until the user
//                   enters a valid choice (1-4). Returns that choice.
// ---------------------------------------------------------------------------
int selectEnclosure(void) {
    int enclosure;
    printBanner("SELECT ENCLOSURE", COL_TITLE);
    printf("   [1] General Admission\n");
    printf("   [2] Grandstand\n");
    printf("   [3] Premium\n");
    printf("   [4] VIP\n\n");
    do {
        enclosure = getValidInt("Enter your choice: ");
        switch (enclosure) {
            case 1: printMessage("  You selected General Admission", COL_OK); break;
            case 2: printMessage("  You selected Grandstand", COL_OK); break;
            case 3: printMessage("  You selected Premium", COL_OK); break;
            case 4: printMessage("  You selected VIP", COL_OK); break;
            default: printMessage("  Invalid enclosure! Try again.", COL_ERR);
        }
    } while (enclosure < 1 || enclosure > 4);
    return enclosure;
}

// ---------------------------------------------------------------------------
// Welcome_scr : prints the welcome screen with the 3 race days
// ---------------------------------------------------------------------------
void Welcome_scr(void) {
    printBanner("WELCOME TO THE GRAND PRIX WEEKEND", COL_WARN);
    printf("\n");
    setColor(COL_TITLE);
    printf("   Friday   ");
    setColor(COL_DEFAULT);
    printf("- Practice Session\n");
    setColor(COL_TITLE);
    printf("   Saturday ");
    setColor(COL_DEFAULT);
    printf("- Qualifying Race\n");
    setColor(COL_TITLE);
    printf("   Sunday   ");
    setColor(COL_DEFAULT);
    printf("- Grand Prix\n");
}

// ---------------------------------------------------------------------------
// ticket_booking : the "day page". Asks which day (1-3) or the full weekend
//                  pass (4). Keeps asking until valid. Returns the choice.
// ---------------------------------------------------------------------------
int ticket_booking(void) {
    int choice;
    printBanner("TICKET BOOKING", COL_TITLE);
    printf("   [1] Friday\n");
    printf("   [2] Saturday\n");
    printf("   [3] Sunday\n");
    printf("   [4] Full Weekend Pass\n\n");
    do {
        choice = getValidInt("Enter your choice: ");
        if (choice < 1 || choice > 4) {
            printMessage("  Invalid choice! Try again.", COL_ERR);
        }
    } while (choice < 1 || choice > 4);
    return choice;
}

// ---------------------------------------------------------------------------
// getTicketQuantity : asks how many tickets. Rejects 0 and negative numbers.
// ---------------------------------------------------------------------------
int getTicketQuantity(void) {
    int ticket;
    do {
        ticket = getValidInt("How many tickets do you want? ");
        if (ticket <= 0) {
            printMessage("  Please enter a valid number!", COL_ERR);
        }
    } while (ticket <= 0);
    printf("  You selected %d tickets\n", ticket);
    return ticket;
}

// ---------------------------------------------------------------------------
// checkAvailibility : checks if enough seats are free for ONE day.
// ---------------------------------------------------------------------------
int checkAvailibility(int seats[3][4], int choice, int enclosure, int ticket) {
    int sub_seats = seats[choice - 1][enclosure - 1];
    printBanner("SEATS BOOKING", COL_TITLE);
    if (sub_seats >= ticket) {
        printMessage("  CONGRATS!!! SEATS AVAILABLE", COL_OK);
        seats[choice - 1][enclosure - 1] -= ticket;
        return 1;
    } else {
        printMessage("  SORRY! Seats not available", COL_ERR);
        return 0;
    }
}

// ---------------------------------------------------------------------------
// getTicketType : asks for V (VIP) or N (Normal), upper or lower case.
// ---------------------------------------------------------------------------
char getTicketType(void) {
	char type;
    int ch;
    do {
        printf("\n");
        printPrompt("Enter ticket type (V for VIP or N for Normal): ");
        scanf(" %c", &type);
        while ((ch = getchar()) != '\n' && ch != EOF); // Flush extra typed letters
        if (type != 'V' && type != 'v' && type != 'N' && type != 'n') {
            printMessage("  Invalid ticket type!", COL_ERR);
            }
        } while (type != 'V' && type != 'v' && type != 'N' && type != 'n');
    return type;
}
// ---------------------------------------------------------------------------
// FullPass : books all 3 days in one go (Full Weekend Pass).
// ---------------------------------------------------------------------------
int FullPass(int seats[3][4], struct Booking *booking, struct Customer *cust) {
    setColor(COL_WARN);
    printf("\n>>> FRIDAY <<<");
    setColor(COL_DEFAULT);
    int enclosure_friday = selectEnclosure();
    setColor(COL_WARN);
    printf("\n>>> SATURDAY <<<");
    setColor(COL_DEFAULT);
    int enclosure_saturday = selectEnclosure();
    setColor(COL_WARN);
    printf("\n>>> SUNDAY <<<");
    setColor(COL_DEFAULT);
    int enclosure_sunday = selectEnclosure();
    int ticket_friday, ticket_saturday, ticket_sunday;
    printf("\n");
    do {
        ticket_friday   = getValidInt("How many tickets for Friday? ");
        ticket_saturday = getValidInt("How many tickets for Saturday? ");
        ticket_sunday   = getValidInt("How many tickets for Sunday? ");
        if (ticket_friday <= 0 || ticket_saturday <= 0 || ticket_sunday <= 0) {
            printMessage("  Please enter valid numbers greater than 0!", COL_ERR);
        }
    } while (ticket_friday <= 0 || ticket_saturday <= 0 || ticket_sunday <= 0);
    int seat_friday   = seats[0][enclosure_friday - 1];
    int seat_saturday = seats[1][enclosure_saturday - 1];
    int seat_sunday   = seats[2][enclosure_sunday - 1];
    if (seat_friday >= ticket_friday && seat_saturday >= ticket_saturday && seat_sunday >= ticket_sunday) {
        printMessage("\n  Seats available for all three days!", COL_OK);
        seats[0][enclosure_friday - 1]   -= ticket_friday;
        seats[1][enclosure_saturday - 1] -= ticket_saturday;
        seats[2][enclosure_sunday - 1]   -= ticket_sunday;
        booking->enclosure[0] = enclosure_friday;
        booking->tickets[0]   = ticket_friday;
        booking->cancelled[0] = 0;
        booking->enclosure[1] = enclosure_saturday;
        booking->tickets[1]   = ticket_saturday;
        booking->cancelled[1] = 0;
        booking->enclosure[2] = enclosure_sunday;
        booking->tickets[2]   = ticket_sunday;
        booking->cancelled[2] = 0;
        int total_tickets = ticket_friday + ticket_saturday + ticket_sunday;
        char type = getTicketType();
        return Bill(total_tickets, type, cust);
    }else {
        printMessage("\n  Sorry! Not enough seats for the requested days.", COL_ERR);
        return 0;
    }
}
// Helper function to scan the array matrix and verify if any seat remains unsold
int areSeatsFullyBooked(int seats[3][4]) {
    int i, j;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) {
            if (seats[i][j] > 0) {
            return 0;
            }
        }
    }
    return 1;
}
// ---------------------------------------------------------------------------
// generateBookingID : gives every booking a new unique ID.
// ---------------------------------------------------------------------------
int generateBookingID(void) {
    int id = 1000;
    FILE *fp = fopen("booking_id.txt", "r");
    if (fp != NULL) {
        if (fscanf(fp, "%d", &id) != 1) {
            id = 1000;
            }
        fclose(fp);
    }
    id++;
    fp = fopen("booking_id.txt", "w");
    if (fp == NULL) {
        printMessage("Error saving booking ID!", COL_ERR);
        return -1;
    }
    fprintf(fp, "%d", id);
    fclose(fp);
    return id;
}
// ---------------------------------------------------------------------------
// Bill : creates the bill for a booking.
// ---------------------------------------------------------------------------
int Bill(int ticket_quantity, char type, struct Customer *cust) {
    int i;
    char primary_name[50];
    int age;
    double price[2] = {275.00, 956.00};     // price[0] = Normal, price[1] = VIP
    double unit_price, discount;
    
    double total_before_discount = 0.00;    // sum of all ticket prices
    double total_discount_amount = 0.00;    // sum of all discounts given
    double grand_total = 0.00;              // amount the customer finally pays
    
    FILE *fp = fopen("bills.txt", "a");     // "a" = append, so old bills are kept
    
	if (fp == NULL) {
        printMessage("Error opening bill file!", COL_ERR);   // STYLE: red error
        return 0;
    }
    
    /////////////////////////////////////////////////// Booking ID function call and generation ///////////////////////////////////
	
	int booking_id = generateBookingID();   // get a fresh unique ID

	if (booking_id == -1) {                 // ID could not be saved -> stop
    	fclose(fp);
    	return 0;
	}
    
    
    // Assign base ticket price
    if (type == 'V' || type == 'v') {
        unit_price = price[1];              // VIP
    } else {
        unit_price = price[0];              // Normal
    }
    
    printBanner("ORDER CONTACT DETAILS", COL_TITLE);   // STYLE: boxed heading
    printPrompt("Enter primary customer name: ");
    scanf(" %49[^\n]", primary_name);       // reads a full name with spaces (max 49 chars)
    
    
    // Process age and individual discounts loop
    printBanner("PASSENGER AGE CONFIGURATION", COL_TITLE);   // STYLE: boxed heading
    for (i = 0; i < ticket_quantity; i++) {                  // one age per ticket
        char agePrompt[40];                                        // Styling 
        sprintf(agePrompt, "Enter age for person %d: ", i + 1);   // STYLE: build the prompt text so it can be coloured
        printPrompt(agePrompt);                                   // styling
        
        do {
            scanf("%d", &age);
            if (age < 0) {                                  // negative age is not possible
                printMessage("  Invalid age! Please enter a realistic age:", COL_ERR);   // STYLE: red error
            }
        } while (age < 0);
        
        // Calculate dynamic personal discount rate
        if (age <= 5) {
            discount = 1.00;        // 5 and under: free (100% off)
        } else if (age <= 18) {
            discount = 0.50;        // 6 - 18: half price
        } else if (age <= 60) {
            discount = 0.00;        // 19 - 60: full price
        } else {
            discount = 0.30;        // over 60: 30% off
        }
        
        // Accumulate cumulative values item by item
        total_before_discount += unit_price;                    // add this ticket's full price
        total_discount_amount += (unit_price * discount);       // add this ticket's discount
    }
    
    grand_total = total_before_discount - total_discount_amount;   // final amount to pay
    
    // STYLE: the receipt on screen is now a neat aligned box (printRow keeps the columns straight)
    char buf[40];                                      /// STYLE: temporary text for each value in the receipt
    printBanner("COMBINED BILL SLIP", COL_WARN);

    sprintf(buf, "%d", booking_id);
    printRow("BOOKING ID", buf, COL_DEFAULT);
    printRow("PRIMARY ACCOUNT", primary_name, COL_DEFAULT);
    printRow("TICKET CATEGORY", (type == 'V' || type == 'v') ? "V (VIP)" : "N (Normal)", COL_DEFAULT);
    sprintf(buf, "%d", ticket_quantity);
    printRow("TOTAL QUANTITY", buf, COL_DEFAULT);
    sprintf(buf, "%.2f", unit_price);
    printRow("BASE UNIT PRICE", buf, COL_DEFAULT);
    sprintf(buf, "%.2f", total_before_discount);
    printRow("SUBTOTAL COST", buf, COL_DEFAULT);
    sprintf(buf, "-%.2f", total_discount_amount);
    printRow("TOTAL SAVINGS", buf, COL_OK);            // green: money saved
    printBorder(COL_WARN);
    sprintf(buf, "%.2f", grand_total);
    printRow("GRAND TOTAL", buf, COL_WARN);            // yellow: final amount stands out
    printBorder(COL_WARN);
    printMessage("      == WELCOME TO RACE WEEKEND ==", COL_TITLE);
    
    // Save the same bill into bills.txt (plain text log)
    fprintf(fp, "========== COMBINED BILL SLIP ==========\n");
    fprintf(fp,"BOOKING ID: %d\n", booking_id);
    fprintf(fp, "PRIMARY ACCOUNT: %s\n", primary_name);
    fprintf(fp, "TICKET CATEGORY: %c\n", (type == 'V' || type == 'v') ? 'V' : 'N');
    fprintf(fp, "TOTAL QUANTITY : %d\n", ticket_quantity);
    fprintf(fp, "BASE UNIT PRICE: %.2f\n", unit_price);
    fprintf(fp, "SUBTOTAL COST  : %.2f\n", total_before_discount);
    fprintf(fp, "TOTAL SAVINGS  : -%.2f\n", total_discount_amount);
    fprintf(fp, "GRAND TOTAL    : %.2f\n", grand_total);
    fprintf(fp, "== WELCOME TO RACE WEEKEND ==\n\n");
    
    printMessage("\nBill processing finished! Record successfully logged to file.", COL_OK);   // STYLE: green success
    fclose(fp);                                        // close the file so everything is written

    // Hand the ID and name back so the caller can store them for sorting/searching
    cust->id = booking_id;
    strcpy(cust->name, primary_name);
    return 1;
}

// ---------------------------------------------------------------------------
// startBooking : the booking flow with recursive re-booking after cancellation
// ---------------------------------------------------------------------------
// startBooking : the original booking flow (day page -> enclosure -> tickets
//                -> bill), moved out of main() so it can be called again.
//
// RECURSION: when fromCancel == 1 the customer has just cancelled a booking.
// After they finish registering, we ask if they want to register another day
// and, if yes, the function CALLS ITSELF to take them back to the day page
// (Friday / Saturday / Sunday / Full Weekend Pass).
// ---------------------------------------------------------------------------

void startBooking(int seats[3][4], struct Customer records[], int *count, int fromCancel) {

    // Stop if the customer list is already full
    if (*count >= MAX_CUSTOMERS) {
        printMessage("\nBooking list is full! No more customers can be added.", COL_ERR);
        return;
    }

    struct Booking booking = {0};      // empty booking, everything starts at 0
    struct Customer cust = {0};        // empty customer, filled in by Bill()
    int saved = 0;                     // becomes 1 only if the booking fully succeeds

    int choice = ticket_booking();     // 1-3 = single day, 4 = full weekend pass

    if (choice == 4) {
        // Full pass: FullPass() handles everything and tells us if it worked
        saved = FullPass(seats, &booking, &cust);
    }
    else {
        // Single day: enclosure -> quantity -> seat check -> ticket type -> bill
        Sleep(3000);                   // wait 3 seconds
        system("cls");                 // clear the screen
        int enclosure = selectEnclosure();
        system("cls");
        int ticket_quantity = getTicketQuantity();
        int available = checkAvailibility(seats, choice, enclosure, ticket_quantity);

        if (available == 1) {          // seats were available (and already subtracted)

            // store this day's details (choice - 1 turns 1/2/3 into index 0/1/2)
            booking.enclosure[choice - 1] = enclosure;
            booking.tickets[choice - 1] = ticket_quantity;
            booking.cancelled[choice - 1] = 0;

            char type = getTicketType();
            saved = Bill(ticket_quantity, type, &cust);
        }
    }

    // Save the confirmed customer so it can be sorted / searched / cancelled later
    if (saved) {
        booking.booking_id = cust.id;
        cust.tickets = booking.tickets[0] + booking.tickets[1] + booking.tickets[2];   // total over all days
        cust.booking = booking;
        records[*count] = cust;        // put the customer in the next free slot
        (*count)++;                    // one more customer stored
    }

    // ---- Recursive part: only after a cancellation ----
    if (fromCancel == 1) {
        char again;
        printf("\n");
        printPrompt("Do you want to register another day? (Y/N): ");
        scanf(" %c", &again);
        if (again == 'Y' || again == 'y') {
            system("cls");
            startBooking(seats, records, count, 1);   // function calls itself -> back to the day page
        }
        // if the answer is not Y, nothing is called -> recursion stops (base case)
    }
}

// ---------------------------------------------------------------------------
// searchByID : linear search. Returns the position of the customer in the
//              list, or -1 if the ID does not exist.
// ---------------------------------------------------------------------------
int searchByID(struct Customer list[], int count, int id) {
    int i;
    for (i = 0; i < count; i++) {          // check customers one by one from the start
        if (list[i].id == id) {
            return i;                      // found -> return its position
        }
    }
    return -1;                             // went through everyone, no match
}

// ---------------------------------------------------------------------------
// searchBooking : asks for an ID, calls searchByID and prints the booking
// ---------------------------------------------------------------------------
void searchBooking(struct Customer list[], int count) {
    int d;
    int id;
    char buf[40];                                      // STYLE: temporary text for each row value
    printBanner("SEARCH BOOKING", COL_TITLE);          // STYLE: boxed heading
    printPrompt("Enter Booking ID: ");
    scanf("%d", &id);

    int pos = searchByID(list, count, id);             // position in the list, or -1
    if (pos == -1) {                                   // not found -> tell the user and stop
        sprintf(buf, "No booking found with ID %d.", id);
        printMessage(buf, COL_ERR);                    // STYLE: red "not found"
        return;
    }

    // STYLE: result shown as an aligned box, same look as the bill
    printMessage("\nBooking found!", COL_OK);
    printBorder(COL_OK);
    sprintf(buf, "%d", list[pos].id);
    printRow("BOOKING ID", buf, COL_DEFAULT);
    printRow("NAME", list[pos].name, COL_DEFAULT);
    sprintf(buf, "%d", list[pos].tickets);
    printRow("TICKETS", buf, COL_DEFAULT);
    for (d = 0; d < 3; d++) {                          // show only the days that were booked
        if (list[pos].booking.tickets[d] > 0) {
            sprintf(buf, "%d ticket(s) - %s", list[pos].booking.tickets[d],
                    enclosureNames[list[pos].booking.enclosure[d] - 1]);   // enclosure number -> name
            printRow(dayNames[d], buf, COL_TITLE);     // one coloured line per booked day
        }
    }
    printBorder(COL_OK);
}
// ---------------------------------------------------------------------------
// compareNames : compares two names ignoring upper/lower case.
//   returns negative -> a comes before b
//   returns 0        -> same name
//   returns positive -> a comes after b
// (Case-insensitive so "ali" and "Bilal" sort correctly)
// ---------------------------------------------------------------------------
int compareNames(const char *a, const char *b) {
    while (*a && *b) {                       // go letter by letter until one name ends
        int x = tolower((unsigned char)*a);  // make both letters lowercase
        int y = tolower((unsigned char)*b);
        if (x != y) {
            return x - y;                    // first different letter decides the order
        }
        a++;                                 // move to the next letter
        b++;
    }
    // one name ended: the shorter name comes first
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}
// ---------------------------------------------------------------------------
// sorting : sorts all customers A -> Z by name (bubble sort) and prints them
//   Bubble sort: compare neighbours and swap them if they are in the wrong
//   order. After each pass the "biggest" name settles at the end.
// ---------------------------------------------------------------------------
void sorting(struct Customer list[], int count) {
    int i, j;
    if (count == 0) {                                        // nothing to sort
        printMessage("\nNo bookings to sort yet.", COL_WARN);   // STYLE: yellow notice
        return;
    }

    for (i = 0; i < count - 1; i++) {                // outer loop = number of passes
        for (j = 0; j < count - i - 1; j++) {        // inner loop = compare neighbours (last i are already in place)
            if (compareNames(list[j].name, list[j + 1].name) > 0) {   // wrong order?
                struct Customer temp = list[j];      // swap the two customers using temp
                list[j] = list[j + 1];
                list[j + 1] = temp;
            }
        }
    }

    // STYLE: the sorted list is now a bordered table with a coloured header row
    printBanner("CUSTOMERS SORTED BY NAME", COL_TITLE);
    setColor(COL_WARN);
    printf("| %-10s | %-27.27s | %-7s |\n", "BOOKING ID", "NAME", "TICKETS");   // name column 27 wide so the table matches WIDTH 54
    setColor(COL_DEFAULT);
    printBorder(COL_TITLE);
    for (i = 0; i < count; i++) {                    // print every customer, now in A-Z order
        printf("| %-10d | %-27.27s | %-7d |\n", list[i].id, list[i].name, list[i].tickets);
    }
    printBorder(COL_TITLE);
}

// ---------------------------------------------------------------------------
// cancelBooking : cancels a booking by ID and gives the seats back.
//
// POLICY: a Full Weekend Pass is ONE package. It cannot be partly cancelled.
//   e.g. someone wants only Friday and Sunday -> the whole pass must be
//   cancelled first, then Friday and Sunday are registered again as
//   individual days.
// After cancelling, startBooking() is called to return to the day page.
// ---------------------------------------------------------------------------

void cancelBooking(int seats[3][4], struct Customer records[], int *count) {
    int d, i;
    int id;
    char confirm;
    char buf[80];                                      // STYLE: temporary text for messages

    printBanner("CANCEL BOOKING", COL_TITLE);          // STYLE: boxed heading
    printPrompt("Enter Booking ID: ");                     
    scanf("%d", &id);                                  	// Enter the ID for cancelling

    int pos = searchByID(records, *count, id);     	 	      // find the booking (reuses the search function)
    if (pos == -1) {									      // -1 means no booking has this ID
        sprintf(buf, "No booking found with ID %d.", id);     // print No booking found
        printMessage(buf, COL_ERR);                 
        return;
    }

    // Count the days that are still active to know if this is a Full Pass
    // (3 active days = full pass, fewer = single day booking)
    int activeDays = 0;
    for (d = 0; d < 3; d++) {
        if (records[pos].booking.tickets[d] > 0 && records[pos].booking.cancelled[d] == 0) {  
            activeDays++;
        }
    }


    // STYLE: policy notice shown in yellow so the customer notices it
    if (activeDays == 3) {                             // full pass -> show the policy
        printMessage("\nThis is a FULL WEEKEND PASS.", COL_WARN);
        printMessage("POLICY: A full pass cannot be partly cancelled. The whole pass will be", COL_WARN);
        printMessage("cancelled and you must register again for the individual days you want.", COL_WARN);
    } else {
        printMessage("\nThis is a single day booking.", COL_WARN);
    }

    // Ask for confirmation before cancelling anything
    sprintf(buf, "Do you really want to cancel booking %d (%s)? (Y/N): ", id, records[pos].name);
    printf("\n");
    printPrompt(buf);
    scanf(" %c", &confirm);
    if (confirm != 'Y' && confirm != 'y') {            // anything other than Y = abort
        printMessage("Cancellation aborted.", COL_WARN);
        return;
    }

    // Give the seats back for every active day
    for (d = 0; d < 3; d++) {
        if (records[pos].booking.tickets[d] > 0 && records[pos].booking.cancelled[d] == 0) {
            seats[d][records[pos].booking.enclosure[d] - 1] += records[pos].booking.tickets[d];   // add the seats back (booking did -=)
            records[pos].booking.cancelled[d] = 1;     // mark that day as cancelled
        }
    }
    sprintf(buf, "\nBooking %d cancelled. Seats have been released.", id);
    printMessage(buf, COL_OK);                      // STYLE: green success

    // Remove the customer from the list (shift the rest one step left)
    // e.g. A B C D, remove B -> A C D  (each customer after B moves one place back)
    for (i = pos; i < *count - 1; i++) {
        records[i] = records[i + 1];
    }
    (*count)--;                                        // one customer fewer

    // Ask if the customer wants to book again; if yes, go back to the day page
    printf("\n");
    printPrompt("Do you want to register for a day now? (Y/N): ");
    scanf(" %c", &confirm);
    if (confirm == 'Y' || confirm == 'y') {
        system("cls");
        startBooking(seats, records, count, 1);     // back to Friday / Saturday / Sunday page (1 = came from a cancel)
    }
}
// ---------------------------------------------------------------------------
// saveData : writes the seat table and all customer records to "bookings.dat"
//            (binary file). Called after every booking / cancellation.
//            File layout: [count][seats table][count x Customer]
// ---------------------------------------------------------------------------
void saveData(int seats[3][4], struct Customer records[], int count) {
    FILE *fp = fopen("bookings.dat", "wb");                // "wb" = write binary (replaces old file)
    if (fp == NULL) {                                      // file could not be opened
        printMessage("Warning: could not save booking data!", COL_ERR);
        return;
    }
    fwrite(&count, sizeof(int), 1, fp);                    // 1) how many customers
    fwrite(seats, sizeof(int), 12, fp);                    // 2) seat table: 3 days x 4 enclosures
    fwrite(records, sizeof(struct Customer), count, fp);   // 3) only the customers that exist
    fclose(fp);
}

// ---------------------------------------------------------------------------
// loadData : reads "bookings.dat" back when the program starts, so searching
//            and cancelling work on bookings from earlier runs.
//            Returns 1 if data was loaded, 0 if there is no (valid) file yet.
// ---------------------------------------------------------------------------
int loadData(int seats[3][4], struct Customer records[], int *count) {
    int savedCount;
    int savedSeats[3][4];                                  // temporary copy, so a bad file can't damage real data
    FILE *fp = fopen("bookings.dat", "rb");                // "rb" = read binary
    if (fp == NULL) {
        return 0;                                          // first run: nothing saved yet
    }
    // Read in the same order it was saved; if any read fails or the count
    // is impossible, treat the file as damaged
    if (fread(&savedCount, sizeof(int), 1, fp) != 1 ||
        savedCount < 0 || savedCount > MAX_CUSTOMERS ||
        fread(savedSeats, sizeof(int), 12, fp) != 12 ||
        fread(records, sizeof(struct Customer), savedCount, fp) != (size_t)savedCount) {
        fclose(fp);
        return 0;                                          // damaged file: start fresh
    }
    fclose(fp);
    memcpy(seats, savedSeats, sizeof(savedSeats));         // everything was fine -> copy seats into the real table
    *count = savedCount;                                   // update main's customer count
    return 1;
}

// ---------------------------------------------------------------------------
// showMenu : prints the main menu options
// ---------------------------------------------------------------------------
void showMenu(void) {
    // STYLE: main menu is a boxed banner with numbered options
    printBanner("MAIN MENU", COL_WARN);
    printf("   [1] Book Tickets\n");
    printf("   [2] Cancel Booking\n");
    printf("   [3] Search Booking by ID\n");
    printf("   [4] Sort Customers by Name\n");
    printf("   [5] Exit\n\n");
}
// ================= MAIN FUNCTION =================
int main() {
    system("cls");
    Welcome_scr();
    int seats[3][4] = {
        {100, 80, 60, 20},                         // free seats: seats[day][enclosure]
        {100, 80, 60, 20},                         // Friday, Saturday, Sunday    (General, Grandstand, Premium, VIP)
        {100, 80, 60, 20}                              
    };
    struct Customer records[MAX_CUSTOMERS];        // all confirmed customers
    int count = 0;                                 // how many customers are stored
    int menu;
    loadData(seats, records, &count);              // bring back bookings + seats saved by earlier runs
    Sleep(3000);
    system("cls");
    do {                                           // keep showing the menu until the user picks Exit (5)
        showMenu();
        menu = getValidInt("Enter your choice: ");
        switch (menu) {
            case 1:
                system("cls");
                startBooking(seats, records, &count, 0);      // book (0 = normal start, not from a cancel), then save
                saveData(seats, records, count);
                break;
            case 2:                                           // cancel, then save
                cancelBooking(seats, records, &count);
                saveData(seats, records, count);
                break;
            case 3:                                          // search by ID
                searchBooking(records, count);
                break;
            case 4:                                         // sort by name
                sorting(records, count);
                break; 
            case 5:
                printMessage("\nThank you! See you at the race weekend!", COL_OK);
                break;
            default :
                printMessage("Invalid choice! Try again.", COL_ERR);
        }
    } while (menu != 5);
    setColor(COL_DEFAULT);
    return 0;
}

