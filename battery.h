#ifndef BATTERY_H
#define BATTERY_H




class Battery {
public:
    enum Status {
        CHARGING,
        DISCHARGING
    };

    static int current_energy();
    static int total_capacity();
    static Status status();

private:
    static const char*  ENERGY_NOW;
    static const char* ENERGY_FULL;
    static const char*      STATUS;
};

#endif
