# 0XATTACK

A collection of beginner-friendly binary exploitation (pwn) challenges designed to introduce players to memory corruption and binary exploitation.

## Challenges

| Challenge | Level | Port | Topic |
|---|---|---:|---|
| **Oopsie** | Beginner | 1338 | Buffer overflow fundamentals |
| **Returnal** | Beginner | 1339 | Return address overwrite |
| **R0P_Me** | Beginner | 1340 | Return-oriented programming (ROP) |

## Requirements

- Docker
- Docker Compose
- Basic knowledge of C and Linux

## Getting Started

Clone the repository:

```bash
git clone https://github.com/iliasse-sudo/Ctf_labs.git
cd Ctf_labs
```

Start the challenges:

```bash
./run.sh
```

Stop the challenges:

```bash
./stop.sh
```

## Connecting

Connect to a challenge using Netcat:

```bash
nc localhost 1338  # Oopsie
nc localhost 1339  # Returnal
nc localhost 1340  # R0P_Me
```

## Disclaimer

For educational purposes and authorized security testing only. Run the challenges in a controlled environment.

---

**0XATTACK** — Learn, exploit, understand.