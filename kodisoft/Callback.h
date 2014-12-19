#ifndef CALLBACK_H
#define CALLBACK_H

#include <windows.h>
#include <functional>

using std::function;

class Callback {

	private:
		bool empty = true;
		function<void()> callback = [](){};
		HANDLE callbackEvent;
 
		void operator()() const;

	public:
		Callback();
		Callback(const Callback &);

		bool isEmpty() const;
		void set(function<void()>);
		void reset();

		Callback & operator = (const Callback &);
		~Callback();

		friend class Process;
};

#endif // CALLBACK_H