#define _XOPEN_SOURCE 600
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "dtreetrawl.h"


char *ROOT_PATH			= NULL;
struct dtreestat *DSTAT		= NULL;
gchar *DELIM			= NULL;
gchar *HASH_TYPE		= NULL;
gint MAX_LEVEL 			= -1;
gboolean IS_FOLLOW_SYMLINK	= FALSE;
gboolean IS_NO_TENT		= FALSE;
gboolean IS_PRINT_ONLY_ROOT_HASH= FALSE;
gboolean IS_TERSE		= FALSE;
gboolean IS_JSON		= FALSE;
gboolean IS_HASH		= FALSE;
gboolean IS_HASH_EXCLUDE_NAME	= FALSE;
gboolean IS_HASH_EXCLUDE_CONTENT= FALSE;
gboolean IS_HASH_SYMLINK	= FALSE;
gboolean IS_HASH_DIRENT		= FALSE;

GChecksumType CHECKSUM_G;
GChecksum *ROOT_CKSUM_G		= NULL;
GSequence *all_hash_dump_g 	= NULL;


char *time_t_to_utc(time_t st_time)
{
	GDateTime *gdt;
	gchar *timestr;
	gdt = g_date_time_new_from_unix_utc((gint64) st_time);
	timestr = g_date_time_format(gdt, "%s");
	g_date_time_unref(gdt);

	return timestr;
}


char *time_t_to_local(time_t st_time)
{
	GDateTime *gdt;
	gchar *timestr;
	gdt = g_date_time_new_from_unix_local((gint64) st_time);
	timestr = g_date_time_format(gdt, "%a, %d %b %y %T %z");
	g_date_time_unref(gdt);

	return timestr;
}


void output_terse_trawlent(struct trawlent *tent, char *delim)
{
	fprintf(stdout, "%s%s", delim, delim);
	fprintf(stdout, "%s%s", tent->path, delim);
	fprintf(stdout, "%s%s", tent->basename, delim);
	fprintf(stdout, "%u%s", tent->level, delim);

	switch (tent->tstat->st_mode & S_IFMT) {
	case S_IFBLK:  fprintf(stdout, "BLK%s", delim);		break;
	case S_IFCHR:  fprintf(stdout, "CHR%s", delim);		break;
	case S_IFDIR:  fprintf(stdout, "DIR%s", delim);		break;
	case S_IFIFO:  fprintf(stdout, "FIFO%s", delim);	break;
	case S_IFLNK:  fprintf(stdout, "LNK%s", delim);		break;
	case S_IFREG:  fprintf(stdout, "REG%s", delim);		break;
	case S_IFSOCK: fprintf(stdout, "SOCK%s", delim);	break;
	default:       fprintf(stdout, "?%s", delim);		break;
	}
	fprintf(stdout, "%s%s", (tent->refname != NULL) ? tent->refname : "", delim);
	fprintf(stdout, "%lld%s", (long long) tent->tstat->st_size, delim);
	fprintf(stdout, "%ld%s", (long) tent->tstat->st_ino, delim);
	fprintf(stdout, "%u%s", tent->ndirent, delim);
	fprintf(stdout, "%04lo%s", (unsigned long) (tent->tstat->st_mode & 07777), delim);
	fprintf(stdout, "%ld%s", (long) tent->tstat->st_nlink, delim);
	fprintf(stdout, "%ld,%ld%s", (long) tent->tstat->st_uid, (long) tent->tstat->st_gid, delim);
	fprintf(stdout, "%ld%s", (long) tent->tstat->st_blksize, delim);
	fprintf(stdout, "%lld%s", (long long) tent->tstat->st_blocks, delim);

	char *ctime_g;
	ctime_g = time_t_to_utc(tent->tstat->st_ctime);
	fprintf(stdout, "%s%s", ctime_g, delim);
	free(ctime_g);

	char *atime_g;
	atime_g = time_t_to_utc(tent->tstat->st_atime);
	fprintf(stdout, "%s%s", atime_g, delim);
	free(atime_g);

	char *mtime_g;
	mtime_g = time_t_to_utc(tent->tstat->st_mtime);
	fprintf(stdout, "%s%s", mtime_g, delim);
	free(mtime_g);
	fprintf(stdout, "%s%s", (tent->hash != NULL) ? tent->hash : "", delim);
	fprintf(stdout, "%s\n", delim);

}


void output_terse_json_trawlent(struct trawlent *tent)
{
	fprintf(stdout, "\t\t\t{\n");
	fprintf(stdout, "\t\t\t\t\"path\":\t\t\"%s\",\n", tent->path);
	fprintf(stdout, "\t\t\t\t\"basename\":\t\"%s\",\n", tent->basename);
	fprintf(stdout, "\t\t\t\t\"level\":\t\"%u\",\n", tent->level);

	fprintf(stdout, "\t\t\t\t\"type\":\t\t");
	switch (tent->tstat->st_mode & S_IFMT) {
	case S_IFBLK:  fprintf(stdout, "\"BLK\",\n");	      break;
	case S_IFCHR:  fprintf(stdout, "\"CHR\",\n");	      break;
	case S_IFDIR:  fprintf(stdout, "\"DIR\",\n");	      break;
	case S_IFIFO:  fprintf(stdout, "\"FIFO\",\n");	      break;
	case S_IFLNK:  fprintf(stdout, "\"LNK\",\n");	      break;
	case S_IFREG:  fprintf(stdout, "\"REG\",\n");	      break;
	case S_IFSOCK: fprintf(stdout, "\"SOCK\",\n");	      break;
	default:       fprintf(stdout, "\"?\",\n");	      break;
	}
	fprintf(stdout, "\t\t\t\t\"refname\":\t\"%s\",\n", (tent->refname != NULL) ? tent->refname : "");
	fprintf(stdout, "\t\t\t\t\"size\":\t\t\"%lld\",\n", (long long) tent->tstat->st_size);
	fprintf(stdout, "\t\t\t\t\"inode\":\t\"%ld\",\n", (long) tent->tstat->st_ino);
	fprintf(stdout, "\t\t\t\t\"ndirent\":\t\"%u\",\n", tent->ndirent);
	fprintf(stdout, "\t\t\t\t\"permission\":\t\"%04lo\",\n", (unsigned long) (tent->tstat->st_mode & 07777));
	fprintf(stdout, "\t\t\t\t\"nlink\":\t\"%ld\",\n", (long) tent->tstat->st_nlink);
	fprintf(stdout, "\t\t\t\t\"ownership\":\t\"%ld,%ld\",\n", (long) tent->tstat->st_uid, (long) tent->tstat->st_gid);
	fprintf(stdout, "\t\t\t\t\"blksize\":\t\"%ld\",\n", (long) tent->tstat->st_blksize);
	fprintf(stdout, "\t\t\t\t\"nblocks\":\t\"%lld\",\n", (long long) tent->tstat->st_blocks);

	char *ctime_g;
	ctime_g = time_t_to_utc(tent->tstat->st_ctime);
	fprintf(stdout, "\t\t\t\t\"ctime\":\t\"%s\",\n", ctime_g);
	free(ctime_g);

	char *atime_g;
	atime_g = time_t_to_utc(tent->tstat->st_atime);
	fprintf(stdout, "\t\t\t\t\"atime\":\t\"%s\",\n", atime_g);
	free(atime_g);

	char *mtime_g;
	mtime_g = time_t_to_utc(tent->tstat->st_mtime);
	fprintf(stdout, "\t\t\t\t\"mtime\":\t\"%s\",\n", mtime_g);
	free(mtime_g);

	fprintf(stdout, "\t\t\t\t\"hash\":\t\t\"%s\"\n", (tent->hash != NULL) ? tent->hash : "");
	fprintf(stdout, "\t\t\t},\n");

}


void output_human_json_trawlent(struct trawlent *tent)
{
	fprintf(stdout, "\t\t\t{\n");
	fprintf(stdout, "\t\t\t\t\"path\":\t\t\"%s\",\n", tent->path);
	fprintf(stdout, "\t\t\t\t\"basename\":\t\"%s\",\n", tent->basename);
	fprintf(stdout, "\t\t\t\t\"level\":\t\"%u\",\n", tent->level);

	fprintf(stdout, "\t\t\t\t\"type\":\t\t");
	switch (tent->tstat->st_mode & S_IFMT) {
	case S_IFBLK:  fprintf(stdout, "\"block device\",\n");		break;
	case S_IFCHR:  fprintf(stdout, "\"character device\",\n");	break;
	case S_IFDIR:  fprintf(stdout, "\"directory\",\n");		break;
	case S_IFIFO:  fprintf(stdout, "\"FIFO\",\n");			break;
	case S_IFLNK:  fprintf(stdout, "\"symlink\",\n");		break;
	case S_IFREG:  fprintf(stdout, "\"regular file\",\n");		break;
	case S_IFSOCK: fprintf(stdout, "\"socket\",\n");		break;
	default:       fprintf(stdout, "\"unknown?\",\n");		break;
	}
	fprintf(stdout, "\t\t\t\t\"refname\":\t\"%s\",\n", (tent->refname != NULL) ? tent->refname : "");
	fprintf(stdout, "\t\t\t\t\"size\":\t\t\"%lld bytes\",\n", (long long) tent->tstat->st_size);
	fprintf(stdout, "\t\t\t\t\"inode\":\t\"%ld\",\n", (long) tent->tstat->st_ino);
	fprintf(stdout, "\t\t\t\t\"ndirent\":\t\"%u\",\n", tent->ndirent);
	fprintf(stdout, "\t\t\t\t\"permission\":\t\"%04lo\",\n", (unsigned long) (tent->tstat->st_mode & 07777));
	fprintf(stdout, "\t\t\t\t\"nlink\":\t\"%ld\",\n", (long) tent->tstat->st_nlink);
	fprintf(stdout, "\t\t\t\t\"ownership\":\t\"uid:%ld, gid:%ld\",\n", (long) tent->tstat->st_uid, (long) tent->tstat->st_gid);
	fprintf(stdout, "\t\t\t\t\"blksize\":\t\"%ld bytes\",\n", (long) tent->tstat->st_blksize);
	fprintf(stdout, "\t\t\t\t\"nblocks\":\t\"%lld\",\n", (long long) tent->tstat->st_blocks);

	char *ctime_g;
	ctime_g = time_t_to_local(tent->tstat->st_ctime);
	fprintf(stdout, "\t\t\t\t\"ctime\":\t\"%s\",\n", ctime_g);
	free(ctime_g);

	char *atime_g;
	atime_g = time_t_to_local(tent->tstat->st_atime);
	fprintf(stdout, "\t\t\t\t\"atime\":\t\"%s\",\n", atime_g);
	free(atime_g);

	char *mtime_g;
	mtime_g = time_t_to_local(tent->tstat->st_mtime);
	fprintf(stdout, "\t\t\t\t\"mtime\":\t\"%s\",\n", mtime_g);
	free(mtime_g);

	fprintf(stdout, "\t\t\t\t\"hash\":\t\t\"%s\"\n", (tent->hash != NULL) ? tent->hash : "");
	fprintf(stdout, "\t\t\t},\n");

}


void output_human_trawlent(struct trawlent *tent)
{
	fprintf(stdout, "\n%s\n", tent->path);
        fprintf(stdout, "\tBase name                    : %s\n", tent->basename);
        fprintf(stdout, "\tLevel                        : %u\n", tent->level);
        fprintf(stdout, "\tType                         : ");

	switch (tent->tstat->st_mode & S_IFMT) {
	case S_IFBLK:  fprintf(stdout, "block device\n");	     break;
	case S_IFCHR:  fprintf(stdout, "character device\n");	     break;
	case S_IFDIR:  fprintf(stdout, "directory\n");		     break;
	case S_IFIFO:  fprintf(stdout, "FIFO/pipe\n");		     break;
	case S_IFLNK:  fprintf(stdout, "symlink\n");		     break;
	case S_IFREG:  fprintf(stdout, "regular file\n");	     break;
	case S_IFSOCK: fprintf(stdout, "socket\n");		     break;
	default:       fprintf(stdout, "unknown?\n");		     break;
	}
        fprintf(stdout, "\tReferent name                : %s\n", (tent->refname != NULL) ? tent->refname : "");
        fprintf(stdout, "\tFile size                    : %lld bytes\n",
		(long long) tent->tstat->st_size);

        fprintf(stdout, "\tI-node number                : %ld\n", (long) tent->tstat->st_ino);
        fprintf(stdout, "\tNo. directory entries        : %u\n", tent->ndirent);
        fprintf(stdout, "\tPermission (octal)           : %04lo\n",
		(unsigned long) (tent->tstat->st_mode & 07777));
        fprintf(stdout, "\tLink count                   : %ld\n", (long) tent->tstat->st_nlink);
        fprintf(stdout, "\tOwnership                    : UID=%ld, GID=%ld\n",
		(long) tent->tstat->st_uid, (long) tent->tstat->st_gid);
        fprintf(stdout, "\tPreferred I/O block size     : %ld bytes\n",
		(long) tent->tstat->st_blksize);
        fprintf(stdout, "\tBlocks allocated             : %lld\n",
		(long long) tent->tstat->st_blocks);

	char *ctime_g;
	ctime_g = time_t_to_local(tent->tstat->st_ctime);
        fprintf(stdout, "\tLast status change           : %s\n", ctime_g);
	free(ctime_g);

	char *atime_g;
	atime_g = time_t_to_local(tent->tstat->st_atime);
        fprintf(stdout, "\tLast file access             : %s\n", atime_g);
	free(atime_g);

	char *mtime_g;
	mtime_g = time_t_to_local(tent->tstat->st_mtime);
        fprintf(stdout, "\tLast file modification       : %s\n", mtime_g);
	free(mtime_g);

        fprintf(stdout, "\tHash                         : %s\n", (tent->hash != NULL) ? tent->hash : "");

}


void output_human_json_dtreestat(struct dtreestat *dstat)
{
	fprintf(stdout, "\t\t\t{\n");
	fprintf(stdout, "\t\t\t\t\"root_path\":\t\t\t\"%s\",\n", ROOT_PATH);
	fprintf(stdout, "\t\t\t\t\"elapsed\":\t\t\t\"%f\",\n", dstat->elapsed);
	fprintf(stdout, "\t\t\t\t\"start_time\":\t\t\t\"%s\",\n", dstat->start_local);
	fprintf(stdout, "\t\t\t\t\"follow_symlink\":\t\t\"%s\",\n", (IS_FOLLOW_SYMLINK) ? "yes" : "no");
	fprintf(stdout, "\t\t\t\t\"hash_exclude_name\":\t\t\"%s\",\n", (IS_HASH_EXCLUDE_NAME) ? "yes" : "no");
	fprintf(stdout, "\t\t\t\t\"hash_exclude_content\":\t\t\"%s\",\n", (IS_HASH_EXCLUDE_CONTENT) ? "yes" : "no");
	fprintf(stdout, "\t\t\t\t\"hash_symlink\":\t\t\t\"%s\",\n", (IS_HASH_SYMLINK) ? "yes" : "no");
	fprintf(stdout, "\t\t\t\t\"hash_dirent\":\t\t\t\"%s\",\n", (IS_HASH_DIRENT) ? "yes" : "no");
	fprintf(stdout, "\t\t\t\t\"hash\":\t\t\t\t\"%s\",\n", (dstat->hash != NULL) ? dstat->hash : "");
	fprintf(stdout, "\t\t\t\t\"hash_type\":\t\t\t\"%s\",\n", (dstat->hash_type != NULL) ? dstat->hash_type : "");
	fprintf(stdout, "\t\t\t\t\"nlevel\":\t\t\t\"%llu\",\n", dstat->nlevel);
	fprintf(stdout, "\t\t\t\t\"nsize\":\t\t\t\"%llu bytes\",\n", dstat->nsize);
	fprintf(stdout, "\t\t\t\t\"nentry\":\t\t\t\"%llu\",\n", dstat->nentry);
	fprintf(stdout, "\t\t\t\t\"ndir\":\t\t\t\t\"%llu\",\n", dstat->ndir);
	fprintf(stdout, "\t\t\t\t\"nreg\":\t\t\t\t\"%llu\",\n", dstat->nreg);
	fprintf(stdout, "\t\t\t\t\"nlnk\":\t\t\t\t\"%llu\",\n", dstat->nlnk);
	fprintf(stdout, "\t\t\t\t\"nblk\":\t\t\t\t\"%llu\",\n", dstat->nblk);
	fprintf(stdout, "\t\t\t\t\"nchr\":\t\t\t\t\"%llu\",\n", dstat->nchr);
	fprintf(stdout, "\t\t\t\t\"nsock\":\t\t\t\"%llu\",\n", dstat->nsock);
	fprintf(stdout, "\t\t\t\t\"nfifo\":\t\t\t\"%llu\"\n", dstat->nfifo);
	fprintf(stdout, "\t\t\t},\n");
}


void output_human_dtreestat(struct dtreestat *dstat)
{
	fprintf(stdout, "\nStats for %s:\n", ROOT_PATH);
        fprintf(stdout, "\tElapsed time                 : %f s\n", dstat->elapsed);
        fprintf(stdout, "\tStart time                   : %s\n", dstat->start_local);
	fprintf(stdout, "\tFollow Symlink               : %s\n", (IS_FOLLOW_SYMLINK) ? "yes" : "no");
	fprintf(stdout, "\tExclude name hash            : %s\n", (IS_HASH_EXCLUDE_NAME) ? "yes" : "no");
	fprintf(stdout, "\tExclude content hash         : %s\n", (IS_HASH_EXCLUDE_CONTENT) ? "yes" : "no");
	fprintf(stdout, "\tHash symlink ref name        : %s\n", (IS_HASH_SYMLINK) ? "yes" : "no");
	fprintf(stdout, "\tHash directory entry names   : %s\n", (IS_HASH_DIRENT) ? "yes" : "no");
        fprintf(stdout, "\tRoot hash                    : %s\n", (dstat->hash != NULL) ? dstat->hash : "");
        fprintf(stdout, "\tHash type                    : %s\n", (dstat->hash_type != NULL) ? dstat->hash_type : "");
        fprintf(stdout, "\tDepth                        : %llu\n", dstat->nlevel);
        fprintf(stdout, "\tSize                         : %llu bytes\n", dstat->nsize);
        fprintf(stdout, "\tEntries                      : %llu\n", dstat->nentry);
        fprintf(stdout, "\tDirectories                  : %llu\n", dstat->ndir);
        fprintf(stdout, "\tRegular files                : %llu\n", dstat->nreg);
        fprintf(stdout, "\tSymlinks                     : %llu\n", dstat->nlnk);
        fprintf(stdout, "\tBlock devices                : %llu\n", dstat->nblk);
        fprintf(stdout, "\tChar devices                 : %llu\n", dstat->nchr);
        fprintf(stdout, "\tSockets                      : %llu\n", dstat->nsock);
        fprintf(stdout, "\tFIFOs/pipes                  : %llu\n", dstat->nfifo);
	fprintf(stdout, "\n");
}


void output_terse_json_dtreestat(struct dtreestat *dstat)
{
	fprintf(stdout, "\t\t\t{\n");
	fprintf(stdout, "\t\t\t\t\"root_path\":\t\t\t\"%s\",\n", ROOT_PATH);
	fprintf(stdout, "\t\t\t\t\"elapsed\":\t\t\t\"%f\",\n", dstat->elapsed);
	fprintf(stdout, "\t\t\t\t\"start_time\":\t\t\t\"%s\",\n", dstat->start_utc);
	fprintf(stdout, "\t\t\t\t\"follow_symlink\":\t\t\"%s\",\n", (IS_FOLLOW_SYMLINK) ? "yes" : "no");
	fprintf(stdout, "\t\t\t\t\"hash_exclude_name\":\t\t\"%s\",\n", (IS_HASH_EXCLUDE_NAME) ? "yes" : "no");
	fprintf(stdout, "\t\t\t\t\"hash_exclude_content\":\t\t\"%s\",\n", (IS_HASH_EXCLUDE_CONTENT) ? "yes" : "no");
	fprintf(stdout, "\t\t\t\t\"hash_symlink\":\t\t\t\"%s\",\n", (IS_HASH_SYMLINK) ? "yes" : "no");
	fprintf(stdout, "\t\t\t\t\"hash_dirent\":\t\t\t\"%s\",\n", (IS_HASH_DIRENT) ? "yes" : "no");
	fprintf(stdout, "\t\t\t\t\"hash\":\t\t\t\t\"%s\",\n", (dstat->hash != NULL) ? dstat->hash : "");
	fprintf(stdout, "\t\t\t\t\"hash_type\":\t\t\t\"%s\",\n", (dstat->hash_type != NULL) ? dstat->hash_type : "");
	fprintf(stdout, "\t\t\t\t\"nlevel\":\t\t\t\"%llu\",\n", dstat->nlevel);
	fprintf(stdout, "\t\t\t\t\"nsize\":\t\t\t\"%llu\",\n", dstat->nsize);
	fprintf(stdout, "\t\t\t\t\"nentry\":\t\t\t\"%llu\",\n", dstat->nentry);
	fprintf(stdout, "\t\t\t\t\"ndir\":\t\t\t\t\"%llu\",\n", dstat->ndir);
	fprintf(stdout, "\t\t\t\t\"nreg\":\t\t\t\t\"%llu\",\n", dstat->nreg);
	fprintf(stdout, "\t\t\t\t\"nlnk\":\t\t\t\t\"%llu\",\n", dstat->nlnk);
	fprintf(stdout, "\t\t\t\t\"nblk\":\t\t\t\t\"%llu\",\n", dstat->nblk);
	fprintf(stdout, "\t\t\t\t\"nchr\":\t\t\t\t\"%llu\",\n", dstat->nchr);
	fprintf(stdout, "\t\t\t\t\"nsock\":\t\t\t\"%llu\",\n", dstat->nsock);
	fprintf(stdout, "\t\t\t\t\"nfifo\":\t\t\t\"%llu\"\n", dstat->nfifo);
	fprintf(stdout, "\t\t\t},\n");
}


void output_terse_dtreestat(struct dtreestat *dstat, char *delim)
{
	fprintf(stdout, "%s", delim);
	fprintf(stdout, "%s%s", ROOT_PATH, delim);
	fprintf(stdout, "%f%s", dstat->elapsed, delim);
	fprintf(stdout, "%s%s", dstat->start_utc, delim);
	fprintf(stdout, "%s%s", (IS_FOLLOW_SYMLINK) ? "yes" : "no", delim);
	fprintf(stdout, "%s%s", (IS_HASH_EXCLUDE_NAME) ? "yes" : "no", delim);
	fprintf(stdout, "%s%s", (IS_HASH_EXCLUDE_CONTENT) ? "yes" : "no", delim);
	fprintf(stdout, "%s%s", (IS_HASH_SYMLINK) ? "yes" : "no", delim);
	fprintf(stdout, "%s%s", (IS_HASH_DIRENT) ? "yes" : "no", delim);
	fprintf(stdout, "%s%s", (dstat->hash != NULL) ? dstat->hash : "", delim);
	fprintf(stdout, "%s%s", (dstat->hash_type != NULL) ? dstat->hash_type : "", delim);
	fprintf(stdout, "%llu%s", dstat->nlevel, delim);
	fprintf(stdout, "%llu%s", dstat->nsize, delim);
	fprintf(stdout, "%llu%s", dstat->nentry, delim);
	fprintf(stdout, "%llu%s", dstat->ndir, delim);
	fprintf(stdout, "%llu%s", dstat->nreg, delim);
	fprintf(stdout, "%llu%s", dstat->nlnk, delim);
	fprintf(stdout, "%llu%s", dstat->nblk, delim);
	fprintf(stdout, "%llu%s", dstat->nchr, delim);
	fprintf(stdout, "%llu%s", dstat->nsock, delim);
	fprintf(stdout, "%llu%s", dstat->nfifo, delim);
	fprintf(stdout, "\n\n");
}


gchar *get_dirent_checksum(struct dirent **entrylist, int count, GChecksumType checksum_type_g)
{
	gchar *hash_str;
	GChecksum *cksum_g = NULL;
	cksum_g = g_checksum_new(checksum_type_g);
	if (cksum_g == NULL) {
		fprintf(stderr, "g_checksum_new returned NULL\n");
		return NULL;
	}

	int i = 0;
	while (i < count) {
		g_checksum_update(cksum_g, (guchar *) entrylist[i], (gssize) strlen((char *)entrylist[i]));
		i++;
	}
	hash_str = g_strdup(g_checksum_get_string(cksum_g));
	g_checksum_free(cksum_g);

	return hash_str;
}

gchar *get_file_checksum(const char *file_path, GChecksumType checksum_type_g)
{
	int rfd;
	int open_flags;
	ssize_t snr;
	char rbuf[4096];

	open_flags =	O_RDONLY	|
			O_NOFOLLOW	|
			O_NOATIME	|
			O_LARGEFILE	|
			O_NOCTTY	|
			O_NONBLOCK;
	rfd = open(file_path, open_flags);
	if (rfd == -1) {
		fprintf(stderr, "%s\n", file_path);
		perror("open");
		return NULL;
	}

	gchar *hash_str;
	GChecksum *cksum_g = NULL;
	cksum_g = g_checksum_new(checksum_type_g);
	if (cksum_g == NULL) {
		fprintf(stderr, "g_checksum_new returned NULL\n");
		return NULL;
	}

	while (pread(rfd, NULL, (size_t) 1, lseek(rfd, 0, SEEK_CUR))) {
		if ((snr = read(rfd, &rbuf, (ssize_t) 4096)) != -1) {
			if (snr == 0) {
				break;
			}
			g_checksum_update(cksum_g, (guchar *) rbuf, (gssize) snr);
		} else {
			perror("read");
			return NULL;
		}
	}
	close(rfd);
	hash_str = g_strdup(g_checksum_get_string(cksum_g));
	g_checksum_free(cksum_g);

	return hash_str;
}


void update_root_checksum(guchar *hash_str)
{
	if (hash_str) {
		g_checksum_update(ROOT_CKSUM_G, (guchar *) hash_str, (gssize) strlen((char *) hash_str));
	}
}

int append_to_hash_dump(GSequence *seq, const char *hashstr)
{
	GSequenceIter *iterp;
	char *hstrp;
	hstrp = strdup(hashstr);
	iterp = g_sequence_append(seq, hstrp);
	if (iterp == NULL) {
		return 1;
	} else {
		return 0;
	}
}

void update_root_checksum_for_each(char *hashstr, gpointer *user_data)
{
	update_root_checksum((guchar *)hashstr);
}

int sequence_compare_data_g(gpointer a, gpointer b, gpointer *user_data)
{
	return strcmp((char *)a, (char *)b);
}

int action_trawlent(struct trawlent *tent)
{
	if (S_ISDIR(tent->tstat->st_mode)) {
		DSTAT->ndir += 1;
		tent->refname = NULL;
		tent->hash = NULL;

		struct dirent **entrylist;
		int nentry;
		nentry = scandir(tent->path, &entrylist, NULL, alphasort);
		if (nentry < 0) {
			perror("scandir");
			return -1;
		}
		tent->ndirent = nentry;

		if (IS_HASH && !IS_HASH_EXCLUDE_CONTENT) {
			char *cksum = NULL;
			cksum = (char *) get_dirent_checksum(entrylist, nentry, CHECKSUM_G);
			if (cksum == NULL) {
				return -1;
			}
			tent->hash = cksum;
		}

		while (nentry--)
			free(entrylist[nentry]);
		free(entrylist);

	} else if (S_ISREG(tent->tstat->st_mode)) {
		DSTAT->nreg += 1;
		tent->ndirent = 0;
		tent->refname = NULL;

		char *cksum = NULL;
		if (IS_HASH && !IS_HASH_EXCLUDE_CONTENT) {
			cksum = (char *) get_file_checksum(tent->path, CHECKSUM_G);
			if (cksum == NULL) {
				return -1;
			}
		}
		tent->hash = cksum;

	} else if (S_ISLNK(tent->tstat->st_mode)) {
		DSTAT->nlnk += 1;
		tent->ndirent = 0;

		char *linkname;
		ssize_t nb;
		linkname = calloc(1, tent->tstat->st_size + 1);
		if (linkname == NULL) {
			fprintf(stderr, "Insufficient memory!\n");
			return -1;
		}
		nb = readlink(tent->path, linkname, tent->tstat->st_size + 1);
		if (nb == -1) {
			perror("readlink");
			return -1;
		}
		if (nb > tent->tstat->st_size) {
			fprintf(stderr, "Symlink increased in size while processing\n");
			return -1;
		}
		linkname[nb] = '\0';
		tent->refname = linkname;

		char *cksum = NULL;
		if (IS_HASH && !IS_HASH_EXCLUDE_CONTENT) {
			cksum = (char *) g_compute_checksum_for_string(CHECKSUM_G, (gchar *) tent->path, (gssize) strlen(tent->path));
			if (cksum == NULL) {
				return -1;
			}
		}
		tent->hash = cksum;

	} else if (S_ISCHR(tent->tstat->st_mode)) {
		DSTAT->nchr += 1;
		tent->refname = NULL;
		tent->hash = NULL;
		tent->ndirent = 0;
	} else if (S_ISBLK(tent->tstat->st_mode)) {
		DSTAT->nblk += 1;
		tent->refname = NULL;
		tent->hash = NULL;
		tent->ndirent = 0;
	} else if (S_ISFIFO(tent->tstat->st_mode)) {
		DSTAT->nfifo += 1;
		tent->refname = NULL;
		tent->hash = NULL;
		tent->ndirent = 0;
	} else if (S_ISSOCK(tent->tstat->st_mode)) {
		DSTAT->nsock += 1;
		tent->refname = NULL;
		tent->hash = NULL;
		tent->ndirent = 0;
	}

	return FTW_CONTINUE;
}


int dtree_check(const char *path, const struct stat *sbuf, int type,
			struct FTW *ftwb)
{
	if (ftwb->level == 0 && type == FTW_D) {
		return FTW_CONTINUE;
	}
	if (MAX_LEVEL != -1 && ftwb->level > MAX_LEVEL) {
		return FTW_SKIP_SIBLINGS;
	}
	switch(type) {
	case FTW_DNR:
	case FTW_NS:
		fprintf(stderr, "Could not read from path %s\n", path);
		return FTW_STOP;
	case FTW_D:
	case FTW_F:
	case FTW_SL: {
		struct trawlent tent = {0};
		tent.tstat = sbuf;
		tent.path = path;
		tent.basename = path + ftwb->base;
		tent.level = ftwb->level;
		if (DSTAT->nlevel < (unsigned) ftwb->level) {
			DSTAT->nlevel = ftwb->level;
		}
		DSTAT->nentry += 1;
		DSTAT->nsize += tent.tstat->st_size;

		if (action_trawlent(&tent)) {
			return FTW_STOP;
		}

		if (IS_HASH) {
			if (!IS_HASH_EXCLUDE_CONTENT) {
				if (S_ISREG(tent.tstat->st_mode)) {
					if (append_to_hash_dump(all_hash_dump_g, tent.hash) == 1) {
						fprintf(stderr, "Failed to append the hash sequence.\n");
					}
				} else if (S_ISLNK(tent.tstat->st_mode) && IS_HASH_SYMLINK) {
					if (append_to_hash_dump(all_hash_dump_g, tent.hash) == 1) {
						fprintf(stderr, "Failed to append the hash sequence.\n");
					}
				} else if (S_ISDIR(tent.tstat->st_mode) && IS_HASH_DIRENT) {
					if (append_to_hash_dump(all_hash_dump_g, tent.hash) == 1) {
						fprintf(stderr, "Failed to append the hash sequence.\n");
					}
				}
			}
			if (!IS_HASH_EXCLUDE_NAME) {
				if (append_to_hash_dump(all_hash_dump_g, path) == 1) {
					fprintf(stderr, "Failed to append the hash sequence.\n");
				}
			}
		}

		if (!IS_PRINT_ONLY_ROOT_HASH && !IS_NO_TENT) {
			if (IS_TERSE) {
				if (IS_JSON) {
					output_terse_json_trawlent(&tent);
				} else {
					output_terse_trawlent(&tent, DELIM);
				}
			} else {
				if (IS_JSON) {
					output_human_json_trawlent(&tent);
				} else {
					output_human_trawlent(&tent);
				}
			}
		}
		free(tent.hash);
		free(tent.refname);

		return FTW_CONTINUE;
	}
	case FTW_SLN:
                fprintf(stderr, "File path is a symbolic link pointing to "
				"a nonexistent file %s. Skipping check!\n", path);
		return FTW_CONTINUE;
	default:
		fprintf(stderr, "Unexpected situation, exiting!\n");
		return FTW_STOP;
	}

}

int main(int argc, char *argv[])
{
	GTimer *timer_g		= NULL;
	GDateTime *gdt		= NULL;
	gchar *now_local	= NULL;
	gchar *now_utc		= NULL;
	GOptionContext *argctx	= NULL;
	GError *error_g 	= NULL;


	GOptionEntry entries_g[] = {
		{ "terse", 't', 0, G_OPTION_ARG_NONE, &IS_TERSE, "Produce a terse output; parsable.", NULL },
		{ "json", 'j', 0, G_OPTION_ARG_NONE, &IS_JSON, "Output as JSON", NULL },
		{ "delim", 'd', 0, G_OPTION_ARG_STRING, &DELIM, "Character or string delimiter/separator for terse output(default ':')", ":" },
		{ "max-level", 'l', 0, G_OPTION_ARG_INT, &MAX_LEVEL, "Do not traverse tree beyond N level(s)", "N" },
		{ "follow-symlink", 'f', 0, G_OPTION_ARG_NONE, &IS_FOLLOW_SYMLINK, "Follow symbolic links", NULL },
		{ "no-tent", 'T', 0, G_OPTION_ARG_NONE, &IS_NO_TENT, "Output only the summary(dstat), no other entries", NULL },
		{ "hash", 0, 0, G_OPTION_ARG_NONE, &IS_HASH, "Enable hashing(default is MD5).", NULL },
		{ "checksum", 'c', 0, G_OPTION_ARG_STRING, &HASH_TYPE, "Valid hashing algorithms: md5, sha1, sha256, sha512.", "md5" },
		{ "only-root-hash", 'R', 0, G_OPTION_ARG_NONE, &IS_PRINT_ONLY_ROOT_HASH, "Output only the root hash. Blank line if --hash is not set", NULL },
		{ "no-name-hash", 'N', 0, G_OPTION_ARG_NONE, &IS_HASH_EXCLUDE_NAME, "Exclude path name while calculating the root checksum", NULL },
		{ "no-content-hash", 'F', 0, G_OPTION_ARG_NONE, &IS_HASH_EXCLUDE_CONTENT, "Do not hash the contents of the file", NULL },
		{ "hash-symlink", 's', 0, G_OPTION_ARG_NONE, &IS_HASH_SYMLINK, "Include symbolic links' referent name while calculating the root checksum", NULL },
		{ "hash-dirent", 'e', 0, G_OPTION_ARG_NONE, &IS_HASH_DIRENT, "Include hash of directory entries while calculating root checksum", NULL },
		{ NULL }
	};

	argctx = g_option_context_new("\"/trawl/me\" [path2,...]");
	(void) g_option_context_add_main_entries(argctx, entries_g, NULL);
	(void) g_option_context_set_description(argctx, "Please report bugs at https://github.com/six-k/dtreetrawl or ramsri.hp@gmail.com");
	(void) g_option_context_set_summary(argctx, "dtreetrawl trawls/traverses directory tree or file path" \
				" to collect stats of every entry in the tree.");

	if (!g_option_context_parse(argctx, &argc, &argv, &error_g)) {
		fprintf(stderr, "Failed parsing arguments: %s\n", error_g->message);
		exit(EXIT_FAILURE);
	}

	if (argc < 2) {
		fprintf(stderr, "%s\n", g_option_context_get_help(argctx, FALSE, NULL));
		exit(EXIT_FAILURE);
	}

	all_hash_dump_g = g_sequence_new(free);

	if (!DELIM) {
		DELIM = (gchar *) strdup(":");
		if (!DELIM) {
			perror("strdup");
			exit(EXIT_FAILURE);
		}
	}

	if (IS_HASH) {
		if (HASH_TYPE) {
			if (strcmp((char *)HASH_TYPE, "sha1") == 0) {
				CHECKSUM_G = G_CHECKSUM_SHA1;
			} else if (strcmp((char *)HASH_TYPE, "sha256") == 0){
				CHECKSUM_G = G_CHECKSUM_SHA256;
			} else if (strcmp((char *)HASH_TYPE, "sha512") == 0){
				CHECKSUM_G = G_CHECKSUM_SHA512;
			} else {
				CHECKSUM_G = G_CHECKSUM_MD5;
				HASH_TYPE = strdup("md5");
				if (HASH_TYPE == NULL) {
					perror("strdup");
					exit(EXIT_FAILURE);
				}
			}
		} else {
			CHECKSUM_G = G_CHECKSUM_MD5;
			HASH_TYPE = strdup("md5");
			if (HASH_TYPE == NULL) {
				perror("strdup");
				exit(EXIT_FAILURE);
			}
		}
	} else {
		HASH_TYPE = NULL;
	}

	int nftw_ret;
	int ftw_flags;
	if (IS_FOLLOW_SYMLINK) {
		ftw_flags       = FTW_ACTIONRETVAL;
	} else {
		ftw_flags       = FTW_PHYS      |
				/* FTW_MOUNT    | */
				FTW_ACTIONRETVAL;
	}

	if (IS_JSON && !IS_PRINT_ONLY_ROOT_HASH) {
		fprintf(stdout, "{\n");
	}

	int i = argc - 1;
	while (i) {
		struct dtreestat dstat = {0};
		DSTAT = &dstat;

		ROOT_PATH = realpath(argv[i], NULL);
		if (ROOT_PATH == NULL) {
			perror("realpath");
			exit(EXIT_FAILURE);
		}

		ROOT_CKSUM_G = g_checksum_new(CHECKSUM_G);
		if (ROOT_CKSUM_G == NULL) {
			fprintf(stderr, "g_checksum_new returned NULL\n");
			exit(EXIT_FAILURE);
		}

		timer_g = g_timer_new();
		gdt = g_date_time_new_now_utc();
		now_utc = g_date_time_format(gdt, "%s");
		now_local = time_t_to_local(g_date_time_to_unix(gdt));
		g_date_time_unref(gdt);

		if (IS_JSON && !IS_PRINT_ONLY_ROOT_HASH) {
			fprintf(stdout, "\t\"%s\": {\n", ROOT_PATH);
			fprintf(stdout, "\t\t\"tentry\": [\n");
		}

		nftw_ret = nftw(ROOT_PATH, dtree_check, 30, ftw_flags);
		if (nftw_ret == -1) {
			perror("nftw");
			exit(EXIT_FAILURE);
		}
		if (nftw_ret == FTW_STOP) {
			fprintf(stderr, "Could not continue the trawl\n");
			exit(EXIT_FAILURE);
		}

		g_timer_stop(timer_g);
		DSTAT->elapsed = g_timer_elapsed(timer_g, NULL);
		g_timer_destroy(timer_g);

		DSTAT->start_local = now_local;
		DSTAT->start_utc = now_utc;

		gchar *root_hash_str = NULL;
		if (IS_HASH) {
			g_sequence_sort(all_hash_dump_g,
					(GCompareDataFunc)sequence_compare_data_g,
					NULL);
			g_sequence_foreach(all_hash_dump_g,
					(GFunc)update_root_checksum_for_each,
					NULL);
			g_sequence_free(all_hash_dump_g);
			root_hash_str = g_strdup(g_checksum_get_string(ROOT_CKSUM_G));
			g_checksum_free(ROOT_CKSUM_G);
			DSTAT->hash = root_hash_str;
			DSTAT->hash_type = HASH_TYPE;
		}

		if (!IS_PRINT_ONLY_ROOT_HASH) {
			if (IS_JSON) {
				fprintf(stdout, "\t\t{}\n");
				fprintf(stdout, "\t\t],\n");
				fprintf(stdout, "\t\t\"dstat\": [\n");
				if (IS_TERSE) {
					output_terse_json_dtreestat(DSTAT);
				} else {
					output_human_json_dtreestat(DSTAT);
				}

				fprintf(stdout, "\t\t{}\n");
				fprintf(stdout, "\t\t]\n");
				fprintf(stdout, "\t},\n");
			} else {
				if (IS_TERSE) {
					output_terse_dtreestat(DSTAT, DELIM);
				} else {
					output_human_dtreestat(DSTAT);
				}
			}
		} else {
			fprintf(stdout, "%s\n", (char *) root_hash_str ? (char *) root_hash_str : "");
		}
		g_free(root_hash_str);
		g_free(now_local);
		g_free(now_utc);

		i--;

	}

	if (IS_JSON && !IS_PRINT_ONLY_ROOT_HASH) {
		fprintf(stdout, "\t\"\":\"\"\n");
		fprintf(stdout, "}\n");
	}


	exit(EXIT_SUCCESS);
}
