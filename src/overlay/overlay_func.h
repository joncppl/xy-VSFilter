#pragma once
#include <queue>
#include <mutex>

#ifdef FROM_OVERLAY
typedef ULONGLONG REFERENCE_TIME;
#endif

#include "../../include/SubRenderIntf.h"

void overlay_send_frame(ISubRenderFrame *sub_frame, long long start, long long stop);
void trigger_paint();
extern BOOL overlay_is_open();

DWORD WINAPI init_overlay(LPVOID lpParam);
void clear_screen();

extern FILE *mylog;

typedef struct {
	ULONGLONG id;
	POINT position;
	SIZE size;
	LPCVOID pixels;
	int pitch;
} tsubdata;

extern ISubRenderFrame *frame;
extern std::queue<std::vector<tsubdata>> subs_queue;
//extern std::vector<tsubdata> subs;
extern RECT clipRect;
extern std::mutex g_sub_queue_mutex;
