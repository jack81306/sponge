#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : cap(capacity), l()
{}

size_t ByteStream::write(const string &data) {
    if (remaining_capacity() == 0) return 0;
    if (data.size() < remaining_capacity()) {
        l.push_back(data);
        total_size += data.size();
        total_write += data.size();
        return data.size();
    }
    // remaining capacity can not put all data
    auto write_bytes = remaining_capacity();
    l.push_back(data.substr(0, write_bytes));
    total_size = cap;
    total_write += write_bytes;
    return write_bytes;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    std::string ret;
    for (auto it = l.begin(); it != l.end() && ret.size() < len; ++it){
        if (ret.size() + it->size() > len){
            ret += it->substr(0, len - ret.size());
        } else {
            ret += (*it);
        }
    }
    return ret;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t pop_cnt = 0;
    while (!l.empty() && pop_cnt < len) {
        if (pop_cnt + l.front().size() > len){
            l.front() = l.front().substr(len - pop_cnt);
            pop_cnt = len;
        } else {
            pop_cnt += l.front().size();
            l.pop_front();
        }
    }
    total_size -= pop_cnt;
    total_read += pop_cnt;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    auto ret = peek_output(len);
    pop_output(len);
    return ret;
}

void ByteStream::end_input() {is_end = true;}

bool ByteStream::input_ended() const { return is_end; }

size_t ByteStream::buffer_size() const { return total_size; }

bool ByteStream::buffer_empty() const { return total_size == 0; }

bool ByteStream::eof() const { return is_end && total_size == 0; }

size_t ByteStream::bytes_written() const { return total_write; }

size_t ByteStream::bytes_read() const { return total_read; }

size_t ByteStream::remaining_capacity() const { return cap - total_size; }
