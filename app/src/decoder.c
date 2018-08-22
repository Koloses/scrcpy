#include "decoder.h"

#include <libavformat/avformat.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_thread.h>
#include <unistd.h>

#include "config.h"
#include "events.h"
#include "frames.h"
#include "lockutil.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bcm_host.h"
#include "ilclient.h"

#define BUFSIZE 0x10000

static int read_packet(void *opaque, uint8_t *buffer) {
    struct decoder *decoder = opaque;
    return net_recv(decoder->video_socket, buffer, BUFSIZE);
}

// set the decoded frame as ready for rendering, and notify
static void push_frame(struct decoder *decoder) {
    SDL_bool previous_frame_consumed = frames_offer_decoded_frame(decoder->frames);
    if (!previous_frame_consumed) {
        // the previous EVENT_NEW_FRAME will consume this frame
        return;
    }
    static SDL_Event new_frame_event = {
        .type = EVENT_NEW_FRAME,
    };
    SDL_PushEvent(&new_frame_event);
}


static void notify_stopped(void) {
    SDL_Event stop_event;
    stop_event.type = EVENT_DECODER_STOPPED;
    SDL_PushEvent(&stop_event);
}

static int run_decoder(void *data) {
    struct decoder *decoder = data;

//-------------------------------------------
// Start of changes - add video_decode component from ilclient openmax
//  For compilation put scrcpy under /opt/vc/src/hello_pi folder and use this settings: -lilclient -L/opt/vc/lib/ -lbrcmGLESv2 -lbrcmEGL -lopenmaxil -lbcm_host -lvcos //  -lvchiq_arm -lpthread -lrt -lm -L/opt/vc/src/hello_pi/libs/ilclient -L/opt/vc/src/hello_pi/libs/vgfont -Wl,--no-whole-archive -rdynamic
//-------------------------------------------


   OMX_VIDEO_PARAM_PORTFORMATTYPE format;
   COMPONENT_T *video_decode = NULL;
   COMPONENT_T *list[5];
   ILCLIENT_T *client;
   FILE *in;
   int status = 0;
   unsigned int data_len = 0;

   memset(list, 0, sizeof(list));

   if((client = ilclient_init()) == NULL)
   {
      fclose(in);
      return -3;
   }

   if(OMX_Init() != OMX_ErrorNone)
   {
      ilclient_destroy(client);
      fclose(in);
      return -4;
   }

   // create video_decode
   if(ilclient_create_component(client, &video_decode, "video_decode", ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS) != 0)
      status = -14;
   list[0] = video_decode;


   memset(&cstate, 0, sizeof(cstate));
   cstate.nSize = sizeof(cstate);
   cstate.nVersion.nVersion = OMX_VERSION;
   cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
   cstate.nWaitMask = 1;

   if(status == 0)
      ilclient_change_component_state(video_decode, OMX_StateIdle);

   memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
   format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
   format.nVersion.nVersion = OMX_VERSION;
   format.nPortIndex = 130;
   format.eCompressionFormat = OMX_VIDEO_CodingAVC;

   if(status == 0 &&
      OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamVideoPortFormat, &format) == OMX_ErrorNone &&
      ilclient_enable_port_buffers(video_decode, 130, NULL, NULL, NULL) == 0)
   {
      OMX_BUFFERHEADERTYPE *buf;
      int port_settings_changed = 0;
      int first_packet = 1;

      ilclient_change_component_state(video_decode, OMX_StateExecuting);

      while((buf = ilclient_get_input_buffer(video_decode, 130, 1)) != NULL)
      {

// feed data and wait until we get port settings changed

         unsigned char *dest = buf->pBuffer;

         data_len += fread(dest, 1, buf->nAllocLen-data_len, decoder->video_socket);
         if(port_settings_changed == 0 &&
            ((data_len > 0 && ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
        if(port_settings_changed == 0 &&
            ((data_len > 0 && ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
             (data_len == 0 && ilclient_wait_for_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
                                                       ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0)))
         {
            port_settings_changed = 1;
         }
         if(!data_len)
            break;

         buf->nFilledLen = data_len;
         data_len = 0;

         buf->nOffset = 0;
         if(first_packet)
         {
            buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
            first_packet = 0;
         }
         else
            buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;

         if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
         {
            status = -6;
            break;
         }
      }

      buf->nFilledLen = 0;
      buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

      if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
         status = -20;

      // need to flush the renderer to allow video_decode to disable its input port

    if (oBufferHeader != NULL)
    {
    {
      OMX_SendCommand(component, OMX_CommandPortDisable, oPortDef.nPortIndex, NULL);
      OMX_FreeBuffer(component, 131, oBufferHeader);
    };
    ErrorCode = OMX_SendCommand(component, OMX_CommandPortEnable,
                                    oPortDef.nPortIndex, NULL);

    PortSettingsChanged = OMX_FALSE;

    GetPortParam();

    ErrorCode = OMX_AllocateBuffer(component, &oBufferHeader,
                                       oPortDef.nPortIndex, NULL,
                                           oPortDef.nBufferSize);

    PortUpdated = OMX_TRUE;


  if (PortUpdated == OMX_TRUE)
  {
    while (outf < inf)
    {
      OMX_FillThisBuffer(component, oBufferHeader);
      if (OutputBufferFilled != OMX_TRUE) OMXWaitForFilled();
      printf("Frame %u OUT.\n", outf);
      outf ++;
      OutputBufferFilled = OMX_FALSE;
    };
    decoder->frames->decoding_frame = outf ;
  };
//------------------------------------------
//  End of changes
//----------------------------------------

        push_frame(decoder);
}
    LOGD("End of frames");

run_end:
    return 0;
}

void decoder_init(struct decoder *decoder, struct frames *frames, socket_t video_socket) {
    decoder->frames = frames;
    decoder->video_socket = video_socket;
}

SDL_bool decoder_start(struct decoder *decoder) {
    LOGD("Starting decoder thread");

    decoder->thread = SDL_CreateThread(run_decoder, "video_decoder", decoder);
    if (!decoder->thread) {
        LOGC("Could not start decoder thread");
        return SDL_FALSE;
    }

    return SDL_TRUE;
}

void decoder_stop(struct decoder *decoder) {
    frames_stop(decoder->frames);
}

void decoder_join(struct decoder *decoder) {
    SDL_WaitThread(decoder->thread, NULL);
}







