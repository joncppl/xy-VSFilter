# XySubFilter - Overlay

This branch (hack) of XySubFilter renders the subtitles in an overlay window on the primary monitor.

Ideal for multitaskers watching foreign shows & films.

Example usage: full screen-windowed game on primary monitor and full screen video on secondary monitor.
subtitles will appear overlayed on the game, 
allowing the user to watch a foreign-language video and play a game at the same time without having to look away from the game. 

Fully functional, maybe unstable? (not thoroughly tested)
Tested with MPC-HC and madVR from [KCP 0.6.0.7](http://haruhichan.com/forum/showthread.php?7545-KCP-Kawaii-Codec-Pack)

Could use the following improvements:

1. Completely ignorant of subtitle timing. Simply displays the most recent subtitle frames rendered. 
Hence the subtitles in the overlay may be out of sync by some number of ms.
    - partially fixed by buffering a few frames of subtitles, but not ideal solution
2. General architectural improvements. 
ie. Find a more elegant way to hook into the render process.
3. Can't configure the monitor. overlay only shows on primary monitor.
