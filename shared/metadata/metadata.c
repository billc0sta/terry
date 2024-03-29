#include "metadata.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int valid_str(const char *str) { return str != 0 && str[0] != '\0'; }

struct FileData make_filedata(const char *name, const char *hash,
                              short ud_month, short ud_day, short ud_year,
			      short lu_month, short lu_day, short lu_year, 
                              long size) {
  struct FileData fd;
  bzero(&fd, sizeof(fd));
  fd.name[0] = '\0';
  fd.hash[0] = '\0';
  
  if (valid_str(name))
    strncpy(fd.name, name, NAME_LEN);
  if (valid_str(hash))
    strncpy(fd.hash, hash, HASH_LEN);
  
  fd.upload_date.month = ud_month;
  fd.upload_date.day   = ud_day;
  fd.upload_date.year  = ud_year;

  fd.last_used.month = lu_month;
  fd.last_used.day = lu_day;
  fd.last_used.year = lu_year;
  
  fd.size = size;
  
  return fd;
}

char *serialize(struct FileData *fd) {
  char *ser = malloc(sizeof(char) * FD_SERIALIZE_LEN);
  snprintf(ser, FD_SERIALIZE_LEN,
           "NAME:           %s\n"
	   "HASH:           %s\n"
           "UPLOAD_DATE:    %hi/%hi/%hi\n"
           "LAST_USED_DATE: %hi/%hi/%hi\n"
           "SIZE:           %ld\n",
           !valid_str(fd->name) ? "NULL" : fd->name,
	   !valid_str(fd->hash) ? "NULL" : fd->hash,
           fd->upload_date.month, fd->upload_date.day, fd->upload_date.year,
	   fd->last_used.month, fd->last_used.day, fd->last_used.year,
           fd->size);
  return ser;
}

struct FileData deserialize(const char *fd_ser) {
  struct FileData fd;
  bzero(&fd, sizeof(fd));

  sscanf(fd_ser,
         "NAME:           %s\n"
         "HASH:           %s\n"
         "UPLOAD_DATE:    %hi/%hi/%hi\n"
         "LAST_USED_DATE: %hi/%hi/%hi\n"
         "SIZE:           %ld\n",
         fd.name, fd.hash, &fd.upload_date.month, &fd.upload_date.day,
         &fd.upload_date.year, &fd.last_used.month, &fd.last_used.day,
         &fd.last_used.year, &fd.size);
  return fd;
}

int fast_test() {
  // test make
  struct FileData fd =
    make_filedata("filename", "hash", 2, 22, 2000, 2, 22, 2024, 1000);

  // test serialize
  char *ser = serialize(&fd);
  printf("ser: \n%s\n", ser);

  // test deser  
  struct FileData deser = deserialize(ser);
  char *ser_of_deser = serialize(&deser);

  printf("ser_of_deser: \n%s\n", ser_of_deser);
  int res = strcmp(ser_of_deser, ser) == 0;

  free(ser);
  free(ser_of_deser);
  
  return res;
}

struct Date get_current_time() {
  time_t t = time(0);
  struct tm time = *localtime(&t);
  struct Date d = {
    .day = time.tm_mday,
    .month = time.tm_mon,
    .year = time.tm_year + 1900
  };
  return d; 
}

int save_metadata(const char *path, struct FileData *fd) {
  int success = 0;
  FILE *write_file = fopen(path, "w");
  char *ser = serialize(fd);
  if (write_file == 0) {
    success = -1;
    goto cleanup;
  }
  
  int ser_len = strlen(ser);
  for (int i = 0; i < ser_len; ++i) {
    fputc(ser[i], write_file);
  }
  
 cleanup:
  fclose(write_file);
  free(ser);
  return success;
 }

int open_metadata(const char *file_path, struct FileData *fd) {   
  FILE *file = fopen(file_path, "r");
  if (file == 0)
    return -1;
  char raw[FD_SERIALIZE_LEN];
  int byte = 0;
  for (int i = 0; i < FD_SERIALIZE_LEN && (byte = fgetc(file)) != EOF; ++i) {
    raw[i] = byte;
  }
  *fd = deserialize(raw);
  return 0;
}
