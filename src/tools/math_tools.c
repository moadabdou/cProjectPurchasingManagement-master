#include<time.h>
#include <stdlib.h>

int generate_id(){
    srand(time(NULL));

    // Generate a random 8-digit number (between 10000000 and 99999999)
    int random_number = 10000000 + rand() % 90000000;

    // Print the generated number
    return random_number;
}