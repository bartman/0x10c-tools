#ifndef __included_uput_h__
#define __included_uput_h__

#define UPUT_MAX_LENGTH 256
extern int uput(const int y, const int x, const int length, const int fg,
		const int bg, char *whole, bool ins, const char *permitted);
extern char* rtrim(char* string, char junk);
extern void insert(char *, const char*, const int);


#define UPUT_EXIT_ESCAPE     5
#define UPUT_EXIT_ENTER      0
#define UPUT_EXIT_UP_ARROW   8
#define UPUT_EXIT_DOWN_ARROW 2
#define UPUT_EXIT_TAB        6
#define UPUT_EXIT_SHIFT_TAB  4
#define UPUT_EXIT_PAGE_UP    9
#define UPUT_EXIT_PAGE_DOWN  3

#endif // __included_uput_h__
