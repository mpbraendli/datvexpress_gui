#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <sys/types.h>
#include <string>
#include "dvb.h"
#include "uhd.h"

#define MDEBUG(fmt, args...) fprintf (stderr, fmt , ## args)

std::string usrpDevice = "master_clock_rate=32000000"; // full UHD device string

uhd::stream_args_t stream_args("sc16"); //complex shorts
uhd::tx_streamer::sptr uhd_tx_stream;

uhd::usrp::multi_usrp::sptr myUsrp;

int  uhd_init()
{
    MDEBUG("OutputUHD:Creating the usrp device with: %s...\n",
            usrpDevice.c_str());

    myUsrp = uhd::usrp::multi_usrp::make(usrpDevice);

    uhd::set_thread_priority_safe();

    double master_clk_rate = myUsrp->get_master_clock_rate();
    MDEBUG("OutputUHD: master clock rate: %f...\n", master_clk_rate);

    uhd_tx_stream = myUsrp->get_tx_stream(stream_args);

    MDEBUG("OutputUHD:UHD ready.\n");
    return 0;
}

void uhd_set_freq( double freq )
{
    MDEBUG("OutputUHD:Setting freq to %f...\n", freq);
    myUsrp->set_tx_freq(freq);
    double real_freq = myUsrp->get_tx_freq();
    MDEBUG("OutputUHD:Actual frequency: %f\n", real_freq);
}

void uhd_set_level( double level )
{
    MDEBUG("OutputUHD:Setting TX gain to %f...\n", level);
    myUsrp->set_tx_gain(level);
    MDEBUG("OutputUHD:Actual TX Gain: %f ...\n", myUsrp->get_tx_gain());
}

double uhd_set_sr( double samplerate )
{
    MDEBUG("OutputUHD:Setting rate to %f...\n", samplerate);
    myUsrp->set_tx_rate(samplerate);
    MDEBUG("OutputUHD:Actual TX Rate: %f Msps...\n", myUsrp->get_tx_rate());

    return myUsrp->get_tx_rate();
}

double uhd_outstanding_queue_size(void)
{
    return 0.0;
}

int  uhd_send_dvb_buffer( dvb_buffer *b )
{
    int res = -1;

    if (b->type == BUF_TS)
    {
        MDEBUG("OutputUHD: Cannot send BUF_TS to USRP\n");
    }
    else if ( b->type == BUF_SCMPLX )
    {
        uhd::tx_metadata_t md;
        md.start_of_burst = false;
        md.end_of_burst = false;
        md.has_time_spec = false;
        md.time_spec = uhd::time_spec_t(0.0);

        const double timeout = 0.2;

        uchar* in = (uchar*)b->b;
        size_t sizeIn = b->len;

        //MDEBUG("OutputUHD: Sending %zu bytes\n", sizeIn);

        size_t usrp_max_num_samps = uhd_tx_stream->get_max_num_samps();

        size_t num_acc_samps = 0;

        while (num_acc_samps < sizeIn) {
            size_t samps_to_send =
                std::min(sizeIn - num_acc_samps, usrp_max_num_samps);

            size_t num_tx_samps = uhd_tx_stream->send(
                    &in[num_acc_samps],
                    samps_to_send, md, timeout);

            if (num_tx_samps == 0) {
                break;
            }

            num_acc_samps += num_tx_samps;
        }

        dvb_buffer_free(b);

        res = 0;
    }
    else {
        MDEBUG("OutputUHD: Cannot send %d to USRP\n", b->type);
    }

    return res;
}


