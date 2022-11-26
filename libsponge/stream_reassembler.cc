#include "stream_reassembler.hh"

#include <stdexcept>
#include <algorithm>
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//if merge return true and merge to first arg
//arg 1 should before arg 2
static bool segment_merge(SEGMENT_DATA &first, SEGMENT_DATA &next){
    if (next.first < first.first) {
        throw std::invalid_argument("wrong merge");
    }

    if (next.first > first.first + first.second.size()){
        return false;
    }
    
    if (next.first + next.second.size() <= first.first + first.second.size()){
        return true;
    }

    first.second += next.second.substr(first.first + first.second.size() - next.first);
    return true;
}

StreamReassembler::StreamReassembler(const size_t capacity) : l(), _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if(eof) {
        end_idx.first = true;
        end_idx.second = index + data.size();
    }

    //if data is empty, just check eof
    if(data.size() == 0){
        if (end_idx.first && end_idx.second == wait_index) {
            _output.end_input();
        }
        return;
    }

    //total exceed capacity
    if (index > _output.bytes_read() + _capacity) return;
    //duplicate data in byte stream
    if (index + data.size() < wait_index) return;

    auto used_data = data;
    auto now_index = index;
    if (now_index < wait_index) {
        used_data = used_data.substr(wait_index - index);
        now_index = wait_index;
    }

    if (now_index + used_data.size() > _output.bytes_read() + _capacity) {
        used_data = used_data.substr(0, _output.bytes_read() + _capacity - now_index);
    }

    SEGMENT_DATA seg;
    seg.first = now_index;
    seg.second = used_data;

    if (l.empty()) {
        l.push_back(seg);
    } else {
        auto ret = std::lower_bound(l.begin(), l.end(), seg, [](const SEGMENT_DATA& a, const SEGMENT_DATA& b){
            return a.first < b.first;
        });
        l.insert(ret, seg);
    }

    total_size = 0;
    for (auto it = l.begin(); it != l.end();) {
        if (it == std::prev(l.end())) {
            total_size += it->second.size();
            break;
        }
        if (segment_merge(*it, *std::next(it))) {
            l.erase(std::next(it));
        } else {
            total_size += it->second.size();
            it++;
        }
    }

    while (l.size() > 0 && l.begin()->first == wait_index) {
        _output.write(l.begin()->second);
        wait_index += l.begin()->second.size();
        total_size -= l.begin()->second.size();
        l.erase(l.begin());
    }
    
    if (end_idx.first && end_idx.second == wait_index) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const {
    return total_size;
}

bool StreamReassembler::empty() const { return unassembled_bytes() == 0; }
