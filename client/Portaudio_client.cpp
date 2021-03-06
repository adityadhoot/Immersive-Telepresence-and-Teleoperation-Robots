 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "portaudio.h"
 #include "Socket.hpp"
 #include <iostream>
 #include <string.h>
 
 #define PACKET_BURST_SIZE 256
 
 /* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
 #define SAMPLE_RATE  (44100)
 #define FRAMES_PER_BUFFER (4096)
 #define NUM_CHANNELS    (2)
 #define NUM_SECONDS     (15)
 /* #define DITHER_FLAG     (paDitherOff)  */
 #define DITHER_FLAG     (0) 
 
 /* @todo Underflow and overflow is disabled until we fix priming of blocking write. */
 #define CHECK_OVERFLOW  (0)
 #define CHECK_UNDERFLOW  (0)
 
 
 /* Select sample format. */
 #if 1
 #define PA_SAMPLE_TYPE  paFloat32
 #define SAMPLE_SIZE (4)
 #define SAMPLE_SILENCE  (0.0f)
 #define CLEAR(a) memset( (a), 0, FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE )
 #define PRINTF_S_FORMAT "%.8f"
 #elif 0
 #define PA_SAMPLE_TYPE  paInt16
 #define SAMPLE_SIZE (2)
 #define SAMPLE_SILENCE  (0)
 #define CLEAR(a) memset( (a), 0,  FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE )
 #define PRINTF_S_FORMAT "%d"
 #elif 0
 #define PA_SAMPLE_TYPE  paInt24
 #define SAMPLE_SIZE (3)
 #define SAMPLE_SILENCE  (0)
 #define CLEAR(a) memset( (a), 0,  FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE )
 #define PRINTF_S_FORMAT "%d"
 #elif 0
 #define PA_SAMPLE_TYPE  paInt8
 #define SAMPLE_SIZE (1)
 #define SAMPLE_SILENCE  (0)
 #define CLEAR(a) memset( (a), 0,  FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE )
 #define PRINTF_S_FORMAT "%d"
 #else
 #define PA_SAMPLE_TYPE  paUInt8
 #define SAMPLE_SIZE (1)
 #define SAMPLE_SILENCE  (128)
 #define CLEAR( a ) { \
     int i; \
     for( i=0; i<FRAMES_PER_BUFFER*NUM_CHANNELS; i++ ) \
         ((unsigned char *)a)[i] = (SAMPLE_SILENCE); \
 }
 #define PRINTF_S_FORMAT "%d"
 #endif
 
 
 /*******************************************************************/
 int main(int argc, char** argv);
 int main(int argc, char** argv)
 {
 
     if (argc != 3)
    {
        printf(" usage: %s ip port\n", argv[0]);
        return 0;
    }
    
    Socket socket;
     PaStreamParameters outputParameters;
     PaStream *stream = NULL;
     PaError err;
     char *sampleBlock;
     int i;
     int numBytes;
	 int numDevices;
	 const PaDeviceInfo* deviceInfo;
	 byte ack;
	 
    if (false == socket.open(argv[1], atoi(argv[2]), Socket::Client, Socket::TCP))
    {
        printf("Failed to open!\n");
        return -1;
    }	 
	 
     
     printf("patest_read_write_wire.c\n"); fflush(stdout);
 
     numBytes = FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE;
     printf("FRAMES_PER_BUFFER, %d\n", FRAMES_PER_BUFFER);
     printf("NUM_CHANNELS, %d\n", NUM_CHANNELS);
     printf("SAMPLE_SIZE, %d\n", SAMPLE_SIZE);
     sampleBlock = (char *) malloc( numBytes );
     if( sampleBlock == NULL )
     {
         printf("Could not allocate record array.\n");
         exit(1);
     }
     CLEAR( sampleBlock );
 
     err = Pa_Initialize();
     if( err != paNoError ) goto error;
 
 	numDevices = Pa_GetDeviceCount();
	for (i = 0; i < numDevices; i++)
	{
		deviceInfo = Pa_GetDeviceInfo(i);
		printf("Device number: %d\n", i);
		printf(deviceInfo->name);
		printf("\n");
	}
 
     outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
     printf( "Output device # %d.\n", outputParameters.device );
     printf( "Output LL: %g s\n", Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency );
     printf( "Output HL: %g s\n", Pa_GetDeviceInfo( outputParameters.device )->defaultHighOutputLatency );
     outputParameters.channelCount = NUM_CHANNELS;
     outputParameters.sampleFormat = PA_SAMPLE_TYPE;
     outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultHighInputLatency;
     outputParameters.hostApiSpecificStreamInfo = NULL;
 
     /* -- setup -- */
 
    err = Pa_OpenStream(
               &stream,
               NULL,
               &outputParameters,
               SAMPLE_RATE,
               FRAMES_PER_BUFFER,
               paClipOff,      /* we won't output out of range samples so don't bother clipping them */
               NULL, /* no callback, use blocking API */
               NULL ); /* no callback, so no callback userData */
     if( err != paNoError ) goto error;
 
 	 //getchar();
 
     err = Pa_StartStream(stream);
     if( err != paNoError ) goto error;	 

     printf("Wire on. Interrupt to stop.\n"); fflush(stdout);
     while( 1 )
     {
        if (false == socket.read((byte*) &ack, 1))
            return -3;
        //else
        std::cout << "ack received!\n";       

        if (false == socket.read((byte*) sampleBlock, numBytes))
        {
            printf("Failed to read1!\n");
            return -2;
        }        
        
        err = Pa_WriteStream( stream, sampleBlock, FRAMES_PER_BUFFER );           

        if( err && CHECK_UNDERFLOW )
            goto xrun;
        else
            std::cout << "PaWriteStream No Error\n" << std::endl;
     }
     
    if (false == socket.close())
    {
        printf("failed to close!\n");
        return -3;
    }
     
     err = Pa_StopStream( stream );
     if( err != paNoError ) goto error;
 
     Pa_CloseStream( stream );
 
     free( sampleBlock );
 
     Pa_Terminate();
     return 0;
 
 xrun:
     if( stream ) {
        Pa_AbortStream( stream );
        Pa_CloseStream( stream );
     }
     free( sampleBlock );
     Pa_Terminate();
     if( err & paInputOverflow )
        fprintf( stderr, "Input Overflow.\n" );
     if( err & paOutputUnderflow )
        fprintf( stderr, "Output Underflow.\n" );

	 getchar();
     return -2;
 
 error:
     if( stream ) {
        Pa_AbortStream( stream );
        Pa_CloseStream( stream );
     }
     free( sampleBlock );
     Pa_Terminate();
     fprintf( stderr, "An error occured while using the portaudio stream\n" );
     fprintf( stderr, "Error number: %d\n", err );
     fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
	 getchar();
     return -1;
 }
