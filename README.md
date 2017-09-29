# Process manager
My solution for C++ task for Kodisoft Internship 2015. The task was to create thread-safe windows process manager.

## Features
- create process by specifying command line with args or open existing one specifying pid;
- restart (with the same command line and args) and close process; 
- retrieve process info (`id`, `handle`, `status`);
- stop process (without restart) and start it again;
- log all events into console or file;
- add `std::function<void()>` callbacks to all events;
- work both with unicode and ascii characters.
