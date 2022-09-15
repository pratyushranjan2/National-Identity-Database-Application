#include <string.h>
#include <stdio.h>
#include "person.h"
#include "pds.h"

#define MAX_STRING_SIZE 50

void log_status(int status) {
    printf("Status: %d  ", status);
                
    if (status == PERSON_SUCCESS) {
        printf("Success!\n");
    }
    else {
        printf("Failure\n");
    }
}

/*
The deleted records on the datafile have a key of -1, although the id of
person(record) still remains the same. The menu driven command 'a' can
be used to view all the records on the datafile. Deleted ones will have a
key -1.

To begin with press '1' to create a new repository. Provide a name for it 
from CLI. Then open that repository by pressing '2' and providing the 
repository's name. All the operations can then be performed on that datafile.

To exit press 'e'. If exit is not performed with 'e', the changes made in
that session will not be saved in the index file.
*/

int main() {
    printf("\nPress 1 to create a new repository\n");
    printf("Press 2 to open an existing repository\n");
    printf("Press 3 to add a new person\n");
    printf("Press 4 to modify an existing person\n");
    printf("Press 5 to modify the first person\n");
    printf("Press 6 to modify the last person\n");
    printf("Press 7 to delete a person by id\n");
    printf("Press 8 to delete the first person\n");
    printf("Press 9 to delete the last person\n");
    printf("Press f to get a person by id\n");
    printf("press g to search person by unique_no\n");
    printf("Press a to print all reacords in data file\n");
    printf("Press e to exit\n\n");

    while (1) {
        char inp;
        char repo_name[MAX_STRING_SIZE];
        int status, id;
        char buffer[MAX_STRING_SIZE];
        struct Person read;
        printf("Enter your command: ");
        scanf("%s", &inp);

        switch (inp) {
            case '1':
                printf("Enter repository name: ");
                scanf("%s",repo_name);

                log_status(pds_create(repo_name));
                break;

            case '2':
                printf("Enter repository name: ");
                scanf("%s",repo_name);

                log_status(pds_open(repo_name, sizeof(struct Person)));
                break;

            case '3':
                printf("Enter in order\n<id> <name> <unique_no>: ");
                struct Person new_contact = {-1, "na", "na"};
                scanf("%d %s %s", &new_contact.person_id, new_contact.person_name, new_contact.unique_no);

                log_status(add_person(&new_contact));
                break;
            
            case '4':
                printf("Enter in order\n<id> <name> <unique_no>: ");
                struct Person contact = {-1, "na", "na"};
                scanf("%d %s %s", &contact.person_id, contact.person_name, contact.unique_no);

                log_status(overwrite_person(&contact));
                break;

            case '5':
                id = -1;
                log_status(get_first_person(&id));
                if (id != -1) {
                    struct Person modified_contact = {id, "na", "na"};
                    printf("Enter in order\n<name> <unique_no>: ");
                    scanf("%s %s", modified_contact.person_name, modified_contact.unique_no);

                    log_status(overwrite_person(&modified_contact));
                }
                break;
            
            case '6':
                id = -1;
                get_last_person(&id);
                if (id != -1) {
                    log_status(0);
                    struct Person modified_contact = {id, "na", "na"};
                    printf("Enter in order\n<name> <unique_no>: ");
                    scanf("%s %s", modified_contact.person_name, modified_contact.unique_no);

                    log_status(overwrite_person(&modified_contact));
                }
                else {
                    log_status(1);
                }
                break;
            
            case '7':
                printf("id: ");
                scanf("%d", &id);

                log_status(delete_person(id));
                break;

            case '8':
                get_first_person(&id);
                if (id != -1) {
                    log_status(delete_person(id));
                }
                else {
                    log_status(1);
                }
                break;
            
            case '9':
                get_last_person(&id);
                if (id != -1) {
                    log_status(delete_person(id));
                }
                else {
                    log_status(1);
                }
                break;

            case 'f':
                printf("id: ");
                scanf("%d", &id);

                status = search_person(id, &read);
                
                printf("Status: %d  ", status);
                
                if (status == PERSON_SUCCESS) {
                    printf("Success!\n");
                    printf("%d %s %s\n", read.person_id, read.person_name, read.unique_no);
                }
                
                else {
                    printf("Failure\n");
                }
                break;

            case 'g':
                printf("Unique no:: ");
                scanf("%s", buffer);

                int count = 0;
                status = search_person_by_unique_no(buffer, &read, &count);

                printf("Status: %d  ", status);
                
                if (status == PERSON_SUCCESS) {
                    printf("Success!\n");
                    printf("%d %s %s\n", read.person_id, read.person_name, read.unique_no);
                }
                
                else {
                    printf("Failure\n");
                }
                break;
        
            case 'a':
                print_all();
                break;
            
            case 'e':
                log_status(pds_close());
                return 0;
                break;

            default:
                printf("invalid command...\n");
        }
        printf("\n");
    }
}