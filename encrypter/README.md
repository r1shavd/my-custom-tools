# Encrypter

A command-line file security utility written in C. It locks and unlocks files using password-based encryption and securely hashes authentication keys using OpenSSL SHA-256 to ensure data integrity. I made it for my personal use, and you may correct and use this as per your need. This is a simple version, for complex i would rather stay with openLUKS.

## Features

* **Dual Operations**: Simple toggles for both encrypting and decrypting data.
* **OpenSSL Integration**: Utilizes industry-standard SHA-256 hashing for verification.
* **Clean UI**: Enhanced with colorful terminal output for warnings, errors, and success updates.
* **Input Validation**: Safeguards against empty passwords or missing argument crashes.

## Dependencies

Before building, ensure you have the OpenSSL development library installed on your system.

* Ubuntu/Debian: `sudo apt install libssl-dev`
* macOS: `brew install openssl`

## Usage

Compile the source files alongside the OpenSSL crypto library:

```bash
gcc main.c encrypt.c decrypt.c -o encrypter -lcrypto
```

### Scan Variations

**1. Encrypt a File**
```bash
./encrypter --file document.txt --encrypt --password MySecurePass123
```

**2. Decrypt a File**
```bash
./encrypter --file document.txt --decrypt --password MySecurePass123
```

**3. Show Help Menu**
```bash
./encrypter --help
```

## Available Options

```text
--file <path>       Specify the targeted file path (Required)
--password <text>   Passphrase for the cryptographic process (Required)
--encrypt           Execute the file locking sequence
--decrypt           Execute the file unlocking sequence
--help              Display the help menu
```
