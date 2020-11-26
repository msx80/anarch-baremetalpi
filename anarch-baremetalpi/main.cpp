//
// main.c
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//


// framebuffer setup
#define WIDTH 640
#define HEIGHT 400
#define DEPTH 8

// defines for Anarch
#define SFG_SCREEN_RESOLUTION_X WIDTH
#define SFG_SCREEN_RESOLUTION_Y HEIGHT
#define SFG_DITHERED_SHADOW 1
#define SFG_FPS 30
#define SFG_BACKGROUND_BLUR 1


#include <circle/util.h>
#include <circle/memory.h>
#include <circle/actled.h>
#include <circle/devicenameservice.h>
#include <circle/usb/usbhcidevice.h>
#include <circle/usb/usbkeyboard.h>
#include <circle/types.h>
#include <circle/bcmframebuffer.h>
#include <circle/interrupt.h>
#include <circle/exceptionhandler.h>
#include <circle/timer.h>
#include <circle/startup.h>
#include <circle/nulldevice.h>
#include <game.h>

// system objects
CMemorySystem           m_Memory;
CActLED                 m_ActLED;
CDeviceNameService	mDeviceNameService;
CBcmFrameBuffer		m_pFrameBuffer(WIDTH, HEIGHT, DEPTH);
CExceptionHandler       m_ExceptionHandler;
CInterruptSystem	mInterrupt;
CTimer			mTimer(&mInterrupt);
CNullDevice 		mNullDevice;
CUSBKeyboardDevice	*pKeyboard = 0;
CLogger			mLogger(LogWarning, &mTimer);
CUSBHCIDevice		mUSBHCI (&mInterrupt, &mTimer, TRUE);

uint16_t 		pitch; 		// stored screen pitch
u32			screenSize;	// stored screen size (pitch*width)
u8*			screen;		// pointer to actual screen memory
u8*			backbuffer;	// backbuffer where we write pixels

u8			keyState[SFG_KEY_COUNT];	// keyboard status to be read from anarch

void SFG_getMouseOffset(int16_t *x, int16_t *y){

}

uint32_t SFG_getTimeMs(){
	return mTimer.GetTicks()*10;
}

void SFG_save(uint8_t data[SFG_SAVE_SIZE]){

}

uint8_t SFG_load(uint8_t data[SFG_SAVE_SIZE]){
	return 0;
}

void SFG_setMusic(uint8_t value){

}

void SFG_playSound(uint8_t soundIndex, uint8_t volume){

}

void SFG_setPixel(uint16_t x, uint16_t y, uint8_t colorIndex){
	backbuffer[y*pitch+x] = colorIndex;
}

void SFG_processEvent(uint8_t event, uint8_t data){

}

int8_t SFG_keyPressed(uint8_t key){
	return keyState[key];
}

void SFG_sleepMs(uint16_t timeMs){
	// not really needed
	// mTimer.MsDelay(timeMs);
}


boolean initPalette()
{
	for (int i=0; i<256; i++)
	{
		m_pFrameBuffer.SetPalette(i, paletteRGB565[i]);
	}
	return m_pFrameBuffer.UpdatePalette();
}

void KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6])
{
	// this gets called with whatever key is currently pressed in RawKeys
	// (plus modifiers).

	// TODO restore lock maybe
	//keyspinlock.Acquire();

	memset(keyState, 0, SFG_KEY_COUNT);

	for (unsigned i = 0; i < 6; i++)
	{
		switch(RawKeys[i])
		{
			case 0x00: break;
			case 0x1d: keyState[SFG_KEY_A] = 1; break;
			case 0x1b: keyState[SFG_KEY_B] = 1; break;
			case 0x04: keyState[SFG_KEY_C] = 1; break;
			case 0x52: keyState[SFG_KEY_UP] = 1; break;
			case 0x51: keyState[SFG_KEY_DOWN] = 1; break;
			case 0x50: keyState[SFG_KEY_LEFT] = 1; break;
			case 0x4F: keyState[SFG_KEY_RIGHT] = 1; break;
		}
	}

	//keyspinlock.Release();

}

int initializeSystem(){
	if (!mInterrupt.Initialize ())
	{
		return -1;
	}

	if (!mTimer.Initialize ())
	{
		return -1;
	}
	if (!mLogger.Initialize(&mNullDevice))
	{
		return -1;
	}
	if (!mUSBHCI.Initialize())
	{
		return -1;
	}
	if (!m_pFrameBuffer.Initialize ())
	{
		return -1;
	}
	if (m_pFrameBuffer.GetDepth () != DEPTH)
	{
		return -1;
	}
	return 0;
}

int main (void)
{
	if(!initializeSystem())
	{
		return -1;
	}

	if(!initPalette()) 
	{
		return -1;
	}

	pitch = m_pFrameBuffer.GetPitch();
	screen = (u8*) (uintptr) m_pFrameBuffer.GetBuffer();
	screenSize = pitch*WIDTH;
	backbuffer = (u8*) m_Memory.HeapAllocate(screenSize, HEAP_ANY);

	// reset buttons state
	memset(keyState, 0, SFG_KEY_COUNT);

	// check for keyboard
	pKeyboard = (CUSBKeyboardDevice *) mDeviceNameService.GetDevice ("ukbd1", FALSE);
	if (pKeyboard){
		pKeyboard->RegisterKeyStatusHandlerRaw (KeyStatusHandlerRaw);
	}

	SFG_init();
	int running = 1;
	while (running)
	{
		if (!SFG_mainLoopBody()){
			running = 0;
		}
		m_pFrameBuffer.WaitForVerticalSync();

		// copy backbuffer over screen
		memcpy(screen, backbuffer, screenSize);
	}
  
	halt();
	return 0;
}
