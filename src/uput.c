#include <ncurses.h>
#include <string.h>
#include <stdbool.h>

#include "uput.h"

#if 0
int main(void)
{
        char whole[UPUT_MAX_LENGTH];
        int returnvalue;
        initscr();
        cbreak();
        noecho();
        keypad(stdscr,TRUE);
        start_color();
        move(7,6);
        addstr("1234567890");
        refresh();
        strcpy(whole,"abcd");
        returnvalue=uput(6,6,6,0,7,whole,false,"123456789/*-+=()");
        move(20,30);
        printw("returnvalue=%d whole=%s\n",returnvalue,whole);
        refresh();
        flushinp();
        getch();
        endwin();
        return 0;
}
#endif

int uput(const int y,const int x,const int length,const int fg,const int bg,char *whole,bool ins,const char *permitted)
{
/*
+------------------------[ WHAT YOU PUT IN ]-------------------------------+
|UPUT(y, x, length, fg, bg, whole, ins, permitted)                        |
+--------------------------------------------------------------------------+
|y -> Row where INPUT will start                                           |
|x -> Column where INPUT will start                                        |
|length -> Maximum length of INPUT                                         |
|fg -> Foreground color of INPUT line                                      |
|bg -> Background color of INPUT line                                      |
|whole -> String to be edited                                              |
|ins -> TRUE or FALSE for INSERT on/off                                    |
| permitted$ = the only valid characters                                   |
+---------------------[ WHAT YOU GET BACK ]--------------------------------+
|                                                                          |
| If UPUT is exited by the user pressing ESCAPE, then the FUNCTION will    |
| return the original string it was given (ie: no changes are made).  If   |
| UPUT is exited any other way (TAB, SHIFT-TAB, UP, DOWN, ENTER), then     |
| the edited string is returned.                                           |
|                                                                          |
| In either case, the SHARED variable "keyflag%" is returned with a value  |
| which is dependent on HOW UPUT was exited, following the chart below     |
|                                      +-----------------------------------+
| ESCAPE     -> keyflag = 5            |The values are based on the KEYPAD!|
| ENTER      -> keyflag = 0            +--------------+--------+-----------+
| UP ARROW   -> keyflag = 8            |       (7)    | UP(8)  | PG UP(9)  |
| DOWN ARROW -> keyflag = 2            +--------------+--------+-----------+
| TAB        -> keyflag = 6            |   SHFT-TAB(4)| ESC(5) |TAB(6)     |
| SHIFT-TAB  -> keyflag = 4            +--------------+--------+-----------+
| PAGE UP    -> keyflag = 9            |       (1)    | DOWN(2)| PG DN(3)  |
| PAGE DOWN  -> keyflag = 3            +--------------+--------+-----------|
|                                      |    ENTER(0)  |                    |
+--------------------------------------+-----------------------------------+
Requires rtrim() and insert() functions
--------------
NCURSES BASIC COLORS
--------------
0 Black   COLOR_BLACK (DOS Color #0)
1 Red     COLOR_RED (DOS Color #4)
2 Green   COLOR_GREEN (DOS Color #2)
3 Brown   COLOR_YELLOW (DOS Color #6)
4 Blue    COLOR_BLUE (DOS Color #1)
5 Magenta COLOR_MAGENTA (DOS Color #5)
6 Cyan    COLOR_CYAN (DOS Color #3)
7 White   COLOR_WHITE (DOS Color #7)
------------------------------------------------
COLORS WITH BOLD ATTRIBUTE SET (Foreground only)
------------------------------------------------
0 Gray (DOS Color #8)
1 Light Red (DOS Color #12)
2 Light Green (DOS Color #10)
3 Yellow (DOS Color #14)
4 Light Blue (DOS Color #9)
5 Light Magenta (DOS Color #13)
6 Light Cyan (DOS Color #11)
7 High Intensity White (DOS Color #15)
*/
        int flag = UPUT_EXIT_ENTER, curspos=0,counter, ky;
        char tempwhole[UPUT_MAX_LENGTH];
        char kystring[2];
        bool exitflag=false;
        kystring[1]='\0';
        strcpy(tempwhole,whole);
        if (fg < 8)
        {
                init_pair(64,fg,bg);
                attrset(COLOR_PAIR(64));
        }
        else
        {
                init_pair(64,fg-8,bg);
                attrset(COLOR_PAIR(64)|A_BOLD);
        }
        do
        {
                move(y,x);
                for (counter=0; counter < length; counter++)
                        addch(' ');
                move (y,x);
                addstr(whole);
                move(y,x+curspos);
                if (ins)
                        curs_set(2);
                else
                        curs_set(1);
                refresh();
                ky=getch();
                switch (ky)
                {
                        case KEY_LEFT:
                                if (curspos != 0)
                                        curspos--;
                                break;
                        case KEY_RIGHT:
                                if (curspos != length-1 && whole[curspos] != '\0')
                                        curspos++;
                                break;
                        case KEY_HOME:
                        //case KEY_A1: =KEY_HOME on Linux so not required 
                                curspos = 0;
                                break;
                        case KEY_END:
                        //case KEY_C1: =KEY_END on Linux so not required
                                rtrim(whole,' ');
                                curspos=strlen(whole);
                                if (strlen(whole) == length)
                                        curspos--;
                                break;
                        case KEY_IC: //insert key
                                ins = !ins;
                                if (ins)
                                        curs_set(2);
                                else
                                        curs_set(1);
                                break;
                        case KEY_DC: //delete key
                                if (curspos > strlen(whole)-1)
                                        break;
                                for (counter=curspos+1;whole[counter] != '\0'; counter++)
                                        whole[counter-1]=whole[counter];
                                whole[counter-1]='\0';
                                break;
                        case KEY_BACKSPACE:
                                if (curspos > 0)
                                        {
                                        for (counter=curspos; whole[counter] != '\0'; counter++)
                                                whole[counter-1]=whole[counter];
                                        whole[counter-1]='\0';
                                        curspos--;
                                        }
                                break;
                        case 10: // enter
                                flag=UPUT_EXIT_ENTER;
                                exitflag=true;
                                break;
                        case KEY_UP: // up-arrow
                                flag=UPUT_EXIT_UP_ARROW;
                                exitflag=true;
                                break;
                        case KEY_DOWN: // down-arrow
                                flag=UPUT_EXIT_DOWN_ARROW;
                                exitflag=true;
                                break;
			case KEY_PPAGE: // page-up
				flag=UPUT_EXIT_PAGE_UP;
				exitflag=true;
				break;
			case KEY_NPAGE: // page-down
				flag=UPUT_EXIT_PAGE_DOWN;
				exitflag=true;
				break;
                        case 9: // tab
                                flag=UPUT_EXIT_TAB;
                                exitflag=true;
                                break;
                        case KEY_BTAB: // shift-tab
                                flag=UPUT_EXIT_SHIFT_TAB;
                                exitflag=true;
                                break;
                        case 27: //esc
                                // esc twice to get out, otherwise eat the chars that don't work
                                //from home or end on the keypad
                                ky=getch();
                                if (ky == 27)
                                        {
                                        strcpy(whole,tempwhole);
                                        flag=UPUT_EXIT_ESCAPE;
                                        exitflag=true;
                                        }
                                else
                                        if (ky=='[')
                                                {
                                                getch();
                                                getch();
                                                }
                                        else 
                                                ungetch(ky);
                                break;
                        default:
                                if (strchr(permitted,ky))
                                {
                                kystring[0]=ky;
                                        if (ins)
                                                {
                                                if (curspos < strlen(whole))
                                                        {
                                                        if (strlen(whole) < length)
                                                                insert(whole,kystring,curspos);
                                                        else
                                                                curspos--;
                                                        }
                                                else
                                                        strcat(whole,kystring);
                                                }
                                        else
                                                if (curspos < strlen(whole)) 
                                                        whole[curspos]=ky;
                                                else
                                                        strcat(whole,kystring);
                                if (curspos < length-1)
                                        ++curspos;
                                }
                }
        } while (!exitflag);
        rtrim(whole,' ');
        return (flag);
}
char* rtrim(char* string, char junk)
{
    char* original = string + strlen(string);
    while(*--original == junk);
    *(original + 1) = '\0';
    return string;
}
void insert(char *st, const char *s2, const int location)
{
        char temp[UPUT_MAX_LENGTH];
        int counter, tempstringlength;
        strncpy(temp,st,location);
        temp[location]='\0';
        strcat(temp,s2);
        tempstringlength=strlen(temp);
        for (counter=location; st[counter] != '\0' && counter < UPUT_MAX_LENGTH; counter++, tempstringlength++)
                temp[tempstringlength]=st[counter];
        temp[tempstringlength]='\0';
        strcpy(st,temp);
        return;
}
