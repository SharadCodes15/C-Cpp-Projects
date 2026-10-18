
#include <stdio.h>

#define TRUE 1
#define FALSE 0

int days_in_month[] = {
    0, 31, 28, 31, 30, 31,
    30, 31, 31, 30, 31, 30, 31};

char *months[] = {
    "",
    "January", "February", "March", "April",
    "May", "June", "July", "August",
    "September", "October", "November", "December"};

int inputYear(void)
{
    int year;
    printf("Enter the year: ");
    scanf("%d", &year);
    return year;
}

int determineLeapYear(int year)
{
    if ((year % 4 == 0 && year % 100 != 0) ||
        year % 400 == 0)
    {
        days_in_month[2] = 29;
        return TRUE;
    }

    days_in_month[2] = 28;
    return FALSE;
}

int determineDayCode(int year)
{
    int d1, d2, d3;

    d1 = (year - 1) / 4;
    d2 = (year - 1) / 100;
    d3 = (year - 1) / 400;

    return (year + d1 - d2 + d3) % 7;
}

void calendar(int year, int daycode)
{
    int month, day;

    for (month = 1; month <= 12; month++)
    {
        printf("\n\n\n%s %d\n", months[month], year);
        printf("Sun Mon Tue Wed Thu Fri Sat\n");

        for (day = 0; day < daycode; day++)
            printf("    ");

        for (day = 1; day <= days_in_month[month]; day++)
        {
            printf("%3d ", day);

            if ((day + daycode) % 7 == 0)
                printf("\n");
        }

        daycode = (daycode + days_in_month[month]) % 7;

        printf("\n");
    }
}

int main(void)
{
    int year, daycode;

    year = inputYear();

    if (year < 1)
    {
        printf("Please enter a valid positive year.\n");
        return 1;
    }

    determineLeapYear(year);
    daycode = determineDayCode(year);

    calendar(year, daycode);

    return 0;
}
