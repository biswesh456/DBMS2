#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contact.h"
#include "pds.h"

// Define the global variable
struct PDS_RepoInfo repo_handle;

int pds_open(char *repo_name, int rec_size) {
  char repo_file[30];
  char index_file[30];

  // Check if status is already open in repo_handle
  if (repo_handle.repo_status == PDS_REPO_OPEN)
    return PDS_REPO_ALREADY_OPEN;

  strcpy(repo_handle.pds_name, repo_name);
  repo_handle.rec_size = rec_size;

  // Open the repository file in binary read-write mode
  strcpy(repo_file, repo_name);
  strcat(repo_file, ".dat");
  repo_handle.pds_data_fp = fopen(repo_file, "ab+");

  // Open the index file in binary read-write mode
  strcpy(index_file, repo_name);
  strcat(index_file, ".ndx");
  repo_handle.pds_ndx_fp = fopen(index_file, "ab+");

  if (repo_handle.pds_data_fp == NULL) {
    perror(repo_file);
  }

  if (repo_handle.pds_ndx_fp == NULL) {
    perror(index_file);
  }

  repo_handle.repo_status = PDS_REPO_OPEN;

  return PDS_SUCCESS;
}

// Seek to the end of the data file
// Write the record at the current file location
// Set status to PDS_SUCCESS on success or appropriate error status as defined
// in pds.h

int put_rec_by_key(int key, void *rec) {
  int offset, status, writesize;

  status = fseek(repo_handle.pds_data_fp, 1, SEEK_END);

  if (status) {
    return PDS_ADD_FAILED;
  }

  offset = ftell(repo_handle.pds_data_fp);
  status = fwrite(rec, repo_handle.rec_size, offset, repo_handle.pds_data_fp);

  if (status != 1) {
    return PDS_ADD_FAILED;
  }

  status = fseek(repo_handle.pds_ndx_fp, 1, SEEK_END);

  if (status) {
    return PDS_ADD_FAILED;
  }

  struct PDS_NdxInfo ndx_inf;
  ndx_inf.key = key;
  ndx_inf.offset = offset;
  status = fwrite(&ndx_inf, sizeof(struct PDS_NdxInfo), offset, repo_handle.pds_ndx_fp);

  status = PDS_SUCCESS;

  return status;
}

int get_rec_by_key(int key, void *rec) {

  int offset, status, readsize;

  offset = 0;

  status = fseek(repo_handle.pds_ndx_fp, offset, SEEK_SET);

  if (status) {
    return PDS_FILE_ERROR;
  }

  struct PDS_NdxInfo ndx_inf;
  while (fread(&ndx_inf, sizeof(struct PDS_NdxInfo), 1, repo_handle.pds_ndx_fp)) {
    if (!feof(repo_handle.pds_ndx_fp)) {
      if (ndx_inf.key == key){
        status = fseek(repo_handle.pds_data_fp, ndx_inf.offset-1, SEEK_SET);

        if (status) {
          return PDS_FILE_ERROR;
        }
        
        fread(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp);
        return PDS_SUCCESS;
      }
    }
  }

  if (!feof(repo_handle.pds_data_fp))
    return PDS_FILE_ERROR;

  return PDS_REC_NOT_FOUND;
}

int pds_close() {
  strcpy(repo_handle.pds_name, "");
  fclose(repo_handle.pds_data_fp);
  repo_handle.repo_status = PDS_REPO_CLOSED;

  return PDS_SUCCESS;
}
