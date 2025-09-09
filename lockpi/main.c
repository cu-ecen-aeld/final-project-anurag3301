#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "socket.h"
#include "person.h"
#include <time.h>


#define KEYPAD_MAGIC 'K'
#define KEYPAD_FLUSH_BUFFER _IO(KEYPAD_MAGIC, 0)

extern Person person_list[MAX_PERSON];
extern int person_count;

char passkey_buf[7] = {0};
int buf_pos = 0;
char lcd_buf[33];
int keypad_fd = -1;

void add_unlock_entry(const char* name){
    printf("Callig for %s\n", name);
    FILE *file = fopen(UNLOCKLOGFILE, "a");
    if (file == NULL) {
        printf("Count open the log file: %s\n", UNLOCKLOGFILE);
        return;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(file, "%d-%02d-%02d %02d:%02d:%02d: %s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, name);
    fclose(file);
}

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

void clear(){
    buf_pos = 0;
    memset(passkey_buf, '\0', 7);
    if (ioctl(keypad_fd, KEYPAD_FLUSH_BUFFER) == -1) {
        perror("ioctl failed");
    }
}

int main(){
    int server_fd = startSocket();
    init_list();
    keypad_fd = open("/dev/keypad", O_RDONLY);
    clear();
    snprintf(lcd_buf, 32, "Enter Passkey\n%s", passkey_buf);
    display_lcd(lcd_buf);
    door_control('0');

    char c;
    while (read(keypad_fd, &c, 1) > 0) {
        passkey_buf[buf_pos++] = c;
        if(c == '*'){
            printf("Clearing\n");
            clear();
        }
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
                add_unlock_entry(name);
                printf("Welcome %s\n\n", name);
                snprintf(lcd_buf, 32, "Welcome %s\nOpening Doors", name);
                display_lcd(lcd_buf);
                door_control('1');
            }
            clear();
            sleep(5);
            snprintf(lcd_buf, 32, "Enter Passkey\n%s", passkey_buf);
            display_lcd(lcd_buf);
            door_control('0');
        }
    }
    free_list();
    close(server_fd);
}
