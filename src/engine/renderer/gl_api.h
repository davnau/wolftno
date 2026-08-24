#pragma once

using glBindBufferProc_t = void (*)(unsigned int target, unsigned int buffer);

extern glBindBufferProc_t _glewBindBufferARB;

extern "C" void glBindTexture(unsigned int target, unsigned int texture);
extern "C" void glTexSubImage2D(unsigned int target, int level, int xOffset,
                                int yOffset, int width, int height,
                                unsigned int format, unsigned int type,
                                const void* pixels);

