#include <sys/time.h>
#include <time.h>
#include "stdio.h"
#include "tamaemu.h"
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>


#include "lcd.h"
#include "benevolentai.h"


Tamagotchi *tama;


void sigintHdlr(int sig)  {
	if (tama->cpu->Trace) exit(1);
	tama->cpu->Trace=1;
}


int getKey() {
	char buff[100];
	fd_set fds;
	struct timeval tv;
	FD_ZERO(&fds);
	FD_SET(0, &fds);
	tv.tv_sec=0;
	tv.tv_usec=0;
	select(1, &fds, NULL, NULL, &tv);
	if (FD_ISSET(0, &fds)) {
		fgets(buff, 99, stdin);
		return buff[0];
	} else {
		return 0;
	}
}


#define FPS 5

int main(int argc, char **argv) {
	unsigned char **rom;
	long us;
	int k;
	int speedup=0;
	struct timespec tstart, tend;
	Display display;
	signal(SIGINT, sigintHdlr);
	rom=loadRoms();
	tama=tamaInit(rom);
	benevolentAiInit();
	while(1) {
		clock_gettime(CLOCK_MONOTONIC, &tstart);
		tamaRun(tama, FCPU/FPS-1);
		lcdRender(tama->dram, tama->lcd.sizex, tama->lcd.sizey, &display);
		lcdShow(&display);
		tamaDumpHw(tama->cpu);
		k=benevolentAiRun(&display);
		clock_gettime(CLOCK_MONOTONIC, &tend);
		us=(tend.tv_nsec-tstart.tv_nsec)/1000;
		us+=(tend.tv_sec-tstart.tv_sec)*1000000L;
		us=(1000000L/FPS)-us;
		printf("Time left in frame: %d us\n", us);
		if (!speedup && us>0) usleep(us);
		k=getKey();
		if (k=='1') tamaPressBtn(tama, 0);
		if (k=='2') tamaPressBtn(tama, 1);
		if (k=='3') tamaPressBtn(tama, 2);
		if (k=='s') speedup=!speedup;
	}
}
