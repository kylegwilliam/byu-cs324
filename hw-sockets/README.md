# Sockets

The purpose of this assignment is to give you hands-on experience with sockets.
Code is provided for a working client and server that communicate over
sockets of type `SOCK_DGRAM` (UDP).  You will experiment with that code and
then modify it to work for `SOCK_STREAM` (TCP), so you become familiar with
both communication types.


# Preparation

 1. Read the following in preparation for this assignment:

    - Sections 11.1 - 11.4 in the book

    Additionally, man pages for the following are also referenced throughout
    the assignment:

    - `udp`
    - `tcp`
    - `send()` / `sendto()` / `write()`
    - `recv()` / `recvfrom()` / `read()`

 2. Run `make` to build two executables: `client` and `server`.  These are
    programs that will communicate with each other as client and server,
    respectively.

 3. Review the [Strings, I/O, and Environment](../hw-strings-io-env)
    assignment.  Reviewing the principles on strings and I/O will greatly help
    you in this assignment!

 4. Log on to a BYU CS lab workstation directly, or log on remotely via SSH
    using the following command:

    ```
    ssh username@schizo.cs.byu.edu
    ```
    (Replace "username" with your actual CS username)

    The exercises in this assignment will only work if run from a CS lab
    machine.

 5. Start a tmux session.  Create two panes, such that the window looks like
    this:

    ```
    ---------------------------
    |  remote   |    local    |
    | (server)  |   (client)  |
    |           |             |
    ---------------------------
    ```

 6. On the left "remote" pane, use SSH to remotely log on to a CS lab machine
    _different_ from the one you are already on.  (see a list of machine names
    [here](https://docs.cs.byu.edu/doku.php?id=open-lab-layout)):

    ```
    ssh username@hostname
    ```
    (Replace "username" with your actual CS username and "hostname" with the
    name of the host to which you wish to log on.)


# Part 1: UDP Sockets

Open `client.c`, and look at what it does.  Then answer the following
questions.

 1. *What two system calls are used to create and prepare a (UDP) client socket
    for reading and writing, before you ever read or write to that socket?*

 2. *Where do the strings come from that are sent to the server (i.e., written
    to the socket)?*

Open `server.c`, and look at what it does.  Specific questions about the server
will come later.

In the next steps, you will be experimenting with UDP sockets on the client and
server side, using the `client` and `server` programs, respectively.

In the left "remote" pane, run the the following command:

```bash
$ ./server -4 port
```

(Replace `port` with a port of your choosing, an integer between 1024 and
65535.  Use of ports with values less than 1023 require root privileges).

The `-4` forces the server to prepare the socket to receive incoming messages
only on its IPv4 addresses.  While it is possible to have a program listen on
both IPv4 _and_ IPv6, that requires _two_ sockets--one for each protocol--and
that goes beyond the scope of this assignment, so using either `-4` or `-6`
designates which specific single address family (IP version) to use, IPv4 or
IPv6, respectively.

While it looks like the server is "hanging", it is actually just awaiting
incoming data from some client.  This is what is referred to as _blocking_.  To
see the system call on which it is blocking, sandwich the line containing the
`recvfrom()` statement between the following two print statements:

```c
printf("before recvfrom()\n"); fflush(stdout);
// recvfrom() goes here...
printf("after recvfrom()\n"); fflush(stdout);
```

Then re-run `make` and restart the server using the same command-line arguments
as before.  While your server is again blocking, at least you can see _where_!

Now, let's run the client to create some interaction between client and server.
In the right "local" pane, run the following:

```bash
$ ./client -4 hostname port foo bar abc123
```

(Replace `hostname` and `port` with name of the "remote" host and the port on
which the server program is now listening, respectively.)

The `-4` forces the client to prepare the socket to send messages to the
server's IPv4 address only.  Remember the server is listening on its IPv4
addresses only.  It is possible to have a your client try to communicate to the
server over whichever works first--IPv4 or IPv6, but there are some challenges
with doing that over UDP, so for the purposes of this assignment, using `-4` is
the best option.

Now run the command a second time:

```bash
$ ./client -4 hostname port foo bar abc123
```

 3. The server prints out the _remote_ address and port associated with the
    incoming message, and the client prints out the _local_ address and port
    associated with its socket.  They should match!  *What do you notice about
    the value of the local port used by the client for _different_ messages
    sent using the _same_ socket (i.e., from running `./client` a single
    time)?*
 4. *What do you notice about the value of the local port used by the client
    for _different_ messages sent using _different_ sockets (i.e., from running
    `./client` multiple times)?*
 5. *Looking inside `server.c`, how many sockets does the server use to
    communicate with multiple clients?*  For example, one for _each_ client,
    one for _all_ clients, etc.

Let's learn a bit more about how and when the _local_ address and port are set
on the socket used by the client.  Modify `client.c` in the following ways:
 - Comment out the line of code containing the call to `connect()`.  You will
   uncomment it later.  _Leave_ the line containing `break;` immediately below!
 - Comment out the line of code containing the call to `write()`.  You will
   uncomment it later.
 - Replace the call to `write()` with a call to `sendto()`.  The new line of
   code will look nearly identical to the one you just commented out, except
   that `sendto()` allows you to specify the recipient (remote IP address and
   port), which is necessary if `connect()` has not been called on the socket.

Re-run `make` to rebuild both binaries.  Interrupt and restart the server in
the left "remote" pane.

With the server running on the remote host, execute (again) the client command
you ran previously in the right "local" pane, sending the same strings as
before.

 6. *How do the local address and port values printed out by the client
     compare to those detected by the server?*

In `client.c` copy the lines of code that retrieve and print the local address
and port from the socket (i.e., starting with `getsockname()` and ending with
`fprintf()`) such that they are executed again _immediately_ after the call to
`sendto()`.

Re-run `make` to rebuild both binaries.  You might get some warnings about
variables that _might_ not have been initialized; for the purposes of this
assignment, you can ignore them.  Interrupt and restart the server in the
left "remote" pane.

With the server running on the remote host, execute (again) the client command
you ran previously in the right "local" pane, sending the same strings as
before.

 7. Analyze the output associated with the `fprintf()` statements that follow
    the calls to `getsockname()`.  *What do the differences in output teach
    you about _when_ the local address and port are set for a given socket?*

Let's make some other observations.  First, note that the lengths (i.e., number
of bytes) of the messages sent were longer than the lengths of the strings
making up the messages.  This is because we _chose_ to explicitly include the
null character for this particular program:

```c
		len = strlen(argv[j]) + 1;
		/* +1 for terminating null byte */
		// ...
		if (write(sfd, argv[j], len) != len) {
```

`strlen()` is used on `argv[j]` only because we know it is a null-terminated
string (i.e., because we wrote the program!).  But `write()` is only concerned
with bytes, so writing with argument `len` will result in writing one more
character than the string is long--the null character.  See
[Strings, I/O, and Environment](../hw-strings-io-env) for more.  When the
server echoes back our message, we can use string operations on it--but only
because we know that it contains the null character that we included when we
sent the message.

Now take note of how the number of calls to `send()` on the client relates to
the number of `recvfrom()` calls on the server.  Let's make some modifications
to both client and server code to better understand what is going on:

 - Modify `server.c`:
   - Sleep for five seconds immediately after calling `recvfrom()` on the
     socket.
   - Remove the `printf()` statements that you added earlier around the
     `recvfrom()` statement.
 - Modify `client.c`:
   - Remove the lines following `sendto()`, beginning with `getsockname()` and
     ending with `fprintf()`, which you added previously.
   - Comment out the code that calls `read()` and `printf()`, such that it does
     not attempt to read from the socket or print what it read after writing
     to the socket.

These changes make it so that the client is no longer waiting for the server to
respond before sending its subsequent messages; it just sends them one after
the other.  The five-second `sleep()` effectively _guarantees_ that the second
and third packets will _both_ have been received by the server's kernel, ready
to be read, before `recvfrom()` is called by the server the second time.

Re-run `make` to rebuild both binaries.  Then interrupt and restart the server
in the left "remote" pane.

With the server running on the remote host, execute (again) the client command
you ran previously in the right "local" pane, sending the same strings as
before.

 8. *How many _total_ calls to `send()` / `sendto()` / `write()` were made by
    the client?* Hint: refer to `client.c`.
 9. *How many messages were received by the kernel of the server-side process
    _before_ the server called `recvfrom()` the second time (i.e., _between_
    the server's first and second calls to `recvfrom()`)?*  You can assume that
    the messages were sent immediately with `write()` and that the network
    delay was negligible.
 10. *How many total calls to `recvfrom()` were required for the server process to
     read all of the messages/bytes that were sent _including_ the first call to
     `recvfrom()`?*  Hint: look at the server output, and refer to `server.c`.
 11. *When more than one message was ready for reading, why didn't the server
     read _all_ the messages that were ready with a single call to
     `recvfrom()`?*  Hint: see the man page for `udp`, specifically within the
     first three paragraphs of the "DESCRIPTION" section.

Change the value of the `len` argument passed to `recvfrom()` in `server.c`
from `BUF_SIZE` to `1`.

Re-run `make` to rebuild both binaries.  Then interrupt and restart the server
in the left "remote" pane.

With the server running on the remote host, execute (again) the client command
you ran previously in the right "local" pane, sending the same strings as
before.

 12. *How many total calls to `recvfrom()` were made?*  Hint: look at the
     server output, and refer to `server.c`.

 13. *Were all the bytes _sent_ by the client _received_ by the application?*
     Hint: look at the server output, and refer to `server.c`.

Restore the value of the `len` argument passed to `recvfrom()` in `server.c` to
`BUF_SIZE`.



## Part 2: TCP Sockets

In the next steps, you will be modifying the programs, so that they communicate
over TCP instead of UDP.  You will experiment with these TCP sockets on the
client and server side, using the `client` and `server` programs, respectively.

Before you begin modifications, make a copy of the UDP version of your client
and server programs:

```bash
$ cp client.c client-udp.c
$ cp server.c server-udp.c
```

Make the following modifications:

 - Modify `client.c`:
   - Make the socket use TCP (`SOCK_STREAM`) instead of UDP (`SOCK_DGRAM`).
   - Uncomment the `connect()` code that you commented out in Part 1.
   - Uncomment the read/print code that you commented out in Part 1.
   - Uncomment the `write()` code that you commented out in Part 1.
   - Remove the `sendto()` code that you added in Part 1 to the take the place
     of `write()`.

 - Modify `server.c`:
   - Make the server socket use TCP instead of UDP.
   - Wrap the entire `for` loop (i.e., `for (;;)`) in _another_ `for` loop with
     the same conditions (i.e., `for (;;)`).
   - Immediately before the _outer_ `for` loop, call the `listen()` function on
     the TCP server socket (you can use a `backlog` value of 100).
   - _Inside_ the outer `for` loop and immediately _before_ the inner `for`
     loop, use the `accept()` function to block, waiting for a client to
     connect, and return a new client socket.
     - You will need to declare a new variable of type `int` to hold the socket
       returned by `accept()`.
     - You can re-use some of the arguments that are currently used with
       `recvfrom()`, further down.
     - You will need to initialize `remote_addr_len` before it is used with
       `accept()`, just as it is currently initialized before being called with
       `recvfrom()`.
   - Change the `recvfrom()` call to `recv()`  and the `sendto()` call to
     `send()`.  Note that you just need to remove some of the arguments for
     each.
   - Comment out the `sleep()` call that you added in Part 1.
   - Use the file descriptor returned by `accept()` in the `recv()` and
     `send()` calls (i.e., the client socket).
   - If `recv()` returns 0 bytes, then:
     - call `close()` on the client socket.  When 0 is returned by `recv()`,
       the client has closed its end of the connection and is effectively EOF.
     - break out of the inner `for` loop; we can now listen for another client.

 14. *How does the role of the original socket (i.e., `sfd`, returned from the
     call to `socket()`), after `listen()` is called on it, compare with the
     role of the socket returned from the call to `accept()`?*  See the man
     pages for `listen()` and `accept()`.

 15. *With the new changes you have implemented, how have the semantics
     associated with the call to `connect()` changed?  That is, what will
     happen now when you call `connect()` that is different from when you
     called `connect()` with a UDP socket?*  See the man pages for `connect()`,
     `tcp`, and `udp`.

Re-run `make` to rebuild both binaries.  Interrupt and restart the server in
the left "remote" pane.

Now run the following command twice:

```bash
$ ./client -4 hostname port foo bar abc123
$ ./client -4 hostname port foo bar abc123
```

(Replace `hostname` and `port` with name of the "remote" host and the port
on which the server program is now listening, respectively.)

 16. The server prints out the _remote_ address and port associated with the
     incoming message, and the client prints out the _local_ address and port
     associated with its socket.  They should match!  *What do you notice about
     the value of the local port used by the client for _different_ messages
     sent using the _same_ socket (i.e., from running `./client` a single
     time)?*
 17. *What do you notice about the value of the local port used by the client
     for _different_ messages sent using _different_ sockets (i.e., from
     running `./client` multiple times)?*
 18. *Looking inside `server.c`, how many sockets does the server use to
     communicate with multiple clients?*  For example, one for _each_ client,
     one for _all_ clients, etc.  Explain how and why this behavior is
     different than that exemplified in question 5.

Make the following modifications, which mirror those made in Part 1
(immediately preceding questions 8 - 11):

  - Modify `server.c` such that it sleeps for five seconds immediately after
    calling `accept()`.
  - Modify `client.c` such that it does not attempt to read from the
    socket--or print what it read--after writing to the socket.  To do this,
    comment out the code that calls `read()` and `printf()` as described.

Similar to the changes made in Part 1, these changes make it so that the client
will have sent all of its messages _and_ those messages will have been received
by the server's kernel, before `recv()` is ever called by the server.

Re-run `make` to rebuild both binaries.  Interrupt and restart the server in
the left "remote" pane.

While the server is running on the remote host in the left "remote"
pane), run the following in the right "local" pane:

```bash
$ ./client -4 hostname port foo bar abc123
```

 19. *How many total calls to `send()` / `write()` were made by the client?*
     Hint: refer to `client.c`.
 20. *How many messages were received by the kernel of the server-side process
     _before_ the server called `recv()`?*  You can assume that the messages
     were sent immediately with `write()` and that the network delay was
     negligible.
 21. *How many total calls to `recv()` were required for the server process to
     read all of the messages/bytes that were sent?*  Hint: look at the server
     output, and refer to `server.c`.  Explain how and why this behavior is
     different than that exemplified in question 10.

Change the value of the `len` argument passed to `recv()` in `server.c` from
`BUF_SIZE` to `1`.

Re-run `make` to rebuild both binaries.  Then interrupt and restart the server
in the left "remote" pane.

With the server running on the remote host, execute (again) the client command
you ran previously in the right "local" pane, sending the same strings as
before.

 22. *How many total calls to `recv()` were made?*  Hint: look at the server
     output, and refer to `server.c`.

 23. *Were all the bytes _sent_ by the client _received_ by the application?*
     Hint: look at the server output, and refer to `server.c`.

Restore the value of the `len` argument passed to `recvfrom()` in `server.c` to
`BUF_SIZE`.


## Part 3: Making Your Client Issue HTTP Requests

Modify `client.c` such that instead of looping through each command-line
argument and writing it to the socket, it does the following, immediately after
the socket connection is established:

 - Write a loop to read (using `fread()`) input from standard input (`stdin`)
   into a buffer (`char []`) until EOF is reached (max total bytes 4096).  You
   can designate a specific "chunk" size (e.g., 512 bytes) to read from the
   file with each loop iteration.
 - Keep track of the bytes that were read from `stdin` during each iteration
   and in total.  Hint: see the return value of `fread()`.  With each iteration
   of the loop, you will want to offset the buffer (the one to which you are
   writing data read from `stdin`) by the number of total bytes read, so the
   bytes read from `stdin` are placed in the buffer immediately following the
   previous bytes read.
 - After _all_ the data has been read from `stdin` (i.e., EOF has been
   reached), loop to send all the data that was received (i.e., the bytes you
   just stored in the buffer), until it has all been sent.  You can designate a
   specific message size (e.g., 512 bytes) to send with each loop iteration.
   You can use `write()` or `send()` to send the bytes.

   Note that `write()` / `send()` will return the number of bytes actually
   sent, which might be less than the number you requested to be sent (see the
   `write()` man page for more!), so you need to keep track of the total bytes
   sent to ensure that all has been sent and write your loop termination test
   accordingly.

In the left "remote" pane, start a netcat (`nc` command) server listening
for incoming TCP connections on a port of your choosing:

```bash
$ nc -l port
```

(Replace `port` with a port of your choosing.)

Now test your client program by running the following in the right "local"
pane:

```bash
$ ./client -4 hostname port < alpha.txt
```

Because the open file descriptor associated with `alpha.txt` will be duplicated
on the standard input of `./client`, the contents of `alpha.txt` should be read
and sent over the socket to `nc`, which should print them out to the console.

To ensure that all bytes from the file were sent by `client` and received by
`server`, re-run `nc`, this time piping its standard output to `sha1sum`:

```bash
$ nc -l port | sha1sum
```

Then re-run the client program:

```bash
$ ./client -4 hostname port < alpha.txt
```

 24. *What is the output of the pipeline ending with `sha1sum`?*

     Hint: Because the bytes sent by the client should match the bytes in
     `alpha.txt`, the output of `sha1sum` should be the same as running `sha1sum`
     against `alpha.txt` itself.

Modify `client.c`:

 - After _all_ the data read from `stdin` has been sent to the socket, write
   another loop to read (using `read()` or `recv()`) from the socket into a
   buffer (`char []`) until the remote host closes its socket--that is, the
   return value from `read()` / `recv()` is 0 (note that this is, effectively,
   EOF).  The maximum _total_ bytes that you will read fom the socket is 16384.
   You can designate a specific "chunk" size (e.g., 512 bytes) to read from the
   socket with each loop iteration.
 - Keep track of the bytes that were read from the socket during each iteration
   and in total.  Hint: see the return value of `read()` / `recv()`.  With each
   iteration of the loop, you will want to offset the buffer (the one to which
   you are writing data read from the socket) by the number of total bytes
   read, so the bytes read from the socket are placed in the buffer immediately
   following the previous bytes read.
 - After _all_ the data has been read from the socket (the remote host has
   closed its socket), write the contents of the buffer to standard output.
   The data received on the socket will not necessarily be a string--i.e.,
   printable characters followed by a null-terminator.  It might be an image, a
   movie, an executable, or something else. Therefore, you should not use
   `printf()` or any other string operators unless you _know_ it is a string
   (see the [Strings, I/O, and Environment](../hw-strings-io-env#printf-and-friends)
   assignment).  Alternatives are `write()` and `fwrite()`.

   Even in the case where you _know_ bytes read from the socket are printable
   characters, if you want to perform any string operations, then you will need
   to add the null byte explicitly yourself.

Now, execute your client program such that:
 - you are sending data to the standard HTTP port (80) at
   www-notls.imaal.byu.edu;
 - you are redirecting the contents of `http-bestill.txt` to the standard input
   of the client process (using input redirection on the shell); and
 - you are redirecting the output of your client process to `bestill.txt`.

Use `cat` to show the contents of `http-bestill.txt`.  You should recognize the
format as an HTTP request.  Your program is now doing the following:
 1. Reading content from the file `http-bestill.txt` (redirected to standard
    input), which happens to be an HTTP request;
 2. Sending the request to an HTTP server;
 3. Reading the HTTP response from the server; and
 4. Writing the response to `bestill.txt` (to which standard output has been
    redirected).

In essence, your client program is acting as a _very_ simple HTTP client.

Note that after you have run your program, `bestill.txt` should contain:
 - the HTTP response code (200);
 - all HTTP headers returned in the HTTP response; and
 - all three verses to a hymn.

You can check this by running the following:

```bash
$ cat bestill.txt
```

 25. *Show the output to the following:*
     ```bash
     $ cat bestill.txt | ./strip_http.py | sha1sum
     ```

     Hint: it should start with `0dd26e...`

The `strip_http.py` script simply strips the HTTP response headers from the
output, so that you are left with just the content.  The `sha1sum` result,
therefore should only be computed with from the lyrics of the hymn, as returned
by the server.

The previous command execution involved an HTTP request for a file of type
"text/plain", which, of course, is a plaintext file.  Now we will try to
retrieve an image file, with arbitrary byte values.  Execute your client
program such that:
 - you are sending data to the standard HTTP port (80) at
   www-notls.imaal.byu.edu;
 - you are redirecting the contents of `http-socket.txt` to the standard input
   of the client process (using input redirection on the shell);
 - you are piping the standard output of your client process to
   `./strip_http.py`; and
 - you are redirecting the standard output of `./strip_http.py` to `socket.jpg`.

The `strip_http.py` script simply strips the HTTP response headers from the
output, so that you are left with just the content.  The file `socket.jpg`
should now contain a jpeg image that you can open and view with a suitable
program (e.g., a Web browser) to check its correctness.

 26. *Show the output to the following:*
     ```bash
     $ sha1sum socket.jpg
     ```

     Hint: it should start with `c03ce5...`


## Part 4: Review Questions

For this final set of questions, you are welcome to refer to previous
code/questions, set up your own experiments, and/or read the man pages.

 27. What happens when you call `read()` (or `recv()`) on an open socket (UDP
     or TCP), and there are no messages are available at the socket for
     reading?  Hint: see the man page for `recv()`, especially the
     "DESCRIPTION" section.  See also the instructions in Part 1.

 28. What happens when you call `read()` (or `recv()`) on an open socket (UDP
     or TCP), and the number of bytes available for reading is less than the
     requested amount?  Hint: see the man page for `read(2)`, especially the
     "RETURN VALUE" section.

 29. What happens you you call `read()` (or `recv()`) on an open UDP socket,
     and you specify a length that is less than the length of the next
     datagram?  Hint: see the man page for `udp`, specifically within the first
     three paragraphs of the "DESCRIPTION" section.  See also questions
     12 and 13.

 30. What happens you you call `read()` (or `recv()`) on an open TCP socket,
     and you specify a length that is less than the number of bytes available
     for reading? Hint: see questions 22 and 23.

Close down all the terminal panes in your `tmux` session to _close_ your `tmux`
session.
