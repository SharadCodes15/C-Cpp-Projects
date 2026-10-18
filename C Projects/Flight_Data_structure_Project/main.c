#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define FLTNUM_LEN 10
#define TEXT_LEN 100
#define NAME_LEN 50
#define CONTACT_LEN 15
#define SENIOR_AGE 60
#define LINE 1024
#define DEFAULT_DB "DSPD_2_BT25CSE056_DEMO_data.txt"

typedef enum
{
    SCHEDULED = 0,
    CHECKIN_OPEN,
    BOARDING,
    DEPARTED,
    CANCELLED
} flightStatus;

//  Ticket type smaller number = higher priority
typedef enum
{
    FIRST_CLASS = 1,
    BUSINESS,
    ECONOMY
} ticketType;

// ---- - - Structs
typedef struct Flight
{
    int flightId;
    char flightNumber[FLTNUM_LEN];
    char source[TEXT_LEN];
    char destination[TEXT_LEN];

    int departureTime; // HHMM
    int arrivalTime;

    int totalSeats;
    int availableSeats;
    flightStatus status;
    struct Flight *prev;
    struct Flight *next;
} Flight;

typedef struct Passenger
{
    int passengerId;
    char name[NAME_LEN];
    int age;
    char gender; // M F O
    char contact[CONTACT_LEN];

    int flightId; // Ref
    int seatNumber;
    ticketType TicketType;
    int checkedIn; // 0 = no

    int boarded;

    struct Passenger *prev;
    struct Passenger *next;
} Passenger;

typedef struct CheckinNode
{
    int passengerId;
    Passenger *passenger; // ref
    struct CheckinNode *next;
} CheckinNode;

typedef struct BoardNode
{
    int passengerId;
    Passenger *passenger; // ref
    int priority;         // ticket type
    struct BoardNode *next;
} BoardNode;

typedef struct HistoryNode
{
    int passengerId;
    char passengerName[NAME_LEN];
    int flightId;
    int seatNumber;
    char boardingTime[TEXT_LEN];
    struct HistoryNode *next;
} HistoryNode;

Flight *flightHead = NULL;
Passenger *passHead = NULL;
// Check in
CheckinNode *ciFront = NULL;
CheckinNode *ciRear = NULL;
// Boarding queue
BoardNode *bqFront = NULL;
HistoryNode *HistoryTop = NULL;

// ================ helper Funct

// copy text to array
void copyText(char *dest, char *src, int size)
{
    int i = 0;
    while (i < size - 1 && src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

int compareText(char *a, char *b)
{

    int i = 0;
    while (a[i] != '\0' && b[i] != '\0')
    {
        if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i]))
        {
            return tolower((unsigned char)a[i]) - tolower((unsigned char)b[i]);
        }
        i++;
    }
    return tolower((unsigned char)a[i]) - tolower((unsigned char)b[i]);
}

void replaceUnderscore(char *s)
{
    int i;
    for (i = 0; s[i] != '\0'; i++)
    {
        if (s[i] == '_')
        {
            s[i] = ' ';
        }
    }
}

int readInt(char *prompt)
{
    char line[100];
    int value;
    while (1)
    {
        printf("%s", prompt);
        if (fgets(line, sizeof(line), stdin) == NULL)
        {
            exit(0);
        }
        if (sscanf(line, "%d", &value) == 1)
        {
            return value;
        }
        printf("Invalid input, Enter the number(only)\n");
    }
}

void readString(char *prompt, char *dest, int size)
{
    char line[200];
    while (1)
    {
        printf("%s", prompt);
        if (fgets(line, sizeof(line), stdin) == NULL)
        {
            exit(0);
        }
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) > 0)
        {
            break;
        }
        printf("Input cant be empty\n");
    }
    copyText(dest, line, size);
}

char readChar(char *prompt)
{
    char line[200];
    readString(prompt, line, 200);
    return line[0];
}

int isValidTime(int t)
{
    int maxt = 2359;
    return (t >= 0 && t <= maxt && (t % 100) < 60);
}

char *statusName(int s) // flightstatus replaced with int
{
    switch (s)
    {
    case SCHEDULED:
        return "Scheduled";
    case CHECKIN_OPEN:
        return "Check in Open";
    case BOARDING:
        return "Boarding";
    case DEPARTED:
        return "Departed";
    case CANCELLED:
        return "Cancelled";
    default:
        return "Unknown";
    }
}

char *ticketName(int t) // same above
{
    switch (t)
    {
    case ECONOMY:
        return "Economy";
    case BUSINESS:
        return "Business";
    case FIRST_CLASS:
        return "First Class";
    default:
        return "Unknown";
    }
}

// ===== Merge sort functions ====

int compareFlights(Flight *a, Flight *b, int mode); // Forward declaratoin
int comparePassengers(Passenger *a, Passenger *b)
{
    if (a->TicketType != b->TicketType)
    {
        return a->TicketType - b->TicketType;
    }
    return a->seatNumber - b->seatNumber;
}

void mergePassengers(Passenger **arr, Passenger **tmp, int left, int mid, int right)
{
    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right)
    {
        if (comparePassengers(arr[i], arr[j]) <= 0)
        {
            tmp[k++] = arr[i++];
        }
        else
        {
            tmp[k++] = arr[j++];
        }
    }
    while (i <= mid)
    {
        tmp[k++] = arr[i++];
    }
    while (j <= right)
    {
        tmp[k++] = arr[j++];
    }
    for (k = left; k <= right; k++)
    {
        arr[k] = tmp[k];
    }
}
void mergeSortPassengersRec(Passenger **arr, Passenger **tmp, int left, int right)
{
    if (left >= right)
        return;
    int mid = (left + right) / 2;
    mergeSortPassengersRec(arr, tmp, left, mid);
    mergeSortPassengersRec(arr, tmp, mid + 1, right);
    mergePassengers(arr, tmp, left, mid, right);
}
int mergeSortPassengers(Passenger **arr, int n)
{
    Passenger **tmp = (Passenger **)malloc(n * sizeof(Passenger *));
    if (tmp == NULL)
    {
        printf("Memory error occured \n");
        return 0;
    }
    mergeSortPassengersRec(arr, tmp, 0, n - 1);
    free(tmp);
    return 1;
}

void mergeFlights(Flight **arr, Flight **temp, int left, int mid, int right, int mode)
{
    int i = left;
    int j = mid + 1;
    int k = left;

    while (i <= mid && j <= right)
    {
        if (compareFlights(arr[i], arr[j], mode) <= 0)
        {
            temp[k++] = arr[i++];
        }
        else
        {
            temp[k++] = arr[j++];
        }
    }

    while (i <= mid)
    {
        temp[k++] = arr[i++];
    }

    while (j <= right)
    {
        temp[k++] = arr[j++];
    }

    for (i = left; i <= right; i++)
    {
        arr[i] = temp[i];
    }
}

void mergeSortFlightsRec(Flight **arr, Flight **temp, int left, int right, int mode)
{
    if (left >= right)
    {
        return;
    }

    int mid = (left + right) / 2;
    mergeSortFlightsRec(arr, temp, left, mid, mode);
    mergeSortFlightsRec(arr, temp, mid + 1, right, mode);
    mergeFlights(arr, temp, left, mid, right, mode);
}

int mergeSortFlights(Flight **arr, int n, int mode)
{
    Flight **temp = (Flight **)malloc(n * sizeof(Flight *));
    if (temp == NULL)
    {
        printf("Memory error occured \n");
        return 0;
    }
    mergeSortFlightsRec(arr, temp, 0, n - 1, mode);
    free(temp);
    return 1;
}

// ========= Find and count functions ===========
Flight *findFlight(int id)
{
    Flight *f = flightHead;
    while (f != NULL)
    {
        if (f->flightId == id)
        {
            return f;
        }
        f = f->next;
    }
    return NULL;
}

Passenger *findPassenger(int id)
{
    Passenger *p = passHead;
    while (p != NULL)
    {
        if (p->passengerId == id)
        {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

// Num of passengers in flight
int countPassengers(int fid)
{
    int c = 0;
    Passenger *p = passHead;
    while (p != NULL)
    {
        if (p->flightId == fid)
        {
            c++;
        }
        p = p->next;
    }
    return c;
}

int countCheckedIn(int fid)
{
    int c = 0;
    Passenger *p = passHead;
    while (p != NULL)
    {
        if (p->flightId == fid && p->checkedIn == 1)
        {
            c++;
        }
        p = p->next;
    }
    return c;
}
int countBoarded(int fid)
{
    int c = 0;
    Passenger *p = passHead;
    while (p != NULL)
    {
        if (p->flightId == fid && p->boarded == 1)
        {
            c++;
        }
        p = p->next;
    }
    return c;
}
int countByTicket(int fid, int ticket)
{
    int c = 0;
    Passenger *p = passHead;
    while (p != NULL)
    {
        if (p->flightId == fid && (int)p->TicketType == ticket)
        {
            c++;
        }
        p = p->next;
    }
    return c;
}

int countSenior(int fid)
{
    int c = 0;
    Passenger *p = passHead;
    while (p != NULL)
    {
        if (p->flightId == fid && p->age >= SENIOR_AGE)
        {
            c++;
        }
        p = p->next;
    }
    return c;
}

int destinationDemand(char *dest)
{
    int total = 0;
    Flight *f = flightHead;
    while (f != NULL)
    {
        if (compareText(f->destination, dest) == 0)
        {
            total = total + countPassengers(f->flightId);
        }
        f = f->next;
    }
    return total;
}

int destinationFirstClass(char *dest)
{
    int total = 0;
    Flight *f = flightHead;
    while (f != NULL)
    {
        if (compareText(f->destination, dest) == 0)
        {
            total = total + countByTicket(f->flightId, FIRST_CLASS);
        }
        f = f->next;
    }
    return total;
}

int seatTaken(int fid, int seat)
{
    Passenger *p = passHead;
    while (p != NULL)
    {
        if (p->flightId == fid && p->seatNumber == seat)
        {
            return 1;
        }
        p = p->next;
    }
    return 0;
}

int allocateSeat(Flight *f)
{
    int s;
    for (s = 1; s <= f->totalSeats; s++)
    {
        if (seatTaken(f->flightId, s) == 0)
        {
            return s;
        }
    }
    return 0;
}

// ======== Stack Queue =====================
void enqueueCheckin(Passenger *p)
{
    CheckinNode *n = (CheckinNode *)malloc(sizeof(CheckinNode));
    if (n == NULL)
    {
        printf("Memory Error  \n");
        return;
    }
    n->passengerId = p->passengerId;
    n->passenger = p;
    n->next = NULL;
    if (ciRear == NULL)
    {
        ciFront = n;
        ciRear = n;
    }
    else
    {
        ciRear->next = n;
        ciRear = n;
    }
}

// remove passg from check in queue
void removeFromCheckinQueue(int pid)
{
    int deleted = 0;
    CheckinNode *cur = ciFront;
    CheckinNode *prev = NULL;
    while (cur != NULL)
    {
        /* code */
        if (cur->passengerId == pid)
        {
            if (prev != NULL)
            {
                prev->next = cur->next;
            }
            else
            {
                ciFront = cur->next;
            }
            if (cur == ciRear)
            {
                ciRear = prev;
            }
            free(cur);
            deleted = 1;
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    if (deleted == 0)
    {
        printf("Passenger %d not found in checkin queue \n", pid);
    }
}

// ------ Boarding queue
// insert by priority

void enqueueBoarding(Passenger *p)
{
    BoardNode *n = (BoardNode *)malloc(sizeof(BoardNode));
    BoardNode *cur;
    if (n == NULL)
    {
        printf("Memory error\n");
        return;
    }
    n->passengerId = p->passengerId;
    n->passenger = p;
    n->priority = p->TicketType;
    n->next = NULL;
    if (bqFront == NULL || n->priority < bqFront->priority)
    {
        n->next = bqFront;
        bqFront = n;
    }
    else
    {
        cur = bqFront;
        while (cur->next != NULL && cur->next->priority <= n->priority)
        {
            /* code */
            cur = cur->next;
        }
        n->next = cur->next;
        cur->next = n;
    }
}

void removeFromBoardingQueue(int pid)
{
    BoardNode *cur = bqFront;
    BoardNode *prev = NULL;
    int deleted = 0;
    while (cur != NULL)
    {
        if (cur->passengerId == pid)
        {
            if (prev != NULL)
            {
                prev->next = cur->next;
            }
            else
            {
                bqFront = cur->next;
            }
            free(cur);
            deleted = 1;
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    if (deleted == 0)
    {
        printf("Passenger %d not found in boarding queue \n", pid);
    }
}

// -------- Boarding History --------
void pushHistory(Passenger *p)
{
    HistoryNode *h = (HistoryNode *)malloc(sizeof(HistoryNode));
    time_t now;
    char *t;
    if (h == NULL)
    {
        printf("Memeory error occured\n");
        return;
    }
    now = time(NULL);
    t = ctime(&now);

    h->passengerId = p->passengerId;
    copyText(h->passengerName, p->name, NAME_LEN);
    h->flightId = p->flightId;
    h->seatNumber = p->seatNumber;

    copyText(h->boardingTime, t, TEXT_LEN);
    h->boardingTime[strcspn(h->boardingTime, "\r\n")] = '\0';
    h->next = HistoryTop;
    HistoryTop = h;
}

// ======= Flight  function ======
int flightBefore(Flight *a, Flight *b)
{
    if (a->departureTime != b->departureTime)
    {
        return a->departureTime < b->departureTime;
    }
    return a->flightId < b->flightId;
}

void printFlightHeader()
{
    printf("\n%-5s %-7s %-14s %-14s %-5s %-5s %-6s %-6s %-13s %-4s %-6s %-7s\n", " ID ", " FLTNO ", " SOURCE ", " DEST ", "DEP", " ARR ", " SEATS ", " AVAIL ", " STATU ", " PAX ", " CHKIN ", " BOARDED ");
    printf("---------------------------------------------------------------------------------------------------------\n");
}

void printFlightRow(Flight *f)
{
    int pax = countPassengers(f->flightId);
    int checkCount = countCheckedIn(f->flightId);
    int BoardedCount = countBoarded(f->flightId);

    printf("%-5d %-7s %-14s %-14s %04d %04d %-6d %-6d %-13s %-4d %-6d %-7d\n",
           f->flightId, f->flightNumber, f->source, f->destination,
           f->departureTime, f->arrivalTime, f->totalSeats, f->availableSeats,
           statusName(f->status), pax, checkCount, BoardedCount);
}

int addFlightData(int id, char *num, char *src, char *dst, int dep, int arr, int total, int status)
{
    Flight *n;
    Flight *cur;
    if (id <= 0)
    {
        printf("Error : Flight id must be positive \n");
        return 0;
    }
    if (findFlight(id) != NULL)
    {
        printf("Error : Flight ID %d already exists (duplicate  rejected) \n", id);
        return 0;
    }
    if (total <= 0)
    {
        printf("Error : total seats must be more than 0 \n");
        return 0;
    }
    if (isValidTime(dep) == 0 || isValidTime(arr) == 0)
    {
        printf("Error ; Time must be in HHMM format (0000 to 2359)\n");
        return 0;
    }
    if (status < SCHEDULED || status > CANCELLED)
    {
        printf("Error : invalid flight status\n");
        return 0;
    }
    n = (Flight *)malloc(sizeof(Flight));
    if (n == NULL)
    {
        printf("Memory error \n");
        return 0;
    }
    n->flightId = id;
    copyText(n->flightNumber, num, FLTNUM_LEN);
    copyText(n->source, src, TEXT_LEN);
    copyText(n->destination, dst, TEXT_LEN);
    n->departureTime = dep;
    n->arrivalTime = arr;
    n->totalSeats = total;
    n->availableSeats = total;
    n->status = (flightStatus)status;
    n->prev = NULL;
    n->next = NULL;

    if (flightHead == NULL)
    {
        flightHead = n;
    }
    else if (flightBefore(n, flightHead))
    {
        n->next = flightHead;
        flightHead->prev = n;
        flightHead = n;
    }
    else
    {
        cur = flightHead;
        while ((cur->next != NULL) && (flightBefore(n, cur->next) == 0))
        {
            cur = cur->next;
        }
        n->next = cur->next;
        n->prev = cur;
        if (cur->next != NULL)
        {
            cur->next->prev = n;
        }
        cur->next = n;
    }
    printf("Flight %d (%s) added  \n", id, n->flightNumber);
    return 1;
}

void addFlight()
{
    int id, dep;
    int arr, total;
    char num[FLTNUM_LEN], src[TEXT_LEN], dst[TEXT_LEN];

    printf("|=== Add Flight ===|\n");
    id = readInt("Flight ID: ");
    if (findFlight(id) != NULL)
    {
        printf("Error : Flight Id %d already exists (duplicate rejected) \n", id);
        printf("Enter a different Flight ID \n");
        return;
    }
    readString("Flight Number: ", num, FLTNUM_LEN);
    readString("source: ", src, TEXT_LEN);
    readString("Destination : ", dst, TEXT_LEN);
    dep = readInt("Departure Time(HHMM): ");
    arr = readInt("Arrival Time (HHMM) : ");
    total = readInt("Total seats: ");
    addFlightData(id, num, src, dst, dep, arr, total, SCHEDULED);
}

void deleteFlight()
{
    int id;
    Flight *f;
    printf("\n| ==== Delete FLight === |\n");
    id = readInt("Flight ID to  delete : ");
    f = findFlight(id);
    if (f == NULL)
    {
        printf("Error : Invalid FLight ID\n");
        return;
    }
    int pax = countPassengers(id);
    if (pax > 0)
    {
        printf("error : Cant delete ,%d passenger still refer to this flight \n", pax);
        return;
    }
    if (f->prev != NULL)
    {
        f->prev->next = f->next;
    }
    else
    {
        flightHead = f->next;
    }
    if (f->next != NULL)
    {
        f->next->prev = f->prev;
    }
    free(f);
    printf("flight %d deleted \n", id);
}

void searchFlight()
{
    int id = 0, found = 0;
    char text[TEXT_LEN] = "";
    Flight *f;

    printf("----------Search Flight ------------\n");
    printf("1 . By Flight ID\n2. By Flight Number\n3. By Source\n4. Destination\n");
    int choice = readInt("Choice : ");
    if (choice == 1)
    {
        id = readInt("Enter Flight Id : ");
    }
    else if (choice >= 2 && choice <= 4)
    {
        readString("Enter Text : ", text, TEXT_LEN);
    }
    else
    {
        printf("Invalid choice : \n");
        return;
    }
    int match;
    f = flightHead;
    while (f != NULL)
    {
        match = 0;
        if (choice == 1 && f->flightId == id)
        {
            match = 1;
        }
        if (choice == 2 && compareText(f->flightNumber, text) == 0)
            match = 1;
        if (choice == 3 && compareText(f->source, text) == 0)
            match = 1;
        if (choice == 4 && compareText(f->destination, text) == 0)
            match = 1;
        if (match)
        {
            if (found == 0)
            {
                printFlightHeader();
            }
            printFlightRow(f);
            found++;
        }
        f = f->next;
    }
    if (found == 0)
    {
        printf("\t\tNo matching flight found \n");
    }
}

int compareFlights(Flight *a, Flight *b, int mode)
{
    int r = 0;
    if (mode == 1)
    {
        r = a->departureTime - b->departureTime;
    }
    else if (mode == 2)
    {
        r = compareText(a->source, b->source);
    }
    else if (mode == 3)
    {
        r = compareText(a->destination, b->destination);
    }
    else
    {
        r = compareText(a->source, b->source);
        if (r == 0)
        {
            r = a->departureTime - b->departureTime;
        }
    }
    if (r == 0)
    {
        r = a->flightId - b->flightId;
    }
    return r;
}
void displayFlightsSorted()
{
    int mode, n = 0;
    Flight **arr, *f;

    printf("\n--- Display Flights (Custom Sort) ---\n");
    printf("1. Sorted by Departure Time\n");
    printf("2. Sorted by Source\n");
    printf("3. Sorted by Destination\n");
    printf("4. Sorted by Source, then Departure Time\n");
    mode = readInt("Choice: ");
    if (mode < 1 || mode > 4)
    {
        printf("\tInvalid choice.\n");
        return;
    }
    for (f = flightHead; f != NULL; f = f->next)
    {
        n++;
    }
    if (n == 0)
    {
        printf("\tNo Available Flights.\n");
        return;
    }
    arr = (Flight **)malloc(n * sizeof(Flight *));
    if (arr == NULL)
    {
        printf("\tMemory error.\n");
        return;
    }
    int i = 0;
    for (f = flightHead; f != NULL; f = f->next)
    {
        arr[i] = f;
        i++;
    }

    if (mergeSortFlights(arr, n, mode) == 0)
    {
        free(arr);
        return;
    }
    printFlightHeader();
    for (i = 0; i < n; i++)
    {
        printFlightRow(arr[i]);
    }
    free(arr);
}

void updateFlightStatus()
{
    int id;
    Flight *f;
    printf("\n--- Update Flight Status ---\n");
    id = readInt("Flight ID: ");
    f = findFlight(id);
    if (f == NULL)
    {
        printf("Error: Invalid Flight ID \n");
        return;
    }
    printf("Current status: %s\n", statusName(f->status));
    if (f->status == CANCELLED || f->status == DEPARTED)
    {
        printf("Error: Flight is %s. Status cant be changed.\n", statusName(f->status));
        return;
    }
    f->status = (flightStatus)(f->status + 1);
    printf("Flight %d status is now: %s\n", id, statusName(f->status));
}

void cancelFlight()
{
    int id;
    Flight *f;
    printf("\n--- Cancel Flight ---\n");
    id = readInt("Flight ID: ");
    f = findFlight(id);
    if (f == NULL)
    {
        printf("\tError: Invalid Flight ID \n");
        return;
    }
    if (f->status == CANCELLED || f->status == DEPARTED)
    {
        printf("\tError: Flight is %s. cant cancel \n", statusName(f->status));
        return;
    }
    f->status = CANCELLED;
    printf("\tFlight %d has been cancelled \n", id);
}

// =========== Passenger functions ==============
void printPassengerHeader()
{
    printf("\n%-5s %-20s %-3s %-6s %-15s %-5s %-6s %-12s %-8s %-7s\n",
           " ID ", " NAME ", " AGE ", " GENDER ", " CONTACT ", " FLIGHT ", " SEAT ", " TICKET TYPE ", " CHECKED IN ", " BOARDED ");
    printf("----------------------------------------------------------------------------------------\n");
}

void printPassengerRow(Passenger *p)
{
    printf("%-5d %-20s %-3d %-6c %-15s %-5d %-6d %-12s %-8s %-7s\n",
           p->passengerId, p->name, p->age, p->gender, p->contact,
           p->flightId, p->seatNumber, ticketName(p->TicketType),
           p->checkedIn ? "Yes" : "No", p->boarded ? "Yes" : "No");
}

int addPassengerData(int pid, char *name, int age, char gender, char *contact, int fid, int ticket)
{
    Flight *f;
    // Passenger *p;
    Passenger *cur;
    int seat;
    gender = toupper((unsigned char)gender);
    if (pid <= 0)
    {
        printf("\tError: Passenger ID must be positive \n");
        return 0;
    }
    if (findPassenger(pid) != NULL)
    {
        printf("\tError: Passenger ID %d already exists (duplicate rejected) \n", pid);
        return 0;
    }
    f = findFlight(fid);
    if (f == NULL)
    {
        printf("\tError: FLight ID %d not found \n", fid);
        return 0;
    }
    if (f->status == CANCELLED)
    {
        printf("\tError: Flight %d is cancelled. cant add passenger \n", fid);
        return 0;
    }
    if (f->availableSeats <= 0)
    {
        printf("\tError: Flight %d has no available seats \n", fid);
        return 0;
    }
    if (f->status == DEPARTED)
    {
        printf("\tError: Flight %d has alraedy departed. Cant add passenger\n", fid);
        return 0;
    }
    if (f->status == BOARDING)
    {
        printf("\tError: Flight %d is boarding now. cant add passenger\n", fid);
        return 0;
    }
    if (age <= 0 || age > 120)
    {
        printf("\tError: Invalid age %d.\n", age);
        return 0;
    }
    if (gender != 'M' && gender != 'F' && gender != 'O')
    {
        printf("\tError: Gender must be M, F, or O(Other).\n");
        return 0;
    }
    if (ticket > ECONOMY || ticket < FIRST_CLASS)
    {
        printf("\tError: ticket type must be \t1(First Class) \t2(Business) \t3(Economy).\n");
        return 0;
    }
    seat = allocateSeat(f);
    if (seat == 0)
    {
        printf("\tError: No available seats on flight %d \n", fid);
        return 0;
    }
    Passenger *n = (Passenger *)malloc(sizeof(Passenger));
    if (n == NULL)
    {
        printf("\tMemory error occured \n");
        return 0;
    }
    n->passengerId = pid;
    copyText(n->name, name, NAME_LEN);
    n->age = age;
    n->gender = gender;
    copyText(n->contact, contact, CONTACT_LEN);
    n->flightId = fid;
    n->seatNumber = seat;
    n->TicketType = (ticketType)ticket;
    n->checkedIn = 0;
    n->boarded = 0;
    n->prev = NULL;
    n->next = NULL;

    if (passHead == NULL || n->passengerId < passHead->passengerId)
    {
        n->next = passHead;
        if (passHead != NULL)
            passHead->prev = n;
        passHead = n;
    }
    else
    {
        cur = passHead;
        while (cur->next != NULL && cur->next->passengerId < n->passengerId)
        {
            cur = cur->next;
        }
        n->next = cur->next;
        if (cur->next != NULL)
            cur->next->prev = n;
        cur->next = n;
        n->prev = cur;
    }
    f->availableSeats--;
    enqueueCheckin(n);
    printf("Passenger %d (%s) added to flight %d, seat %d \n",
           pid, n->name, fid, seat);
    return 1;
}

void addPassenger()
{
    int pid, fid, ticket;
    char name[NAME_LEN], contact[CONTACT_LEN];
    printf("\n--- Add Passenger ---\n");
    pid = readInt("Passenger ID: ");
    if (findPassenger(pid) != NULL)
    {
        printf("Error: Passenger ID %d already exists ( duplicate rejected ).\n", pid);
        return;
    }
    readString("Name: ", name, NAME_LEN);
    int age = readInt("Age: ");
    char gender = readChar(" Gender (M/F/O): ");
    readString("Contact: ", contact, CONTACT_LEN);
    fid = readInt("FLight ID: ");
    ticket = readInt("Ticket Type (1=First, 2=Business, 3=Economy): ");
    addPassengerData(pid, name, age, gender, contact, fid, ticket);
}

void deletePassenger()
{
    int pid;
    Passenger *p;
    Flight *f;
    printf("\n=-- Delete Passenger --=\n");
    pid = readInt("Passenger ID to delete: ");
    p = findPassenger(pid);
    if (p == NULL)
    {
        printf("Error: Invalid Passenger ID \n");
        return;
    }
    f = findFlight(p->flightId);
    if (f != NULL)
    {
        f->availableSeats++;
    }
    removeFromCheckinQueue(pid);
    removeFromBoardingQueue(pid);
    if (p->prev != NULL)
    {
        p->prev->next = p->next;
    }
    else
    {
        passHead = p->next;
    }
    if (p->next != NULL)
    {
        p->next->prev = p->prev;
    }
    printf("Passenger %d (%s) deleted.\n seat: %d of flight %d is free now \n", pid, p->name, p->seatNumber, p->flightId);
    free(p);
}

void displayPassengers()
{
    int fid, found = 0;
    printf("\n=-- Display Passengers --=\n");
    fid = readInt("Flight ID: ");
    Passenger *p;
    p = passHead;
    while (p != NULL)
    {
        if (p->flightId == fid)
        {
            if (found == 0)
            {
                printPassengerHeader();
            }
            printPassengerRow(p);
            found++;
        }
        p = p->next;
    }
    if (found == 0)
    {
        printf("\tNo passengers found for flight Num :  %d \n", fid);
    }
}

void seatReallocation()
{
    int id, n, i;
    Flight *f;
    Passenger *p;
    Passenger **arr;
    // Passenger *temp;
    printf("\n--- Reallocate Seats by Ticket Priority ---\n");
    id = readInt("Flight ID: ");
    f = findFlight(id);
    if (f == NULL)
    {
        printf("\tError: Invalid Flight ID \n");
        return;
    }
    if (f->status == CANCELLED || f->status == DEPARTED)
    {
        printf("\tError: Flight is %s. cant reallocate seats \n", statusName(f->status));
        return;
    }
    n = countPassengers(id);
    if (n == 0)
    {
        printf("\tNo Passengers found for flight %d \n", id);
        return;
    }
    arr = (Passenger **)malloc(n * sizeof(Passenger *));
    if (arr == NULL)
    {
        printf("\tMemory error \n");
        return;
    }
    i = 0;
    p = passHead;
    while (p != NULL)
    {
        if (p->flightId == id)
        {
            arr[i] = p;
            i++;
        }
        p = p->next;
    }

    if (mergeSortPassengers(arr, n) == 0)
    {
        free(arr);
        return;
    }

    for (i = 0; i < n; i++)
    {
        arr[i]->seatNumber = i + 1;
    }
    f->availableSeats = f->totalSeats - n;
    printf("Seats reallocated for flight %d based on ticket priority \n", id);
    printf("Seat reallocation completed. Updated passenger list: \n");
    printPassengerHeader();
    for (i = 0; i < n; i++)
    {
        printPassengerRow(arr[i]);
    }
    free(arr);
}
// Check in and Boarding functions

int checkinPassenger(Passenger *p)
{
    int pid;
    Flight *f;
    if (p == NULL)
    {
        printf("\n--- Register Check-in ---\n");
        pid = readInt("Passenger ID: ");
        p = findPassenger(pid);
        if (p == NULL)
        {
            printf("Error: Invalid Passenger ID \n");
            return 0;
        }
    }
    pid = p->passengerId;
    // p = findPassenger(pid);
    if (p->checkedIn)
    {
        printf("\tPassenger %d (%s) has already checked in \n", pid, p->name);
        return 0;
    }
    f = findFlight(p->flightId);
    if (f == NULL)
    {
        printf("\tError: FLight not found for passenger %d \n", pid);
        return 0;
    }
    if (f->status != CHECKIN_OPEN)
    {
        printf("\tError: Flight %d is not open for check-in. Current status: %s\n", f->flightId, statusName(f->status));
        return 0;
    }
    removeFromCheckinQueue(pid);
    p->checkedIn = 1;
    enqueueBoarding(p);
    printf("\tPassenger %d (%s) checked in successfully for flight %d \n", pid, p->name, f->flightId);
    return 1;
}

void checkById()
{
    int pid;
    Passenger *p;
    printf("\n--- Check Passenger Status by ID ---\n");
    pid = readInt("Passenger ID: ");
    p = findPassenger(pid);
    if (p == NULL)
    {
        printf("Error: Invalid Passenger ID \n");
        return;
    }
    printPassengerHeader();
    printPassengerRow(p);
}

void processNextCheckin()
{
    CheckinNode *cur = ciFront;
    printf("\n--- Process Next Check-in Queue Entry ---\n");
    if (ciFront == NULL)
    {
        printf("\tNo passengers in check-in queue \n");
        return;
    }
    while (cur != NULL)
    {
        Flight *f = findFlight(cur->passenger->flightId);

        if (f != NULL && f->status == CHECKIN_OPEN)
        {
            checkinPassenger(cur->passenger);
            return;
        }
        cur = cur->next;
    }
    printf("\tNo passengers in  queue for flights that are open for check-in \n");
}

void displayCheckinQueue()
{
    CheckinNode *cur = ciFront;
    int pos = 1;
    printf("\n--- Check-in Queue ---\n");
    if (cur == NULL)
    {
        printf("\tNo passengers in check-in queue \n");
        return;
    }
    while (cur != NULL)
    {
        Flight *f = findFlight(cur->passenger->flightId);
        if (f != NULL)
        {
            printf("%d,Passenger ID: %d, Name: %s, Flight ID: %d, Flight Status: %s\n",
                   pos++, cur->passengerId, cur->passenger->name, cur->passenger->flightId, statusName(f->status));
        }
        cur = cur->next;
    }
}

void displayBoardingQueue()
{
    BoardNode *cur = bqFront;
    int idx = 1;
    printf("\n\n--- Boarding Queue ---\n");
    if (cur == NULL)
    {
        printf("\tNo passengers in boarding queue \n");
        return;
    }
    while (cur != NULL)
    {
        Flight *f = findFlight(cur->passenger->flightId);
        if (f != NULL)
        {
            printf("%d,Passenger ID: %d, Name: %s, Flight ID: %d, Flight Status: %s, Ticket Type: %s\n",
                   idx++, cur->passengerId, cur->passenger->name, cur->passenger->flightId,
                   statusName(f->status), ticketName(cur->passenger->TicketType));
        }
        cur = cur->next;
    }
}

void BoardNextPassenger()
{
    BoardNode *cur = bqFront;
    BoardNode *prev = NULL;
    Passenger *p;
    Flight *f;
    printf("\n--- Board Next Eligible Passenger ---\n");
    if (cur == NULL)
    {
        printf("\tNo passengers in boarding queue \n");
        return;
    }
    while (cur != NULL)
    {
        f = findFlight(cur->passenger->flightId);
        if (f != NULL && f->status == BOARDING)
        {
            break;
        }
        prev = cur;
        cur = cur->next;
    }
    if (cur == NULL)
    {
        printf("\tNo eligible passengers inboarding queue for flights that are boarding \n");
        return;
    }
    p = cur->passenger;
    if (p->checkedIn == 0)
    {
        printf("\tPassenger %d (%s) has not checked in yet\n", p->passengerId, p->name);
        return;
    }
    if (p->boarded)
    {
        printf("\tPassenger %d (%s) has already boarded \n", p->passengerId, p->name);
        return;
    }
    if (prev != NULL)
    {
        prev->next = cur->next;
    }
    else
    {
        bqFront = cur->next;
    }
    p->boarded = 1;
    pushHistory(p);
    printf("Passenger %d (%s) boarded successfully for flight %d\n", p->passengerId, p->name, f->flightId);
    free(cur);
    /* code */
}

void displayBoardingHistory()
{
    HistoryNode *cur = HistoryTop;
    printf("\n--- Boarding History ---\n");
    if (cur == NULL)
    {
        printf("\tNo passengers have boarded yet \n");
        return;
    }
    int idx = 1;
    while (cur != NULL)
    {
        printf("%d,Passenger ID: %d, Name: %s, Flight ID: %d, Seat Number: %d, Boarding Time: %s\n",
               idx++, cur->passengerId, cur->passengerName, cur->flightId,
               cur->seatNumber, cur->boardingTime);
        cur = cur->next;
    }
}

// Real time analysis

void flightAnalysis()
{
    // vars
    Flight *f, *crowdedFlight = NULL, *occupiedFlight = NULL, *seniorFlight = NULL;
    Flight *emptiestFlight = NULL, *mostCheckinFlight = NULL, *mostBoardedFlight = NULL;
    Flight *popularDestFlight = NULL, *firstDestFlight = NULL;
    int cnt, checkedIn, boarded, senior;
    int totalFlights = 0, totalPassengers = 0, totalCheckedIn = 0, totalBoarded = 0;
    int maxcount = 0, maxcheckin = 0, maxboarded = 0, maxAvailable = -1;
    int maxFirst = 0;
    double occ, maxOcc = -1.0, sen, maxSen = -1.0;
    int demand, maxdemand = 0;

    printf("\n--- Real-time Flight Analysis ---\n");
    if (flightHead == NULL)
    {
        printf("\tNo flights Aval for analysis.\n");
        return;
    }

    for (f = flightHead; f != NULL; f = f->next)
    {
        totalFlights++;
        cnt = countPassengers(f->flightId);
        checkedIn = countCheckedIn(f->flightId);
        boarded = countBoarded(f->flightId);
        senior = countSenior(f->flightId);
        totalPassengers += cnt;
        totalCheckedIn += checkedIn;
        totalBoarded += boarded;

        if (cnt > maxcount)
        {
            maxcount = cnt;
            crowdedFlight = f;
        }
        if (checkedIn > maxcheckin)
        {
            maxcheckin = checkedIn;
            mostCheckinFlight = f;
        }
        if (boarded > maxboarded)
        {
            maxboarded = boarded;
            mostBoardedFlight = f;
        }
        if (cnt > 0)
        {
            sen = senior * 100.0 / cnt;
            if (sen > maxSen)
            {
                maxSen = sen;
                seniorFlight = f;
            }
        }
        int fc = destinationFirstClass(f->destination);
        if (fc > maxFirst)
        {
            maxFirst = fc;
            firstDestFlight = f;
        }
        if (f->availableSeats > maxAvailable)
        {
            maxAvailable = f->availableSeats;
            emptiestFlight = f;
        }
        demand = destinationDemand(f->destination);
        if (demand > maxdemand)
        {
            maxdemand = demand;
            popularDestFlight = f;
        }
        if (f->totalSeats > 0)
        {
            occ = (double)cnt / (double)f->totalSeats;
        }
        else
        {
            occ = 0.0;
        }

        if (occ > maxOcc)
        {
            maxOcc = occ;
            occupiedFlight = f;
        }
    }
    printf("\n------------ Summary -----------------\n");
    printf("Total Flights: %d\n", totalFlights);
    printf("Total Passengers: %d\n", totalPassengers);
    printf("Total Checked-in: %d\n", totalCheckedIn);
    printf("Total Boarded: %d\n", totalBoarded);

    printf("\nMost Crowded Flight: ");
    if (crowdedFlight != NULL)
        printf("Flight %d (%s -> %s), passengers=%d\n", crowdedFlight->flightId, crowdedFlight->source,
               crowdedFlight->destination, countPassengers(crowdedFlight->flightId));
    else
        printf("N/A\n");

    printf("Most Occupied Flight: ");
    if (occupiedFlight != NULL)
        printf("Flight %d (%s -> %s), occupancy=%.2f%%\n", occupiedFlight->flightId, occupiedFlight->source,
               occupiedFlight->destination, maxOcc * 100.0);
    else
        printf("N/A\n");

    printf("Highest senior %%: ");
    if (seniorFlight != NULL)
        printf("Flight %d (%.1f%%)\n", seniorFlight->flightId, maxSen);
    else
        printf("N/A\n");

    printf("Max First-Class destination: ");
    if (firstDestFlight != NULL)
        printf("%s (%d)\n", firstDestFlight->destination, maxFirst);
    else
        printf("N/A\n");

    printf("Emptiest Flight: ");
    if (emptiestFlight != NULL)
        printf("Flight %d  (%s -> %s), available seats = %d\n", emptiestFlight->flightId, emptiestFlight->source,
               emptiestFlight->destination, emptiestFlight->availableSeats);
    else
        printf("N/A\n");

    printf("Flight with Most Checked-in Passengers: ");
    if (mostCheckinFlight != NULL)
        printf("Flight %d, checked-in=%d\n", mostCheckinFlight->flightId, countCheckedIn(mostCheckinFlight->flightId));
    else
        printf("N/A\n");

    printf("Flight with Most Boarded Passengers: ");
    if (mostBoardedFlight != NULL)
        printf("Flight %d, boarded=%d\n", mostBoardedFlight->flightId, countBoarded(mostBoardedFlight->flightId));
    else
        printf("N/A\n");

    printf("Most Popular Destination: ");
    if (popularDestFlight != NULL)
        printf("%s (passenger count=%d)\n", popularDestFlight->destination, maxdemand);
    else
        printf("N/A\n");

    printf("\nDetailed flight overview:\n");
    for (f = flightHead; f != NULL; f = f->next)
    {
        double occ = f->totalSeats > 0 ? ((double)countPassengers(f->flightId) / (double)f->totalSeats) * 100.0 : 0.0;
        printf("Flight %d | %s --> %s | status=%s | passengers=%d | checked-in=%d | boarded=%d | available=%d | occupancy=%.2f%%\n", f->flightId, f->source, f->destination, statusName(f->status), countPassengers(f->flightId), countCheckedIn(f->flightId), countBoarded(f->flightId), f->availableSeats, occ);
    }
}

// txt file readonly
void readTxtvalue()
{
    // vars
    char fileName[1000] = DEFAULT_DB;
    FILE *fp;
    char type;
    int ch;
    int id, dep, arr, total, status, age, fid, ticket;
    char num[FLTNUM_LEN], src[TEXT_LEN], dst[TEXT_LEN];
    char name[NAME_LEN], contact[CONTACT_LEN], gender;
    int flightsAdded = 0, passengersAdded = 0;
    Passenger *p;
    Flight *f;
    printf("\n--- Load Data From Text File ---\n");
    fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        printf("\tError: Cannot open file '%s' \n", fileName);
        readString("Enter text file name (example: data.txt): ", fileName, sizeof(fileName));
        fp = fopen(fileName, "r");
        if (fp == NULL)
        {
            printf("\tError: Cannot open file '%s'.\n", fileName);
            return;
        }
    }

    while (fscanf(fp, " %c", &type) == 1)
    {
        if (type == '#')
        {
            while ((ch = fgetc(fp)) != '\n' && ch != EOF)
            {
            }
            continue;
        }
        else if (type == 'F')
        {
            if (fscanf(fp, "%d %9s %99s %99s %d %d %d %d", &id, num, src, dst,
                       &dep, &arr, &total, &status) != 8)
            {
                printf("Bad flight line in file. Stopped reading \n");
                break;
            }
            replaceUnderscore(src);
            replaceUnderscore(dst);
            flightsAdded += addFlightData(id, num, src, dst, dep, arr, total, status);
        }
        else if (type == 'P')
        {
            if (fscanf(fp, "%d %49s %d %c %14s %d %d", &id, name, &age, &gender,
                       contact, &fid, &ticket) != 7)
            {
                printf("Bad passenger line in file. Stopped reading \n");
                break;
            }
            replaceUnderscore(name);
            passengersAdded += addPassengerData(id, name, age, gender, contact, fid, ticket);
        }
        else if (type == 'C')
        {
            if (fscanf(fp, "%d", &id) != 1)
            {
                printf("Bad check-in line in file. Stopped reading \n");
                break;
            }
            p = findPassenger(id);
            if (p == NULL)
                printf("Error: Check-in - invalid Passenger ID ( %d ) \n", id);
            else
                checkinPassenger(p);
        }
        else if (type == 'S')
        {
            if (fscanf(fp, "%d %d", &id, &status) != 2)
            {
                printf("Bad status line in file. Stopped reading.\n");
                break;
            }
            f = findFlight(id);
            if (f == NULL)
                printf("Error: Status change - invalid Flight ID %d \n", id);
            else if (status < SCHEDULED || status > CANCELLED)
                printf("Error: Invalid status value %d.\n", status);
            else
            {
                f->status = (flightStatus)status;
                printf("Flight %d status set to %s \n", id, statusName(status));
            }
        }
        else
        {
            printf("Unknown line type '%c' in file. Stopped reading...\n", type);
            break;
        }
    }
    fclose(fp);
    printf("\nFile loaded: %d flight(s) and %d passenger(s) added\n",
           flightsAdded, passengersAdded);
}

void showMenu()
{
    printf("\n+----------------------------------------------------------+\n");
    printf("|     =====    DIGIYATRI MANAGEMENT SYSTEM   ======          |\n");
    printf("|    |__FLIGHTS__|        -          |__PASSENGERS__|         |\n");
    printf("|  1. Add Flight          |  7. Add Passenger              |\n");
    printf("|  2. Delete Flight       |  8. Delete Passenger           |\n");
    printf("|  3. Search Flight       |  9. Display Passengers         |\n");
    printf("|  4. Display Flights     | 10. Reallocate Seats           |\n");
    printf("|  5. Update Status       |                               |\n");
    printf("|  6. Cancel Flight       |                               |\n");
    printf("-----------------------------------------------------------\n");
    printf("|             |__CHECK-IN & BOARDING__|                    |\n");
    printf("| 11. Register Check-in   | 12. Passenger Status by ID     |\n");
    printf("| 13. Process Check-in    | 14. Display Check-in Queue     |\n");
    printf("| 15. Display Boarding Q. | 16. Board Next Passenger       |\n");
    printf("| 17. Boarding History                                    |\n");
    printf("------------------------------------------------------------\n");
    printf("| 18. Real-time Analysis | 19. Load Data | 0. Exit          |\n");
    printf("+----------------------------------------------------------+\n");
}

int main()
{
    int mainchoice, choice;
    mainchoice = readInt("\tEnter main choice:\n0. [0] Exit\n1. [1] Load data from text file\n2. [2 or Press any key] User Menu\nChoice: ");
    if (mainchoice == 0)
    {
        printf("Exiting the program.....\n");
        return 0;
    }
    if (mainchoice == 1)
    {
        readTxtvalue();
    }
    do
    {
        showMenu();
        choice = readInt("\tEnter your choice \n(0-19) :");
        switch (choice)
        {
        case 1:
            addFlight();
            break;
        case 2:
            deleteFlight();
            break;
        case 3:
            searchFlight();
            break;
        case 4:
            displayFlightsSorted();
            break;
        case 5:
            updateFlightStatus();
            break;
        case 6:
            cancelFlight();
            break;
        case 7:
            addPassenger();
            break;
        case 8:
            deletePassenger();
            break;
        case 9:
            displayPassengers();
            break;
        case 10:
            seatReallocation();
            break;
        case 11:
            checkinPassenger(NULL);
            break;
        case 12:
            checkById();
            break;
        case 13:
            processNextCheckin();
            break;
        case 14:
            displayCheckinQueue();
            break;
        case 15:
            displayBoardingQueue();
            break;
        case 16:
            BoardNextPassenger();
            break;
        case 17:
            displayBoardingHistory();
            break;
        case 18:
            flightAnalysis();
            break;
        case 19:
            readTxtvalue();
            break;
        case 0:
            printf("\t\tExiting the program....\n");
            return 0;
        default:
            printf("Invalid choice. Please try again...\n");
        }
    } while (1);
}
