#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define MAX_PERSON 50
#define FILEPATH  "passkey.txt"

typedef struct {
    char passkey[7];
    char *name;
} Person;

Person person_list[MAX_PERSON];
int person_count;

int init_list(){
    person_count = 0;
    FILE *file = fopen(FILEPATH, "r");
    if (file == NULL) {
        return 1;
    }

    char line[1024];
    while(fgets(line, sizeof(line), file)){
        memcpy(person_list[person_count].passkey, line, 6); 
        person_list[person_count].passkey[6] = '\0';
        uint name_size = strlen(line)-7;
        person_list[person_count].name = calloc(name_size+1, 1);
        memcpy(person_list[person_count].name, line+6, name_size); 
        person_count++;
    }

    fclose(file);
    return 0;
}

void free_list(){
    for(int i=0; i<person_count; i++){
        free(person_list[i].name);
    }
    person_count = 0;
}

void dump_list(){
    for(int i=0; i<person_count; i++){
        printf("%s %s\n", person_list[i].passkey, person_list[i].name);
    }
}

void add_person(Person p){
    FILE *file = fopen(FILEPATH, "a");
    if (file == NULL) {
        return;
    }
    fprintf(file, "%s%s\n", p.passkey, p.name);
    fclose(file);
    free_list();
    init_list();
}

char* checkCredentials(char* passkey){
    for(int i=0; i<person_count; i++){
        if(strcmp(person_list[i].passkey, passkey) == 0){
            return person_list[i].name;
        }
    }
    return NULL;
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

int main(){
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
}
