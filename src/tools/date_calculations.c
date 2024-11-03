#include "tools.h"
#include  <time.h>

int days_between_dates(int year, int month, int day) {
    // Create a tm struct for the input date
    struct tm stored_date = {0};
    stored_date.tm_year = year - 1900; // tm_year is years since 1900
    stored_date.tm_mon = month - 1;    // tm_mon is months since January (0-11)
    stored_date.tm_mday = day;

    // Get the current date
    time_t current_time = time(NULL);
    struct tm *current_date = localtime(&current_time);

    // Convert both dates to time_t for easy difference calculation
    time_t stored_time = mktime(&stored_date);
    time_t current_time_sec = mktime(current_date);

    // Calculate the difference in seconds, then convert to days
    double difference_in_seconds = difftime(current_time_sec, stored_time);
    int difference_in_days = difference_in_seconds / (60 * 60 * 24);

    return difference_in_days;
}

struct tm* get_current_date(){
    //current_date
    char current_date[15] ;
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    return t;
}
