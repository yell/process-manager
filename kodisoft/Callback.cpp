#include <windows.h>
#include <functional>

#include "Callback.h"

using std::function;

void Callback::operator() () const { 

	WaitForSingleObject(callbackEvent, INFINITE);
	callback(); 
	SetEvent(callbackEvent);
}

Callback::Callback() {

	callbackEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
}

Callback::Callback(const Callback & c) {

	WaitForSingleObject(c.callbackEvent, INFINITE);

	callbackEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	empty = c.empty; 
	callback = c.callback;

	SetEvent(c.callbackEvent);
}

bool Callback::isEmpty() const {

	WaitForSingleObject(callbackEvent, INFINITE);
	bool b = empty;
	SetEvent(callbackEvent);

	return b;
}

void Callback::set(function<void()> callback) { 

	WaitForSingleObject(callbackEvent, INFINITE);

	empty = false;
	this->callback = callback; 

	SetEvent(callbackEvent);
}

void Callback::reset() { 

	WaitForSingleObject(callbackEvent, INFINITE);

	empty = true;
	callback = [](){}; 

	SetEvent(callbackEvent);
}

Callback & Callback::operator = (const Callback & c) { 

	HANDLE h[2] = { callbackEvent, c.callbackEvent };

	WaitForMultipleObjects(2, h, TRUE, 0);

	if (this != &c) {

		empty = c.empty;
		callback = c.callback;
	}

	SetEvent(callbackEvent);
	SetEvent(c.callbackEvent);

	return *this; 
}

Callback::~Callback() {

	WaitForSingleObject(callbackEvent, INFINITE);
	CloseHandle(callbackEvent);
}