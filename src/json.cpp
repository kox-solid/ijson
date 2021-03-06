
#include "json.h"


void json::unescape(Buffer &s, int start) {
    char *p = s.ptr();
    int n = start;
    bool prefix = false;
    for(int i=start;i<s.size();i++) {
        char a = p[i];
        if(prefix) {
            switch(a) {
            case 'b':
                a = 8;
                break;
            case 't':
                a = 9;
                break;
            case 'n':
                a = '\n';
                break;
            case 'f':
                a = '\f';
                break;
            case 'r':
                a  ='\r';
                break;
            case 'u':
                if(i + 4 < s.size()) {
                    Slice hex(&p[i + 1], 4);
                    i += 4;
                    auto code = hex.hextoi();
                    std::cout << code << std::endl;
                    if(code <= 0x7f) a = code;
                    else if(code <= 0x7ff) {
                        p[n++] = 0xc0 | (code >> 6);
                        a = 0x80 | (code & 0x3f);
                    } else {
                        p[n++] = 0xe0 | (code >> 12);
                        p[n++] = 0x80 | ((code >> 6) & 0x3f);
                        a = 0x80 | (code & 0x3f);
                    }
                }

                break;
            // case '"':
            // case '/':
            // case '\\':
            };
            prefix = false;
        } else if(a == '\\') {
            prefix = true;
            continue;
        }
        p[n++] = a;
    }
    s.resize(0, n);
}


void json::escape(Buffer &s, int start) {
    int col = 0;
    for(int i = start;i<s.size();i++) {
        if(s.ptr()[i] == '"') col++;
    }
    if(!col) return;

    s.resize(0, s.size() + col);
    char *ptr = s.ptr();
    for(int i=s.size() -1;i>=start;i--) {
        char a = ptr[i - col];
        ptr[i] = a;
        if(a == '"') {
            i--;
            ptr[i] = '\\';
            col--;
            if(!col) break;
        }
    }
}


bool Json::scan() {
    if(_data.empty()) return false;
    strip();
    if(_status == 0) {
        if(next() != '{') throw error::InvalidData();
        strip();
        if(next() == '}') return false;
        index--;
    } else if(_status == 1) {
        char a = next();
        if(a == '}') return false;
        if(a != ',') throw error::InvalidData();
    }

    strip();
    key = read_string();
    strip();
    if(next() != ':') throw error::InvalidData();
    strip();
    char a = next();
    index--;

    if(a == '"') value = read_string();
    else if(a == '{' || a == '[') value = read_object();
    else value = read_value();

    _status = 1;
    return true;
}


void Json::strip() {
    while(index < _data.size()) {
        char a = _data.ptr()[index];
        if(a == ' ' || a == '\n' || a == '\r' || a == '\t') {
            index++;
            continue;
        }
        return;
    }
    throw error::InvalidData();
}


Slice Json::read_string() {
    if(next() != '"') throw error::InvalidData();
    int start = index;
    const char *ptr = _data.ptr();
    for(;index < _data.size();index++) {
        if(ptr[index] == '"' && ptr[index-1] != '\\') {
            index++;
            return Slice(&ptr[start], index - start - 1);
        }
    }
    throw error::InvalidData();
}


Slice Json::read_value() {
    // 0-9, null, true, false
    int start = index;
    char a;
    bool is_number = true;
    while(true) {
        a = next();
        if(a == ' ' || a == ',' || a == '}' || a == '\n' || a == '\r') break;
        if(a < '0' || a > '9') is_number = false;
        if(!is_number && (index - start > 5)) throw error::InvalidData();
    }
    index--;
    Slice result(&_data.ptr()[start], index - start);
    if(!is_number) {
        if(!(result == "null" || result == "true" || result == "false")) throw error::InvalidData();
    }
    return result;
}


Slice Json::read_object() {
    int lvl = 0;
    int start = index;
    while(true) {
        char a = next();
        if(a == '{' || a == '[') {
            lvl++;
        } else if(a == '}' || a == ']') {
            lvl--;
            if(lvl==0) return Slice(&_data.ptr()[start], index - start);
        } else if(a == '"') {
            index--;
            read_string();
            continue;
        }
    }
}


void Json::decode_value(Buffer &dest) {
    dest.set(value);
    json::unescape(dest);
}
