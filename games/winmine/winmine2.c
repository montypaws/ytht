#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>

#include "bbs.h"
#include "winmine2.h"

#define WINMINE2PATH MY_BBS_HOME "/etc/winmine2/"
int a[32][18];			//雷
int m[32][18];			//marked
int o[32][18];			//opened
char topID[20][20], topFROM[20][20], prize[20][3];
char userid[20] = "unknown.", fromhost[20] = "unknown.";
int topT[20], gameover = 0;
static char buf[10000];		// output buffer
struct termios oldtty, newtty;
int colorful = 1;

int
main(int n, char *cmd[])
{
	tcgetattr(0, &oldtty);
	cfmakeraw(&newtty);
	tcsetattr(0, TCSANOW, &newtty);
	if (n >= 2)
		strsncpy(userid, cmd[1], sizeof(userid));
	if (n >= 3)
		strsncpy(fromhost, cmd[2], sizeof(fromhost));
	winmine2log("ENTER");
	winmine();
	tcsetattr(0, TCSANOW, &oldtty);
	return 0;
}

void
show_issue()
{
	clear();
	prints
	    ("\r\n\r\n\r\n随着时间进入了公元二十一世纪，扫雷工具逐渐智能化，于是。。。\r\n");
	prints("\r\n[1;32m感应式扫雷[m出现了！\r\n\r\n");
	prints
	    ("[20;1H[1;33mftp://dii.nju.edu.cn/pub/bbs/dosmine.exe [m还有本游戏的Windows鼠标版, 欢迎下载.\r\n");
	refresh();
	pressanykey();
}

void
winmine()
{
	int x, y;
	show_issue();
	win_showrec();
	clear();
	prints("Enable ANSI color?[Y/N]");
	refresh();
	if (strchr("Nn", egetch()))
		colorful = 0;
	while (1) {
		clear();
		for (x = 0; x <= 31; x++)
			for (y = 0; y <= 17; y++) {
				a[x][y] = 0;
				m[x][y] = 0;
				o[x][y] = 0;
			}
		winrefresh();
		winloop();
		pressanykey();
	}
}

int
num_mine_beside(int x1, int y1)
{
	int dx, dy, s;
	s = 0;
	for (dx = x1 - 1; dx <= x1 + 1; dx++)
		for (dy = y1 - 1; dy <= y1 + 1; dy++)
			if (!(dx == x1 && dy == y1) && a[dx][dy])
				s++;
	return s;
}

int
num_mark_beside(int x1, int y1)
{
	int dx, dy, s;
	s = 0;
	for (dx = x1 - 1; dx <= x1 + 1; dx++)
		for (dy = y1 - 1; dy <= y1 + 1; dy++)
			if (!(dx == x1 && dy == y1) && m[dx][dy])
				s++;
	return s;
}

void
wininit(int x1, int y1)
{
	int n, x, y;
	srand(time(NULL) + getpid());
	for (n = 1; n <= 99; n++) {
		do {
			x = rand() % 30 + 1;
			y = rand() % 16 + 1;
		}
		while (a[x][y] != 0 || (abs(x - x1) < 2 && abs(y - y1) < 2));
		a[x][y] = 1;
	}
}

/* 双键 */
void
dblclick(int x, int y)
{
	int dx, dy;
	if (x < 1 || x > 30 || y < 1 || y > 16)
		return;
	if (!o[x][y])
		return;
	if (num_mine_beside(x, y) != num_mark_beside(x, y))
		return;
	for (dx = x - 1; dx <= x + 1; dx++)
		for (dy = y - 1; dy <= y + 1; dy++)
			windig(dx, dy);
}

/* 左键 */
void
windig(int x, int y)
{
	int dx, dy;
	if (x < 1 || x > 30 || y < 1 || y > 16)
		return;
	if (o[x][y] || m[x][y])
		return;
	o[x][y] = 1;
	winsh(x, y);
	if (a[x][y]) {
		gameover = 1;
		return;
	}
	if (num_mine_beside(x, y) == 0) {
		for (dx = x - 1; dx <= x + 1; dx++)
			for (dy = y - 1; dy <= y + 1; dy++)
				windig(dx, dy);
	}
}

/* 显示[x][y]处 */
void
winsh(int x, int y)
{
	move(x * 2 - 2, y - 1);
	winsh0(x, y);
}

/* 同上, 加快速度 */
void
winsh0(int x, int y)
{
	int c, d;
	static char word[9][10] = {
		"·", "１", "２", "３", "４", "５", "６", "７", "８"
	};
	static int cc[9] = { 38, 37, 32, 31, 33, 35, 36, 40, 39 };
	char buf[100];
	if (!o[x][y] && !m[x][y]) {
		prints("※");
		return;
	}
	if (m[x][y]) {
		prints("●");
		return;
	}
	if (a[x][y]) {
		prints("[1;31m雷[m");
		return;
	}
	c = num_mine_beside(x, y);
	d = 1;
	if (c == 0)
		d = 0;
	if (colorful)
		sprintf(buf, "[%d;%dm%s[m", d, cc[c], word[c]);
	else
		strcpy(buf, word[c]);
	prints(buf);
}

int marked;

void
winloop()
{
	int x, y, c, t0, inited;
	int oldx = -1, oldy = -1;
	char buf[100];
	x = 10;
	y = 8;
	inited = 0;
	marked = 0;
	clearbuf();
	t0 = time(0);
	while (1) {
		c = egetch();
		if (c == 257 && y > 1)
			y--;
		if (c == 258 && y < 16)
			y++;
		if (c == 260 && x > 1)
			x--;
		if (c == 259 && x < 30)
			x++;
		move(0, 20);
		sprintf(buf, "时间: %d ", (int) (time(0) - t0));
		prints(buf);
		move(40, 20);
		sprintf(buf, "标记: %d ", marked);
		prints(buf);
		move(0, 22);
		sprintf(buf, "坐标: %3d, %3d", x, y);
		prints(buf);
		move(x * 2 - 2, y - 1);
		if (c == 'h' || c == 'H')
			winhelp();
		if (c == 'd' || c == 'D')
			winrefresh();
		if (x != oldx || y != oldy) {
			windig2(x, y);
			oldx = x;
			oldy = y;
		}
		if (c == 'a' || c == 'A') {
			if (!inited) {
				wininit(x, y);
				inited = 1;
			}
			dig(x, y);
		}
		if ((c == 83 || c == 115) && !o[x][y]) {
			if (m[x][y]) {
				m[x][y] = 0;
				marked--;
			} else {
				m[x][y] = 1;
				marked++;
			}
			winsh(x, y);
		}
		if (checkwin() == 1) {
			move(0, 22);
			prints("祝贺你！你成功了！                    ");
			refresh();
			{
				char buf[100];
				sprintf(buf, "finished in %d s.",
					(int) (time(0) - t0));
				win_checkrec(time(0) - t0);
				winmine2log(buf);
				sleep(2);
			}
			gameover = 0;
			return;
		}
		if (gameover) {
			move(0, 22);
			prints("很遗憾，你失败了... 再来一次吧！    ");
			refresh();
			sleep(1);
			{
				char buf[100];
				sprintf(buf, "failed in %d s.",
					(int) (time(0) - t0));
				winmine2log(buf);
			}
			gameover = 0;
			return;
		}
		move(x * 2 - 2, y - 1);
		refresh();
	}
}

int
checkwin()
{
	int x, y, s;
	s = 0;
	for (x = 1; x <= 30; x++)
		for (y = 1; y <= 16; y++)
			if (!o[x][y])
				s++;
	if (s == 99)
		return 1;
	return 0;
}

void
dig(int x, int y)
{
	if (!o[x][y])
		windig(x, y);
	else
		dblclick(x, y);
}

int
num_untouched_beside(int x, int y)
{
	int x1, y1;
	int s = 0;
	for (x1 = x - 1; x1 <= x + 1; x1++)
		for (y1 = y - 1; y1 <= y + 1; y1++) {
			if (x1 < 1 || x1 > 30 || y1 < 1 || y1 > 16)
				continue;
			if (!o[x1][y1])
				s++;
		}
	return s;
}

void
windig2(int x, int y)
{
	int x1, y1;
	dblclick(x, y);
	if (o[x][y]) {
		if (num_untouched_beside(x, y) == num_mine_beside(x, y)) {
			for (x1 = x - 1; x1 <= x + 1; x1++)
				for (y1 = y - 1; y1 <= y + 1; y1++)
					if (!o[x1][y1] && !m[x1][y1]
					    && (x1 > 0 && x1 < 31 && y1 > 0
						&& y1 < 17)) {
						m[x1][y1] = 1;
						marked++;
						winsh(x1, y1);
					}

		}
	}
}

void
winrefresh()
{
	int x, y;
	clear();
	move(0, 23);
	prints
	    ("[1;32m扫雷[mfor bbs v1.00 by zhch.nju 00.3, press '[1;32mh[m' to get help, '[1;32m^C[m') to exit.");
	for (y = 1; y <= 16; y++) {
		move(0, y - 1);
		for (x = 1; x <= 30; x++)
			winsh0(x, y);
	}
	refresh();
}

void
winhelp()
{
	clear();
	prints
	    ("==欢迎来[1;35m" MY_BBS_NAME
	     "[m玩键盘扫雷游戏==\r\n---------------------------------\\r\n\r\n");
	prints("玩法很简单，和[1;34mwindows[m下的鼠标扫雷差不多.\r\n");
	prints
	    ("  '[1;32mA[m'键的作用相当于鼠标的左键及双击的作用， 程序根据你点击的位置\r\n");
	prints("  自动判断要进行哪种操作。\r\n");
	prints("  '[1;32mS[m'键则相当于鼠标右键的功能, 可用来标雷.\r\n");
	prints("  '[1;32mH[m'键用来显示本帮助信息.\r\n");
	prints("  '[1;32mQ[m'键退出游戏.\r\n");
	prints("  当屏幕乱掉时，可用'[1;32mD[m'可用来刷新屏幕。\r\n");
	prints
	    ("建议用[1;32mNetterm[m来玩(当然njuterm也可以,:)),[1;32mtelnet[m效果不是太好\r\n");
	prints("第一次点击一定会开一片，很舒服吧。\r\n");
	prints("熟练后，速度还是很快的，几乎可以达到鼠标扫雷的速度.\r\n");
	pressanykey();
	winrefresh();
}

void
win_loadrec()
{
	FILE *fp;
	int n;
	for (n = 0; n <= 19; n++) {
		strcpy(topID[n], "null.");
		topT[n] = 999;
		strcpy(topFROM[n], "unknown.");
		strcpy(prize[n], "NA");
	}
	fp = fopen(WINMINE2PATH "mine3.rec", "r");
	if (fp == NULL) {
		win_saverec();
		return;
	}
	for (n = 0; n <= 19; n++)
		fscanf(fp, "%s %d %s %s\n", topID[n], &topT[n], topFROM[n], prize[n]);
	fclose(fp);
}

void
win_saverec()
{
	FILE *fp;
	int n;
	fp = fopen(WINMINE2PATH "mine3.rec", "w");
	if (!fp)
		return;
	for (n = 0; n <= 19; n++) {
		fprintf(fp, "%s %d %s %s\n", topID[n], topT[n], topFROM[n], prize[n]);
	}
	fclose(fp);
}

void
win_showrec()
{
	char buf[200];
	int n;
	win_loadrec();
	clear();
	prints
	    ("[44;37m                      -" MY_BBS_NAME " BBS 感应式扫雷排行榜-                             \r\n[m");
	prints
	    ("[41m 名次       名字        耗时                      来自                     抽奖[m\r\n");
	for (n = 0; n <= 19; n++) {
		sprintf(buf, "[1;37m%3d[32m%13s[0;37m%12d[m%29s\033[33m%20s\r\n", n + 1,
			topID[n], topT[n], topFROM[n], prize[n]);
		prints(buf);
	}
	sprintf(buf,
		"[41m                                                                               [m\r\n");
	prints(buf);
	pressanykey();
}

void
win_checkrec(int dt)
{
	int n;
	win_loadrec();
	for (n = 0; n <= 19; n++)
		if (!strcmp(topID[n], userid)) {
			if (dt < topT[n]) {
				topT[n] = dt;
				strsncpy(topFROM[n], fromhost, sizeof(topFROM[0]));
				strcpy(prize[n], "未");
				win_sort();
			}
			return;
		}
	if (dt < topT[19]) {
		strsncpy(topID[19], userid, sizeof(topID[0]));
		topT[19] = dt;
		strsncpy(topFROM[19], fromhost, sizeof(topFROM[0]));
		strcpy(prize[19], "未");
		win_sort();
		return;
	}
}

void
win_sort()
{
	int n, n2, tmp;
	char tmpID[sizeof(topID[0])];
	for (n = 0; n <= 18; n++) {
		for (n2 = n + 1; n2 <= 19; n2++) {
			if (topT[n] > topT[n2]) {
				tmp = topT[n];
				topT[n] = topT[n2];
				topT[n2] = tmp;
				strcpy(tmpID, topID[n]);
				strcpy(topID[n], topID[n2]);
				strcpy(topID[n2], tmpID);
				strcpy(tmpID, topFROM[n]);
				strcpy(topFROM[n], topFROM[n2]);
				strcpy(topFROM[n2], tmpID);
				strcpy(tmpID, prize[n]);
				strcpy(prize[n], prize[n2]);
				strcpy(prize[n2], tmpID);
			}
		}
	}
	win_saverec();
	clear();
	prints("祝贺! 您刷新了自己的纪录!\r\n");
	pressanykey();
	win_showrec();
}

void
clear()
{
	prints("[H[J");
}

void
refresh()
{
	write(0, buf, strlen(buf));
	buf[0] = 0;
}

void
prints(char *b)
{
	strcat(buf, b);
}

void
move(int x, int y)
{
	char c[100];
	sprintf(c, "[%d;%dH", y + 1, x + 1);
	prints(c);
}

int
egetch()
{
	int c, d, e;
	c = getch0();
	if (c == 3 || c == 4 || c == -1)
		quit();
	if (c != 27)
		return c;
	d = getch0();
	e = getch0();
	if (e == 'A')
		return 257;
	if (e == 'B')
		return 258;
	if (e == 'C')
		return 259;
	if (e == 'D')
		return 260;
	return 0;
}

int
getch0()
{
	char c;
	if (read(0, &c, 1) <= 0)
		quit();
	return c;
}

void
quit()
{
	tcsetattr(0, TCSANOW, &oldtty);
	clear();
	refresh();
	winmine2log("QUIT");
	exit(0);
}

void
pressanykey()
{
	refresh();
	clearbuf();
}

void
clearbuf()
{
	char buf[128];
	refresh();
	read(0, buf, 100);
}

void
winmine2log(char *cc)
{
	FILE *fp;
	time_t t;
	fp = fopen(WINMINE2PATH "winmine2.log", "a");
	if (!fp)
		return;
	t = time(0);
	fprintf(fp, "%s did %s on %s", userid, cc, ctime(&t));
	fclose(fp);
}
