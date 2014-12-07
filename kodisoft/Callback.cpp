#include <functional>

#include "Callback.h"

using std::function;

void Callback::operator() () const { 

	callback(); 
}

Callback::Callback() {}

Callback::Callback(const Callback & c) {

	empty = c.empty; 
	callback = c.callback;
}

bool Callback::isEmpty() const {

	return empty;
}

void Callback::set(function<void()> callback) { 

	empty = false;
	this->callback = callback; 
}

void Callback::reset() { 

	empty = true;
	callback = [](){}; 
}

Callback & Callback::operator = (const Callback & c) { 

	if (&c == this) 
		return *this; 
	empty = c.empty; 
	callback = c.callback; 
	return *this; 
}

Callback::~Callback() {}