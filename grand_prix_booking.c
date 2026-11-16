#include <stdio.h>      // printf, scanf, file functions (fopen, fprintf ...)
#include <stdlib.h>     // system("cls"), malloc, calloc, free
#include <string.h>     // strlen, strcpy, memcpy
#include <ctype.h>      // tolower (used when comparing names)
#include <windows.h>    // Sleep() and console colours (Windows only)

#define INITIAL_CAPACITY 10 // starting size for dynamic customer array (grows automatically)

///////////////////// STYLE SETTINGS /////////////////////
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
//////////////////////////////////////////////////////////

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

// Dynamic memory & core functions
int  ensureCapacity(struct Customer **records, int count, int *capacity, int requiredCount);
void startBooking(int seats[3][4], struct Customer **records, int *count, int *capacity, int fromCancel);
void cancelBooking(int seats[3][4], struct Customer **records, int *count, int *capacity);
int  searchByID(struct Customer list[], int count, int id);
void searchBooking(struct Customer list[], int count);
void sorting(struct Customer list[], int count);
void showMenu(void);

// Data saving / loading - keeps bookings and seats after the program is closed
void saveData(int seats[3][4], struct Customer records[], int count);
int  loadData(int seats[3][4], struct Customer **records, int *count, int *capacity);

// STYLE & INPUT helper declarations
void setColor(int color);
void printBorder(int color);
void printBanner(const char *title, int color);
void printRow(const char *label, const char *value, int color);
void printPrompt(const char *text);
void printMessage(const char *text, int color);
int  getValidInt(const char *prompt);


///////////////////////////////////////////////////////////////////////////
//                    DYNAMIC MEMORY HELPER FUNCTION
///////////////////////////////////////////////////////////////////////////

// Expands the dynamic Customer array using calloc + memcpy + free when needed
int ensureCapacity(struct Customer **records, int count, int *capacity, int requiredCount) {
    if (requiredCount <= *capacity) {
        return 1; // Current capacity is already enough
    }

    int newCapacity = (*capacity > 0) ? *capacity : INITIAL_CAPACITY;
    while (newCapacity < requiredCount) {
        newCapacity *= 2;
    }

    // Allocate a new zero-initialized block using calloc
    struct Customer *newBlock = (struct Customer *)calloc(newCapacity, sizeof(struct Customer));
    if (newBlock == NULL) {
        printMessage("Memory allocation failed! Cannot store more customers.", COL_ERR);
        return 0;
    }

    // Copy existing customer records into the new block and free the old one
    if (*records != NULL && count > 0) {
        memcpy(newBlock, *records, count * sizeof(struct Customer));
    }
    free(*records);

    *records = newBlock;
    *capacity = newCapacity;
    return 1;
}


///////////////////////////////////////////////////////////////////////////
//                    STYLE & INPUT HELPER FUNCTIONS
///////////////////////////////////////////////////////////////////////////

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

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

void printBanner(const char *title, int color) {
    int len = (int)strlen(title);
    int left = (WIDTH - 2 - len) / 2;
    int right = WIDTH - 2 - len - left;

    printf("\n");
    printBorder(color);
    setColor(color);
    printf("|%*s%s%*s|\n", left, "", title, right, "");
    setColor(COL_DEFAULT);
    printBorder(color);
}

void printRow(const char *label, const char *value, int color) {
    setColor(color);
    printf("| %-16s: %-*.*s |\n", label, WIDTH - 22, WIDTH - 22, value);
    setColor(COL_DEFAULT);
}

void printPrompt(const char *text) {
    setColor(COL_PROMPT);
    printf("  > %s", text);
    setColor(COL_DEFAULT);
}

void printMessage(const char *text, int color) {
    setColor(color);
    printf("%s\n", text);
    setColor(COL_DEFAULT);
}

int getValidInt(const char *prompt) {
    int value;
    int status;
    while (1) {
        printPrompt(prompt);
        status = scanf("%d", &value);

        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);

        if (status == 1) {
            return value;
        }
        printMessage("  Invalid input! Please enter a valid number.", COL_ERR);
    }
}


///////////////////////////////////////////////////////////////////////////
//                           CORE SYSTEM FUNCTIONS
///////////////////////////////////////////////////////////////////////////

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

char getTicketType(void) {
    char type;
    int ch;
    do {
        printf("\n");
        printPrompt("Enter ticket type (V for VIP or N for Normal): ");
        scanf(" %c", &type);
        while ((ch = getchar()) != '\n' && ch != EOF);
        if (type != 'V' && type != 'v' && type != 'N' && type != 'n') {
            printMessage("  Invalid ticket type!", COL_ERR);
        }
    } while (type != 'V' && type != 'v' && type != 'N' && type != 'n');
    return type;
}

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
    } else {
        printMessage("\n  Sorry! Not enough seats for the requested days.", COL_ERR);
        return 0;
    }
}

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

int Bill(int ticket_quantity, char type, struct Customer *cust) {
    int i;
    char primary_name[50];
    int age;
    double price[2] = {275.00, 956.00};
    double unit_price, discount;

    double total_before_discount = 0.00;
    double total_discount_amount = 0.00;
    double grand_total = 0.00;

    FILE *fp = fopen("bills.txt", "a");

    if (fp == NULL) {
        printMessage("Error opening bill file!", COL_ERR);
        return 0;
    }

    int booking_id = generateBookingID();

    if (booking_id == -1) {
        fclose(fp);
        return 0;
    }

    if (type == 'V' || type == 'v') {
        unit_price = price[1];
    } else {
        unit_price = price[0];
    }

    printBanner("ORDER CONTACT DETAILS", COL_TITLE);
    printPrompt("Enter primary customer name: ");
    scanf(" %49[^\n]", primary_name);

    printBanner("PASSENGER AGE CONFIGURATION", COL_TITLE);
    for (i = 0; i < ticket_quantity; i++) {
        char agePrompt[40];
        sprintf(agePrompt, "Enter age for person %d: ", i + 1);
        printPrompt(agePrompt);

        do {
            scanf("%d", &age);
            if (age < 0) {
                printMessage("  Invalid age! Please enter a realistic age:", COL_ERR);
            }
        } while (age < 0);

        if (age <= 5) {
            discount = 1.00;
        } else if (age <= 18) {
            discount = 0.50;
        } else if (age <= 60) {
            discount = 0.00;
        } else {
            discount = 0.30;
        }

        total_before_discount += unit_price;
        total_discount_amount += (unit_price * discount);
    }

    grand_total = total_before_discount - total_discount_amount;

    char buf[40];
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
    printRow("TOTAL SAVINGS", buf, COL_OK);
    printBorder(COL_WARN);
    sprintf(buf, "%.2f", grand_total);
    printRow("GRAND TOTAL", buf, COL_WARN);
    printBorder(COL_WARN);
    printMessage("      == WELCOME TO RACE WEEKEND ==", COL_TITLE);

    fprintf(fp, "========== COMBINED BILL SLIP ==========\n");
    fprintf(fp, "BOOKING ID: %d\n", booking_id);
    fprintf(fp, "PRIMARY ACCOUNT: %s\n", primary_name);
    fprintf(fp, "TICKET CATEGORY: %c\n", (type == 'V' || type == 'v') ? 'V' : 'N');
    fprintf(fp, "TOTAL QUANTITY : %d\n", ticket_quantity);
    fprintf(fp, "BASE UNIT PRICE: %.2f\n", unit_price);
    fprintf(fp, "SUBTOTAL COST  : %.2f\n", total_before_discount);
    fprintf(fp, "TOTAL SAVINGS  : -%.2f\n", total_discount_amount);
    fprintf(fp, "GRAND TOTAL    : %.2f\n", grand_total);
    fprintf(fp, "== WELCOME TO RACE WEEKEND ==\n\n");

    printMessage("\nBill processing finished! Record successfully logged to file.", COL_OK);
    fclose(fp);

    cust->id = booking_id;
    strcpy(cust->name, primary_name);
    return 1;
}

void startBooking(int seats[3][4], struct Customer **records, int *count, int *capacity, int fromCancel) {

    // Grow dynamic capacity using calloc if full
    if (!ensureCapacity(records, *count, capacity, *count + 1)) {
        return;
    }

    struct Booking booking = {0};
    struct Customer cust = {0};
    int saved = 0;

    int choice = ticket_booking();

    if (choice == 4) {
        saved = FullPass(seats, &booking, &cust);
    }
    else {
        Sleep(3000);
        system("cls");
        int enclosure = selectEnclosure();
        system("cls");
        int ticket_quantity = getTicketQuantity();
        int available = checkAvailibility(seats, choice, enclosure, ticket_quantity);

        if (available == 1) {
            booking.enclosure[choice - 1] = enclosure;
            booking.tickets[choice - 1] = ticket_quantity;
            booking.cancelled[choice - 1] = 0;

            char type = getTicketType();
            saved = Bill(ticket_quantity, type, &cust);
        }
    }

    if (saved) {
        booking.booking_id = cust.id;
        cust.tickets = booking.tickets[0] + booking.tickets[1] + booking.tickets[2];
        cust.booking = booking;
        (*records)[*count] = cust;
        (*count)++;
    }

    if (fromCancel == 1) {
        char again;
        printf("\n");
        printPrompt("Do you want to register another day? (Y/N): ");
        scanf(" %c", &again);
        if (again == 'Y' || again == 'y') {
            system("cls");
            startBooking(seats, records, count, capacity, 1);
        }
    }
}

int searchByID(struct Customer list[], int count, int id) {
    int i;
    for (i = 0; i < count; i++) {
        if (list[i].id == id) {
            return i;
        }
    }
    return -1;
}

void searchBooking(struct Customer list[], int count) {
    int d;
    int id;
    char buf[40];
    printBanner("SEARCH BOOKING", COL_TITLE);
    printPrompt("Enter Booking ID: ");
    scanf("%d", &id);

    int pos = searchByID(list, count, id);
    if (pos == -1) {
        sprintf(buf, "No booking found with ID %d.", id);
        printMessage(buf, COL_ERR);
        return;
    }

    printMessage("\nBooking found!", COL_OK);
    printBorder(COL_OK);
    sprintf(buf, "%d", list[pos].id);
    printRow("BOOKING ID", buf, COL_DEFAULT);
    printRow("NAME", list[pos].name, COL_DEFAULT);
    sprintf(buf, "%d", list[pos].tickets);
    printRow("TICKETS", buf, COL_DEFAULT);
    for (d = 0; d < 3; d++) {
        if (list[pos].booking.tickets[d] > 0) {
            sprintf(buf, "%d ticket(s) - %s", list[pos].booking.tickets[d],
                    enclosureNames[list[pos].booking.enclosure[d] - 1]);
            printRow(dayNames[d], buf, COL_TITLE);
        }
    }
    printBorder(COL_OK);
}

int compareNames(const char *a, const char *b) {
    while (*a && *b) {
        int x = tolower((unsigned char)*a);
        int y = tolower((unsigned char)*b);
        if (x != y) {
            return x - y;
        }
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

void sorting(struct Customer list[], int count) {
    int i, j;
    if (count == 0) {
        printMessage("\nNo bookings to sort yet.", COL_WARN);
        return;
    }

    for (i = 0; i < count - 1; i++) {
        for (j = 0; j < count - i - 1; j++) {
            if (compareNames(list[j].name, list[j + 1].name) > 0) {
                struct Customer temp = list[j];
                list[j] = list[j + 1];
                list[j + 1] = temp;
            }
        }
    }

    printBanner("CUSTOMERS SORTED BY NAME", COL_TITLE);
    setColor(COL_WARN);
    printf("| %-10s | %-27.27s | %-7s |\n", "BOOKING ID", "NAME", "TICKETS");
    setColor(COL_DEFAULT);
    printBorder(COL_TITLE);
    for (i = 0; i < count; i++) {
        printf("| %-10d | %-27.27s | %-7d |\n", list[i].id, list[i].name, list[i].tickets);
    }
    printBorder(COL_TITLE);
}

void cancelBooking(int seats[3][4], struct Customer **records, int *count, int *capacity) {
    int d, i;
    int id;
    char confirm;
    char buf[80];

    printBanner("CANCEL BOOKING", COL_TITLE);
    printPrompt("Enter Booking ID: ");
    scanf("%d", &id);

    int pos = searchByID(*records, *count, id);
    if (pos == -1) {
        sprintf(buf, "No booking found with ID %d.", id);
        printMessage(buf, COL_ERR);
        return;
    }

    int activeDays = 0;
    for (d = 0; d < 3; d++) {
        if ((*records)[pos].booking.tickets[d] > 0 && (*records)[pos].booking.cancelled[d] == 0) {
            activeDays++;
        }
    }

    if (activeDays == 3) {
        printMessage("\nThis is a FULL WEEKEND PASS.", COL_WARN);
        printMessage("POLICY: A full pass cannot be partly cancelled. The whole pass will be", COL_WARN);
        printMessage("cancelled and you must register again for the individual days you want.", COL_WARN);
    } else {
        printMessage("\nThis is a single day booking.", COL_WARN);
    }

    sprintf(buf, "Do you really want to cancel booking %d (%s)? (Y/N): ", id, (*records)[pos].name);
    printf("\n");
    printPrompt(buf);
    scanf(" %c", &confirm);
    if (confirm != 'Y' && confirm != 'y') {
        printMessage("Cancellation aborted.", COL_WARN);
        return;
    }

    for (d = 0; d < 3; d++) {
        if ((*records)[pos].booking.tickets[d] > 0 && (*records)[pos].booking.cancelled[d] == 0) {
            seats[d][(*records)[pos].booking.enclosure[d] - 1] += (*records)[pos].booking.tickets[d];
            (*records)[pos].booking.cancelled[d] = 1;
        }
    }
    sprintf(buf, "\nBooking %d cancelled. Seats have been released.", id);
    printMessage(buf, COL_OK);

    for (i = pos; i < *count - 1; i++) {
        (*records)[i] = (*records)[i + 1];
    }
    (*count)--;

    printf("\n");
    printPrompt("Do you want to register for a day now? (Y/N): ");
    scanf(" %c", &confirm);
    if (confirm == 'Y' || confirm == 'y') {
        system("cls");
        startBooking(seats, records, count, capacity, 1);
    }
}

void saveData(int seats[3][4], struct Customer records[], int count) {
    FILE *fp = fopen("bookings.dat", "wb");
    if (fp == NULL) {
        printMessage("Warning: could not save booking data!", COL_ERR);
        return;
    }
    fwrite(&count, sizeof(int), 1, fp);
    fwrite(seats, sizeof(int), 12, fp);
    fwrite(records, sizeof(struct Customer), count, fp);
    fclose(fp);
}

// ---------------------------------------------------------------------------
// loadData : uses malloc to stage records safely from file before copying
// ---------------------------------------------------------------------------
int loadData(int seats[3][4], struct Customer **records, int *count, int *capacity) {
    int savedCount;
    int savedSeats[3][4];
    FILE *fp = fopen("bookings.dat", "rb");
    if (fp == NULL) {
        return 0;
    }

    if (fread(&savedCount, sizeof(int), 1, fp) != 1 ||
        savedCount < 0 ||
        fread(savedSeats, sizeof(int), 12, fp) != 12) {
        fclose(fp);
        return 0;
    }

    if (savedCount == 0) {
        fclose(fp);
        memcpy(seats, savedSeats, sizeof(savedSeats));
        *count = 0;
        return 1;
    }

    // Allocate a temporary buffer using malloc to read from file safely
    struct Customer *tempRecords = (struct Customer *)malloc(savedCount * sizeof(struct Customer));
    if (tempRecords == NULL) {
        fclose(fp);
        return 0;
    }

    if (fread(tempRecords, sizeof(struct Customer), savedCount, fp) != (size_t)savedCount) {
        free(tempRecords);
        fclose(fp);
        return 0;
    }
    fclose(fp);

    // Ensure main records array has enough capacity, then copy
    if (!ensureCapacity(records, *count, capacity, savedCount)) {
        free(tempRecords);
        return 0;
    }

    memcpy(*records, tempRecords, savedCount * sizeof(struct Customer));
    free(tempRecords);

    memcpy(seats, savedSeats, sizeof(savedSeats));
    *count = savedCount;
    return 1;
}

void showMenu(void) {
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

    // Allocate the 3x4 seats grid dynamically using malloc
    int (*seats)[4] = (int (*)[4])malloc(3 * sizeof(*seats));
    if (seats == NULL) {
        printMessage("Fatal Error: Seat table allocation failed!", COL_ERR);
        return 1;
    }
    int initialSeats[3][4] = {
        {100, 80, 60, 20},
        {100, 80, 60, 20},
        {100, 80, 60, 20}
    };
    memcpy(seats, initialSeats, sizeof(initialSeats));

    // Allocate zero-initialized customer records using calloc
    int capacity = INITIAL_CAPACITY;
    struct Customer *records = (struct Customer *)calloc(capacity, sizeof(struct Customer));
    if (records == NULL) {
        free(seats);
        printMessage("Fatal Error: Initial customer memory allocation failed!", COL_ERR);
        return 1;
    }

    int count = 0;
    int menu;
    loadData(seats, &records, &count, &capacity);
    Sleep(3000);
    system("cls");
    do {
        showMenu();
        menu = getValidInt("Enter your choice: ");
        switch (menu) {
            case 1:
                system("cls");
                startBooking(seats, &records, &count, &capacity, 0);
                saveData(seats, records, count);
                break;
            case 2:
                cancelBooking(seats, &records, &count, &capacity);
                saveData(seats, records, count);
                break;
            case 3:
                searchBooking(records, count);
                break;
            case 4:
                sorting(records, count);
                break;
            case 5:
                printMessage("\nThank you! See you at the race weekend!", COL_OK);
                break;
            default:
                printMessage("Invalid choice! Try again.", COL_ERR);
        }
    } while (menu != 5);

    // Free all dynamically allocated memory
    free(records);
    free(seats);
    setColor(COL_DEFAULT);
    return 0;
}
