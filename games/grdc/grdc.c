/*
 * Copyright (c) 2002 Amos Shapir.
 * Public domain.
 *
 * Grand digital clock for curses compatible terminals
 *
 * Usage: grdc [-s] [-d msecs] [n]   -- run for n seconds (default infinity)
 * Flags:	-s: scroll (default scroll duration 120msec)
 *		-d msecs: specify scroll duration (implies -s)
 *
 * modified 10-18-89 for curses (jrl)
 * 10-18-89 added signal handling
 * 03-23-04 added centering, scroll delay (cap)
 *
 * $FreeBSD: src/games/grdc/grdc.c,v 1.8.2.1 2001/10/02 11:51:49 ru Exp $
 */

#include <err.h>
#include <ncurses.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define XLENGTH 58
#define YDEPTH  7

static int hascolor = 0;
static int xbase, ybase, xmax, ymax;
static long old[6], next[6], new[6], mask;
static volatile sig_atomic_t sigtermed;

static void cleanup(void);
static void draw_row(int, int);
static void set(int, int);
static void sighndl(int);
static int  snooze(long);
static void standt(int);
static void usage(void) __dead2;

static void
sighndl(int signo)
{
	sigtermed = signo;
}

int
main(int argc, char **argv)
{
	int i, s, k, n, ch;
	bool scrol, forever;
	long scroll_msecs;
	time_t now;
	struct tm *tm;

	n = 0;
	forever = true;
	scrol = false;
	scroll_msecs = 120;

	while ((ch = getopt(argc, argv, "d:s")) != -1) {
		switch (ch) {
		case 'd':
			scroll_msecs = atol(optarg);
			if (scroll_msecs < 0)
				errx(1, "scroll duration may not be negative");
			/* FALLTHROUGH */
		case 's':
			scrol = true;
			break;
		case '?':
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 1) {
		usage();
		/* NOTREACHED */
	}

	if (argc > 0) {
		n = atoi(*argv);
		forever = false;
	}

	initscr();

	getmaxyx(stdscr, ymax, xmax);
	if (ymax < YDEPTH + 2 || xmax < XLENGTH + 4) {
		endwin();
		errx(1, "terminal too small");
	}
	xbase = (xmax - XLENGTH) / 2 + 2;
	ybase = (ymax - YDEPTH) / 2 + 1;

	signal(SIGINT, sighndl);
	signal(SIGTERM, sighndl);
	signal(SIGHUP, sighndl);

	cbreak();
	noecho();
	curs_set(0);

	hascolor = has_colors();

	if (hascolor) {
		start_color();
		init_pair(1, COLOR_BLACK, COLOR_RED);
		init_pair(2, COLOR_RED, COLOR_BLACK);
		init_pair(3, COLOR_WHITE, COLOR_BLACK);
		attrset(COLOR_PAIR(2));
	}

	clear();
	refresh();

	if (hascolor) {
		attrset(COLOR_PAIR(3));

		mvaddch(ybase - 2, xbase - 3, ACS_ULCORNER);
		hline(ACS_HLINE, XLENGTH);
		mvaddch(ybase - 2, xbase - 2 + XLENGTH, ACS_URCORNER);

		mvaddch(ybase + YDEPTH - 1, xbase - 3, ACS_LLCORNER);
		hline(ACS_HLINE, XLENGTH);
		mvaddch(ybase + YDEPTH - 1, xbase - 2 + XLENGTH, ACS_LRCORNER);

		move(ybase - 1, xbase - 3);
		vline(ACS_VLINE, YDEPTH);

		move(ybase - 1, xbase - 2 + XLENGTH);
		vline(ACS_VLINE, YDEPTH);

		attrset(COLOR_PAIR(2));
		refresh();
	}

	do {
		mask = 0;
		time(&now);
		if (scrol)
			now += scroll_msecs / 1000;
		tm = localtime(&now);
		set(tm->tm_sec % 10, 0);
		set(tm->tm_sec / 10, 4);
		set(tm->tm_min % 10, 10);
		set(tm->tm_min / 10, 14);
		set(tm->tm_hour % 10, 20);
		set(tm->tm_hour / 10, 24);
		set(10, 7);
		set(10, 17);
		for (k = 0; k < 6; k++) {
			if (scrol) {
				if (snooze(scroll_msecs / 6) == 1)
					goto out;
				for(i = 0; i < 5; i++) {
					new[i] = (new[i] & ~mask) |
						 (new[i+1] & mask);
				}
				new[5] = (new[5] & ~mask) | (next[k] & mask);
			} else {
				new[k] = (new[k] & ~mask) | (next[k] & mask);
			}
			next[k] = 0;
			for (s = 1; s >= 0; s--) {
				standt(s);
				for (i = 0; i < 6; i++)
					draw_row(i, s);
				if (!s) {
					move(ybase, 0);
					refresh();
				}
			}
		}
		move(ybase, 0);
		refresh();
		if (snooze(1000 - (scrol ? scroll_msecs : 0)) == 1)
			goto out;
	} while (forever ? 1 : --n);

out:
	cleanup();

	return (0);
}

static int
snooze(long msecs)
{
	static struct pollfd pfd = {
		.fd = STDIN_FILENO,
		.events = POLLIN,
	};
	struct timespec ts;
	char c;
	int rv;

	if (msecs <= 0) {
		goto out;
	} else if (msecs < 1000) {
		ts.tv_sec = 0;
		ts.tv_nsec = 1000000 * msecs;
	} else {
		ts.tv_sec = msecs / 1000;
		ts.tv_nsec = 1000000 * (msecs % 1000);
	}

	rv = ppoll(&pfd, 1, &ts, NULL);
	if (rv == 1) {
		read(pfd.fd, &c, 1);
		if (c == 'q')
			return (1);
	}

out:
	if (sigtermed) {
		cleanup();
		errx(1, "terminated by signal %d", (int)sigtermed);
	}

	return (0);
}

static void
cleanup(void)
{
	standend();
	clear();
	refresh();
	endwin();
}

static void
draw_row(int i, int s)
{
	long a, t;
	int j;

	if ((a = (new[i] ^ old[i]) & (s ? new : old)[i]) != 0) {
		for (j = 0, t = 1 << 26; t; t >>= 1, j++) {
			if (a & t) {
				if (!(a & (t << 1))) {
					move(ybase + i, xbase + 2 * j);
				}
				addstr("  ");
			}
		}
	}
	if (!s) {
		old[i] = new[i];
	}
}

static void
set(int t, int n)
{
	static short disp[11] = {
		075557, 011111, 071747, 071717, 055711,
		074717, 074757, 071111, 075757, 075717, 002020
	};
	int i, m;

	m = 7 << n;
	for (i = 0; i < 5; i++) {
		next[i] |= ((disp[t] >> (4 - i) * 3) & 07) << n;
		mask |= (next[i] ^ old[i]) & m;
	}
	if (mask & m)
		mask |= m;
}

static void
standt(int on)
{
	if (on) {
		if (hascolor) {
			attron(COLOR_PAIR(1));
		} else {
			attron(A_STANDOUT);
		}
	} else {
		if (hascolor) {
			attron(COLOR_PAIR(2));
		} else {
			attroff(A_STANDOUT);
		}
	}
}

static void
usage(void)
{
	fprintf(stderr, "usage: grdc [-s] [-d msecs] [n]\n");
	exit(1);
}
