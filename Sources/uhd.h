#ifndef UHD_H
#define UHD_H
#include "dvb.h"
#include "dvb_types.h"
#include "dvb_gen.h"
#include "dvb_buffer.h"

#include <uhd/utils/thread_priority.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>


// Set up the USRP, return zero on success
int  uhd_init(void);

void uhd_set_freq( double freq );
void uhd_set_level( double level );
double uhd_set_sr( double samplerate );

// Number of outstanding samples left to send on the hardware
double uhd_outstanding_queue_size(void);

// Write data to the device
int  uhd_send_dvb_buffer( dvb_buffer *b );


#endif // UHD_H

