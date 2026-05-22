# gethash

A fast Windows command-line utility for generating file hashes using the native Windows Crypto API. It is designed to be simple, fast, and script-friendly.

## Features

- Fast hashing using Windows CryptoAPI
- No external dependencies
- Supports MD5, SHA1, SHA256 (default), SHA512
- Clean CLI output
- Proper error handling
- Works globally via PATH

## Usage
```
gethash <file> <algorithm>
```
If no algorithm is provided, SHA256 is used by default.

Examples:
```
gethash file.txt  
gethash file.txt md5  
gethash file.zip sha512  
```
Output format:
```
sha256: <hash>
```
## Supported Algorithms

md5  
sha1  
sha256  
sha512  

## Installation

Download or build gethash.exe and place it in a permanent folder such as:
```
C:\Tools\gethash\
```
## Add to PATH (Windows)

Press Win + S and search Environment Variables. Open Edit the system environment variables. Click Environment Variables. Under User variables select Path. Click Edit. Click New and add:
```
C:\Tools\gethash\
```
Click OK on all windows and restart CMD or PowerShell.

After that you can run gethash from anywhere.

## Build from Source

Clone the repository:
```
git clone https://github.com/Anonymi69/gethash.git  
```
```
cd gethash  
```
Build using MSVC (Visual Studio Developer Command Prompt):
```
cl /EHsc /O2 gethash.cpp /Fe:gethash.exe  
```
Run:
```
gethash -h  
```
## Verify Installation
```
gethash -h
```
If installed correctly, the usage menu will appear.

## Exit Codes
```
0 = success  
1 = error  
```
## Notes

Default algorithm is SHA256.  
Only the hash is printed for scripting usage.  
Designed for speed, simplicity, and reliability.

