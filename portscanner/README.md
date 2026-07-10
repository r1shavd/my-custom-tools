# Port Scannner

This does what is is named as. Written in C for it's binary speed and also to facilitate my journey into low level programming. This README.md file is generated via AI agents, so beware of any f up (as i may not have curated it better).

---

## Features

*   **Asynchronous Scanning**: Utilizes non-blocking TCP sockets (`SOCK_STREAM`) and `select()` multiplexing to avoid thread-blocking hangs.
*   **Adaptive Threading Engine**: Automatically checks available system resources and scales execution threads up to 4x the available CPU core count for massive speed improvements during range scans.
*   **Smart Status Reporting**: Intelligently toggles result granularity (`SHOW` vs `HIDE`) based on the type of scan to eliminate terminal clutter.
*   **Input Sanitization**: Implements robust boundary check validation using pattern matching to catch malformed IPv4 targets.

---

## Internal Architecture

### Multi-Threading & Core Optimization Math
When running range scans, the engine calculates thread allocation dynamically:

$$\text{Threads} = \begin{cases} \text{Online CPU Cores} \times 4, & \text{if Cores} > 0 \\ 32, & \text{otherwise} \end{cases}$$

A POSIX mutex (`pthread_mutex_t`) guarantees thread safety, ensuring thread workers safely grab the next port in line without resource conflicts or duplicate scans.

### Connection State Matrix
The scanner evaluates the target port behavior over a strict **1-second timeout** window using `getsockopt()` and `SO_ERROR`:

| Network Response / Scenario | Inferred State | Behavior with Single Port Scans (`show_status = 1`) | Behavior with Range Scans (`show_status = 0`) |
| :--- | :--- | :--- | :--- |
| Handshake completes instantly or `so_error == 0` | **OPEN** | Displays explicitly | Displays explicitly |
| Host returns a TCP Reset packet (`ECONNREFUSED`) | **CLOSED** | Displays explicitly | Hidden from terminal |
| Host/Router returns an ICMP failure packet | **FILTERED** | Displays explicitly | Hidden from terminal |
| Select timeout limits hit (No packet response) | **FILTERED** | Displays explicitly | Hidden from terminal |
| Immediate network failure | **UNREACHABLE** | Displays explicitly | Hidden from terminal |

---

## Usage 

### Basic Syntax
```bash
./portscan <ip> <command> [arguments]
```

### Supported Commands
*   `--port <number>`: Scans a single, target port with a full verbose output report.
*   `--port-range <start> <end>`: Scans a wide selection of ports concurrently using thread workers.
*   `--ports`: Used for comma-separated or specific custom port configurations.

### Code Examples

**Scan a single port (Normal mode):**
```bash
./portscan 192.168.1.1 --port 80
```

**Scan a custom range (CurrentHigh-Speed Mode):**
```bash
./portscan 192.168.1.1 --port-range 1 1024
```
