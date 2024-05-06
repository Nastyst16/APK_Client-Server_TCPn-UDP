#include <stdio.h>
#include <string.h>

int main() {

    char topic[] = "upb/precis/100/humidity";
    char client_topic[] = "*/100/*";

// upb/precis/100/humidity
// */100/*


    int j = 0, k = 0;
    while (j < strlen(topic) && k < strlen(client_topic)) {

        if (topic[j] == client_topic[k]) {
            j++;
            k++;
            continue;
        }

        if (client_topic[k] == '*') {
            k++;
            if (client_topic[k] == '/')
                k++;

        while (client_topic[k] != topic[j] && j < strlen(topic)) {
            j++;
        }

        }

        if (client_topic[k] == '+') {
        k++;
        // if (client_topic[k] == '/')
        //     k++;

        while (topic[j] != '/' && j < strlen(topic)) {
            j++;
        }
        
        }

    }

    if (k == strlen(client_topic) && j == strlen(topic)) {
        // string_ok = 1;

        printf("Matched\n");


    }


    int i = 0;
}   