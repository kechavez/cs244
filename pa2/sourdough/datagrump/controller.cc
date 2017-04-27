#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug) 
  : debug_( debug ),
  wsize_(1),
  r_wsize(0),
  timeout_ack(0)
{
  cout << "wSIZE IS : " << wsize_ << endl;
}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  unsigned int the_window_size = wsize_;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << the_window_size << endl;
  }

  return the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{
  /* Default: take no action */

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << endl;
  }
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  /* Default: take no action */
  int64_t rtt = timestamp_ack_received - send_timestamp_acked;
	
  // cout << "recv is " << timestamp_ack_received << endl;
  // cout << "send is " << send_timestamp_acked << endl;
  // cout << "rtt is " << rtt << endl;
  if (timestamp_ack_received - timeout_ack > Controller::timeout_ms() 
      && wsize_ >=2 && rtt > Controller::timeout_ms()) {
    timeout_ack = timestamp_ack_received;
    wsize_ /= 2;
    r_wsize = 0;
  }
  else if (timestamp_ack_received - timeout_ack > Controller::timeout_ms() && rtt + 5 > 		   Controller::timeout_ms()) {
    timeout_ack = timestamp_ack_received;
    r_wsize = 0;
    wsize_=(3*wsize_)/4;
    if (wsize_<1) {
	wsize_=1;
    }
  }
  else if (rtt < (Controller::timeout_ms()/2)) {
    r_wsize++;
    if (wsize_ - r_wsize == 0) {
      wsize_ += 4;
      r_wsize = 0;
    }
  }
  else {
    r_wsize++;
    if (wsize_ - r_wsize == 0) {
      wsize_ += 1;
      r_wsize = 0;
    }
    // cout << "controller wsize = " << wsize_ << endl;
  }

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 105; /* timeout of one second */
}

