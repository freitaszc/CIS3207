#include <stdio.h>
#include <string.h>

struct Auth {
    char input[12];
    char valid[12];
};

int check_password(char* input, char* password) {
    int i = 0;
    while (password[i] != '\0') {
        if (input[i] != password[i]) {
            return 0;
        }
        i++;
    }
    return 1; 
}

int main() {
    struct Auth data;

    strcpy(data.valid, "Gabriel123");
    printf("Correct password: %s\n", data.valid);

    printf("Enter the password: ");
    scanf("%s", data.input); 

    printf("\nDebug Info:\n");
    printf("Input buffer contains: %s\n", data.input);
    printf("Valid buffer contains: %s\n\n", data.valid);

    if (check_password(data.input, data.valid) == 1) {
        printf("Success: Access Granted!\n");
    } else {
        printf("Failure: Access Denied.\n");
    }

    return 0;
}
