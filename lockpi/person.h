
#ifndef __PERSON_H
#define __PERSON_H

#define MAX_PERSON 50
#ifndef FILEPATH
#define FILEPATH "passkey.txt"
#endif

typedef struct {
    char passkey[7];
    char *name;
} Person;

int init_list();
void free_list();
void dump_list();
void add_person(Person p);
char* checkCredentials(char* passkey);

#endif //__SOCKET_H
