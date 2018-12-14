#include <unistd.h> 
#include <stdio.h> 
#include <signal.h> 
void sigroutine(int unused) {
        printf("Catch a signal SIGINT "); 
} 

int main() {
        signal(SIGINT, sigroutine); 
        pause(); 
        printf("receive a signal "); 
}
