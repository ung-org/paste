/*
 * UNG's Not GNU
 * 
 * Copyright (c) 2011-2019, Jakob Kaivo <jkk@ung.org>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#define _XOPEN_SOURCE 700
#include <errno.h>
#include <inttypes.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <unistd.h>

static char *delimiter = "\t";

void putdelim(void)
{
	static int i = 0;
	if (delimiter[i] == '\\') {
		switch (delimiter[++i]) {
		case 'n':
			putchar('\n');
			break;

		case 't':
			putchar('\t');
			break;

		case '\\':
			putchar('\\');
			break;

		case '0':
			/* empty string */
			break;

		default:
			putchar(delimiter[i]);
			break;
		}
	} else {
		putchar(delimiter[i]);
	}
	i = (i + 1) % strlen(delimiter);;
}

int paste_serial(char *files[])
{
	int ret = 0;
	int i = 0;

	do {
		FILE *f = strcmp(files[i], "-") ? fopen(files[i], "r") : stdin;

		if (f == NULL) {
			perror("paste:fopen:");
			ret = 1;
			continue;
		}

		int c;
		while ((c = fgetc(f)) != EOF) {
			/* FIXME: off-by-one on last line */
			if (c == '\n') {
				putdelim();
			} else {
				putchar(c);
			}
		}

		if (f != stdin) {
			fclose(f);
		}

		putchar('\n');
	} while (files[++i]);

	return ret;
}

int paste(int nfiles, char *files[])
{
	int ret = 0;
	int nopen = 0;

	FILE *f[nfiles];
	for (int i = 0; i < nfiles; i++) {
		f[i] = strcmp(files[i], "-") ? fopen(files[i], "r") : stdin;
		if (f[i] == NULL) {
			perror("paste:fopen");
			ret = 1;
		} else {
			nopen++;
		}
	}

	char *line = NULL;
	size_t n = 0;

	while (nopen > 0) {
		/* TODO: delim */
		for (int i = 0; i < nfiles; putdelim(), i++) {
			if (f[i] == NULL) {
				continue;
			}

			int len;
			if ((len = getline(&line, &n, f[i])) == -1) {
				if (f[i] != stdin) {
					fclose(f[i]);
				}

				f[i] = NULL;
				nopen--;

				continue;
			}

			if (line[len-1] == '\n') {
				line[len-1] = '\0';
			}

			printf("%s", line);
		}
		putchar('\n');
	}

	if (line) {
		free(line);
	}

	return ret;
}

int main(int argc, char *argv[])
{
	int serial = 0;

	setlocale(LC_ALL, "");
 
	int c;
	while ((c = getopt(argc, argv, "d:s")) != -1) {
		switch (c) {
		case 'd':
			delimiter = optarg;
			break;

		case 's':
			serial = 1;
			break;

		default:
			return 1;
		}
	}

	if (serial) {
		return paste_serial(argv + optind);
	}

	return paste(argc - optind, argv + optind);
}
