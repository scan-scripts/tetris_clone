#include <stdio.h>
#include <string.h>

int main() {
    char name[100];
    printf("What is your name? ");
    
    // Read the entire line, including spaces
    if (fgets(name, sizeof(name), stdin) != NULL) {
        // Remove newline character if present
        size_t len = strlen(name);
        if (len > 0 && name[len - 1] == '\n') {
            name[len - 1] = '\0';
        }
        
        // Handle empty input
        if (name[0] == '\0') {
            printf("Hello\n");
        } else {
            printf("Hello %s\n", name);
        }
    } else {
        printf("Hello\n");
    }
    
    return 0;
}
