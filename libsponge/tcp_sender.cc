#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>
#include <iostream>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.
template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout(retx_timeout)
    , rto(retx_timeout)
    , _stream(capacity)
    , keep_seg(){}

uint64_t TCPSender::bytes_in_flight() const { return bytes_flight; }

void TCPSender::fill_window() {
    do{
        if (send_fin) return;
        if (wsize < bytes_flight) return;

        TCPSegment seg;

        uint8_t is_syn = 0;
        if (!send_syn) {
            send_syn = true;
            seg.header().syn = true;
            is_syn = 1;
        }

        uint64_t max_bytes_can_send = std::min(TCPConfig::MAX_PAYLOAD_SIZE, wsize - bytes_flight);
        if (wsize > bytes_flight + is_syn) {
            auto s = _stream.read(max_bytes_can_send);
            if (s.size() > 0) {
                seg.payload() = Buffer(std::move(s));
            }
        }

        if (seg.length_in_sequence_space() < wsize - bytes_flight - is_syn && _stream.eof() && !send_fin) {
            seg.header().fin = true;
            send_fin = true;
        }

        if (seg.length_in_sequence_space() == 0) return;
        if (retransmission_timeout == 0) {
            retransmission_timeout = rto;
        }


        seg.header().seqno = wrap(_next_seqno, _isn);

        bytes_flight += seg.length_in_sequence_space();
        _next_seqno += seg.length_in_sequence_space();
        _segments_out.push(seg);
        keep_seg.push_back(seg);
    }while (wsize - bytes_flight > 0 && (!_stream.buffer_empty() || (_stream.eof() && !send_fin)));
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    wsize = window_size == 0 ? 1 : window_size;
    wsize_zero = window_size == 0 ? true : false;
    auto ack = unwrap(ackno, _isn, absolute_ack);
    if (ack > absolute_ack && ack <= _next_seqno) {
        if (window_size > 0) rto = _initial_retransmission_timeout;
        while (!keep_seg.empty()) {
            auto seq = unwrap(keep_seg.front().header().seqno, _isn, absolute_ack);
            if (seq + keep_seg.front().length_in_sequence_space() <= ack) {
                bytes_flight -= keep_seg.front().length_in_sequence_space();
                keep_seg.pop_front();
            } else{
                break;
            }
        }
        if (keep_seg.size() > 0) {
            retransmission_timeout = rto;
        } else {
            retransmission_timeout = 0;
        }
        absolute_ack = ack;
        consecutive_cnt = 0;
    } else if (ack < absolute_ack) {
        //already recv
    } else {

    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (retransmission_timeout != 0 && retransmission_timeout <= ms_since_last_tick) {
        consecutive_cnt++;
        if(!wsize_zero) rto *= 2;
        retransmission_timeout = rto;
        if (keep_seg.size() > 0) {
            _segments_out.push(keep_seg.front());
        } else {
            //throw error
        }
    } else if (retransmission_timeout != 0 && retransmission_timeout > ms_since_last_tick) {
        retransmission_timeout -= ms_since_last_tick;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return consecutive_cnt; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    _segments_out.push(seg);
}
