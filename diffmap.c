#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAME "diffmap"

void
usage (void)
{
    fprintf (stderr, "usage: " NAME " [-b block-size] [-w screen-width] file1 file2\n");
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

    FILE *file1, *file2;

    if (!(file1 = fopen (argv[0], "r")))
	err (EXIT_FAILURE, "%s", argv[0]);
    if (!(file2 = fopen (argv[1], "r")))
	err (EXIT_FAILURE, "%s", argv[1]);

    char *buf1, *buf2;
    if (!(buf1 = malloc (block_size)))
	err (EXIT_FAILURE, "Cannot allocate memory");
    if (!(buf2 = malloc (block_size)))
	err (EXIT_FAILURE, "Cannot allocate memory");

    int line_position = 0;
    while (!feof (file1) || !feof (file2)) {
	int read1, read2;

	read1 = fread (buf1, 1, block_size, file1);
	read2 = fread (buf2, 1, block_size, file2);

	if ((read1 != read2) && (read1 == 0))
	    line_position += fprintf (stdout, "/");
	else if ((read1 != read2) && (read2 == 0))
	    line_position += fprintf (stdout, "\\");
	else if (memcmp (buf1, buf2, read1))
	    line_position += fprintf (stdout, "X");
	else
	    line_position += fprintf (stdout, ".");

	if (line_position == screen_width) {
	    fprintf (stdout, "\n");
	    line_position = 0;
	}
    }
    fprintf (stdout, "\n");

    fclose (file1);
    fclose (file2);

    exit(EXIT_SUCCESS);
}
