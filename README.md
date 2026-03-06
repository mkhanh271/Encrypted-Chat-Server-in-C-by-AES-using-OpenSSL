# Encrypted-Chat-Server-in-C-by-AES-using-OpenSSL

> A secure real-time chat application implemented in C, establishing an **SSL/TLS encrypted connection** between a client and server using **OpenSSL**. Traffic is fully encrypted — verified via Wireshark packet capture showing no plaintext data in transit.

---
## My personal project about encryption theory and packet analysis

## 📋 Table of Contents

- [Overview](#overview)
- [How It Works](#how-it-works)
- [Project Structure](#project-structure)
- [Requirements](#requirements)
- [Setup: Generate Certificate & Key](#setup-generate-certificate--key)
- [Compile & Run](#compile--run)
- [Wireshark Verification](#wireshark-verification)
- [References](#references)

---

## Overview

This project demonstrates a **TLS-encrypted TCP chat** between a server and a client, both written in C using the OpenSSL library. All messages exchanged are encrypted at the transport layer — a Wireshark capture confirms that no plaintext is visible on the wire.

**Key features:**
- TLS handshake using X.509 self-signed certificate
- Bidirectional encrypted messaging (both sides can send and reply)
- Graceful disconnect: either side can type `bye` to terminate the session
- Port: **4443**

---

## How It Works

```
  Client                              Server
    │                                   │
    │──── TCP connect (port 4443) ──────►│
    │                                   │
    │◄─────── TLS Handshake ────────────►│
    │      (cert.pem + key.pem)          │
    │                                   │
    │──── SSL_write(encrypted msg) ─────►│
    │◄─── SSL_write(encrypted reply) ───│
    │              ...                  │
    │──── "bye" ────────────────────────►│
    │              SSL_shutdown          │
    └───────────────────────────────────┘
```

1. Server loads `cert.pem` and `key.pem`, binds to port **4443**, and waits for a connection
2. Client connects and performs the TLS handshake — verifying the server's certificate
3. After handshake, all messages are encrypted using the negotiated cipher (e.g., AES-256-GCM)
4. Either side sends `bye` to cleanly shut down the SSL session

---

## Project Structure

```
.
├── server_SSL.c    # TLS server: accepts connection, handles SSL handshake & messaging
├── client.c        # TLS client: connects to server, encrypts and sends messages
├── cert.pem        # Self-signed X.509 certificate (generated locally)
├── key.pem         # Private key (generated locally — DO NOT commit to public repos)
└── README.md
```

---

## Requirements

- Linux (Ubuntu 20.04+ recommended)
- GCC
- OpenSSL development library

### Install OpenSSL

```bash
sudo apt update
sudo apt install openssl libssl-dev
```

---

## Setup: Generate Certificate & Key

The server requires **two separate files**: `cert.pem` (certificate) and `key.pem` (private key).

Run this command in your project directory:

```bash
openssl req -x509 -nodes -days 365 \
  -newkey rsa:2048 \
  -keyout key.pem \
  -out cert.pem \
  -subj "/C=VN/ST=Hanoi/L=Hanoi/O=Dev/CN=localhost"
```

This generates:
- `cert.pem` — self-signed X.509 certificate (public, loaded by server)
- `key.pem` — RSA private key (secret, never share this)

> If you want a single combined file (legacy style), use `-keyout mycert.pem -out mycert.pem`, but the server code expects them **separate**.

---

## Compile & Run

### Step 1 — Compile

```bash
# Compile server
gcc -Wall -o ssl-server server_SSL.c -lssl -lcrypto

# Compile client
gcc -Wall -o ssl-client client.c -lssl -lcrypto
```

### Step 2 — Run the Server

Open **Terminal 1**:

```bash
./ssl-server
```

Expected output:
```
Server listening on port 4443...
```

### Step 3 — Run the Client

Open **Terminal 2**:

```bash
./ssl-client
```

Expected output:
```
Connected with TLS_AES_256_GCM_SHA384 encryption
Enter Message:
```

### Step 4 — Chat

- Type a message in the **client** terminal → server receives and displays it
- Server types a reply → client receives it
- Either side types `bye` to end the session

---

### Example Session

**Client terminal:**
```
Connected with TLS_AES_256_GCM_SHA384 encryption
Enter Message: Hello from client!
Server: Hello back from server!
Enter Message: bye
Client initiated disconnect.
Client terminated.
```

**Server terminal:**
```
Server listening on port 4443...
Connection from 127.0.0.1:52341
SSL/TLS handshake completed
Client: Hello from client!
Enter Reply: Hello back from server!
Client: bye
Client initiated disconnect.
Server terminated.
```

---

## Wireshark Verification

To confirm all traffic is encrypted:

1. Open **Wireshark** and start capture on `lo` (loopback interface)
2. Apply filter: `tcp.port == 4443`
3. Run the server and client, exchange messages
4. Inspect packets in Wireshark — the payload will show as **TLSv1.3 Application Data** with no readable text

This confirms the messages are fully encrypted on the wire and cannot be intercepted as plaintext.

---

## .gitignore Recommendation

```gitignore
# Private key — never commit this
key.pem

# Compiled binaries
ssl-server
ssl-client
```

---

## References

- [OpenSSL Official Documentation](https://docs.openssl.org/)
- [OpenSSL Ubuntu Guide](https://help.ubuntu.com/community/OpenSSL)
- Original project inspiration: [Encrypted-Chat-Server-IN-C](https://github.com/WarenGonzaga/Encrypted-Chat-Server-IN-C)

---

## License

MIT License — see [LICENSE](LICENSE) for details.
