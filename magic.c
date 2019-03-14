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
#include <stdlib.h>
#include <string.h>
#include <cpio.h>
#include <search.h>
#include <sys/types.h>
#include <tar.h>

enum magic_type {
	CHAR,
	UCHAR,
	SHORT,
	USHORT,
	INT,
	UINT,
	LONG,
	ULONG,
	STRING
};

size_t sizes[] = {
	[CHAR] = sizeof(signed char),
	[UCHAR] = sizeof(unsigned char),
	[SHORT] = sizeof(short),
	[USHORT] = sizeof(unsigned short),
	[INT] = sizeof(int),
	[UINT] = sizeof(unsigned int),
	[LONG] = sizeof(long),
	[ULONG] = sizeof(unsigned long),
};

union magic_value {
	signed char schar;
	unsigned char uchar;
	short sshort;
	unsigned short ushort;
	int sint;
	unsigned int uint;
	long slong;
	unsigned long ulong;
	char *string;
};

struct magic {
	struct magic *next;
	struct magic *prev;
	off_t offset;
	enum magic_type type;
	union magic_value value;
	char *message;
	struct magic *child;
};

static struct magic *head = NULL;
static struct magic *tail = NULL;
static int docontext = 0;

static char *sh_strings[] = { "#!", 0 };
static char *c_strings[] = { "/*", "#include ", "#define ", 0 };
static char *fortran_strings[] = { "program ", "PROGRAM ", "c ", "C ", 0 };
static struct {
	char *identifier;
	char **strings;
} context[] = {
	{ "commands text", sh_strings },
	{ "c program text", c_strings },
	{ "fortran program text", fortran_strings },
	{ 0, 0 },
};
		
static int addmagic(struct magic *parent, off_t o, enum magic_type t,
	union magic_value v, char *m)
{
	struct magic *magic = calloc(1, sizeof(*magic));
	if (magic == NULL) {
		fprintf(stderr, "file: out of memory\n");
		exit(1);
	}

	magic->offset = o;
	magic->type = t;
	if (t == STRING) {
		magic->value.string = strdup(v.string);
	} else {
		memcpy(&(magic->value), &v, sizeof(v));
	}
	magic->message = strdup(m);

	if (parent) {
	}

	insque(magic, tail);
	if (tail == NULL) {
		head = magic;
		tail = magic;
	}

	return 0;
}

static int loadinternal(void)
{
	union magic_value cpio;
	cpio.string = MAGIC;
	addmagic(NULL, 0, STRING, cpio, "cpio archive");

	union magic_value tar;
	tar.string = TMAGIC;
	addmagic(NULL, 0x101, STRING, tar, "tar archive");

	union magic_value ar;
	ar.string = "!<arch>";
	addmagic(NULL, 0, STRING, ar, "archive");

	docontext = 1;

	return 0;
}

int readmagic(const char *path)
{
	if (path == NULL) {
		return loadinternal();
	}

	return 0;
}

int idmagic(const char *path)
{
	int id = 0;

	FILE *f = fopen(path, "rb");
	if (f == NULL) {
		printf(" regular file cannot open (%s)", strerror(errno));
		return 1;
	}

	for (struct magic *cursor = head; cursor; cursor = cursor->next) {
		size_t s = sizes[cursor->type];
		if (cursor->type == STRING) {
			s = strlen(cursor->value.string) + 1;
		}
		char buf[s];

		if (fseeko(f, cursor->offset, SEEK_SET) != 0) {
			break;
		}

		if (fread(buf, s, 1, f) == 0) {
			break;
		}

		if (cursor->type == STRING) {
			buf[s-1] = '\0';
		}

		if ((cursor->type == STRING && !strcmp(buf, cursor->value.string))
			|| memcmp(buf, &cursor->value, s) == 0) {
			printf(" %s", cursor->message);
			id = 1;
		}
	}

	if (docontext) {
		fseeko(f, 0, SEEK_SET);
		char *line = NULL;
		size_t n = 0;
		getline(&line, &n, f);
		for (int i = 0; context[i].identifier; i++) {
			for (int j = 0; context[i].strings[j]; j++) {
				char *s = context[i].strings[j];
				if (!strncmp(line, s, strlen(s))) {
					printf(" %s", context[i].identifier);
					id = 1;
				}
			}
		}
	}

	if (id == 0) {
		printf(" data");
	}

	fclose(f);

	return 0;
}
