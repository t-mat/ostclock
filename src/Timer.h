#ifndef TIMER_H
#define TIMER_H

struct Timer {
	Timer() {}

	const Timer& operator=(const Timer&) = delete;

	~Timer() {
		killTimer();
	}

	void start(HWND hWnd, int interval = 1000) {
		this->hWnd = hWnd;
		setTimer(interval);
	}

	void killTimer() {
		if(hWnd) {
			KillTimer(hWnd, id);
		}
	}

	void update(const SYSTEMTIME& t, int threshold = 200, int interval = 1000) {
		if(t.wMilliseconds > threshold) {
			adjusting = true;
			setTimer(interval + 1 - t.wMilliseconds);
		} else if(adjusting) {
			adjusting = false;
			setTimer(interval);
		}
	}

protected:
	void setTimer(int interval) {
		if(hWnd) {
			killTimer();
			SetTimer(hWnd, id, interval, nullptr);
		}
	}

	HWND hWnd { nullptr };
	const int id { 1 };
	bool adjusting { false };
};

#endif
