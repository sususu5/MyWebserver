## Reactor Mode
### The Reactor pattern is a synchronous event dispatch model where the main thread or a main reactor listens for events and dispatches them to appropriate handlers for processing.
### The Reactor listens for events (e.g., I/O read/write, connection requests) in an event loop.
### When an event occurs, the Reactor dispatches it to the corresponding handler.
### The handler processes the event synchronously, completing the required logic.

## Proactor Mode
### The application requests an asynchronous operation (e.g., asynchronous read/write) through the operating system.
### The OS completes the operation and notifies the application.
### The application processes the event upon receiving the notification.

---

## select
1. The number of fd has a maximum limit
2. Use iteration to check active fd

## poll
1. Use a linked list to represent
2. Use iteration to check active fd

## epoll
1. Use a black-red tree to represent
2. Activities will call the callback function, efficient
3. Support both LT mode and ET mode