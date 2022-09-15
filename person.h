#ifndef PERSON_H
#define PERSON_H

#define PERSON_SUCCESS 0
#define PERSON_FAILURE 1
 
struct Person{
	int person_id;
	char person_name[50];
	char unique_no[50];
};

extern struct PDS_RepoInfo *repoHandle;

// Add the given contact into the repository by calling put_rec_by_key
int add_person( struct Person *c );

// Overwite existing contact with the given contact pds_overwrite
// Hint: call the PDS function as follows
// pds_overwrite(c->person_id, c);
int overwrite_person( struct Person *c );

// Display contact info in a single line as a CSV without any spaces
void print_person( struct Person *c );

// Use get_rec_by_key function to retrieve contact
int search_person( int person_id, struct Person *c );

// Load all the contacts from a CSV file
int store_persons( char *contact_data_file );

// Use get_rec_by_non_ndx_key function to retrieve contact
int search_person_by_unique_no( char *unique_no, struct Person *c, int *io_count );

/* Return 0 if unique_no of the contact matches with unique_no parameter */
/* Return 1 if unique_no of the contact does NOT match */
/* Return > 1 in case of any other error */
int match_person_unique_no( void *rec, void *key );

// Returns id of the first contact in the repository
int get_first_person(int *id);

// Returns id of the last contact in the repository
int get_last_person(int *id);

// Function to delete contact by ID
int delete_person ( int person_id );

// Print all contacts
void print_all();

void printer(void *rec);

#endif
