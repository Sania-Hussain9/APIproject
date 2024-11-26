
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Callback function to write data to a file
size_t write_callback(void *ptr, size_t size, size_t nmemb, FILE *data) {
    return fwrite(ptr, size, nmemb, data);
}

// Function to extract the temperature value manually from JSON data
double extract_temperature(const char *data) {
    const char *key = "\"temp\":";
    char *start = strstr(data, key);  // Find the "temp" key in the JSON
    if (start) {
        start += strlen(key);  // Move the pointer to the value
        double temperature;
        if (sscanf(start, "%lf", &temperature) == 1) {
            return temperature;  // Successfully extracted the temperature
        }
    }
    fprintf(stderr, "Error: Temperature not found in JSON data.\n");
    return -1;
}

// Function to calculate the average temperature
double calculate_average(double temperatures[], int count) {
    double sum = 0.0;
    for (int i = 0; i < count; i++) {
        sum += temperatures[i];
    }
    return sum / count;  // Return the average
}

// Function to URL-encode spaces in city names
void encode_url(char *url) {
    for (char *p = url; *p; p++) {
        if (*p == ' ') {
            *p = '%';
            memmove(p + 3, p + 1, strlen(p + 1) + 1); // Shift the remaining string
            p[1] = '2';
            p[2] = '0';
        }
    }
}

int main() {
    CURL *curl;
    CURLcode res;
    FILE *file;
    char *data = NULL;
    size_t data_size = 0;

    // Cities to fetch data for
    const char *cities[] = {"Berlin", "New York", "Paris"};
    int city_count = sizeof(cities) / sizeof(cities[0]);
    double temperatures[city_count];

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error initializing CURL.\n");
        return 1;
    }

    for (int i = 0; i < city_count; i++) {
        // File to save raw response for each city
        char filename[50];
        sprintf(filename, "response_%s.txt", cities[i]);

        // Replace spaces in the filename (optional, for simplicity)
        for (char *p = filename; *p; p++) {
            if (*p == ' ') *p = '_';
        }

        // Set the API URL for the current city
        char url[256];
        sprintf(url, "https://api.openweathermap.org/data/2.5/weather?q=%s&appid=da940ec35c27673bb21c9f83ddb721d5", cities[i]);

        // Encode spaces in the URL
        encode_url(url);

        file = fopen(filename, "wb");
        if (!file) {
            perror("File opening failed");
            return 1;
        }

        // Set CURL options
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);  // Disable SSL peer verification
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);  // Disable host verification
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

        // Perform the CURL request
        res = curl_easy_perform(curl);
        fclose(file);  // Close the file after saving the response

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed for %s: %s\n", cities[i], curl_easy_strerror(res));
            return 1;
        }

        // Read the data from the file
        file = fopen(filename, "rb");
        if (!file) {
            perror("File opening failed");
            return 1;
        }

        fseek(file, 0, SEEK_END);
        data_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        data = malloc(data_size + 1);
        if (data) {
            fread(data, 1, data_size, file);
            data[data_size] = '\0';  // Null-terminate the string
        }
        fclose(file);

        // Extract the temperature for the city
        if (data) {
            temperatures[i] = extract_temperature(data);
            if (temperatures[i] > 300.0) {
                    printf("\a");
            }
            free(data);
        } else {
            temperatures[i] = -1;  // Mark as invalid if data couldn't be read
        }
    }

    curl_easy_cleanup(curl);  // Cleanup CURL

    // Calculate the average temperature
    double average_temp = calculate_average(temperatures, city_count);

    // Write the results to a CSV file
    FILE *csv_file = fopen("processed_results.csv", "w");
    if (!csv_file) {
        perror("Error opening processed_results.csv for writing");
        return 1;
    }

    fprintf(csv_file, "City,Temperature (Kelvin)\n");
    for (int i = 0; i < city_count; i++) {
        fprintf(csv_file, "%s,%.2f\n", cities[i], temperatures[i]);
    }
    fprintf(csv_file, "Average,%.2f\n", average_temp);
    fclose(csv_file);

    printf("Processed results saved to 'processed_results.csv'.\n");
    return 0;
}

