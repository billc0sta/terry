#ifndef METADATA
#define METADATA
#define FD_SERIALIZE_LEN 500
#define NAME_LEN 255
#define HASH_LEN 15

struct Date {
  short month;
  short day;
  short year;
};

struct FileData {
  char name[NAME_LEN];
  struct Date upload_date;
  struct Date last_used;
  char hash[HASH_LEN];
  long size;
};



struct FileData make_filedata(const char *name, const char *hash,
                              short ud_month, short ud_day, short ud_year,
			      short lu_month, short lu_day, short lu_year, 
                              long size);
char *serialize(struct FileData *fd);
struct Date get_current_time();
struct FileData deserialize(const char *fd);
int fast_test();
int save_metadata(const char *path, struct FileData *fd);
int open_metadata(const char *file_path, struct FileData *fd);
#endif 
