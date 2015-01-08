Process Manager
========
This is my solution for C++ task for Kodisoft Internship 2015. 

Thread-safe windows process manager allows:
- create process by specifying command line with args or open existing one specifying pid;
- restart (with the same command line and args) and close process; 
- retrieve process info (id, handle, status);
- stop process (without restart) and start it again;
- log all events into console or file;
- add callbacks `std::function<void()>` to all events.
