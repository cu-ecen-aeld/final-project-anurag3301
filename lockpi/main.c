#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "socket.h"
#include "person.h"

extern Person person_list[MAX_PERSON];
extern int person_count;


void display_lcd(char* buf){
    FILE *file = fopen("/dev/lcd1602", "w");
    if (file == NULL) {
        printf("/dev/lcd1602 not found!, maybe driver is not loaded\n");
        return;
    }
    fprintf(file, "%s", buf);
    fclose(file);
}

void door_control(char c){
    FILE *file = fopen("/dev/doorlock", "w");
    if (file == NULL) {
        printf("/dev/doorlock not found!, maybe driver is not loaded\n");
        return;
    }
    fprintf(file, "%c\n", c);
    fclose(file);
}

int main(){
    int server_fd = startSocket();
    init_list();
    char passkey_buf[7] = {0};
    int buf_pos = 0;
    char lcd_buf[33];
    int fd = open("/dev/keypad", O_RDONLY);

    snprintf(lcd_buf, 32, "Enter Passkey\n%s", passkey_buf);
    display_lcd(lcd_buf);
    door_control('0');

    char c;
    while (read(fd, &c, 1) > 0) {
        passkey_buf[buf_pos++] = c;
        snprintf(lcd_buf, 32, "Enter Passkey\n%s", passkey_buf);
        display_lcd(lcd_buf);
        if(buf_pos == 6){
            printf("\nGOT PASSKEY: %s\n", passkey_buf);
            char* name = checkCredentials(passkey_buf);
            if(name == NULL){
                printf("Invalid Credentials\n\n");
                snprintf(lcd_buf, 32, "Invalid Passkey\nTry Again");
                display_lcd(lcd_buf);
            }
            else{
                printf("Welcome %s\n\n", name);
                snprintf(lcd_buf, 32, "Welcome %s\nOpening Doors", name);
                display_lcd(lcd_buf);
                door_control('1');
            }
            buf_pos = 0;
            memset(passkey_buf, '\0', 7);
            sleep(5);
            snprintf(lcd_buf, 32, "Enter Passkey\n%s", passkey_buf);
            display_lcd(lcd_buf);
            door_control('0');
        }
    }
    free_list();
    close(server_fd);
}
