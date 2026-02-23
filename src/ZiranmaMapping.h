#ifndef ZIRANMA_MAPPING_H
#define ZIRANMA_MAPPING_H

#include <Arduino.h>

struct ZiranmaMapping {
    static const char* getInitial(char c) {
        switch (c) {
            case 'b': return "b";
            case 'p': return "p";
            case 'm': return "m";
            case 'f': return "f";
            case 'd': return "d";
            case 't': return "t";
            case 'n': return "n";
            case 'l': return "l";
            case 'g': return "g";
            case 'k': return "k";
            case 'h': return "h";
            case 'j': return "j";
            case 'q': return "q";
            case 'x': return "x";
            case 'r': return "r";
            case 'z': return "z";
            case 'c': return "c";
            case 's': return "s";
            case 'y': return "y";
            case 'w': return "w";
            case 'v': return "zh";
            case 'i': return "ch";
            case 'u': return "sh";
            default: return nullptr;
        }
    }

    static bool isInitial(char c) {
        switch (c) {
            case 'b': case 'p': case 'm': case 'f':
            case 'd': case 't': case 'n': case 'l':
            case 'g': case 'k': case 'h':
            case 'j': case 'q': case 'x':
            case 'r': case 'z': case 'c': case 's':
            case 'y': case 'w':
            case 'v': case 'i': case 'u':
                return true;
            default: return false;
        }
    }

    static const char* getFinalByInitial(char initial, char finalKey) {
        switch (finalKey) {
            case 'a': return "a";
            case 'e': return "e";
            case 'i': return "i";
            case 'u': return "u";
            
            case 'l': return "ai";
            case 'j': return "an";
            case 'h': return "ang";
            case 'k': return "ao";
            case 'z': return "ei";
            case 'f': return "en";
            case 'g': return "eng";
            case 'b': return "ou";
            case 'c': return "iao";
            case 'n': return "in";
            case 'm': return "ian";
            case 'x': return "ie";
            case 'q': return "iu";
            case 'r': return "uan";
            case 'p': return "un";
            
            case 'w':
                if (initial == 'j' || initial == 'q' || initial == 'x') return "ia";
                if (initial == 'g' || initial == 'k' || initial == 'h' ||
                    initial == 'i' || initial == 'u') return "ua";
                return nullptr;
                
            case 't':
                if (initial == 'j' || initial == 'q' || initial == 'x') return "ve";
                return "ue";
                
            case 'y':
                if (initial == '\0') return "ing";
                return "uai";
                
            case 'o':
                if (initial == 'b' || initial == 'p' || initial == 'm'|| initial == 'w') return "o";
                return "uo";
                
            case 's':
                if (initial == 'j' || initial == 'q' || initial == 'x' || initial == 'y') return "iong";
                return "ong";
                
            case 'd':
                if (initial == 'j' || initial == 'q' || initial == 'x') return "iang";
                if (initial == 'g' || initial == 'k' || initial == 'h') return "uang";
                return nullptr;
                
            case 'v':
                if (initial == '\0') return "v";
                return "ui";
                
            default: return nullptr;
        }
    }

    static const char* getZeroInitial(char c1, char c2) {
        if (c1 == 'a' && c2 == 'a') return "a";
        if (c1 == 'o' && c2 == 'o') return "o";
        if (c1 == 'e' && c2 == 'e') return "e";
        if (c1 == 'i' && c2 == 'i') return "i";
        if (c1 == 'u' && c2 == 'u') return "u";
        if (c1 == 'v' && c2 == 'v') return "v";
        if (c1 == 'a' && c2 == 'o') return "ao";
        if (c1 == 'a' && c2 == 'i') return "ai";
        if (c1 == 'a' && c2 == 'n') return "an";
        if (c1 == 'e' && c2 == 'i') return "ei";
        if (c1 == 'e' && c2 == 'n') return "en";
        if (c1 == 'e' && c2 == 'r') return "er";
        if (c1 == 'o' && c2 == 'u') return "ou";
        if (c1 == 'i' && c2 == 'a') return "ia";
        if (c1 == 'i' && c2 == 'e') return "ie";
        if (c1 == 'i' && c2 == 'u') return "iu";
        if (c1 == 'i' && c2 == 'n') return "in";
        if (c1 == 'u' && c2 == 'a') return "ua";
        if (c1 == 'u' && c2 == 'o') return "uo";
        if (c1 == 'u' && c2 == 'i') return "ui";
        if (c1 == 'u' && c2 == 'n') return "un";
        if (c1 == 'v' && c2 == 'e') return "ve";
        
        if (c1 == 'a' && c2 == 'h') return "ang";
        if (c1 == 'e' && c2 == 'g') return "eng";
        if (c1 == 'y') return "ing";
        if (c1 == 's') return "ong";
        if (c1 == 'd') return "uang";
        if (c1 == 'w') return "ia";
        if (c1 == 't') return "ue";
        
        return nullptr;
    }

    static bool convertDoublePinyin(const char* input, char* output, int maxLen) {
        int inLen = strlen(input);
        int outPos = 0;
        
        for (int i = 0; i < inLen && outPos < maxLen - 10; i += 2) {
            if (i + 1 >= inLen) {
                const char* initial = getInitial(input[i]);
                if (initial) {
                    int len = strlen(initial);
                    if (outPos + len < maxLen) {
                        strcpy(output + outPos, initial);
                        outPos += len;
                    }
                } else {
                    output[outPos++] = input[i];
                }
                break;
            }
            
            char c1 = input[i];
            char c2 = input[i + 1];
            
            if (isInitial(c1)) {
                const char* initial = getInitial(c1);
                const char* final = getFinalByInitial(c1, c2);
                
                if (initial && final) {
                    int initLen = strlen(initial);
                    int finalLen = strlen(final);
                    if (outPos + initLen + finalLen < maxLen) {
                        strcpy(output + outPos, initial);
                        outPos += initLen;
                        strcpy(output + outPos, final);
                        outPos += finalLen;
                    }
                } else if (initial) {
                    int len = strlen(initial);
                    if (outPos + len < maxLen) {
                        strcpy(output + outPos, initial);
                        outPos += len;
                    }
                    if (final) {
                        len = strlen(final);
                        if (outPos + len < maxLen) {
                            strcpy(output + outPos, final);
                            outPos += len;
                        }
                    } else {
                        output[outPos++] = c2;
                    }
                }
            } else {
                const char* zeroInit = getZeroInitial(c1, c2);
                if (zeroInit) {
                    int len = strlen(zeroInit);
                    if (outPos + len < maxLen) {
                        strcpy(output + outPos, zeroInit);
                        outPos += len;
                    }
                } else {
                    const char* final = getFinalByInitial('\0', c2);
                    output[outPos++] = c1;
                    if (final) {
                        int len = strlen(final);
                        if (outPos + len < maxLen) {
                            strcpy(output + outPos, final);
                            outPos += len;
                        }
                    } else {
                        output[outPos++] = c2;
                    }
                }
            }
        }
        
        output[outPos] = '\0';
        return true;
    }
};

#endif
