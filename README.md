# mini_serv

*This project has been created as part of the 42 curriculum by pjelinek.*

## Description

`mini_serv` is a minimal chat server written in C. It listens on `127.0.0.1` on a given port and lets multiple clients connect and talk to each other in real time over TCP.

Each client receives an incremental id on connection (the first client gets `0`). The server broadcasts arrivals, departures and messages to all other connected clients:

```
server: client 0 just arrived
client 0: hello
server: client 0 just left
```

A single message can contain several lines. The server splits it on `\n` and prefixes every line with `client <id>: ` before forwarding it.

All I/O is non-blocking and multiplexed through a single `select()` call. There is no forking and no threading. Clients that do not read their messages are not disconnected.

### Error handling

| Situation | Output on stderr | Exit status |
|---|---|---|
| No port argument given | `Wrong number of arguments` | 1 |
| System call fails before the server accepts connections | `Fatal error` | 1 |
| Memory allocation fails | `Fatal error` | 1 |

The program uses only the functions allowed by the subject, contains no `#define` and frees all memory and closes all file descriptors when a client disconnects.

## Instructions

### Compile

```bash
cc -Wall -Wextra -Werror mini_serv.c -o mini_serv
```

### Run

```bash
./mini_serv <port>
```

Example:

```bash
./mini_serv 8081
```

### Test with nc

Open two or more additional terminals and connect a client in each:

```bash
nc 127.0.0.1 8081
```

Everything typed in one terminal appears in all the others, prefixed with the sender's id.

## Resources

### Network Programming

- Beej's Guide to Network Programming: https://beej.us/guide/bgnet/
- `man 2 select`
- `man 2 socket`
- `man 2 bind`
- `man 2 listen`
- `man 2 accept`
- `man 2 recv`
- `man 2 send`

### AI Usage

AI was used to write this README.

All generated content was reviewed before use.