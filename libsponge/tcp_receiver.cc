#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//syn can not push substring
void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (!isn && !seg.header().syn) {
        return;
    }

    if (seg.header().syn && seg.header().fin) {
        isn = seg.header().seqno;
        fin = seg.payload().size() + 1;
        _reassembler.push_substring(seg.payload().copy(), 0, true);
        return;
    } else if (seg.header().syn){
        isn = seg.header().seqno;
        _reassembler.push_substring(seg.payload().copy(), 0, false);
        return;
    } else if (seg.header().fin) {
        fin = unwrap(seg.header().seqno + seg.payload().size(), *isn, stream_out().bytes_read());
    }
    // -1 means syn
    _reassembler.push_substring(seg.payload().copy(),
        unwrap(seg.header().seqno, *isn, stream_out().bytes_read()) - 1,
        seg.header().fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (isn) {
        if (!fin || stream_out().bytes_written() != (*fin) - 1) {
            return wrap(stream_out().bytes_written() + 1, *isn);
        }
        return wrap(stream_out().bytes_written() + 2, *isn);
    }
    return {};
}

size_t TCPReceiver::window_size() const { 
    return _capacity - stream_out().buffer_size();
}
