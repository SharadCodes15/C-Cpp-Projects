#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
typedef struct Battery_tag
{
    char name[10];
    int percentage;
    char status[20];
} Battery;

int getBatteryName(char *buffer, size_t buffer_size)
{
    DIR *directory;
    struct dirent *entry;
    directory = opendir("/sys/class/power_supply/");

    if (directory == NULL)
    {
        printf("Could Not open Power Supply directory.\n");
        return 0;
    }

    while ((entry = readdir(directory)) != NULL)
    {
        if (strncmp(entry->d_name, "BAT", 3) == 0)
        {
            // printf("Found Battery : %s\n", entry->d_name);
            snprintf(buffer, buffer_size, "%s", entry->d_name);
            closedir(directory);
            return 1;
        }
    }
    closedir(directory);
    return 0;
}

int getBatteryPercentage(Battery *battery)
{
    FILE *file;
    char FileName[100];

    snprintf(FileName, sizeof(FileName), "/sys/class/power_supply/%s/capacity", battery->name);
    file = fopen(FileName, "r");
    if (file == NULL)
    {
        return 0;
    }
    if (fscanf(file, "%d", &battery->percentage) != 1)
    {
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

int getBatteryStatus(Battery *battery)
{
    FILE *file;
    char FileName[100];

    snprintf(
        FileName,
        sizeof(FileName),
        "/sys/class/power_supply/%s/status",
        battery->name);

    file = fopen(FileName, "r");

    if (file == NULL)
    {
        return 0;
    }

    if (fgets(battery->status, sizeof(battery->status), file) == NULL)
    {
        fclose(file);
        return 0;
    }
    battery->status[strcspn(battery->status, "\n")] = '\0';

    fclose(file);

    return 1;
}

int getCheckInterval(int percentage)
{
    if (percentage > 80)
        return 15 * 60; // 15 minutes

    if (percentage > 60)
        return 10 * 60; // 10 minutes

    if (percentage > 40)
        return 5 * 60; // 5 minutes

    if (percentage > 30)
        return 2 * 60; // 2 minutes

    return 1 * 60; // 1 minute
}
void sendNotification(Battery *battery)
{
    char command[200];

    snprintf(
        command,
        sizeof(command),
        "notify-send 'Battery Warning' 'Battery is at %d%%'",
        battery->percentage);

    int result = system(command);

    if (result == -1)
    {
        printf("Failed to execute notify-send.\n");
    }
}
int main(void)
{
    Battery battery = {0};
    int warned = 0;

    if (!getBatteryName(battery.name, sizeof(battery.name)))
    {
        printf("No Battery Found!\n");
        return 1;
    }

    printf("Battery Found: %s\n", battery.name);

    while (1)
    {
        if (!getBatteryPercentage(&battery))
        {
            printf("Could Not Read Battery Percentage.\n");
            return 1;
        }

        if (!getBatteryStatus(&battery))
        {
            printf("Could Not Read Battery Status.\n");
            return 1;
        }

        printf("\n");
        printf("Battery Name   : %s\n", battery.name);
        printf("Battery Status : %s\n", battery.status);
        printf("Battery        : %d%%\n", battery.percentage);

        if (battery.percentage > 30)
        {
            warned = 0;
        }

        if (battery.percentage <= 30 &&
            strcmp(battery.status, "Discharging") == 0 &&
            warned == 0)
        {
            // printf("WARNING: Battery is low!\n");
            sendNotification(&battery);
            warned = 1;
        }

        int duration = getCheckInterval(battery.percentage);

        printf("Next check in %d minutes.\n", duration / 60);

        sleep(duration);
    }

    return 0;
}