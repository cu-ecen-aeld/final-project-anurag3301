#include "person.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "socket.h"

Person person_list[MAX_PERSON];
int person_count;

#define PASSKEYFILE FILEPATH "/passkey.txt"

int init_list(){
    person_count = 0;
    FILE *file = fopen(PASSKEYFILE, "r");
    if (file == NULL) {
        printf("Couldnt open file %s\n", PASSKEYFILE);
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
    FILE *file = fopen(PASSKEYFILE, "a");
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

