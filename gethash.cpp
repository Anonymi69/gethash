#include <windows.h>
#include <wincrypt.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

#pragma comment(lib, "advapi32.lib")

struct AlgorithmInfo {
    std::string name;
    ALG_ID algId;
};

const AlgorithmInfo algorithms[] = {
    {"md5", CALG_MD5},
    {"sha1", CALG_SHA1},
    {"sha256", CALG_SHA_256},
    {"sha512", CALG_SHA_512}
};

void print_usage() {
    std::cout <<
R"(gethash - Fast file hashing utility

USAGE:
  gethash <file>
  gethash <file> <algorithm>
  gethash -h

ARGUMENTS:
  <file>         Target file path

  <algorithm>    Hash algorithm to use
                 Default: sha256

SUPPORTED ALGORITHMS:
  md5
  sha1
  sha256
  sha512

EXAMPLES:
  gethash file.txt
  gethash archive.zip sha256
  gethash movie.mkv sha512
  gethash setup.exe md5

OUTPUT:
  Prints the hash value to stdout.

EXIT CODES:
  0 = Success
  1 = Error
)" << std::endl;
}

const AlgorithmInfo* find_algorithm(std::string algo) {
    std::transform(algo.begin(), algo.end(), algo.begin(), ::tolower);

    for (const auto& a : algorithms) {
        if (a.name == algo)
            return &a;
    }

    return nullptr;
}

std::string get_windows_error() {
    DWORD errorMessageID = GetLastError();

    if (errorMessageID == 0)
        return "Unknown error";

    LPSTR messageBuffer = nullptr;

    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorMessageID,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer,
        0,
        NULL
    );

    std::string message(messageBuffer, size);

    LocalFree(messageBuffer);

    return message;
}

std::string hash_file(const std::string& path, ALG_ID algId) {
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;

    constexpr DWORD BUFFER_SIZE = 8192;
    BYTE buffer[BUFFER_SIZE];

    std::ifstream file(path, std::ios::binary);

    if (!file)
        throw std::runtime_error("Cannot open file.");

    if (!CryptAcquireContext(
            &hProv,
            nullptr,
            nullptr,
            PROV_RSA_AES,
            CRYPT_VERIFYCONTEXT)) {

        throw std::runtime_error(
            "CryptAcquireContext failed: " + get_windows_error()
        );
    }

    if (!CryptCreateHash(hProv, algId, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);

        throw std::runtime_error(
            "CryptCreateHash failed: " + get_windows_error()
        );
    }

    while (file.good()) {
        file.read(reinterpret_cast<char*>(buffer), BUFFER_SIZE);

        std::streamsize bytesRead = file.gcount();

        if (bytesRead > 0) {
            if (!CryptHashData(
                    hHash,
                    buffer,
                    static_cast<DWORD>(bytesRead),
                    0)) {

                CryptDestroyHash(hHash);
                CryptReleaseContext(hProv, 0);

                throw std::runtime_error(
                    "CryptHashData failed: " + get_windows_error()
                );
            }
        }
    }

    DWORD hashSize = 0;
    DWORD sizeSize = sizeof(DWORD);

    if (!CryptGetHashParam(
            hHash,
            HP_HASHSIZE,
            reinterpret_cast<BYTE*>(&hashSize),
            &sizeSize,
            0)) {

        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);

        throw std::runtime_error(
            "CryptGetHashParam failed: " + get_windows_error()
        );
    }

    std::vector<BYTE> hash(hashSize);

    if (!CryptGetHashParam(
            hHash,
            HP_HASHVAL,
            hash.data(),
            &hashSize,
            0)) {

        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);

        throw std::runtime_error(
            "Failed to retrieve hash value: " + get_windows_error()
        );
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    static const char hex[] = "0123456789abcdef";

    std::string result;
    result.reserve(hash.size() * 2);

    for (BYTE b : hash) {
        result.push_back(hex[b >> 4]);
        result.push_back(hex[b & 0x0F]);
    }

    return result;
}

int main(int argc, char* argv[]) {
    try {

        if (argc == 1) {
            print_usage();
            return 0;
        }

        std::string firstArg = argv[1];

        if (firstArg == "-h") {
            print_usage();
            return 0;
        }

        if (argc > 3) {
            std::cerr << "[ERROR] Too many arguments.\n\n";
            print_usage();
            return 1;
        }

        std::string filePath = argv[1];
        std::string algoName = "sha256";

        if (argc == 3)
            algoName = argv[2];

        const AlgorithmInfo* algo = find_algorithm(algoName);

        if (!algo) {
            std::cerr
                << "[ERROR] Unsupported algorithm: "
                << algoName
                << "\n\n";

            print_usage();
            return 1;
        }

        DWORD attrs = GetFileAttributesA(filePath.c_str());

        if (attrs == INVALID_FILE_ATTRIBUTES) {
            std::cerr
                << "[ERROR] File does not exist:\n"
                << filePath
                << "\n";
            return 1;
        }

        if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
            std::cerr
                << "[ERROR] Path is a directory, not a file:\n"
                << filePath
                << "\n";
            return 1;
        }

        std::string hash = hash_file(filePath, algo->algId);

        std::cout
            << algo->name
            << ": "
            << hash
            << std::endl;

        return 0;
    }

    catch (const std::exception& e) {
        std::cerr
            << "[FATAL ERROR] "
            << e.what()
            << "\n";

        return 1;
    }

    catch (...) {
        std::cerr
            << "[FATAL ERROR] Unknown exception occurred.\n";

        return 1;
    }
}
