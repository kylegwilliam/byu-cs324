# Socket Treasure Hunt

The purpose of this assignment is to help you become more familiar with the
concepts associated with sockets, including UDP communications, local and
remote port assignment, IPv4 and IPv6, message parsing, and more.

# Table of Contents

 - [Overview](#overview)
 - [Preparation](#preparation)
   - [Reading](#reading)
 - [Instructions](#instructions)
   - [Level 0](#level-0)
   - [Level 1](#level-1)
   - [Level 2](#level-2)
   - [Level 3](#level-3)
   - [Level 4 (Extra Credit)](#level-4-extra-credit)
 - [Helps and Hints](#helps-and-hints)
   - [Message Formatting](#message-formatting)
   - [Error Codes](#error-codes)
   - [Op Codes](#op-codes)
   - [UDP Socket Behaviors](#udp-socket-behaviors)
   - [Testing Servers](#testing-servers)
 - [Automated Testing](#automated-testing)
 - [Evaluation](#evaluation)
 - [Submission](#submission)


# Overview

This lab involves a game between two parties: the *client* and the *server*.
The server runs on a CS lab machine, awaiting incoming communications.
The client, also running on a CS lab machine, initiates communications with the
server, requesting the first chunk of the treasure, as well as directions to
get the next chunk.  The client and server continue this pattern of requesting
direction and following direction, until the full treasure has been received.
Your job is to write the client.


# Preparation


## Reading

man pages for the following are referenced throughout the assignment:

 - `udp`
 - `ip`
 - `ipv6`
 - `socket`
 - `socket()`
 - `send()`
 - `recv()`
 - `bind()`
 - `getaddrinfo()`
 - `htons()`
 - `ntohs()`
 - `getpeername()`
 - `getsockname()`
 - `getnameinfo()`


# Instructions


## Level 0

First, open `treasure_hunter.c` and look around.  You will note that there are
two functions and one `#define` statement.

Replace `PUT_USERID_HERE` with your numerical user ID, which you can find by
running `id -u` on a CS lab machine.  From now on you can use `USERID` as an
integer wherever you need to use your user ID in the code.

Now take a look the `print_bytes()` function.  This function was created to
help you see what is in a given message that is about to be sent or has just
been received.  It is called by providing a pointer to a memory location, e.g.,
an array of `unsigned char`, and a length.  It then prints the hexadecimal
value for each byte, as well as the ASCII character equivalent for values less
than 128 (see `man ascii`).


### Command-Line Arguments

Your program should have the following usage:

```
$ ./treasure_hunter server port level seed
```

 - `server`: the domain name of the server.
 - `port`: the port on which the server is expecting initial communications.
 - `level`: the level to follow, a value between 0 and 4.
 - `seed`: an integer used to initialize the pseudo-random number generator on
   the server.

Store each of the arguments provided on the command line in variables.  Note
that `port`, `level`, and `seed` are all integers, so you will want to convert
them to integers. However, because `getaddrinfo()` takes a `char *` for port,
you might want to maintain a string (`char []`) version of the port as well.

It would be a good idea here to check that all command-line variables have been
stored appropriately from the command line.  Create some print statements to
that effect.  Then build your program by running the following:

```bash
$ make
```

Then run it:

```bash
$ ./treasure_hunter server 32400 0 7719
```

Try running it with different values for server, port, level, and seed to make
sure everything looks as it should.


### Initial Request

The very first message that the client sends to the server should be exactly
eight bytes long and have the following format:

<table border="1">
<tr>
<th>00</th><th>01</th><th>02</th><th>03</th><th>04</th><th>05</th><th>06</th><th>07</th></tr>
<tr>
<td colspan="1">0</td>
<td colspan="1">Level</td>
<td colspan="4">User ID</td>
<td colspan="2">Seed</td></tr>
</table>

The following is an explanation of each field:

 - Byte 0: 0
 - Byte 1: an integer 0 through 4, corresponding to the *level* of the
   course.  This comes from the command line.
 - Bytes 2 - 5: a `unsigned int` corresponding to the user ID of the user in
   network byte order (i.e., big-endian or most significant bytes first).
   You can populate this with the value of `USERID`, for which you created a
   `#define` with the appropriate value.
 - Bytes 6 - 7: an `unsigned short` used, along with the user ID and the level,
   to seed the pseudo-random number generator used by the server, in network
   byte order.  This comes from the command line.

   This is used to allow the client to experience consistent behavior every
   time they interact with the server, to help with development and
   troubleshooting.

In the `main()` function, declare an array of `unsigned char` to hold the bytes
that will make up your request.  Populate that array with values from the
command line.

You might be wondering how to populate the array of `unsigned char` with values
longer than bytes.  Please take a moment to read the section on
[message formatting](#message-formatting), which will provide some background
and some examples for building your message.

Now call `print_bytes()`, specifying the message buffer that you have populated
as the first argument and 8 as your second argument (i.e., because it should be
an 8-byte message).

Re-build and run your file with the following:

```bash
$ make
$ ./treasure_hunter server 32400 0 7719
```

Check the `print_bytes()` output to make sure that the message you intend to
send looks correct.  Bytes 0 and 1 should both be zero.  Bytes 2 through 5
should have the value of your user ID in network byte order.  Bytes 6 and 7
should have the value of your seed in network byte order.  For example assuming
a user ID of 12345, the output would be:

```bash
$ ./treasure_hunter server 32400 0 7719

00: 00 00 00 00  30 39 1E 27  . . . . 0 9 . '
```

Beyond the offset (`00:`), you can see the byte representing level 0 (`00`),
the bytes representing the user ID (`00 00 30 39`), and the bytes representing
the seed (`1E 27`).  Because this message is not "text", there are no useful
ASCII representations of the byte values, so the output is mostly `.`.  Note
that you can find the hexadecimal representation of your user ID, the seed, and
any other integer, by running the following from the command line:

```bash
$ printf "%08x\n" 12345
```

substituting `12345` with the integer that you wish you represent as
hexadecimal.

If your message looks good, based on the output to `print_bytes()`, continue
on!

Of course, you have not sent or received any messages at this point, but you
now know how to *format* the initial message appropriately.


### Sending and Receiving

With your first message created, set up a UDP client socket, with
`getaddrinfo()` and `socket()`, specifying `AF_INET` and `SOCK_DGRAM` as the
address family and socket type, respectively.  Remember that the port passed as
the second argument (`service`) to `getaddrinfo()` is type `char *`.  Thus, if
you only have the port as an integer, then you should convert it (not cast
it!).

It is recommended that you _not_ call `connect()` on the socket but rather save
the value of the remote address set by `getaddrinfo()` into a
`struct sockaddr_in` (or a `struct sockaddr_in6` for IPv6) that you have
declared.  Additionally, because the kernel _implicitly_ assigns the local
address and port to a given socket when none has been _explicitly_ assigned
using `bind()`, you will find it useful to learn the port that has been
assigned using `getsockname()` and save it to a `struct sockaddr_in` (or
`struct sockaddr_in6` for IPv6) that you have declared.  Finally, you will find
it useful to keep track of your address family and address length.  Note that
for levels 0 through 3, your client will only use IPv4 (i.e., `AF_INET`), so
these values will always be the same.  Nonetheless, keeping track of the
address family and length is good practice, and you will find it useful in for
[level 4 (extra credit)](#level-4-extra-credit) when IPv6 is added.

To keep track of all this, you might declare variables like the following:

```c
	int addr_fam;
	socklen_t addr_len;

	struct sockaddr_in remote_addr_in;
	struct sockaddr_in6 remote_addr_in6;
	struct sockaddr *remote_addr;

	struct sockaddr_in local_addr_in;
	struct sockaddr_in6 local_addr_in6;
	struct sockaddr *local_addr;
```

See the [sockets homework assignment](../hw-sockets) for example code.

A note about the `local_addr` and `remote_addr` variables.  The functions
`sendto()`, `recvfrom()`, and `bind()` take type `struct sockaddr *`
as an argument, rather than `struct sockaddr_in *` or `struct sockaddr_in6 *`.
Per the `bind()` man page: "The only purpose of this structure is to cast the
structure pointer passed in addr in order to avoid compiler warnings." In
essence, this structure makes it so that `bind()` (and the other functions) can
have a single definition that supports both IPv4 and IPv6.  As long as the
`struct sockaddr *` points to valid `struct sockaddr_in` or
`struct sockaddr_in6` instance, then the function will work properly.  You can
think of this as C's version of polymorphism.  All that being said, if your
client maintains a `struct sockaddr *` (e.g., `local_addr`) that points to
whichever of `local_addr_in` or `local_addr_in6` is correct for the current
address family being used, you can simply use `local_addr` in the calls to
`bind()` and friends.

When everything is set up, send your message using `sendto()`.  Then read the
server's response with `recvfrom()` call.  Remember, it is just one call to
each!  Store the return value of  `recvfrom()`, which reflects the number of
bytes you received.  Unlike the [initial request](#create-an-initial-request)
you sent, which is always eight bytes, the size of the response is variable
(but will never be more than 256 bytes).  Finally, call `print_bytes()` to
print out the contents of the message received by the server.

Re-build and re-run your program:

```bash
$ make
$ ./treasure_hunter server 32400 0 7719
```

At this point, you need to supply the name of an actual server. See
[this section](#testing-servers) for a list of servers and ports that you may
use.


### Directions Response

Take a look at the response from the server, as printed by `print_bytes()`.
Responses from the server are of variable length (but any given message will
consist of fewer than 256 bytes) and will follow this format:

<table border="1">
<tr>
<th>00</th><th>01</th><th>::</th><th>n</th><th>n + 1</th><th>n + 2</th><th>n + 3</th><th>n + 4</th>
<th>n + 5</th><th>n + 6</th><th>n + 7</th></tr>
<tr>
<td colspan="1">Chunk Length</td>
<td colspan="3">Chunk</td>
<td colspan="1">Op Code</td>
<td colspan="2">Op Param</td>
<td colspan="4">Nonce</td></tr>
</table>

The following is an explanation of each field:

 - Byte 0: an `unsigned char`.
   - If 0: the hunt is over.  All chunks of the treasure have been received in
     previous messages from the server.  At this point the client can exit.
   - If between 1 and 127: A chunk of the message, having length corresponding
     to the value of byte 0, immediately follows, beginning with byte 1.
   - If greater than 127: The server detected an error and is alerting the
     client of the problem, with the value of the byte corresponding to the
     error encountered.  See [Error Codes](#error-codes) for more details.
   Note that in the case where byte 0 has value 0 or a value greater than 127,
   the entire message will only be one byte long.

 - Bytes 1 - `n` (where `n` matches the value of byte 0; only applies where `n`
   is between 1 and 127): The chunk of treasure that comes immediately after
   the one received most recently.

 - Byte `n + 1`: This is the op-code, i.e., the "directions" to follow to get
   the next chunk of treasure and the next nonce.  At this point, the op-code
   value you get from the server should be 0, which means "communicate with the
   server the same way you did before" or simply "no change." For future
   levels, this field will have values other than 0, each of which will
   correspond to a particular change that should be made with regard to how you
   contact the server.  See [Op Codes](#op-codes) for a summary, and the
   instructions for levels [1](#level-1), [2](#level-2), [3](#level-3), and
   [4](#level-4-extra-credit) for a detailed description of each.

 - Bytes `n + 2` - `n + 3`: These bytes, an `unsigned short` in network byte
   order is the parameter used in conjunction with the op-code.  For op-code 0,
   the field exists, but can simply be ignored.

 - Bytes `n + 4` - `n + 7`: These bytes, an `unsigned int` in network byte
   order, is a nonce.  The value of this nonce, plus one, should be returned in
   every communication back to the server.

Extract the chunk length, the treasure chunk, the op-code, the op-param,
and the nonce using the hints in the [message formatting](#message-formatting)
section), storing them in variables of the appropriate types, so you can work
with them.  Print them out to verify that you have extracted them properly, and
pay attention to endian-ness.  For example, if you receive the nonce
0x12345678, then printing out the value of the variable in which you have
stored the nonce, e.g., with:

```
printf("%x", nonce);
```

should result in the following output:

```
12345678
```

Remember to add a null byte after the treasure chunk, or `printf()` will not
know how to treat it properly.  Also, the op-param has no use for level 0, and
the value might actually be 0.  This means that endian-ness is hard to check at
this point.  But you can check it in future levels.

You will be sending the nonce (well, a variant of it) back to the server, in
exchange for additional chunks, until you have received the whole treasure.


### Follow-Up Request

After the initial request, every subsequent request will be exactly four bytes
and will have the following format:

<table border="1">
<tr>
<th>00</th><th>01</th><th>02</th><th>03</th></tr>
<tr>
<td colspan="4">Nonce + 1</td></tr>
</table>

 - Bytes 0 - 3: an `unsigned int` having a value of one more than the nonce
   most recently sent by the server, in network byte order.  For example, if
   the server previously sent 100, then this value would be 101.

Build your follow-up request using the guidance in the
[message formatting helps](#message-formatting) section, and use
`print_bytes()` to make sure it looks the way it should.  Re-build and re-run
your program:

```bash
$ make
$ ./treasure_hunter server 32400 0 7719
```

If everything looks good, then use `sendto()` to send your follow-up request
and `recvfrom()` to receive your next directions response.


### Program Output

Now generalize the pattern of sending
[follow-up requests](#follow-up-request) in response to
[directions responses](#directions-response), receiving the entire treasure,
one chunk at a time.  Add each new chunk received to the treasure you already
have.  You will want to create a loop with the appropriate termination test
indicating that the entire treasure has been received
(i.e., [byte 0 has a value of 0](#directions-response)).

The overall interaction is illustrated in the following image:

<img src="treasure_hunt.png" width="800">

Once your client has collected all of the treasure chunks, it should print the
entire treasure to standard output, followed by a newline (`\n`).  For example,
if the treasure hunt yielded the following chunks:

 - `abc`
 - `de`
 - `fghij`

Then the output would be:

```
abcdefghij
```

No treasure will be longer than 1,024 characters, so you may use that as your
buffer size.  Remember to add a null byte after the characters comprising your
output, so they can be used with `printf()`!

At this point, make sure that the treasure is the only program output.  Remove
print statements that you have added to your code for debugging by commenting
them out or otherwise taking them out of the code flow (e.g., with
`if (verbose)`).


### Checkpoint 0

At this point, you can also test your work with
[automated testing](#automated-testing).  Level 0 should work at this point.

Now would be a good time to save your work, if you haven't already.


## Level 1

With level 0 working, you have a general framework for client-server
communications.  The difference now is that the directions response will
contain real actions.

For level 1, responses from the server will use op-code 1.  The client should
be expected to do everything it did at level 0, but when op-code 1 is provided
in the [directions response](#directions-response), it should extract the
op-param (bytes `n + 2` and `n + 3`) from the response and use that value as
the remote port for future communications with the server.  That value is an
`unsigned short` stored in network byte order (see
[Directions Response](#directions-response) and
[Message Formatting](#message-formatting)).

Some guidance follows as to how to use the new remote port in future
communications.

It was recommended [previously](#sending-and-receiving) that you maintain local
and remote addresses and ports using structures of type `struct sockaddr_in`
(or `struct sockaddr_in6` for IPv6).  We now take a closer look at these
structures to see how things are stored.  The data structures used for holding
local or remote address and port information are defined as follows (see the
man pages for `ip(7)` and `ipv6(7)`, respectively).

For IPv4 (`AF_INET`):
```c
           struct sockaddr_in {
               sa_family_t    sin_family; /* address family: AF_INET */
               in_port_t      sin_port;   /* port in network byte order */
               struct in_addr sin_addr;   /* internet address */
           };

           /* Internet address. */
           struct in_addr {
               uint32_t       s_addr;     /* address in network byte order */
           };
```

For IPv6 (`AF_INET6`):
```c
           struct sockaddr_in6 {
               sa_family_t     sin6_family;   /* AF_INET6 */
               in_port_t       sin6_port;     /* port number */
               uint32_t        sin6_flowinfo; /* IPv6 flow information */
               struct in6_addr sin6_addr;     /* IPv6 address */
               uint32_t        sin6_scope_id; /* Scope ID (new in 2.4) */
           };

           struct in6_addr {
               unsigned char   s6_addr[16];   /* IPv6 address */
           };
```

Thus, the port of the structure can be updated simply by doing the following:

```c
	remote_addr_in.sin_port = htons(port);
```

Note that the `sin_port` of the `struct sockaddr_in` member contains the port
in *network* byte ordering (See [Message Formatting](#message-formatting)),
hence the use of `htons()`.

The same for IPv6:

```c
	remote_addr_in6.sin6_port = htons(port);
```

Just as with level 0, have your code
[collect all the chunks and printing the entire treasure to standard output](#program-output).


### Checkpoint 1

At this point, you can also test your work with
[automated testing](#automated-testing).  Levels 0 and 1 should both work at
this point.

Now would be a good time to save your work, if you haven't already.


## Level 2

For level 2, responses from the server will use either op-code 1 or op-code 2,
selected at random.  The client should be expected to do everything it did at
levels 0 and 1, but when op-code 2 is provided in the
[directions response](#directions-response), it should extract the op-param
(bytes `n + 2` and `n + 3`) from the response and use that value as the local
port for future communications with the server.  That value is an
`unsigned short` stored in network byte order (see
[Directions Response](#directions-response) and
[Message Formatting](#message-formatting)).

Association of a local address and port to a socket is done with the `bind()`
function.  The following tips associated with `bind()` are not specific to UDP
sockets (type `SOCK_DGRAM`) but are nonetheless useful for this lab:

 - The local address and port can be associated with a socket using `bind()`.
   See the man pages for `udp` and `bind()`.
 - `bind()` can only be called *once* on a socket.  See the man page for
   `bind()`.
 - Even if `bind()` has *not* been called on a socket, if a local address and
   port have been associated with the socket implicitly (i.e., when `write()`,
   `send()`, or `sendto()` is called on that socket), `bind()` cannot be called
   on that socket.

Therefore, every time the client is told to use a new local port (i.e., with
op-code 2 in a directions response), _the current socket must be closed_, and a
new one must be created.  Then `bind()` is called on the new socket.

It was recommended [previously](#sending-and-receiving) that you maintain local
and remote addresses and ports using structures of type `struct sockaddr_in`
(or `struct sockaddr_in6` for IPv6).  Because you have done this, you can now
run something like this to bind the newly-created socket to a specific port:

```c
	local_addr_in.sin_family = AF_INET; // use AF_INET (IPv4)
	local_addr_in.sin_port = htons(port); // specific port
	local_addr_in.sin_addr.s_addr = 0; // any/all local addresses

	local_addr_in6.sin6_family = AF_INET6; // IPv6 (AF_INET6)
	local_addr_in6.sin6_port = htons(port); // specific port
	bzero(local_addr_in6.sin6_addr.s6_addr, 16); // any/all local addresses

	if (bind(sfd, local_addr, addr_len) < 0) {
		perror("bind()");
	}

```

Note that this code prepares both `local_addr_in` and `local_addr_in6` with the
appropriate values for receiving on a given port.  However, it's not that we're
using both.  If `local_addr` is pointing to the one that matters at the moment,
as has been suggested [previously](#sending-and-receiving), then only the one
pointed to will matter.  However, there is no harm preparing both to avoid
conditionals based on address family.


### Checkpoint 2

At this point, you can also test your work with
[automated testing](#automated-testing).  Levels 0 through 2 should all work
at this point.

Now would be a good time to save your work, if you haven't already.


## Level 3

For level 3, responses from the server will use any of op-code 1, 2, or 3,
selected at random.  The client should be expected to do everything it did at
levels 0 through 2, but also support op-code 3.  When op-code 3 is provided in
the [directions response](#directions-response), the client should do the
following to prepare the next directions request:

 - Extract the op-param (bytes `n + 2` and `n + 3`) as an `unsigned short` in
   network byte order.  The extracted value will be referred to hereafter as
   `m`.   While `m` takes up two bytes, for consistency with op-params used
   with other op-codes, its value will be only between 1 and 7.
 - Read `m` datagrams from the socket using the _local_ port to which the
   socket is already bound.  Each of these datagrams will come from a
   randomly-selected _remote_ port on the server, so `recvfrom()` must be used
   by the client to read them to determine which port they came from.  You
   should declare a new `struct sockaddr_in` (or `struct sockaddr_in6` for
   IPv6) variable for use with `recvfrom()`, so the remote port with which your
   client had been sending directions requests is not lost; the next directions
   request will be sent to that port, once it is prepared.

   Each of the `m` datagrams received will have 0 length.  However, the contents
   of the datagrams are not what is important; what is important is the remote
   _ports_ from which they originated (i.e., the _remote_ ports populated with
   `recvfrom()`).
 - Sum the values of the remote ports of the `m` datagrams to derive the nonce.
   Remember the following:

   - Each of the ports must be in host order before its value is added.
   - The sum of the ports might exceed the value of the 16 bits associated with
     an `unsigned short` (16 bits), so you will want to keep track of their
     _sum_ with an `unsigned int` (32 bits).
 - Add 1 to the nonce, and prepare the new directions request with that value.
 - Send the directions request with the same local and remort ports with which
   the most recent directions response was received.


### Checkpoint 3

At this point, you can also test your work with
[automated testing](#automated-testing).  Levels 0 through 3 should all work
at this point.

Now would be a good time to save your work, if you haven't already.


## Level 4 (Extra Credit)

For level 4, responses from the server will use any of op-code 1, 2, 3, or 4,
selected at random.  The client should be expected to do everything it did at
levels 0 through 3, but also support op-code 4.  When op-code 4 is provided in
the [directions response](#directions-response), the client should do the
following the prepare the next directions request:

 - Extract the op-param (bytes `n + 2` and `n + 3`) from the response and use
   that value as the remote port for future communications with the server.
   This part is the same as with [op-code 1](#level-1).
 - Prepare a new client socket using whichever address family was _not_ being
   used before.  If IPv4 (`AF_INET`) was in being used, then switch to IPv6
   (`AF_INET6`) and vice-versa.  Follow the instructions from
   [Sending and Receiving](#sending-and-receiving) to prepare your socket,
   using the new address family and new remote port with `getaddrinfo()` (the
   hostname will not change!).
 - Remember to save your remote and local address and port information as shown
   [previously](#sending-and-receiving).
 - Update any variables that are specific to address family.  Depending on your
   implementation, these might include: `addr_fam`, `addr_len`, `remote_addr`,
   and `local_addr`.
 - Close the old socket.  It is no longer needed!


Note that handling a socket that might be one of two different address families
requires a bit of logical complexity.
[However, it has been suggested](#sending-and-receiving) that you maintain two
variables for the current local address (`local_addr_in` and `local_addr_in6`)
and remote address (`remote_addr_in` and `remote_addr_in6`).  If you are using
instances of `struct sockaddr *` to always point to the address structure
associated with the appropriate address family, then this should "just work".


### Checkpoint 4

At this point, you can also test your work with
[automated testing](#automated-testing).  Levels 0 through 4 should all work
at this point.

Now would be a good time to save your work, if you haven't already.


# Helps and Hints

## Message Formatting

When writing to or reading from a socket, a buffer must be specified.  This can
be a pointer to data at any memory location, but the most versatile data
structure is an array of `unsigned char`.

For example, in preparing the initial directions request to the server, a
buffer might be declared like this:

```c
unsigned char buf[64];
```

If that initial message has values 1, 3985678983 (0xed90a287), and 7719
(or, equivalently, 0x1e27) for level, user ID, and seed, respectively, the
values stored in that buffer might look like this, after it is populated:

```c
buf = { 0, 1, 0xed, 0x90, 0xa2, 0x87, 0x1e, 0x27 };
```

Of course, you cannot simply populate the array with the above code because the
values will not be known ahead of time.  You will need to assign them.  So how
do you fit an `unsigned short` (16 bits) into multiple slots of `unsigned
char` (8 bits)?  There are several ways.  Consider the following program:

```c
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#define BUFSIZE 8

int main() {
	unsigned char buf[BUFSIZE];
	unsigned short val = 0xabcd;
	int i = 0;
	bzero(buf, BUFSIZE);

	memcpy(&buf[6], &val, 2);
	for (i = 0; i < BUFSIZE; i++) {
		printf("%x ", buf[i]);
	}
	printf("\n");
}
```

When you compile and run this program, you will see that indexes 6 and 7 of
`buf` contain the value of `val`, exactly as they are stored in memory; that is
what the call to `memcpy()` is doing.  You will *most likely* find them to be
in the *reverse* order from what you might expect.  If so, is because the
architecture that you are using is *little endian*.  This is problematic for
network communications, which expect integers to be in *network* byte order
(i.e., *big endian*).  To remedy this, there are functions provided for you by
the system, including `htons()` and `ntohs()` ("host to network short" and
"network to host short").  See their man pages for more information.  Try
modifying the code above to assign `htons(0xabcd)` to `val` instead, and see
how the output changes.

The example above is specific to storing an `unsigned short` integer value into
an arbitrary memory location (in this case an array of `unsigned char`) in
network byte order.  You will need to use this principle to figure out how to
do similar conversions for other cirumstances, including working with integers
other than `unsigned short` and extracting integers of various lengths from
arrays of `unsigned char`.  Hint: see the man page for `ntohs()` for related
functions.


## Error Codes

Any error codes sent by the server will be one of the following:

 - 129: The message was sent from an unexpected port (i.e., the source port of
   the packet received by the server).
 - 130: The message was sent to the wrong port (i.e., the remote port of the
   packet received by the server).
 - 131: The message had an incorrect length.
 - 132: The value of the nonce was incorrect.
 - 133: After multiple tries, the server was unable to bind properly to the
   address and port that it had attempted.
 - 134: After multiple tries, the server was unable to detect a remote port on
   which the client could bind.
 - 135: A bad level was sent the server on the initial request, or the first
   byte of the initial request was not zero.
 - 136: A bad user id was sent the server on the initial request, such that a
   username could not be found on the system running the server.
 - 137: An unknown error occurred.


## Op-Codes

The op-codes sent by the server will be one of the following:

 - 0: Communicate with the server as you did previously, i.e., don't change the
   remote or local ports, nor the address family.
 - 1: Communicate with the server using a new remote (server-side) port
   designated by the server.
 - 2: Communicate with the server using a new local (client-side) port
   designated by the server.
 - 3: Same as op-code 0, but instead of sending a nonce that is provided by the
   server, derive the nonce by adding the remote ports associated with the `m`
   communications sent by the server.
 - 4: Communicate with the server using a new address family, IPv4 or
   IPv6--whichever is not _currently_ being used.


## UDP Socket Behaviors

For this lab, all communications between client and server are over UDP (type
`SOCK_DGRAM`).  As such, the following are tips for socket creation and
manipulation:

 - Sending every message requires exactly one call to `write()`, `send()`, or
   `sendto()`.  See the man page for `udp`.
 - Receiving every message requires exactly one call to `read()`, `recv()`, or
   `recvfrom()`.  In some cases (e.g., op-code 3) `recvfrom()` _must_ be used.
   See the man page for `udp`.
 - When 0 is returned by a call to `read()` or `recv()` on a socket of type
   `SOCK_DGRAM`, it simply means that there was no data/payload in the datagram
   (i.e., it was an "empty envelope").  See "RETURN VALUE" in the `recv()` man
   page.

   Note that this is different than the behavior associated with a socket of
   type `SOCK_STREAM`, in which if `read()` or `recv()` returns 0, it is a
   signal that `close()` has been called on the remote socket, and the
   connection has been shut down.  With UDP (type `SOCK_DGRAM`), there is no
   connection to be shutdown.
 - Generally, either `connect()` must be used to associate a remote address and
   port with the socket, or `sendto()` must be used when sending messages.
   However, for this lab, it is much easier to just use `sendto()` every time
   over using `connect()` because of all the changing ports.
 - `sendto()` can be used to override the remote address and port associated
   with the socket.  See the man page for `udp`.


## Testing Servers

The following domain names and ports correspond to the servers where the games
might be initiated:

 - canada:32400
 - cambodia:32400
 - belgium:32400
 - australia:32400
 - atlanta:32400
 - houston:32400
 - hongkong:32400
 - lasvegas:32400
 - carolina:32400
 - alaska:32400
 - arizona:32400
 - hawaii:32400

Note that communicating with any server should result the same behavior.
However, to balance the load and to avoid servers that might be down for one
reason or another, we have created the following script, which will show
both a status of servers the *primary* machine that *you* should use:

```
$ ./server_status.py
```


# Automated Testing

For your convenience, a script is also provided for automated testing.  This is
not a replacement for manual testing but can be used as a sanity check.  You
can use it by simply running the following:

```
./driver.py server port [level]
```

Replace `server` and `port` with a server and port from the set of
[servers designated for testing](#testing-servers) (i.e., preferably the one
corresponding to your username).

Specifying `level` is optional.  If specified, then it will test
[all seeds](#evaluation) against a given level.  If not specified, it will test
_all_ levels.


# Evaluation

Your score will be computed out of a maximum of 100 points based on the
following distribution:

 - 25 points for each of 4 levels (0 through 3)
 - For each level, 5 points for each seed:
   - 7719
   - 33833
   - 20468
   - 19789
   - 59455
 - 5 points extra credit for level 4


# Submission

Upload `treasure_hunter.c` to the assignment page on LearningSuite.
