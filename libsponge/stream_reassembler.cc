#include "stream_reassembler.hh"
#include <stack>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    size_t unused_bytes;
    if ((unused_bytes = _capacity - _output.buffer_size() - _unassembled_size) <= 0) {
        // if there are unassembled bytes, try assemble
        if (_unassembled_size != 0)
            try_assemble();
        return;
    }

    insert_unassembled(std::make_pair(index, data));
    try_assemble();

    if (eof)  input_ended = true;
    if (!unassembled_bytes() && input_ended)  _output.end_input();
}

//! \details check the first substring in the unassembled substrings
substr_t StreamReassembler::top_unassembled() {
    std::map<size_t, std::string>::iterator it = _unassembled.begin();
    return (*it);
}

//! \details remove the first substring in the unassembled substrings
void StreamReassembler::pop_unassembled() {
    std::map<size_t, std::string>::iterator it = _unassembled.begin();
    _unassembled.erase(it);
}

//! \details put substring into underlying data structures which holds
//! unassembled substrings. If the to-be inserted substring is "encapsulated"
//! in an already inserted substring, the insertion is aborted. Remove
//! the already inserted substring if the opposite happens
void StreamReassembler::insert_unassembled(substr_t p) {
    std::map<size_t, std::string>::iterator it = _unassembled.begin();
    std::stack<size_t> removal_list;
    size_t p_last = p.first + p.second.length();
    size_t it_last;

    for (; it != _unassembled.end(); it++) {
        it_last = it->first + it->second.length();
        if (p.first >= it->first && p_last < it_last) return;
        if (it->first >= p.first && it_last < p_last) {
            _unassembled_size -= it->second.length();
            removal_list.push(it->first);
        }
    }
    // removal_list contains all redundant previously inserted substring
    while (!removal_list.empty()) {
        size_t i = removal_list.top();
        _unassembled.erase(_unassembled.find(i));
        removal_list.pop();
    }
    _unassembled.insert(p);
    _unassembled_size += p.second.length();
}

//! \details try to assemble the unassembled substring to ByteStream
void StreamReassembler::try_assemble() {
    substr_t p;
    std::string tobe_written;
    size_t bytes_written;
    while (!empty()) {
        p = top_unassembled();
        tobe_written = "";
        bytes_written = 0;

        // if the index of the unassembled substring is less than expected
        if (p.first < _ackno) {
            if (p.second.length() > (_ackno - p.first)) {
                tobe_written = p.second.substr(_ackno - p.first, p.second.length());
                bytes_written = _output.write(tobe_written);
                _unassembled_size -= (bytes_written + _ackno - p.first);
                _ackno += bytes_written;
            }
            else {
                // discard the unassembled substring
                _unassembled_size -= p.second.length();
            }
        }
        // if the index of the unassembled substring matchs expectation
        else if (p.first == _ackno) {
            tobe_written = p.second;
            bytes_written = _output.write(tobe_written);
            _unassembled_size -= bytes_written;
            _ackno += bytes_written;
        }
        else
            break;

        pop_unassembled();
        if (bytes_written != tobe_written.length()) {
            tobe_written = tobe_written.substr(bytes_written, tobe_written.length());
            insert_unassembled(std::make_pair(_ackno - 1, tobe_written));
        }
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_size; }

bool StreamReassembler::empty() const { return !_unassembled.size(); }
