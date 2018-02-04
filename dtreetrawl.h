#ifndef DTREETRAWL_H
#define DTREETRAWL_H

#include <glib.h>
#include <time.h>
#include <ftw.h>


extern gint MAX_LEVEL;
extern gchar *DELIM;
extern gchar *HASH_TYPE;
extern gboolean IS_FOLLOW_SYMLINK;
extern gboolean IS_NO_TENT;
extern gboolean IS_PRINT_ONLY_ROOT_HASH;
extern gboolean IS_TERSE;
extern gboolean IS_JSON;
extern gboolean IS_HASH;
extern gboolean IS_HASH_EXCLUDE_NAME;
extern gboolean IS_HASH_EXCLUDE_CONTENT;
extern gboolean IS_HASH_SYMLINK;
extern gboolean IS_HASH_DIRENT;

extern GChecksumType CHECKSUM_G;
extern GChecksum *ROOT_CKSUM_G;
extern struct dtreestat *DSTAT;

struct trawlent {
	const struct stat	*tstat;
	const char		*path;
	const char		*basename;
	unsigned		level;
	unsigned		ndirent;
	char			*refname;
	char			*hash;

};

struct dtreestat {
	unsigned long long	ndir;
	unsigned long long	nreg;
	unsigned long long	nlnk;
	unsigned long long	nblk;
	unsigned long long	nchr;
	unsigned long long	nfifo;
	unsigned long long	nsock;
	unsigned long long	nlevel;
	unsigned long long	nsize;
	unsigned long long	nentry;
	double			elapsed;
	char			*hash;
	char			*hash_type;
	char			*start_local;
	char			*start_utc;
};


char *time_t_to_utc(time_t st_time);
char *time_t_to_local(time_t st_time);

void output_human_trawlent(struct trawlent *tent);
void output_human_json_trawlent(struct trawlent *tent);
void output_terse_trawlent(struct trawlent *tent, char *delim);
void output_terse_json_trawlent(struct trawlent *tent);
void output_human_dtreestat(struct dtreestat *dstat);
void output_human_json_dtreestat(struct dtreestat *dstat);
void output_terse_dtreestat(struct dtreestat *dstat, char *delim);
void output_terse_json_dtreestat(struct dtreestat *dstat);

gchar *get_file_checksum(const char *file_path, GChecksumType checksum_type_g);

int action_trawlent(struct trawlent *tent);
int dtree_check(const char *path, const struct stat *sbuf, int type, struct FTW *ftwb);

#endif
