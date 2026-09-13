#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    int hour, min, second;
    hour = min = second = 0;
    while (1)
    {
        // Clear Screen
        system("clear");
        // print time in HH:MM:SS format
        printf("%02d : %02d : %02d ", hour, min, second);
        // Clear output buffer in gcc
        fflush(stdout);
        // increase sec
        second++;
        // update min hour sec
        if (second == 60)
        {
            min += 1;
            second = 0;
        }
        if (min == 60)
        {
            hour += 1;
            min = 0;
        }
        if (hour == 24)
        {
            hour = min = second = 0;
        }
        sleep(1);
    }
    return 0;
}