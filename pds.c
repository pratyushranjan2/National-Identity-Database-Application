#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>

#include "pds.h"
#include "bst.h"

struct PDS_RepoInfo repo_handle;

// struct Contact{
//   int id;
//   char name[30];
//   int age;
// };


int pds_create(char *repo_name) 
{
  char filename[30], indexfile[30];
  strcpy(filename,repo_name);
  strcpy(indexfile,repo_name);
  strcat(filename,".dat");
  strcat(indexfile,".ndx");
  FILE *fp = fopen(filename,"wb+");
  FILE *ifp = fopen(indexfile,"wb+");
  if(fp  == NULL || ifp == NULL) return PDS_FILE_ERROR;\
  fclose(fp);
  fclose(ifp);
  
  return PDS_SUCCESS;
}


int pds_open(char* repo_name, int rec_size) // Same as before
{
// Open the data file and index file in rb+ mode
// Update the fields of PDS_RepoInfo appropriately
// Build BST and store in pds_bst by reading index entries from the index file
// Close only the index file
    char filename[30], indexfile[30];
    strcpy(filename,repo_name);
    strcpy(indexfile,repo_name);
    strcat(filename,".dat");
    strcat(indexfile,".ndx");
    
    if (strcmp(repo_name, repo_handle.pds_name) == 0 && repo_handle.repo_status == PDS_REPO_OPEN) {
        return PDS_REPO_ALREADY_OPEN;
    }
// Open the data file and index file in rb+ mode
    repo_handle.pds_data_fp = fopen(filename,"r+");
    repo_handle.pds_ndx_fp = fopen(indexfile,"r");
    if (repo_handle.pds_data_fp == NULL || repo_handle.pds_ndx_fp == NULL) {
        return PDS_FILE_ERROR;
    }
// Update the fields of PDS_RepoInfo appropriately
    strcpy(repo_handle.pds_name, repo_name);
    repo_handle.repo_status = PDS_REPO_OPEN;
    repo_handle.rec_size = rec_size;
// Build BST and store in pds_bst by reading index entries from the index file
    int status = pds_load_ndx();
    if (status != 0) {
      return PDS_FILE_ERROR;
    }
// Close only the index file
    fclose(repo_handle.pds_ndx_fp);
    repo_handle.pds_ndx_fp = NULL;
    return PDS_SUCCESS;
}

int pds_load_ndx() // Same as before
{
// Internal function used by pds_open to read index entries into BST
    struct PDS_NdxInfo ndx_info;
    while(fread(&ndx_info, sizeof(struct PDS_NdxInfo), 1, repo_handle.pds_ndx_fp)) {
        struct PDS_NdxInfo *data = (struct PDS_NdxInfo*)malloc(sizeof(struct PDS_NdxInfo));
        data->key = ndx_info.key;
        data->offset = ndx_info.offset;
        int status = bst_add_node(&repo_handle.pds_bst, ndx_info.key, data);
        if (status != 0) {
            return PDS_LOAD_NDX_FAILED;
        }
    }
    return PDS_SUCCESS;
}

int put_rec_by_key(int key, void*rec)
{
  // Seek to the end of the data file
  // Create an index entry with the current data file location using ftell
  // (NEW) ENSURE is_deleted is set to 0 when creating index entry
  // Add index entry to BST using offset returned by ftell
  // Write the key at the current data file location
  // Write the record after writing the key
    if (repo_handle.repo_status != PDS_REPO_OPEN) {
        return PDS_REPO_CLOSED;
    }
    struct BST_Node *node = bst_search(repo_handle.pds_bst, key);
    struct PDS_NdxInfo *node_data;
    if (node != NULL) {
        node_data = node->data;
        if (node_data->is_deleted == 0){
          return PDS_ADD_FAILED;
        }
    }
  // Seek to the end of the data file
    int status;
    status = fseek(repo_handle.pds_data_fp, 0, SEEK_END);
    if (status != 0) {
      return PDS_ADD_FAILED;
    }
// Create an index entry with the current data file location using ftell
    int offset = ftell(repo_handle.pds_data_fp)/(sizeof(int) + repo_handle.rec_size);
    printf("offset: %d\n", offset);
    if (offset < 0) {
        return PDS_FILE_ERROR;
    }
// Add index entry to BST using offset returned by ftell
    // int* offset_ptr = (int*)malloc(1*sizeof(int));
    // *offset_ptr = offset;
    struct PDS_NdxInfo *data = (struct PDS_NdxInfo*)malloc(sizeof(struct PDS_NdxInfo));
    data->key = key;
    data->offset = offset;
    data->is_deleted = 0;

    // status = fseek(repo_handle.pds_data_fp,0,SEEK_END);
    // if (status != 0) {
    //     return PDS_FILE_ERROR;
    // }
    // Write the key at the current data file location
    status = fwrite(&key,sizeof(int),1,repo_handle.pds_data_fp);
    if (status == 0) {
        return PDS_ADD_FAILED;
    }
// Write the record after writing the key
    status = fwrite(rec,repo_handle.rec_size,1,repo_handle.pds_data_fp);
    if (status == 0) {
        return PDS_ADD_FAILED;
    }

    // If node already present with same key but in deleted form, 
    // we overwrite the node
    if (node != NULL && node_data->is_deleted == 1) {
      node->data = data;
    } 
    else {
      bst_add_node(&repo_handle.pds_bst, key, data);
    }
    
    return PDS_SUCCESS;
}

int get_rec_by_ndx_key(int key, void*rec)
{
  // Search for index entry in BST
  // (NEW) Check if the entry is deleted and if it is deleted, return PDS_REC_NOT_FOUND
  // Seek to the file location based on offset in index entry
  // Read the key at the current file location 
  // Read the record after reading the key
    if (repo_handle.repo_status != PDS_REPO_OPEN) {
        return PDS_REPO_CLOSED;
    }
    int status;
  // Search for index entry in BST
    struct BST_Node *node = bst_search(repo_handle.pds_bst, key);
    if (node == NULL) {
        //printf("Node was null\n");
        return PDS_REC_NOT_FOUND;
    }
    // int *offset_ptr = node->data;
// Seek to the file location based on offset in index entry

    struct PDS_NdxInfo *data = node->data;
    if (data->is_deleted == 1) {
      //printf("record deleted\n");
      return PDS_REC_NOT_FOUND; // rec has been deleted
    }
    int offset = data->offset;printf("offset: %d\n", offset);
    
    status = fseek(repo_handle.pds_data_fp, offset*(sizeof(int)+repo_handle.rec_size), SEEK_SET);
    if (status != 0) {
        return PDS_FILE_ERROR;
    }
// Read the key at the current file location
    int rec_key;
    status = fread(&rec_key, sizeof(int), 1, repo_handle.pds_data_fp);
    if (status == 0) {
        return PDS_FILE_ERROR;
    }
// Read the record after reading the key
    status = fread(rec,repo_handle.rec_size,1,repo_handle.pds_data_fp);
    if (status == 0) {
        return PDS_FILE_ERROR;
    }

    return PDS_SUCCESS;
}

int pds_unload_bst(struct BST_Node *root) {
    // traverses bst in pre-order and unloads the data into the index file
    if (root != NULL) {
        int key = root->key;
        // int *offset_ptr = root->data;
        struct PDS_NdxInfo *data = root->data;
        int offset = data->offset;
        struct PDS_NdxInfo ndx_data = { key, offset };
        if (data->is_deleted == 0) {
            int status = fwrite(&ndx_data, sizeof(struct PDS_NdxInfo), 1, repo_handle.pds_ndx_fp);
            if (status == 0) {
                return PDS_FILE_ERROR;
            }
        }
        int left_status =  pds_unload_bst(root->left_child);
        int right_status = pds_unload_bst(root->right_child);
        return left_status & right_status;
    }

    return PDS_SUCCESS;
}



int pds_close() 
{
// Open the index file in wb mode (write mode, not append mode)
// Unload the BST into the index file by traversing it in PRE-ORDER (overwrite the entire index file)
// (NEW) Ignore the index entries that have already been deleted. 
// Free the BST by calling bst_destroy()
// Close the index file and data file
    int status;
    char indexfile[30];
    strcpy(indexfile,repo_handle.pds_name);
    strcat(indexfile,".ndx");
// Open the index file in wb mode (write mode, not append mode)
// Unload the BST into the index file by traversing it in PRE-ORDER (overwrite the entire index file)
// Free the BST by calling bst_destroy()
    if (repo_handle.repo_status == PDS_REPO_OPEN) {
        repo_handle.pds_ndx_fp = fopen(indexfile, "w");
        if (repo_handle.pds_ndx_fp == NULL) {
            return PDS_NDX_SAVE_FAILED;
        }
        status = pds_unload_bst(repo_handle.pds_bst);
        if (status != PDS_SUCCESS) {
            return PDS_NDX_SAVE_FAILED;
        }
        bst_destroy(repo_handle.pds_bst);
        
        if (fclose(repo_handle.pds_ndx_fp) || fclose(repo_handle.pds_data_fp)) {
            return PDS_FILE_ERROR;
        }
        repo_handle.repo_status = PDS_REPO_CLOSED;
        repo_handle.pds_bst = NULL;
        repo_handle.pds_data_fp = NULL;
        repo_handle.pds_ndx_fp = NULL;
    
        return PDS_SUCCESS;
    }
    return PDS_REPO_CLOSED;

}

int get_rec_by_non_ndx_key(void *key, void *rec, int (*matcher)(void *rec, void *key), int *io_count)
{
  // Seek to beginning of file
  // Perform a table scan - iterate over all the records
  //   Read the key and the record
  //   Increment io_count by 1 to reflect count no. of records read
  //   Use the function in function pointer to compare the record with required key
  //   (NEW) Check the entry of the record in the BST and see if it is deleted. If so, return PDS_REC_NOT_FOUND
  // Return success when record is found
    int status;
    status = fseek(repo_handle.pds_data_fp, 0, SEEK_SET);
    if (status != 0) {
      return PDS_FILE_ERROR;
    }
    int currentKey;
    while (fread(&currentKey, sizeof(int), 1, repo_handle.pds_data_fp)) {
      status = fread(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp);
      if (status == 0) {
        return PDS_FILE_ERROR;
      }
      *io_count = *io_count + 1;
      if (matcher(rec, key) == PDS_SUCCESS) {
        struct BST_Node *node = bst_search(repo_handle.pds_bst, currentKey);
        if (node == NULL) {
          return PDS_FILE_ERROR;
        }
        struct PDS_NdxInfo *data = node->data;
        if (data->is_deleted == 1) {
          return PDS_REC_NOT_FOUND;
        }
        return PDS_SUCCESS;
      } 
    }
    return PDS_REC_NOT_FOUND;
}

int delete_rec_by_ndx_key( int key) // New Function
{
  // Search for the record in the BST using the key
  // If record not found, return PDS_DELETE_FAILED
  // If record is found, check if it has already been deleted, if so return PDS_DELETE_FAILED
  // Else, set the record to deleted and return PDS_SUCCESS
    struct BST_Node *node = bst_search(repo_handle.pds_bst, key);
    if (node == NULL) {
      return PDS_DELETE_FAILED;
    }
    struct PDS_NdxInfo *data = node->data;
    if (data->is_deleted == 1) {
      return PDS_DELETE_FAILED;
    }
    data->is_deleted = 1;
    /*node->key = -1;*/
    // if you do node->key=-1, it will break the bst. because there is an
    // ordering in bst relative to key. you break that ordering when you
    // assign node->key = -1 to any node already placed.

    int offset = data->offset;
    int status = fseek(repo_handle.pds_data_fp, offset*(sizeof(int)+repo_handle.rec_size), SEEK_SET);
    if (status != 0) {
        return PDS_FILE_ERROR;
    }
    // The deleted records will get assigned the key value of -1, although
    // the id of the original record stays intact
    int del_mark = -1;
    status = fwrite(&del_mark, sizeof(int), 1, repo_handle.pds_data_fp);
    if (status != 1) {
      return PDS_DELETE_FAILED;
    }

    return PDS_SUCCESS;
}

int pds_overwrite(int key, void *rec) {
    struct BST_Node *node = bst_search(repo_handle.pds_bst, key);
    if (node == NULL) {
      return PDS_UPDATE_FAILED;
    }
    struct PDS_NdxInfo *data = node->data;
    if (data == NULL) {
      return PDS_UPDATE_FAILED;
    }
    if (data->is_deleted == 1) {
      return PDS_UPDATE_FAILED;
    }

    int offset = data->offset;
    int status = fseek(repo_handle.pds_data_fp, offset*(sizeof(int)+repo_handle.rec_size)+sizeof(int), SEEK_SET);

    if (status != 0) {
      return PDS_UPDATE_FAILED;
    }
    
    status = fwrite(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp);
  
    if (status != 1) {
      return PDS_UPDATE_FAILED;
    }

    return PDS_SUCCESS;
}

void printAllRecords(void (*printer)(void *rec), void *read) {
    fseek(repo_handle.pds_data_fp,0,SEEK_SET);
    int key;
    while(fread(&key, sizeof(int), 1, repo_handle.pds_data_fp)) {
      printf("Key: %d    ", key);
      fread(read, repo_handle.rec_size, 1, repo_handle.pds_data_fp);
      printer(read);
    }
}

int get_id_first_record(int *id) {
  int status = fseek(repo_handle.pds_data_fp, 0, SEEK_SET);
  if (status != 0) {
    return PDS_FILE_ERROR;
  }

  status = fread(id, sizeof(int), 1, repo_handle.pds_data_fp);
  if (status != 1) {
    return PDS_FILE_ERROR;
  }

  while (*id) {
    if (*id != -1) {
      return PDS_SUCCESS;
    }
    status = fseek(repo_handle.pds_data_fp, repo_handle.rec_size, SEEK_CUR);
    if (status != 0) {
      return PDS_FILE_ERROR;
    }
    status = fread(id, sizeof(int), 1, repo_handle.pds_data_fp);
    if (status != 1) {
      return PDS_FILE_ERROR;
    }
  }

  return PDS_SUCCESS;
}

int get_id_last_record(int *id) {
  int curr_id = -1;
  
  int status = fseek(repo_handle.pds_data_fp, 0, SEEK_SET);
  if (status != 0) {
    return PDS_FILE_ERROR;
  }

  status = fread(&curr_id, sizeof(int), 1, repo_handle.pds_data_fp);
  if (status != 1) {
    return PDS_FILE_ERROR;
  }

  while (curr_id) {
    if (curr_id != -1) {
      *id = curr_id;
    }
    status = fseek(repo_handle.pds_data_fp, repo_handle.rec_size, SEEK_CUR);
    if (status != 0) {
      return PDS_FILE_ERROR;
    }
    status = fread(&curr_id, sizeof(int), 1, repo_handle.pds_data_fp);
    if (status != 1) {
      return PDS_FILE_ERROR;
    }
  }

  return PDS_SUCCESS;
}

// int main(void) {
//   printf("%d\n",pds_create("testing"));
//   printf("%d\n",pds_open("testing", sizeof(struct Contact)));
  
//   struct Contact c1 = {1001, "A", 10};
//   struct Contact c2 = {1002, "B", 20};
//   struct Contact c3 = {1003, "C", 30};
//   struct Contact ovrt = {1003, "CcCc", 3030};
//   struct Contact read;

//   printf("%d\n",put_rec_by_key(1001, &c1));
//   printf("%d\n",put_rec_by_key(1002, &c2));
//   printf("%d\n",put_rec_by_key(1003, &c3));

//   // printf("***\n");
//   // printf("%d\n",pds_overwrite(1003, &ovrt));
//   // printf("@@@\n");
//   printf("%d\n",delete_rec_by_ndx_key(ovrt.id));
//   printf("%d\n",put_rec_by_key(ovrt.id, &ovrt));
//   // printf("%d\n",get_rec_by_ndx_key(1003, &read));
//   // printf("%d  %s  %d\n",read.id,read.name,read.age);
//   printf("%d\n",pds_close());

//   FILE *fp = fopen("testing.dat", "r");
//   int key;
//   fread(&key, sizeof(int), 1, fp);
//   while (fread(&read, sizeof(struct Contact), 1, fp)) {
//     printf("key: %d     ",key);
//     printf("%d  %s  %d\n",read.id,read.name,read.age);
//     fread(&key, sizeof(int), 1, fp);
//   }
//   fclose(fp);
// }