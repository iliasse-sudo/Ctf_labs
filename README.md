# 0XATTACK — Binary Exploitation Labs

A hands-on collection of beginner-friendly **binary exploitation (pwn) challenges** designed to introduce core concepts in software security through practical exercises.

The project provides isolated challenge environments using Docker, allowing learners to explore vulnerabilities, analyze binaries, and develop exploitation techniques in a controlled setting.

## Challenges

| Challenge | Difficulty | Topic |
|---|---|---|
| **Oopsie** | Beginner | Buffer overflow fundamentals |
| **Returnal** | Beginner | Return address overwriting |
| **R0P_Me** | Beginner | Return-oriented programming (ROP) |

Additional challenges are in development.

## Requirements

- [Docker](https://www.docker.com/)
- Docker Compose
- Basic familiarity with C and Linux

## Getting Started

Clone the repository:

```bash
git clone https://github.com/iliasse-sudo/Ctf_labs.git
cd Ctf_labs
```

Make the scripts executable and start the challenge environments:

```bash
chmod +x run.sh stop.sh
./run.sh
```

To stop the environments:

```bash
./stop.sh
```

## Connecting to Challenges

Once the environments are running, connect using Netcat:

```bash
nc localhost 1338  # Oopsie
nc localhost 1339  # Returnal
nc localhost 1340  # R0P_Me
```

Refer to each challenge's documentation for its objectives and instructions.

## Learning Objectives

- Understand stack memory and function execution.
- Identify common binary vulnerabilities.
- Learn how return addresses can be overwritten.
- Explore basic return-oriented programming techniques.
- Practice analyzing and exploiting vulnerable binaries.

## License

This project is licensed under the [MIT License](LICENSE).

## Disclaimer

This project is intended for educational purposes and authorized security testing only. Run the challenges in controlled environments and do not use the techniques against systems without explicit permission.