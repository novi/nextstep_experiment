//Written by J. Laroche at the Center for Music Experiment at UCSD, San Diego //California. December 1990.
// File useful to test the stream control capabilities of the driver for 
// writing streams (to the DACS). Currently (V. 2.0 prerelease fushia), 
// aborting the stream wedges more or less the driver.
// Moreover, pause and resume work on any tagged region, whatever its tag is.

#import <sound/sound.h>
#import <sound/sounddriver.h>
#import <mach/mach.h>
#import <mach/mach_error.h>
#import <stdio.h>
#import <libc.h>

static int write_done_flag = 0;
static port_t reply_port;

#define Error(A,B) if((A)) {fprintf(stderr,"%s %s\n",B, SNDSoundError((A)));\
mach_error(B,(A)); }

static void read_completed(void *arg, int tag)
{
    fprintf(stderr,"playing completed message called tag %d\n", tag);
    write_done_flag--;
}

static void read_started(void *arg, int tag)
{
    fprintf(stderr,"Playing Started message received \n");
}

static void read_aborted(void *arg, int tag)
{
    fprintf(stderr,"Playing Aborted message received \n");
}

static void read_paused(void *arg, int tag)
{
    fprintf(stderr,"Playing Paused message received \n");
}

static void read_resumed(void *arg, int tag)
{
    fprintf(stderr,"Playing Resumed message received \n");
}

static void read_overflow(void *arg, int tag)
{
    fprintf(stderr,"Playing underrun message received \n");
}

static snddriver_handlers_t handlers = { 0, 0, 
    read_started,read_completed,read_aborted,read_paused,read_resumed, read_overflow, 0};

static any_t msg_thread_func(any_t arg)
{
    kern_return_t k_err;
    msg_header_t* reply_msg = (msg_header_t *)malloc(MSG_SIZE_MAX);
    while (1) {
        reply_msg->msg_size = MSG_SIZE_MAX;
        reply_msg->msg_local_port = reply_port;
        k_err = msg_receive(reply_msg, RCV_TIMEOUT, 5000);
        printf("msg received\n");
        Error(k_err,"Message  ");
        if(!k_err) snddriver_reply_handler(reply_msg,&handlers);
    }
    return NULL;
}

int main (int argc, char *argv[])
{
    int i;
    static port_t dev_port, owner_port;
    static port_t write_port;
    kern_return_t k_err;
    SNDSoundStruct *sound;
    short *location;
    int location_index = 0;
    int low_water = 48*1024;
    int high_water = 128*1024;    // Choose a higher value for more security
    int DMASIZE = 4096;
    int length,protocol;
    
    int tag_id = 1;
    cthread_t msg_thread;
    
    if(argc == 1) { printf("I need a 16bit 2ch linear sound(.au or .snd) file...\n");
    exit(1);}

    k_err = SNDAcquire(SND_ACCESS_OUT,0,0,0,
            NULL_NEGOTIATION_FUN,0,&dev_port,&owner_port); 
    Error(k_err,"SND acquisition  ");
    
    k_err = port_allocate(task_self(),&reply_port);
    
    msg_thread = cthread_fork(msg_thread_func, NULL);

    k_err = SNDReadSoundfile(argv[1], &sound);
    Error(k_err,argv[1]);

    k_err = SNDGetDataPointer(sound,(char**)&location,&length,&i);
    Error(k_err,"Data Pointer");
    
    printf("samples=%d,width=%d,bytes=%d,vmpagesize=%d\n", length, i, length*i, vm_page_size);

    k_err = snddriver_stream_setup(dev_port, owner_port,
            SNDDRIVER_STREAM_TO_SNDOUT_44,
            DMASIZE, 2, 
            low_water, high_water,
            &protocol, &write_port);
    Error(k_err,"Stream  ");
    
    while (1) {
        int write_samples;
        if (write_done_flag >= 4) {
            usleep(1000);
            continue;
        }
        printf("queue=%d\n", write_done_flag);
        write_done_flag++;
        write_samples = length <= (location_index+DMASIZE*16) ?
        length - location_index - 1 : DMASIZE*16;
        k_err = snddriver_stream_start_writing(write_port, 
            &location[location_index],
            write_samples, tag_id,1,0,0,1,0,0,0,0, reply_port);
        Error(k_err,"Write Command  ");
        location_index += write_samples;
        tag_id++;        
        printf("write done %d samples\n", write_samples);
        if (location_index+1 == length) {
            // end of the file
            while (write_done_flag > 0) {
                usleep(100); // wait for play finish
            }
            exit(0);         
        }
    }
    return 0;
}
