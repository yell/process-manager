#ifndef CALLBACK_H
#define CALLBACK_H

#include <functional>

using std::function;

class Callback {

	private:
		bool empty = true;
		function<void()> callback = [](){};

		void operator()() const;

	public:
		Callback();
		Callback(const Callback &);

		void set(function<void()>);
		void reset(void);
		bool isEmpty() const;

		Callback & operator = (const Callback &);
		~Callback();

		friend class Process;
};

#endif // CALLBACK_H