#include "stdafx.h"
#include "overlay_func.h"
#include <assert.h>
#include "overlay.h"
#include "overlay_config.h"

//note: this code runs within the subtitle renderer's render thread.
//if it's slow, it will make the video lag
void overlay_send_frame(ISubRenderFrame *sub_frame, long long start, long long stop)
{
	if (!IsEnabled())
	{
		return;
	}

	static BOOL first_time = TRUE;
	if (first_time)
	{
		LoadConfig();
	}

	if (!overlay_is_open() && (first_time || DoRelaunch()))
	{
		CreateThread(0, NULL, init_overlay, NULL, NULL, NULL);
		first_time = FALSE;
	}

	if (!sub_frame)
	{
		clear_screen();
		return;
	}

	RECT outputRect;
	sub_frame->GetClipRect(&clipRect);
	sub_frame->GetOutputRect(&outputRect);

	frame = sub_frame;
	int count = 0;
	sub_frame->GetBitmapCount(&count);	

	std::vector<tsubdata> subs;
	subs.clear();

	for (int i = 0; i < count; i++)
	{
		tsubdata x;
		sub_frame->GetBitmap(i, &x.id, &x.position, &x.size, &x.pixels, &x.pitch);

		tsubdata y;
		y.id = x.id;
		y.position = x.position;
		y.size = x.size;
		y.pitch = x.pitch;

		//make our own copy of pixels, and then free the frame
		
		LPCVOID pxlPtr = x.pixels;
		const int offset = (y.pitch - y.size.cx * 4) / -4;
		size_t nBytes = y.size.cy*(y.size.cx - offset) * 4;
		if (nBytes != 0) {
			y.pixels = malloc(nBytes);
#ifdef DEBUG
			assert(y.pixels != NULL);
#endif
			if (y.pixels)
			{
				memcpy((void *)y.pixels, pxlPtr, nBytes);
			}
		}
		else
		{
			y.pixels = NULL;
		}
				
		subs.push_back(y);
	}

	g_sub_queue_mutex.lock();
	subs_queue.push(subs);
	g_sub_queue_mutex.unlock();

	//free the frame
	//sub_frame->Release();
	//releasing the frame here causes the renderer to crash
	//also by docs the memory is released upon the the next render anyway

	//however this function launches in a separate thread for screen rendering
	trigger_paint();
}
