#ifndef __included_curses_utils_h__
#define __included_curses_utils_h__
#include <curses.h>

static inline void draw_rect(int height, int width, int line, int col)
{
	mvaddch(line,              col,             ACS_ULCORNER);
	mvaddch(line + height - 1, col,             ACS_LLCORNER);
	mvaddch(line,              col + width - 1, ACS_URCORNER);
	mvaddch(line + height - 1, col + width - 1, ACS_LRCORNER);

	move(line, col + 1);
	hline(ACS_HLINE, width - 2);

	move(line + height - 1, col + 1);
	hline(ACS_HLINE, width - 2);

	move(line + 1, col);
	vline(ACS_VLINE, height - 2);

	move(line + 1, col + width - 1);
	vline(ACS_VLINE, height - 2);
}

static inline WINDOW * new_frame(int height, int width, int line, int col)
{
	WINDOW *win;

	//win = newwin(height, width, line, col);
	win = subwin(stdscr, height - 4, width - 4, line + 2, col + 2);

	draw_rect(height, width, line, col);

#if 1
	{
	int y;
	for (y=0; y<height; y++)
		mvwprintw(win, y, 8, " %u ", y);

	mvwprintw(win, 0, 8, "size{%u,%u} start{%u,%u}",
			height, width, line, col);
	}
#endif

	return win;
}

#endif // __included_curses_utils_h__
