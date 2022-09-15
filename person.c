#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "pds.h"
#include "person.h"

// Load all the contacts from a CSV file
int store_persons( char *contact_data_file )
{
	FILE *cfptr;
	char contact_line[500], token;
	struct Person c, dummy;

	cfptr = (FILE *) fopen(contact_data_file, "r");
	while(fgets(contact_line, sizeof(contact_line)-1, cfptr)){
		//printf("line:%s",contact_line);
		sscanf(contact_line, "%d%s%s", &(c.person_id),c.person_name,c.unique_no);
		print_person( &c );
		add_person( &c );
	}
}

void print_person( struct Person *c )
{
	printf("%d,%s,%s\n", c->person_id,c->person_name,c->unique_no);
}

// Use get_rec_by_key function to retrieve contact
int search_person( int person_id, struct Person *c )
{
	int status = get_rec_by_ndx_key( person_id, c );
	if (status != PERSON_SUCCESS) {
		return PERSON_FAILURE;
	}
	//printf("search result: %d %s %s\n",c->person_id, c->person_name, c->unique_no);
	return PERSON_SUCCESS;
}

// Add the given contact into the repository by calling put_rec_by_key
int add_person( struct Person *c )
{
	int status;

	status = put_rec_by_key( c->person_id, c );

	if( status != PDS_SUCCESS ){
		fprintf(stderr, "Unable to add contact with key %d. Error %d\n", c->person_id, status );
		return PERSON_FAILURE;
	}
	return status;
}

// Use get_rec_by_non_ndx_key function to retrieve contact
// Hint: get_rec_by_non_ndx_key( unique_no, c, &match_contact_unique_no, io_count );
int search_person_by_unique_no( char *unique_no, struct Person *c, int *io_count )
{
	// Call function
	int status = get_rec_by_non_ndx_key(unique_no, c, match_person_unique_no, io_count);
	if (*io_count <= 0) {
		printf("io_count <= 0 error\n");
		return 1;
	}
	return status;
}

/* Return 0 if unique_no of the contact matches with unique_no parameter */
/* Return 1 if unique_no of the contact does NOT match */
/* Return > 1 in case of any other error */
int match_person_unique_no( void *rec, void *key )
{
	// Store the rec in a struct contact pointer
    // Store the key in a char pointer
    // Compare the unique_no values in key and record
    // Return 0,1,>1 based on above condition
	// Store the rec in a struct contact pointer
	struct Person *contact = rec;
    // Store the key in a char pointer
	char *unique_no = key;
    // Compare the unique_no values in key and record
	if (contact == NULL || unique_no == NULL) {
		return 2;
	}
	int cmp = strcmp(contact->unique_no, key);
	if (cmp != 0) {
		return 1;
	}
	return cmp;
}

// Function to delete a record based on ndx_key
int delete_person ( int person_id )
{
	// Call the delete_contact_ndx_key function
	// Return PERSON_SUCCESS or PERSON_FAILURE based on status of above call
	int status = delete_rec_by_ndx_key(person_id);
	if (status != 0) {
		return PERSON_FAILURE;
	}
	return PERSON_SUCCESS;
}

int overwrite_person( struct Person *c ) {
	//struct Person cont;
	//printf("BEFORE\n");
	//printAllRecords(printer, &cont);
	int status = pds_overwrite(c->person_id, c);
	if (status != PERSON_SUCCESS) {
		return PERSON_FAILURE;
	}
	//printf("AFTER\n");
	//printAllRecords(printer, &cont);
	return PERSON_SUCCESS;
}

void print_all() {
	struct Person cont;
	printAllRecords(printer, &cont);
}

void printer(void *rec) {
	struct Person *c = rec;
	printf("%d %s %s\n",c->person_id, c->person_name, c->unique_no);
}

int get_first_person(int *id) {
	return get_id_first_record(id);
}

int get_last_person(int *id) {
	return get_id_last_record(id);
}