#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Function prototypes
size_t write_callback(void *ptr, size_t size, size_t nmemb, FILE *data);
double extract_temperature(const char *data);
double calculate_average(double temperatures[], int count);
void encode_url(char *url);
void beep();

#endif // MAIN_H

