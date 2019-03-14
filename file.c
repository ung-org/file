/*
 * UNG's Not GNU
 *
 * Copyright (c) 2011-2017, Jakob Kaivo <jkk@ung.org>
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

#include "file.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int file(const char *path, int domagic)
{
	printf("%s:", path);
	struct stat st;
	if (lstat(path, &st) == -1) {
		printf(" cannot open (%s)\n", strerror(errno));
		return 1;
	}

	if (S_ISLNK(st.st_mode)) {
		char link[st.st_size];
		readlink(path, link, st.st_size);
		printf(" symbolink link to %s\n", link);
		return 0;
		//stat(path, &st);
	}

	if (domagic && S_ISREG(st.st_mode)) {
		if (st.st_size == 0) {
			printf(" empty");
		}

		if (access(path, X_OK) == 0) {
			printf(" executable");
		}

	}


	if (S_ISDIR(st.st_mode)) {
		printf(" directory");
	} else if (S_ISFIFO(st.st_mode)) {
		printf(" fifo");
	} else if (S_ISSOCK(st.st_mode)) {
		printf(" socket");
	} else if (S_ISBLK(st.st_mode)) {
		printf(" block special");
	} else if (S_ISCHR(st.st_mode)) {
		printf(" character special");
	} else {
		if (domagic) {
			idmagic(path);
		} else {
			printf(" regular file");
		}
	}

	putchar('\n');
	return 0;
}

int main(int argc, char *argv[])
{
	enum { NONE = 0, TODO, DONE } magic = TODO;
	int c;
	while ((c = getopt(argc, argv, "dhM:m:i")) != -1) {
		switch (c) {
		case 'd':	/** include system default magic **/
			readmagic(NULL);
			magic = DONE;
			break;

		case 'h':	/** identify symbolic links **/
			break;

		case 'i':	/** perform no magic **/
			magic = NONE;
			break;

		case 'M':	/** specify magic file **/
			readmagic(optarg);
			magic = DONE;
			break;

		case 'm':	/** specify magic file **/
			readmagic(optarg);
			magic = DONE;
			break;

		default:
			return 1;
		}
	}

	if (magic == TODO) {
		readmagic(NULL);
	}

	if (optind >= argc) {
		fprintf(stderr, "file: At least one file required\n");
		return 1;
	}

	int r = 0;
	while (optind < argc) {
		r |= file(argv[optind++], magic);
	}

	return r;
}
