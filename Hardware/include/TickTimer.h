#pragma once
#include <sys/types.h>

class TickTimer {
	public:
		TickTimer();
		TickTimer(ulong tick_micros);
		void init();
		void setTickMicros(ulong tick_micros);
		ulong getTickMicros();
		bool shouldTick();

	private:
		ulong lastMicros;
		ulong targetTickMicros;
};