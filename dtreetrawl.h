#ifndef DTREETRAWL_H
#define DTREETRAWL_H

#include <glib.h>
#include <time.h>
#include <ftw.h>

struct trawlent {
        const struct stat       *tstat;
        const char              *path;
        const char              *basename;
        unsigned                level;
        unsigned                ndirent;
        char                    *refname;
        char                    *hash;

};

struct dtreestat {
        unsigned long long      ndir;
        unsigned long long      nreg;
        unsigned long long      nlnk;
        unsigned long long      nblk;
        unsigned long long      nchr;
        unsigned long long      nfifo;
        unsigned long long      nsock;
        unsigned long long      nlevel;
        unsigned long long      nsize;
        unsigned long long      nentry;
        double                  elapsed;
        char                    *hash;
        char                    *hash_type;
        char                    *start_local;
        char                    *start_utc;
};


static gchar    *DELIM;
static gchar    *HASH_TYPE;
static gboolean IS_TERSE = FALSE;
static gboolean IS_HASH = FALSE;
static gboolean IS_HASH_SYMLINK = FALSE;
//static gboolean IS_HASH_DIRENT;

static GOptionEntry entries_g[] = {
        { "terse", 't', 0, G_OPTION_ARG_NONE, &IS_TERSE, "Produce a terse output; parsable.", NULL },
        { "delim", 'd', 0, G_OPTION_ARG_STRING, &DELIM, "Character or string delimiter/separator for terse output(default ':')", ":" },
        { "hash", 0, 0, G_OPTION_ARG_NONE, &IS_HASH, "Hash the files to produce checksums(default is MD5).", NULL },
        { "checksum", 'c', 0, G_OPTION_ARG_STRING, &HASH_TYPE, "Valid hashing algorithms: md5, sha1, sha256, sha512.", "md5" },
        { "hash-symlink", 's', 0, G_OPTION_ARG_NONE, &IS_HASH_SYMLINK, "Include symbolic links' referent name while calculating the root checksum", NULL },
        { NULL }
};


static GChecksumType CHECKSUM_G;
static GChecksum *ROOT_CKSUM_G;
static struct dtreestat *DSTAT;

char *time_t_to_utc(time_t st_time);
char *time_t_to_local(time_t st_time);

void output_terse_trawlent(struct trawlent *tent, char *delim);
void output_human_trawlent(struct trawlent *tent);
void output_human_dtreestat(struct dtreestat *dstat);
void output_terse_dtreestat(struct dtreestat *dstat, char *delim);

gchar *get_file_checksum(const char *file_path, GChecksumType checksum_type_g);

int action_trawlent(struct trawlent *tent);
int dtree_check(const char *path, const struct stat *sbuf, int type, struct FTW *ftwb);

#endif
