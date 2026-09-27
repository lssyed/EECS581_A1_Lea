// EECS 561 - Software Engineering II
// Professor Saiedian
// Author: Lea Sulthana Syed
// Student ID: 3229388
// External Sources: Gemini 3.8 Flash consulted on September 27, 2026

#include <iostream>
#include <string>
#include <cctype>

// Checks whether character belongs to candidate IPv4 token
bool isTokenChar(char ch){
    return std::isdigit(static_cast<unsigned char>(ch)) || ch == '.' || ch == ':';
}

// Parses string slice into an integer and validates whether:
// 1. Length isn't empty
// 2. Contains only digits
// 3. No leading zeros
// 4. Value is within [minVal, maxVal]
// COverts digits manually using: value = value * 10 + (ch - '0')
bool parseSegment(const std::string& str, int start, int end, int maxVal, int& outVal) {
    if (start >= end) {
        return false; // Empty slice is invalid
    }

    // Checks for leading zero, if length > 1 and starts with '0' -> reject
    if ((end - start) > 1 && str[start] == '0') {
        return false;
    }

    int value = 0;
    for (int i = start; i < end; ++i) {
        char ch = str[i];
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }

        int digit = ch - '0';

        if (value > (maxVal - digit) / 10) {
            return false;
        }

        value = value * 10 + digit;
    }

    if (value < 0 || value > maxVal) {
        return false;
    }

    outVal = value;
    return true;
}

// Verifies and extracts IPv4 and optional port from single candidate token
bool validateCandidate(const std::string& token, unsigned long& outAddress, int& outPort) {
    int len = static_cast<int>(token.length());
    if (len == 0) return false;

    // Rejects token starting/ending with seperators
    if (token[0] == '.' || token[0] == ':' || token[len - 1] == '.' || token[len - 1] == ':') {
        return false;
    }

    // Locates optional port delimiter ':'
    int colonPos = -1;
    for (int i = 0; i < len; ++i) {
        if (token[i] == ':') {
            if (colonPos != -1) {
                return false; // Having more than one colon is invalid
            }
            colonPos = i;
        }
    }

    // Determines boundaries for IP octets vs port
    int ipEnd = (colonPos != -1) ? colonPos : len;

    // Validates port if its present
    int portValue = -1;
    if (colonPos != -1) {
        int portStart = colonPos + 1;
        int portLen = len - portStart;
        if (portLen < 1 || portLen > 5) {
            return false; // Port must be 1-5 digits
        }
        if (!parseSegment(token, portStart, len, 65535, portValue)) {
            return false;
        }
    }

    // COunts dots and locates their indices within IP
    int dotCount = 0;
    int dotIndices[3];
    for (int i = 0; i < ipEnd; ++i) {
        if (token[i] == '.') {
            if (dotCount >= 3) {
                return false; // Havign more than three dots means that there are more than 4 octets
            }
            dotIndices[dotCount] = i;
            dotCount++;
        }
    }

    if (dotCount != 3) {
        return false; // IPv4 address must have only three dots
    }

    // Slicing for each of the 4 octets: 
    // Octet 0: [0, dotIndices[0])
    // Octet 1: [dotIndices[0] + 1, dotIndices[1])
    // Octet 2: [dotIndices[1] + 1, dotIndicies[2])
    // Octet 3: [dotIndices[2] + 1, ipEnd)
    int octetRanges[4][2] = {
        {0, dotIndices[0]},
        {dotIndices[0] + 1, dotIndices[1]},
        {dotIndices[1] + 1, dotIndices[2]},
        {dotIndices[2] + 1, ipEnd}
    };

    unsigned long ipAccumulator = 0;

    for (int i = 0; i < 4; ++i) {
        int start = octetRanges[i][0];
        int end = octetRanges[i][1];
        int octetLen = end - start;

        // Each octet must be 1-3 digits long
        if (octetLen < 1 || octetLen > 3) {
            return false;
        }

        int octetVal = 0;
        if (!parseSegment(token, start, end, 255, octetVal)) {
            return false;
        }

        // Packs octet into 32-bit unsigned long using arithmetic shifts
        // Octet 0 shifts by 24-bits, Octet 1 shifts by 16-bits, Octet shifts by 8-bits, Octet 3 shifts by 0-bits
        ipAccumulator = (ipAccumulator << 8) | static_cast<unsigned long>(octetVal);
    }

    outAddress = ipAccumulator;
    outPort = portValue;
    return true;
}

// Scans entire string for candidate tokens and extracts first valid IPv4 address
bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort) {
    int strLen = static_cast<int>(str.length());
    int i = 0;

    while (i < strLen) {
        // SKips leading garbage (non-token characters)
        while (i < strLen && !isTokenChar(str[i])) {
            ++i;
        }

        if (i >= strLen) break;

        // FOund from the start of candidate token
        int start = i;
        while (i < strLen && isTokenChar(str[i])) {
            ++i;
        }
        int end = i;

        // Extracts the slice from start to end
        std::string candidate = str.substr(start, end - start);
        
        // Attempts validation
        if (validateCandidate(candidate, outAddress, outPort)) {
            return true;
        }
    }

    // Resets to default after failure
    outAddress = 0 ;
    outPort = -1;
    return false;
}

int main() {
    std::string line;

    while (true) {
        std::cout << "Enter a string (or 'END' to quit): ";
        if (!std::getline(std::cin, line)) {
            break;
        }

        if (line == "END") {
            std::cout << "Program terminated." << std::endl;
            break;
        }

        unsigned long address = 0;
        int port = -1;

        if (extractIPv4(line, address, port)) {
            // Unpacks 32-bit integer back to dotted-decimal for printing
            unsigned int a = (address >> 24) & 0xFF;
            unsigned int b = (address >> 16) & 0xFF;
            unsigned int c = (address >> 8) & 0xFF;
            unsigned int d = address & 0xFF;

            std::cout << "Extracted IPv4 address: " << a << "." << b << "." << c << "." << d << " (decimal value: " << address << ", port: ";

            if (port == -1) {
                std::cout << "none";
            } else {
                std::cout << port;
            }
            std::cout << ")" << std::endl;
        } else {
            std::cout << "Invalid input: no valid IPv4 address found" << std::endl;
        }
    }

    return 0;
}