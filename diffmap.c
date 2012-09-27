/*-
 * Copyright (c) 2012 Romain Tartière <romain@blogreen.org>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAME "diffmap"

void		 usage (void);
int		 compare_regular_files (char *filename1, char *filename2, int block_size, int screen_width);
int		 compare_directories (char *filename1, char *filename2, int block_size, int screen_width);
int		 compare_files (char *filename1, char *filename2, int block_size, int screen_width);

void
usage (void)
{
    fprintf (stderr, "usage: " NAME " [-b block-size] [-w screen-width] file1 file2\n");
}

int
compare_regular_files (char *filename1, char *filename2, int block_size, int screen_width)
{
    int block_count = 0;
    int identical_block_count = 0;

    FILE *file1, *file2;

    if (!(file1 = fopen (filename1, "r")))
	err (EXIT_FAILURE, "%s", filename1);
    if (!(file2 = fopen (filename2, "r")))
	err (EXIT_FAILURE, "%s", filename2);

    char *buf1, *buf2;
    if (!(buf1 = malloc (block_size)))
	err (EXIT_FAILURE, "Cannot allocate memory");
    if (!(buf2 = malloc (block_size)))
	err (EXIT_FAILURE, "Cannot allocate memory");

    int line_position = 0;
    while (!feof (file1) || !feof (file2)) {
	int read1, read2;

	block_count++;

	read1 = fread (buf1, 1, block_size, file1);
	read2 = fread (buf2, 1, block_size, file2);

	if ((read1 != read2) && (read1 == 0))
	    line_position += fprintf (stdout, "/");
	else if ((read1 != read2) && (read2 == 0))
	    line_position += fprintf (stdout, "\\");
	else if (memcmp (buf1, buf2, read1))
	    line_position += fprintf (stdout, "X");
	else {
	    identical_block_count++;
	    line_position += fprintf (stdout, ".");
	}

	if (line_position == screen_width) {
	    fprintf (stdout, "\n");
	    line_position = 0;
	}
    }
    fprintf (stdout, "\n");

    free (buf1);
    free (buf2);

    fclose (file1);
    fclose (file2);

    return block_count - identical_block_count;
}

int
compare_directories (char *dirname1, char *dirname2, int block_count, int screen_width)
{
    DIR *dir;
    int ret = 0;

    if (!(dir = opendir (dirname1)))
	err (EXIT_FAILURE, "%s", dirname1);

    struct dirent *dp;
    while ((dp = readdir (dir))) {
	char *filename1, *filename2;

	if (0 == strcmp (dp->d_name, ".") ||
	    0 == strcmp (dp->d_name, ".."))
	    continue;

	asprintf (&filename1, "%s/%s", dirname1, dp->d_name);
	asprintf (&filename2, "%s/%s", dirname2, dp->d_name);

	ret += compare_files (filename1, filename2, block_count, screen_width);

	free (filename1);
	free (filename2);
    }

    closedir (dir);

    // search in dir2 files not present in dir1

    if (!(dir = opendir (dirname2)))
	err (EXIT_FAILURE, "%s", dirname2);

    while ((dp = readdir (dir))) {
	char *filename1, *filename2;

	if (0 == strcmp (dp->d_name, ".") ||
	    0 == strcmp (dp->d_name, ".."))
	    continue;

	asprintf (&filename1, "%s/%s", dirname1, dp->d_name);
	asprintf (&filename2, "%s/%s", dirname2, dp->d_name);

	free (filename1);
	free (filename2);
    }

    closedir (dir);

    return ret;
}

int
compare_files (char *filename1, char *filename2, int block_size, int screen_width)
{
    int ret = 0;

    struct stat sb1, sb2;
    if (stat (filename1, &sb1) < 0) {
	errx (EXIT_FAILURE, "Can't stat %s", filename1);
    }
    if (stat (filename2, &sb2) < 0) {
	warnx ("Can't stat %s", filename2);
	return (S_ISREG (sb1.st_mode) ? sb1.st_size / block_size : 1);
    }

    if ((sb1.st_ino == sb2.st_ino) && (sb1.st_dev == sb2.st_dev)) {
	warnx ("\"%s\" and \"%s\" are the same file.  Skipping comparison.", filename1, filename2);
	return 0;
    }

    if (S_ISREG (sb1.st_mode) && S_ISREG (sb2.st_mode)) {
	fprintf (stdout, "\\\\\\ %s\n", filename1);
	fprintf (stdout, "/// %s\n", filename2);
	ret = compare_regular_files (filename1, filename2, block_size, screen_width);
    } else if (S_ISDIR (sb1.st_mode) && S_ISDIR (sb2.st_mode)) {
	fprintf (stdout, "Entering %s\n", filename1);
	ret = compare_directories (filename1, filename2, block_size, screen_width);
	fprintf (stdout, "Leaving %s\n", filename1);
    } else
	errx (EXIT_FAILURE, "both arguments must be files or directories");

    return ret;
}


int
main (int argc, char *argv[])
{
    int block_size = 512;
    int screen_width = 80;

    char junk;
    char ch;
    while ((ch = getopt(argc, argv, "b:w:")) != -1) {
	switch (ch) {
	case 'b':
	    if (1 != sscanf (optarg, "%d%c", &block_size, &junk))
		errx (EXIT_FAILURE, "\"%s\" is not a valid number", optarg);
	    break;
	case 'w':
	    if (1 != sscanf (optarg, "%d%c", &screen_width, &junk))
		errx (EXIT_FAILURE, "\"%s\" is not a valid number", optarg);
	    break;
	case '?':
	default:
	    usage ();
	    exit (EXIT_FAILURE);
	}
    }
    argc -= optind;
    argv += optind;

    if (argc != 2) {
	usage ();
	exit (EXIT_FAILURE);
    }

    return compare_files (argv[0], argv[1], block_size, screen_width);
}
