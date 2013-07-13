#include "battery.h"
#include <stdio.h>
#include <string.h>

const char*  Battery::ENERGY_NOW = "/sys/class/power_supply/BAT0/energy_now";
const char*  Battery::ENERGY_FULL = "/sys/class/power_supply/BAT0/energy_full";
const char*  Battery::STATUS = "/sys/class/power_supply/BAT0/status";

int Battery::current_energy()
{
    int energy_now = 0;

    FILE* f = fopen(ENERGY_NOW, "r");
    fscanf(f, "%d", &energy_now);
    fclose(f);

    return energy_now;
}

int Battery::total_capacity()
{
    int energy_full = 0;

    FILE* f = fopen(ENERGY_FULL, "r");
    fscanf(f, "%d", &energy_full);
    fclose(f);

    return energy_full;
}

Battery::Status Battery::status()
{
    char b_status[16];
    FILE* f = fopen(Battery::STATUS, "r");
    fscanf(f, "%s", b_status);
    fclose(f);

    if (strcmp(b_status, "Charging") == 0)
        return Battery::CHARGING;
    else
        return Battery::DISCHARGING;
}


//  float cap = (float) energy_now / (float) energy_full * 100;
  
//  printf("Battery at %.0f%% (%s)\n", cap, status);
