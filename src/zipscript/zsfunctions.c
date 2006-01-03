#include <errno.h>
#include <fnmatch.h>
#include <strings.h>
#include <stdarg.h>

#include "zsfunctions.h"

#include "abs2rel.h"
#include "constants.h"
#include "convert.h"
#include "race-file.h"
#include "errors.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef HAVE_STRLCPY_H
# include "strlcpy.h"
#endif

/*
 * d_log - create/put comments in a .debug file
 * Last revised by: js
 *        Revision: r1217
 */
void 
d_log(int level, char *fmt,...)
{
#if ( debug_mode == TRUE )
	time_t		timenow;
	FILE           *file;
	va_list		ap;
# if ( debug_altlog == TRUE )
	static char	debugpath[PATH_MAX];
	static char	debugname[PATH_MAX];
# else
	static char	debugname[] = ".debug";
# endif
#endif

	if (fmt == NULL || level > debug_level)
		return;
#if ( debug_mode == TRUE )
	va_start(ap, fmt);
	timenow = time(NULL);

# if ( debug_altlog == TRUE )
	getcwd(debugpath, PATH_MAX);
	snprintf(debugname, PATH_MAX, "%s/%s/.debug",
	         storage, debugpath);
# endif

	if ((file = fopen(debugname, "a+"))) {
		fprintf(file, "%.24s - %.6d - ", ctime(&timenow), getpid());
		vfprintf(file, fmt, ap);
		fclose(file);
	}
	
	chmod(debugname, 0666);
	va_end(ap);
#endif

	return;
}

/*
 * create_missing - create a <filename>-missing 0byte file.
 * Last revised by: js
 *        Revision: 1218
 */
void 
create_missing(char *f)
{
	char	fname[NAME_MAX];

	snprintf(fname, NAME_MAX, "%s-missing", f);
	createzerofile(fname);
}

/*
 * findfilext - find a filename with a matching extension in current dir.
 * Last Modified by: d1
 *         Revision: r1 (2002.01.16)
 */
char *
findfileext(char *path, char *fileext)
{
	int			k;
	static DIR		*dir;
	static struct dirent	*dp;

	if (!(dir = opendir(path))) {
		d_log(1, "opendir(%s): %s\n", path, strerror(errno));
		return NULL;
	}
	
	while ((dp = readdir(dir))) {
		if ((k = NAMLEN(dp)) < 4)
			continue;
		if (strcasecmp(dp->d_name + k - 4, fileext) == 0) {
			closedir(dir);
			return dp->d_name;
		}
	}

	closedir(dir);

	return NULL;
}

char *
findfileext_dir(DIR *dir, char *fileext)
{
	int			k;
	static struct dirent	*dp;

	rewinddir(dir);
	while ((dp = readdir(dir))) {
		if ((k = NAMLEN(dp)) < 4)
			continue;
		if (strcasecmp(dp->d_name + k - 4, fileext) == 0) {
			return dp->d_name;
		}
	}

	return NULL;
}

/*
 * findfilextparent - find a filename with a matching extension in parent dir.
 * Last Modified by: psxc
 *         Revision: ?? (2004.10.06)
 */
/*char *
findfileextparent(DIR *dir, char *fileext)
{
	int			k;
	static struct dirent	*dp;

	rewinddir(dir);
	while ((dp = readdir(dir))) {
		if ((k = NAMLEN(dp)) < 4)
			continue;
		if (strcasecmp(dp->d_name + k - 4, fileext) == 0) {
			return dp->d_name;
		}
	}
	return NULL;
}*/

/*
 * findfilextcount - count files with given extension
 * Last Modified by: daxxar
 *         Revision: ?? (2003.12.11)
 */
int 
findfileextcount(char *path, char *fileext)
{
	int			fnamelen, c = 0;
	static struct dirent	*dp;
	static DIR		*dir;
	
	dir = opendir(path);

	while ((dp = readdir(dir))) {
		if ((fnamelen = NAMLEN(dp)) < 4)
			continue;
		if (!strcasecmp((dp->d_name + fnamelen - 4), fileext))
			c++;
	}

	closedir(dir);
	
	return c;
}

int
check_dupefile(char *path, char *fname)
{
	int			 found = 0;
	static struct dirent	*dp;
	static DIR		*dir;

	dir = opendir(path);

	while ((dp = readdir(dir)))
		if (strcasecmp(dp->d_name, fname) == 0)
			found++;

	closedir(dir);
	
	return (found - 1);
}

/*
 * hexstrtodec - make sure a valid hex number is present in sfv
 * Last modified by: psxc
 *         Revision: r1219
 */
unsigned int 
hexstrtodec(char *s)
{
	unsigned int	n = 0;
	unsigned char	r;

	if (((int)strlen(s) > 8 ) || (!(int)strlen(s)))
		return 0;

	while (1) {
		if ((unsigned char)*s >= 48 && (unsigned char)*s <= 57) {
			r = 48;
		} else if ((unsigned char)*s >= 65 && (unsigned char)*s <= 70) {
			r = 55;
		} else if ((unsigned char)*s >= 97 && (unsigned char)*s <= 102) {
			r = 87;
		} else if ((unsigned char)*s == 0) {
			return n;
		} else {
			return 0;
		}
		n <<= 4;
		n += *s++ - r;
	}
}

/*
 * selector - dangling links
 * Last modified by: js(?)
 *         Revision: ??
 */
/*
 * dangling links
 */
#if defined(__linux__)
int 
selector(const struct dirent *d)
#elif defined(__NetBSD__)
	int		selector   (const struct dirent *d)
#else
int 
selector(struct dirent *d)
#endif
{
	struct stat	st;
	if ((stat(d->d_name, &st) < 0))
		return 0;
	return 1;
}

/*
 * del_releasedir - remove all files in dir path.
 * Last modified by: psxc
 *         Revision: ??
 */
void 
del_releasedir(char *path, char *relname)
{
	static struct dirent	*dp;
	static DIR		*dir;

	dir = opendir(path);
	
	while ((dp = readdir(dir)))
		unlink(dp->d_name);

	rmdir(relname);

	closedir(dir);
}


/*
 * strtolower - make a string all lowercase
 * Last modified by: d1
 *         Revision: ?? (2002.01.16)
 */
void 
strtolower(char *s)
{
	while ((*s = tolower(*s)))
		s++;
}

/*
 * unlink_missing - remove <filename>-missing and <filename>.bad
 * Last modified by: psxc
 *         Revision: r1221
 */
void 
unlink_extra(char *fname, char *end)
{
	static char	file[NAME_MAX];
	long		loc;

	snprintf(file, NAME_MAX, "%s%s", fname, end);
	unlink(file);
#if (sfv_cleanup_lowercase)
	strtolower(file);
	unlink(file);
#endif
	if ((loc = findfile(".", file)))
		remove_at_loc(".", loc);
}

void
unlink_missing(char *fname)
{
	unlink_extra(fname, "-missing");
	unlink_extra(fname, ".bad");
}
	
/*
 * israr - define a file as rar.
 * Last modified by: d1
 *         Revision: ?? (2002.01.16)
 */
char 
israr(char *fileext)
{
	if ((*fileext == 'r' || *fileext == 's' || isdigit(*fileext)) &&
	    ((isdigit(*(fileext + 1)) && isdigit(*(fileext + 2))) ||
	     (*(fileext + 1) == 'a' && *(fileext + 2) == 'r')) &&
	    *(fileext + 3) == 0)
		return 1;
	return 0;
}

/*
 * Created    : 02.20.2002 Author     : dark0n3
 * 
 * Description: Checks if file is known mpeg/avi file
 */
char 
isvideo(char *fileext)
{
	switch (*fileext++) {
		case 'm':
			if (!memcmp(fileext, "pg", 3) ||
			    !memcmp(fileext, "peg", 4) ||
			    !memcmp(fileext, "2v", 3) ||
			    !memcmp(fileext, "2p", 3))
				return 1;
			break;
		case 'a':
			if (!memcmp(fileext, "vi", 3))
				return 1;
			break;
	}

	return 0;
}

/*
 * Modified: 2004-11-17 (psxc) - added support to modify the chars used in the progressbar
 */
void 
buffer_progress_bar(struct VARS *raceI)
{
	int		n;

	raceI->misc.progress_bar[14] = 0;
	if (raceI->total.files > 0) {
		for (n = 0; n < (raceI->total.files - raceI->total.files_missing) * 14 / raceI->total.files; n++)
			raceI->misc.progress_bar[n] = *charbar_filled;
		for (; n < 14; n++)
			raceI->misc.progress_bar[n] = *charbar_missing;
	}
}

/*
 * Modified: 01.16.2002
 */
void 
move_progress_bar(unsigned char delete, struct VARS *raceI, struct USERINFO *userI, struct GROUPINFO *groupI)
{
	char           *bar;
	char	       *delbar = 0, regbuf[100];
	int		m = 0, regret;
	regex_t		preg;
	regmatch_t	pmatch[1];

	DIR		*dir;
	struct dirent *dp;

	d_log(1, "move_progress_bar: del_progressmeter: %s\n", del_progressmeter);
	delbar = convert5(del_progressmeter);
	d_log(1, "move_progress_bar: del_progressmeter: %s\n", delbar);
	//d_log(1, "move_progress_bar: raceI->total.files: %i\n", raceI->total.files);
	//d_log(1, "move_progress_bar: raceI->total.files_missing: %i\n", raceI->total.files_missing);
	regret = regcomp(&preg, delbar, REG_NEWLINE | REG_EXTENDED);
	if (!regret) {
		if ((dir = opendir("."))) {

			if (delete) {
				while ((dp = readdir(dir))) {
					if (regexec(&preg, dp->d_name, 1, pmatch, 0) == 0) {
						d_log(1, "move_progress_bar: Found progress bar, removing\n");
						remove(dp->d_name);
						*dp->d_name = 0;
						m = 1;
					}
				}
				if (!m)
					d_log(1, "move_progress_bar: Progress bar could not be deleted, not found!\n");
			} else {
				if (!raceI->total.files) {
					closedir(dir);
					regfree(&preg);
					return;
				}
	
				bar = convert(raceI, userI, groupI, progressmeter);
				while ((dp = readdir(dir))) {
					if (regexec(&preg, dp->d_name, 1, pmatch, 0) == 0) {
						if (!m) {
							d_log(1, "move_progress_bar: Found progress bar, renaming.\n");
							rename(dp->d_name, bar);
							m = 1;
						} else {
							d_log(1, "move_progress_bar: Found (extra) progress bar, removing\n");
							remove(dp->d_name);
							*dp->d_name = 0;
							m = 2;
						}
					}
				}
				if (!m) {
					d_log(1, "move_progress_bar: Progress bar could not be moved, creating a new one now!\n");
					createstatusbar(bar);
				}
			}
			closedir(dir);
		} else
			d_log(1, "move_progress_bar: opendir() failed : %s\n", strerror(errno));
		d_log(1, "move_progress_bar: Freeing regpointer\n");
	} else {
		regerror(regret, &preg, regbuf, sizeof(regbuf));
		d_log(1, "move_progress_bar: regex failed: %s\n", regbuf);
	}
	regfree(&preg);
}

/*
 * Modified: Unknown
 */
long
findfile(char *path, char *filename)
{
	static off_t		dirloc;
	static struct dirent	*dp;
	static DIR		*dir;

	dir = opendir(path);
	
	while ((dp = readdir(dir))) {
#if (sfv_cleanup_lowercase)
		if (!strcasecmp(dp->d_name, filename))
#else
		if (!strcmp(dp->d_name, filename))
#endif
		{
			dirloc = telldir(dir);
			closedir(dir);
			return dirloc;
		}
	}

	closedir(dir);

	return 0;
}

long
findfilenocase(DIR *dir, char *filename)
{
	struct dirent	*dp;

	rewinddir(dir);
	while ((dp = readdir(dir)))
		if (!strcasecmp(dp->d_name, filename))
			return telldir(dir);
	return 0;
}


void
removedotfiles(DIR *dir)
{
	struct dirent *dp;

	rewinddir(dir);
	while ((dp = readdir(dir)))
		if ((!strncasecmp(dp->d_name, ".", 1)) &&
		    (strlen(dp->d_name) > 2))
			unlink(dp->d_name);
}

char *
findfilename(char *filename, char *dest)
{
	DIR		*dir;
	struct dirent 	*dp;

	dir = opendir(".");
	while ((dp = readdir(dir))) {
		if (strlen(dp->d_name) && !strcasecmp(dp->d_name, filename)) {
			dest = ng_realloc(dest, sizeof(dp->d_name) + 1, 1, 1, 0);
			strncpy(dest, dp->d_name, sizeof(dp->d_name));
			break;
		}
	}
	
	closedir(dir);
	return dest;
}

/*
 * Modified: 01.16.2002
 */
void 
removecomplete()
{
	char		*mydelbar = 0, regbuf[100];
	regex_t		preg;
	regmatch_t	pmatch[1];
	int		regret;

	DIR		*dir;
	struct dirent	*dp;

#ifdef message_file_name
		unlink(message_file_name);
#endif
	
	mydelbar = convert5(del_completebar);
	d_log(1, "removecomplete: del_completebar: %s\n", mydelbar);
	regret = regcomp(&preg, mydelbar, REG_NEWLINE | REG_EXTENDED);
	if (!regret) {
		if ((dir = opendir("."))) {
			while ((dp = readdir(dir))) {
				if (regexec(&preg, dp->d_name, 1, pmatch, 0) == 0) {
					if ((int)pmatch[0].rm_so == 0 && (int)pmatch[0].rm_eo == (int)NAMLEN(dp)) {
						remove(dp->d_name);
						*dp->d_name = 0;
					}
				}
			}
			closedir(dir);
		} else
			d_log(1, "removecomplete: opendir failed : %s\n", strerror(errno));
	} else {
		regerror(regret, &preg, regbuf, sizeof(regbuf));
		d_log(1, "move_progress_bar: regex failed: %s\n", regbuf);
	}
	regfree(&preg);
}

/*
 * Modified: 01.16.2002
 */
short int 
matchpath(char *instr, char *path)
{
	int		pos = 0;

	if ( (int)strlen(instr) < 2 || (int)strlen(path) < 2 )
		return 0;
	do {
		switch (*instr) {
		case 0:
		case ' ':
			if (!strncmp(instr - pos, path, pos - 1)) {
				return 1;
			}
			pos = 0;
			break;
		default:
			pos++;
			break;
		}
	} while (*instr++);
	return 0;
}

/*
 * Modified: 01.16.2002
 */
short int 
strcomp(char *instr, char *searchstr)
{
	int		pos = 0,	k;

	k = (int)strlen(searchstr);

	if ( (int)strlen(instr) == 0 || k == 0 )
		return 0;

	do {
		switch (*instr) {
		case 0:
		case ',':
			if (k == pos && !strncasecmp(instr - pos, searchstr, pos)) {
				return 1;
			}
			pos = 0;
			break;
		default:
			pos++;
			break;
		}
	} while (*instr++);
	return 0;
}

short int 
matchpartialpath(char *instr, char *path)
{
	int	pos = 0;
	char	partstring[PATH_MAX + 2];

	if ( (int)strlen(instr) < 2 || (int)strlen(path) < 2 )
		return 0;

	sprintf(partstring, "%s/", path);
	do {
		switch (*instr) {
		case 0:
		case ' ':
			if (!strncasecmp(instr - pos, partstring + (int)strlen(partstring) - pos, pos)) {
				return 1;
			}
			pos = 0;
			break;
		default:
			pos++;
			break;
		}
	} while (*instr++);
	return 0;
}


/* check for matching subpath
   psxc - 2004-12-18
 */
short int 
subcomp(char *directory)
{
	int 	k = strlen(directory);
	int	m = strlen(subdir_list);
	int	pos = 0, l = 0, n = 0, j = 0;
	char	tstring[m + 1];

	if ( k < 2 )
		return 0;

	do {
		switch (subdir_list[l]) {
		case 0:
			break;
		case ',':
			tstring[j] = '\0';
			if (k <= j && !strncasecmp(tstring, directory, j - n)) {
				return 1;
			}
			pos = l;
			n = 0;
			j=0;
			break;
		case '?':
			tstring[j] = subdir_list[l];
			tstring[j+1] = '\0';
			n++;
			j++;
			break;
		default:
			tstring[j] = subdir_list[l];
			tstring[j+1] = '\0';
			pos++;
			j++;
			break;
		}
	m--;
	l++;
	} while (m);
	if (k <= j && !strncasecmp(tstring, directory, j - n)) {
		return 1;
	}
	return 0;
}

/* Checks if file exists */
short int 
fileexists(char *f)
{
	if (access(f, R_OK) == -1)
		return 0;
	return 1;

}

/* Create symbolic link (related to mp3 genre/year/group etc)
 * Last midified by: psxc
 *         Revision: r1228
 */
void 
createlink(char *factor1, char *factor2, char *source, char *ltarget)
{

#if ( userellink == 1 )
	char		result	[MAXPATHLEN];
#endif
	char		org	[PATH_MAX];
	char		dest	[NAME_MAX];
	char	       *target = org;
	int		l1 = strlen(factor1) + 1,
			l2 = strlen(factor2) + 1,
			l3 = strlen(ltarget) + 1;
	DIR		*dir;
        struct dirent   *dp;

	bzero(dest, sizeof(dest));
	if (!(dir = opendir(factor1))) {
		d_log(1, "createlink: Failed to open dir %s : %s\n", factor1, strerror(errno));
		closedir(dir);
		return;
	}
        while ((dp = readdir(dir))) {
                if (strlen(dp->d_name) && !strcasecmp(dp->d_name, factor2)) {
                        strncpy(dest, dp->d_name, sizeof(dp->d_name));
			d_log(1, "createlink: found %s\n", dest);
                        break;
                }
        }
	closedir(dir);
	if (*dest == '\0')
		memcpy(dest, factor2, l2);

	memcpy(target, factor1, l1);
	target += l1 - 1;
	if (*(target - 1) != '/') {
		*(target) = '/';
		target += 1;
	}

	memcpy(target, dest, l2);
	target += l2;
	memcpy(target - 1, "/", 2);

	if (mkdir(org, 0777) == -1 && errno != EEXIST)
		d_log(1, "createlink: Failed to mkdir %s : %s\n", org, strerror(errno));

#if ( userellink == 1 )
	abs2rel(source, org, result, MAXPATHLEN);
#endif

	memcpy(target, ltarget, l3);

#if ( userellink == 1 )
	symlink(result, org);
#else
	symlink(source, org);
#endif
}


void 
readsfv_ffile(struct VARS *raceI)
{
	int		fd, line_start = 0, index_start,
			ext_start, n;
	char		*buf = NULL, *fname;
	
	DIR		*dir;

	fd = open(raceI->file.name, O_RDONLY);
	buf = ng_realloc(buf, raceI->file.size + 2, 1, 1, 1);
	read(fd, buf, raceI->file.size);
	close(fd);

	dir = opendir(".");

	for (n = 0; n <= raceI->file.size; n++) {
		if (buf[n] == '\n' || n == raceI->file.size) {
			index_start = n - line_start;
			if (buf[line_start] != ';') {
				while (buf[index_start + line_start] != ' ' && index_start--);
				if (index_start > 0) {
					buf[index_start + line_start] = 0;
					fname = buf + line_start;
					ext_start = index_start;
#if (sfv_cleanup_lowercase == TRUE)
					while ((fname[ext_start] = tolower(fname[ext_start])) != '.' && ext_start > 0)
#else
					while (fname[ext_start] != '.' && ext_start > 0)
#endif
						ext_start--;
					if (fname[ext_start] != '.') {
						ext_start = index_start;
					} else {
						ext_start++;
					}
					index_start++;
					raceI->total.files++;
					if (!strcomp(ignored_types, fname + ext_start)) {
						if (findfile(".", fname)) {
							raceI->total.files_missing--;
						}
					}
				}
			}
			line_start = n + 1;
		}
	}
	raceI->total.files_missing = raceI->total.files + raceI->total.files_missing;
	ng_free(buf);
	closedir(dir);
}

void 
get_rar_info(struct VARS *raceI)
{
	FILE           *file;

	if ((file = fopen(raceI->file.name, "r"))) {
		fseek(file, 45, SEEK_CUR);
		fread(&raceI->file.compression_method, 1, 1, file);
		if ( ! (( 47 < raceI->file.compression_method ) && ( raceI->file.compression_method < 54 )) )
			raceI->file.compression_method = 88;
		fclose(file);
	}
}

/*
 * Modified   : 27.02.2005 Author     : js
 * 
 * Description: Executes external program and returns return value
 *
 * The first argument is the number of args passed
 * 
 */
int
execute(int args, ...)
{
	int i, status = -1;
	pid_t pid;
	char *cmdv[args+1];
	va_list ap;

	va_start(ap, args);
	
	for (i = 0; i < args; i++)
		cmdv[i] = strdup(va_arg(ap, char *));

	va_end(ap);

	cmdv[args] = '\0';
	
	switch ((pid = fork())) {
		case -1:
			break;
		case 0:
			close(STDOUT_FILENO);
			close(STDERR_FILENO); 
			execv(cmdv[0], cmdv);
			exit(-1);
		default:
			waitpid(pid, &status, WUNTRACED);
			break;
	}

	for (i = 0; i < args; i++)
		free(cmdv[i]);

	return WEXITSTATUS(status);
}

/*int
execute(char *s)
{
	int	i = 0;

	if ((i = system(s)) == -1)
		d_log(1, "execute (old): %s\n", strerror(errno));
	return i;
}*/

char *
get_g_name(GDATA *gdata, gid_t gid)
{
	int		n;
	for (n = 0; n < gdata->num_groups; n++)
		if (gdata->group[n].id / 100 == gid / 100)
			return gdata->group[n].name;
	return "NoGroup";
}

char *
get_u_name(UDATA *udata, uid_t uid)
{
	int	n;
	for (n = 0; n < udata->num_users; n++)
		if (udata->user[n].id == uid)
			return udata->user[n].name;
	return "Unknown";
}

/* Buffer groups file */
void 
buffer_groups(GDATA *gdata, char *groupfile, int setfree)
{
	char		*f_buf = NULL, *g_name;
	gid_t		g_id;
	off_t		f_size;
	int		f, n, m, g_n_size, l_start = 0;
	int		GROUPS = 0;
	struct stat	fileinfo;

	if (setfree != 0) {
		ng_free(gdata->group);
		return;
	}

	f = open(groupfile, O_NONBLOCK);

	fstat(f, &fileinfo);
	f_size = fileinfo.st_size;
	f_buf = ng_realloc(f_buf, f_size, 1, 1, 1);
	read(f, f_buf, f_size);

	for (n = 0; n < f_size; n++)
		if (f_buf[n] == '\n')
			GROUPS++;
			
	gdata->group = ng_realloc(gdata->group, GROUPS * sizeof(struct GROUP), 1, 1, 1);
	gdata->num_groups = 0;

	for (n = 0; n < f_size; n++) {
		if (f_buf[n] == '\n' || n == f_size) {
			f_buf[n] = 0;
			m = l_start;
			while (f_buf[m] != ':' && m < n)
				m++;
			if (m != l_start) {
				f_buf[m] = 0;
				g_name = f_buf + l_start;
				g_n_size = m - l_start;
				m = n;
				while (f_buf[m] != ':' && m > l_start)
					m--;
				f_buf[m] = 0;
				while (f_buf[m] != ':' && m > l_start)
					m--;
				if (m != n) {
					g_id = strtol(f_buf + m + 1, NULL, 10);
					strlcpy(gdata->group[gdata->num_groups].name, g_name, 15);
					gdata->group[gdata->num_groups].id = g_id;
					gdata->num_groups++;
				}
			}
			l_start = n + 1;
		}
	}

	close(f);
	ng_free(f_buf);
}

/* Buffer users file */
void
buffer_users(UDATA *udata, char *passwdfile, int setfree)
{
	char	*f_buf = NULL, *u_name;
	uid_t		u_id;
	off_t		f_size;
	int		f, n, m, l, u_n_size, l_start = 0;
	int		USERS = 0;
	struct stat	fileinfo;

	if (setfree != 0) {
		ng_free(udata->user);
		return;
	}

	f = open(passwdfile, O_NONBLOCK);
	fstat(f, &fileinfo);
	f_size = fileinfo.st_size;
	f_buf = ng_realloc(f_buf, f_size, 1, 1, 1);
	read(f, f_buf, f_size);

	for (n = 0; n < f_size; n++)
		if (f_buf[n] == '\n')
			USERS++;
			
	udata->user = ng_realloc(udata->user, USERS * sizeof(struct USER), 1, 1, 1);
	udata->num_users = 0;

	for (n = 0; n < f_size; n++) {
		if (f_buf[n] == '\n' || n == f_size) {
			f_buf[n] = 0;
			m = l_start;
			while (f_buf[m] != ':' && m < n)
				m++;
			if (m != l_start) {
				f_buf[m] = 0;
				u_name = f_buf + l_start;
				u_n_size = m - l_start;
				m = n;
				for (l = 0; l < 4; l++) {
					while (f_buf[m] != ':' && m > l_start)
						m--;
					f_buf[m] = 0;
				}
				while (f_buf[m] != ':' && m > l_start)
					m--;
				if (m != n) {
					u_id = strtol(f_buf + m + 1, NULL, 10);
					strlcpy(udata->user[udata->num_users].name, u_name, 24);
					udata->user[udata->num_users].id = u_id;
					udata->num_users++;
				}
			}
			l_start = n + 1;
		}
	}

	close(f);
	ng_free(f_buf);
}

unsigned long 
sfv_compare_size(char *fileext, unsigned long fsize)
{
	int		k = 0;
	unsigned long	l = 0;
	struct stat	filestat;

	DIR		*dir;
	struct dirent	*dp;

	dir = opendir(".");

	while ((dp = readdir(dir))) {
		if ((k = NAMLEN(dp)) < 4)
			continue;
		if (strcasecmp(dp->d_name + k - 4, fileext) == 0) {
			if (stat(dp->d_name, &filestat) != 0)
				filestat.st_size = 1;
			l = l + filestat.st_size;
			continue;
		}
	}
	
	if (!(l = l - fsize) > 0)
		l = 0;
	
	closedir(dir);

	return l;
}

void
mark_as_bad(char *filename)
{
#if (mark_file_as_bad)
	char	newname[NAME_MAX];

	if (!fileexists(filename)) {
		d_log(1, "mark_as_bad: \"%s\" doesn't exist\n", filename);
		return;
	}
	sprintf(newname, "%s.bad", filename);
	if (rename(filename, newname)) {
		d_log(1, "mark_as_bad: Error - failed to rename %s to %s\n", filename, newname);
	} else {
		createzerofile(filename);
		chmod(newname, 0644);
	}
#endif
	d_log(1, "mark_as_bad: File (%s) marked as bad\n", filename);
}

void 
writelog(GLOBAL *g, char *msg, char *status)
{
	FILE           *glfile;
	char           *date;
	char           *line, *newline;
	time_t		timenow;

	if (!matchpath(group_dirs, g->l.path) && g->v.misc.write_log) {
		timenow = time(NULL);
		date = ctime(&timenow);
		if (!(glfile = fopen(log, "a+"))) {
			d_log(1, "writelog: fopen(%s): %s\n", log, strerror(errno));
			return;
		}
		line = newline = msg;
		while (1) {
			switch (*newline++) {
			case 0:
				fprintf(glfile, "%.24s %s: \"%s\" %s\n", date, status, g->l.path, line);
				fclose(glfile);
				return;
			case '\n':
				fprintf(glfile, "%.24s %s: \"%s\" %.*s\n", date, status, g->l.path, (int)(newline - line - 1), line);
				line = newline;
				break;
			}
		}
	}
}

void
buffer_paths(GLOBAL *g, char path[2][PATH_MAX], int *k, int len)
{
	int		cnt, n = 0;

	for (cnt = len; *k && cnt; cnt--) {
		if (g->l.path[cnt] == '/') {
			(*k)--;
			strlcpy(path[*k], g->l.path + cnt + 1, n+1);
			path[*k][n] = 0;
			n = 0;
		} else {
			n++;
		}
	}
}

void 
remove_nfo_indicator(GLOBAL *g)
{
	int		k = 2;
	char		path[2][PATH_MAX];

	buffer_paths(g, path, &k, ((int)strlen(g->l.path)-1));

	g->l.nfo_incomplete = i_incomplete(incomplete_nfo_indicator, path, &g->v);
	if (fileexists(g->l.nfo_incomplete))
		unlink(g->l.nfo_incomplete);
	g->l.nfo_incomplete = i_incomplete(incomplete_base_nfo_indicator, path, &g->v);
	if (fileexists(g->l.nfo_incomplete))
		unlink(g->l.nfo_incomplete);
}

void 
getrelname(GLOBAL *g)
{
	int		k = 2, subc;
	char		path[2][PATH_MAX];

	bzero(path[0], sizeof(path[0]));
	bzero(path[1], sizeof(path[1]));
	buffer_paths(g, path, &k, ((int)strlen(g->l.path)-1));

	subc = subcomp(path[1]);
	
	d_log(2, "getrelname():\tsubc:\t\t%d\n", subc);
	d_log(2, "\t\t\tpath[0]:\t%s\n", path[0]);
	d_log(2, "\t\t\tpath[1]:\t%s\n", path[1]);
	d_log(2, "\t\t\tg->l_path:\t%s\n", path[1]);

	if (subc) {
		snprintf(g->v.misc.release_name, PATH_MAX, "%s/%s", path[0], path[1]);
		strlcpy(g->l.link_source, g->l.path, PATH_MAX);
		strlcpy(g->l.link_target, path[1], PATH_MAX);
		g->l.incomplete = c_incomplete(incomplete_cd_indicator, path, &g->v);
		g->l.nfo_incomplete = i_incomplete(incomplete_base_nfo_indicator, path, &g->v);
		g->l.in_cd_dir = 1;
	} else {
		strlcpy(g->v.misc.release_name, path[1], PATH_MAX);
		strlcpy(g->l.link_source, g->l.path, PATH_MAX);
		strlcpy(g->l.link_target, path[1], PATH_MAX);
		g->l.incomplete = c_incomplete(incomplete_indicator, path, &g->v);
		g->l.nfo_incomplete = i_incomplete(incomplete_nfo_indicator, path, &g->v);
		g->l.in_cd_dir = 0;
	}
	
	d_log(2, "\t\t\tlink_source:\t%s\n", g->l.link_source);
	d_log(2, "\t\t\tlink_target:\t%s\n", g->l.link_target);
}

/*unsigned char 
get_filetype(GLOBAL *g, char *ext)
{
	if (!strcasecmp(ext, "zip"))
		return 0;
	if (!strcasecmp(ext, "sfv"))
		return 1;
	if (!strcasecmp(ext, "nfo"))
		return 2;
	if (strcomp(allowed_types, ext) && !matchpath(allowed_types_exemption_dirs, g->l.path))
		return 4;
	if (!strcomp(ignored_types, ext))
		return 3;

	return 255;
}*/

#if ( audio_group_sort == TRUE )
char *
remove_pattern(param, pattern, op)
	char	*param, *pattern;
	int		op;
{
	register int	len;
	register char  *end;
	register char  *p, *ret, c;

	if (param == NULL || *param == '\0')
		return (param);
	if (pattern == NULL || *pattern == '\0')	/* minor optimization */
		return (param);

	len = (int)strlen(param);
	end = param + len;

	switch (op) {
	case RP_LONG_LEFT:	/* remove longest match at start */
		for (p = end; p >= param; p--) {
			c = *p;
			*p = '\0';
			if ((fnmatch(pattern, param, 0)) != FNM_NOMATCH) {
				*p = c;
				return (p);
			}
			*p = c;
		}
		break;

	case RP_SHORT_LEFT:	/* remove shortest match at start */
		for (p = param; p <= end; p++) {
			c = *p;
			*p = '\0';
			if (fnmatch(pattern, param, 0) != FNM_NOMATCH) {
				*p = c;
				return (p);
			}
			*p = c;
		}
		break;


	case RP_LONG_RIGHT:	/* remove longest match at end */
		for (p = param; p <= end; p++) {
			if (fnmatch(pattern, param, 0) != FNM_NOMATCH) {
				c = *p;
				*p = '\0';
				ret = param;
				*p = c;
				return (ret);
			}
		}
		break;

	case RP_SHORT_RIGHT:	/* remove shortest match at end */
		for (p = end; p >= param; p--) {
			if (fnmatch(pattern, param, 0) != FNM_NOMATCH) {
				c = *p;
				*p = '\0';
				ret = param;
				*p = c;
				return (ret);
			}
		}
		break;
	}
	return (param);		/* no match, return original string */
}
#endif

void *
ng_realloc(void *mempointer, int memsize, int zero_it, int exit_on_error, int zero_pointer)
{
	if (zero_pointer)
		mempointer = malloc(memsize);
	else
		mempointer = realloc(mempointer, memsize);
	if (mempointer == NULL) {
		d_log(1, "ng_realloc: realloc failed: %s\n", strerror(errno));
		if (exit_on_error) {
			exit(EXIT_FAILURE);
		}
	} else if (zero_it)
		bzero(mempointer, memsize);
	return mempointer;
}

void
ng_free(void *mempointer)
{
	if (mempointer)
		free(mempointer);
}

int
remove_at_loc(char *path, off_t loc)
{
	static DIR		*dir;
	static struct dirent	*dp;

	dir = opendir(path);
	
	seekdir(dir, loc);
	dp = readdir(dir);

	if (unlink(dp->d_name) == -1) {
		d_log(1, "remove_at_loc: unlink(%s) failed: %s\n", dp->d_name, strerror(errno));
		closedir(dir);
		return -1;
	}

	closedir(dir);

	return 0;
}

