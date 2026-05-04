# Secure Multi-Client File Transfer System 🔐📡

## 📌 Project Overview

This project implements a secure, reliable, and efficient file transfer system using low-level socket programming in C. It ensures data integrity and confidentiality using SSL/TLS and reliability using Selective Repeat ARQ.

---

## 🎯 Objective

* Secure communication using SSL/TLS
* Reliable data transfer with Selective Repeat ARQ
* Support multiple concurrent clients
* Handle packet loss and retransmissions
* Measure performance metrics

---

## 🏗️ System Architecture

```
Multiple Clients (Senders)
        ↓
   TCP + SSL/TLS
        ↓
 Multi-threaded Server (Receiver)
```

---

## 🔁 Communication Flow

1. Client connects using TCP + SSL
2. Sends file metadata (name & size)
3. File split into packets
4. Packets sent using sliding window
5. Receiver buffers packets
6. ACKs sent for received packets
7. Lost packets retransmitted

---

## 📡 Protocol Design

| Field  | Description            |
| ------ | ---------------------- |
| seq_no | Packet sequence number |
| size   | Data size              |
| data   | File content           |
| ack_no | Acknowledgment number  |

---

## ⚙️ Core Implementation

### 🔌 Socket Programming

* `socket()` → create socket
* `bind()` → bind server
* `listen()` → wait for clients
* `accept()` → accept connections
* `connect()` → client connection

### 🔐 SSL/TLS Security

* `SSL_CTX_new()` → create context
* `SSL_connect()` → client handshake
* `SSL_accept()` → server handshake
* `SSL_write()` / `SSL_read()` → secure communication

---

## 🚀 Features

### 👥 Multi-Client Support

* Implemented using `pthread`
* Handles multiple clients concurrently

### 🔄 Selective Repeat ARQ

* Sliding window protocol
* Out-of-order packet handling
* Individual ACKs

### 📉 Packet Loss Simulation

```c
if (seq_no % 3 == 0)
```

### 🔁 Retransmission Mechanism

* Timeout-based retransmission
* Only lost packets are resent

### 📂 File Transfer

* Supports all file types
* Binary mode (`rb`, `wb`)

### 🔐 Encryption

* SSL/TLS encryption
* Additional XOR encryption layer

---

## 📊 Performance Evaluation

### 📈 Metrics

* Throughput (KB/s)
* Latency
* Packet loss rate
* Retransmission count

### 🧪 Observations

* Reliable transfer even with packet loss
* Slight throughput drop under heavy retransmissions
* Multi-client improves scalability

---

## 🔧 Optimizations & Fixes

* Fixed retransmission bugs
* Per-client packet tracking
* `usleep()` to reduce CPU usage
* Used `select()` for non-blocking I/O
* Improved stability and removed deadlocks

---

## 🛡 Error Handling

* File not found
* SSL handshake failure
* Client disconnection
* Invalid inputs

---

## ▶️ How to Run

### ⚙️ Compile

```
gcc receiver.c -o receiver -lssl -lcrypto -lpthread
gcc sender.c -o sender -lssl -lcrypto
```

### ▶️ Run

```
./receiver
./sender
```

---

## 🎯 Demo Outcome

* Secure file transfer achieved
* Multiple clients supported
* Packet loss handled correctly
* Reliable retransmission

---

## 🧠 Concepts Used

* Socket Programming
* Multi-threading
* SSL/TLS Security
* Selective Repeat ARQ
* Networking Protocol Design

---
## 👨‍💻 Contributors

This was a group project.

* Tushar Kulkarni

---

## 📝 Conclusion

This project demonstrates a complete system for secure and reliable file transfer using low-level networking concepts. It highlights scalability, efficiency, and real-world applicability.
