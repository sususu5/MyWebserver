# MyWebserver
A high-performance web server implemented in C++.

## Features
1. Uses IO multiplexing technology (Epoll) and thread pool to implement a multi-threaded Reactor concurrency model.
2. Parses HTTP request messages using regular expressions and a state machine, handling static resource requests.
3. Implements an automatically expanding buffer by encapsulating `char` using standard library containers.
4. Implements a timer based on a min-heap to close inactive connections that have timed out.
5. Utilizes the Singleton pattern and a blocking queue to implement an asynchronous logging system, recording the server's runtime status.
6. Implements a database connection pool using the RAII (Resource Acquisition Is Initialization) mechanism, reducing the overhead of establishing and closing database connections, while also supporting user registration and login functionality.

## Directory Tree
```
.
├── code            Source code
│   ├── buffer
│   ├── http
│   ├── log
│   ├── timer
│   ├── pool
│   ├── server
│   └── main.cpp
├── test            Tests for log and threadpool
│   ├── Makefile
│   └── test.cpp
├── resources       Static resources
│   ├── index.html
│   ├── images
│   ├── video
│   ├── fonts
│   ├── js
│   └── css
├── webbench-1.5    Stress test
├── build          
│   └── Makefile
├── Makefile
├──.gitignore
└── README.md
```

## Project Initiation
Download MySQL first, replace "yourdb" to the name of the databse you wnat to create.
```
// create a new database
create database yourdb;

// create the user table
USE yourdb;
CREATE TABLE user(
    username char(50) NULL,
    password char(50) NULL
)ENGINE=InnoDB;

// add data
INSERT INTO user(username, password) VALUES('name', 'password');

// compile the project
make

// run the project
./bin/server
```

## Run test for Log and ThreadPool
```
cd test
make
./test
```

## Stress Testing
Start the server before running the following test
```
cd webbench-1.5
make
./webbench -c [number of client] -t [number of seond] http://ip:1316/
```
This is my testing result  
![stress-testing-image](/resources/images/stress-testing.png)