// getcam.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <string>
#include "DS_AudioVideoDevices.h"
#include <windows.h>
#include<queue>


using namespace std;
extern "C"

{
# include <sdl\SDL.h>
#include "libavdevice/avdevice.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"


}

#define FF_REFRESH_EVENT (SDL_USEREVENT)
#define SDL_EVENT_INTERFACE_FRESH (SDL_USEREVENT+1)
#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)


int thread_exit = 0;
int screenwith = 0;
int screenhight = 0;
int quit = 0;

SDL_Window *window = NULL;
SDL_Texture *texture;
SDL_Renderer *render = NULL;
AVCodecContext *pCodecCtx;

AVPacket *packet = av_packet_alloc();


AVFrame * frame = av_frame_alloc();;

AVFrame* frame_yuv = av_frame_alloc();

//int event_handler(void * data) {
//	while (!thread_exit) {
//		SDL_Event event;
//		event.type = SDL_EVENT_INTERFACE_FRESH;
//		SDL_PushEvent(&event);
//		/*SDL_Log("this is test11");*/
//		Sleep(40);
//	/*	SDL_Delay(40);*/
//	
//	}
//
//	return 0;
//
//}

typedef struct PacketQueue {
	AVPacketList   *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	SDL_mutex *mutex;
	SDL_cond *cond;
} PacketQueue;

void packet_queue_init(PacketQueue *q) {
	memset(q, 0, sizeof(PacketQueue));
	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
}




typedef struct ThreadForRender {
	AVFormatContext* ctx;
	AVCodecContext* pCodecCtx;
	SDL_Renderer *render;
	SDL_Texture *texture;
	PacketQueue* vq;
}ThreadForRender;






int packet_queue_put(PacketQueue *q, AVPacket *pkt) {

	AVPacketList *pkt1;




//#pragma warning(suppress : 4996)
//	if (av_dup_packet(pkt) < 0) {
//		return -1;
//	}
	pkt1 =(AVPacketList*) av_malloc(sizeof(AVPacketList));
	if (!pkt1)
		return -1;

	av_packet_ref(&pkt1->pkt, pkt);
	pkt1->pkt = *pkt;
	pkt1->next = NULL;

	

	SDL_LockMutex(q->mutex);

	if (!q->last_pkt)
		q->first_pkt = pkt1;
	else
		q->last_pkt->next = pkt1;
	q->last_pkt = pkt1;
	q->nb_packets++;
	q->size += pkt1->pkt.size;
	SDL_CondSignal(q->cond);


	SDL_UnlockMutex(q->mutex);
	//av_packet_unref(pkt);
	return 0;
}

int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
	AVPacketList *pkt1;
	int ret;

	SDL_LockMutex(q->mutex);

	for (;;) {

		if (quit) {
			ret = -1;
			break;
		}

		pkt1 = q->first_pkt;
		if (pkt1) {
			q->first_pkt = pkt1->next;
			if (!q->first_pkt)
				q->last_pkt = NULL;
			q->nb_packets--;
			q->size -= pkt1->pkt.size;
			*pkt = pkt1->pkt;
			av_free(pkt1);
			ret = 1;
			break;
		}
		else if (!block) {
			ret = 0;
			break;
		}
		else {
			SDL_CondWait(q->cond, q->mutex);
		}
	}
	SDL_UnlockMutex(q->mutex);
	return ret;
}















int packet_to_render(void * data){

	SDL_Log("into packet_to_render");
	ThreadForRender *pam = (ThreadForRender *)data;
	AVFormatContext* ctx = pam->ctx;
	AVCodecContext* pCodecCtx = pam->pCodecCtx;
	//SDL_Renderer* render = pam->render;
	//SDL_Texture* texture = pam->texture;
	PacketQueue* vq = pam->vq;
	/*AVFormatContext* ctx, AVCodecContext* pCodecCtx, SDL_Renderer *render, SDL_Texture *texture*/

	struct SwsContext *sws_ctx = NULL;
	sws_ctx = sws_getContext(pCodecCtx->width,
		pCodecCtx->height,
		pCodecCtx->pix_fmt,
		pCodecCtx->width,
		pCodecCtx->height,
		AV_PIX_FMT_YUV420P,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
	);

	
	int bufSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
	uint8_t* buffer = (uint8_t *)av_malloc(bufSize);
	av_image_fill_arrays(frame_yuv->data, frame_yuv->linesize, (const uint8_t *)buffer,
		AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
	frame_yuv->format = AV_PIX_FMT_YUV420P;

	SDL_Rect        rect;

	for (;;) {
	
			
		if (av_read_frame(ctx, packet) >= 0) {
		if (packet->stream_index == 0) {
			/*SDL_Log("av_read_frame frame hahaha,size is %d" , packet->size);*/
			packet_queue_put(vq, packet);		



		/*	av_packet_unref(packet);*/
		}
	
		/*av_packet_unref(packet);*/
		SDL_Delay(40);
	}

	}
}


static Uint32 sdl_refresh_timer_cb(Uint32 interval, void *opaque) {
	SDL_Event event;
	event.type = FF_REFRESH_EVENT;
	event.user.data1 = opaque;
	SDL_PushEvent(&event);
	return 0; /* 0 means stop timer */
}




void schedule_refresh(PacketQueue *queue ,int delay) {
	SDL_AddTimer(delay, sdl_refresh_timer_cb, queue);

}


void video_refresh_timer(void *userdata) {
	PacketQueue *queue = (PacketQueue *) userdata;
	schedule_refresh(queue, 40);
	//SDL_Log("into video refresh timer");	

	AVPacket * packet;
	packet = av_packet_alloc();
	packet_queue_get(queue, packet,1);
	//SDL_Log("into video refresh timer %d ", packet->size);
	//return;
	//AVPacket *packet;
	//packet = av_packet_alloc();

	AVFrame * frame;
	frame = av_frame_alloc();

	struct SwsContext *sws_ctx = NULL;
	sws_ctx = sws_getContext(pCodecCtx->width,
		pCodecCtx->height,
		pCodecCtx->pix_fmt,
		pCodecCtx->width,
		pCodecCtx->height,
		AV_PIX_FMT_YUV420P,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
	);

	AVFrame* frame_yuv = av_frame_alloc();
	int bufSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
	uint8_t* buffer = (uint8_t *)av_malloc(bufSize);
	av_image_fill_arrays(frame_yuv->data, frame_yuv->linesize, (const uint8_t *)buffer,
		AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
	frame_yuv->format = AV_PIX_FMT_YUV420P;

	SDL_Rect        rect;





	int ret = avcodec_send_packet(pCodecCtx, packet);
			//if (ret!=NULL) {
			//	SDL_Log("Cannot decoder packet. ret is %d\n",ret);
			//	return ;
			//}

			if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
				char err_msg[1024] = { 0 };
				av_strerror(ret, err_msg, sizeof(err_msg));
				std::cout << "avcodec_send_packet: " << ret << "错误原因是:"<<err_msg<<std::endl;
				return ;
			}

			if (avcodec_receive_frame(pCodecCtx, frame) >= 0) {

				sws_scale(sws_ctx, (uint8_t const * const *)frame->data,
					frame->linesize, 0, pCodecCtx->height,
					frame_yuv->data,
					frame_yuv->linesize);

				if (frame_yuv->format == AV_PIX_FMT_YUV420P) {
					/*	SDL_Log("success get frame  yuv");*/
					SDL_UpdateYUVTexture(texture, NULL,
						frame_yuv->data[0], frame_yuv->linesize[0],
						frame_yuv->data[1], frame_yuv->linesize[1],
						frame_yuv->data[2], frame_yuv->linesize[2]
					);

					// Set Size of Window
					rect.x = 0;
					rect.y = 0;
					//rect.w = screenwith;
					//rect.h = screenhight;
					rect.w = screenwith;
					rect.h = screenhight;

					SDL_RenderClear(render);
					SDL_RenderCopy(render, texture, NULL, &rect);
					SDL_RenderPresent(render);
				}

			}
	/*		av_packet_unref(packet);
			av_frame_unref(frame);*/
	/*		av_frame_unref(frame_yuv);*/




}






int listDevicesds(void) {
	AVFormatContext *pFmtCtx = avformat_alloc_context();
	AVInputFormat *fmt = av_find_input_format("dshow");
	AVDeviceInfoList* devlist = nullptr;
	int rets, mWidth, mHeight;
	AVPacket packet;
	static AVCodec *videoCodec = NULL;

	AVDictionary* options = NULL;
	av_dict_set(&options, "list_devices", "true", 0);
	avformat_open_input(&pFmtCtx, "video=dummy", fmt, &options);




	if (fmt == NULL) {
		cout << "faild fmt open" << endl;
		return -1;
	}



	AVFormatContext* ctx = avformat_alloc_context();
	/*AVFormatContext* ctx = NULL;*/



	int ret = avformat_open_input(&ctx, "video=XiaoMi USB 2.0 Webcam", fmt, nullptr);
	if (ret != 0) {
		fprintf(stderr, "fail to open input stream: %d\n", ret);
		return -1;
	}


	ret = avformat_find_stream_info(ctx, nullptr);
	if (ret < 0) {
		fprintf(stderr, "fail to get stream information: %d\n", ret);
		return -1;
	}
	else {
		cout << "success open stream" << endl;
	}
	mWidth = ctx->streams[0]->codecpar->width;
	mHeight = ctx->streams[0]->codecpar->height;
	/*cout << "codec is :"<<ctx->streams[0]->codec->codec->name << endl;*/
	cout << "width is :" << mWidth << endl;
	cout << "high is :" << mHeight << endl;

	videoCodec = avcodec_find_decoder(ctx->streams[0]->codecpar->codec_id);

	cout << "codec is :" << videoCodec->name << endl;
	//av_read_frame(ctx, pcaket);
	//std::cout << "packet size:" << (pcaket->size) << std::endl;

	while (av_read_frame(ctx, &packet) >= 0) {







		/*	cout << "packet stream is" << packet.stream_index << endl;*/


		std::cout << "packet size:" << (packet.size) << std::endl;

		av_packet_unref(&packet);
	}

	avformat_close_input(&ctx);

	return 1;
}


int opencam()
{	   	 
	
	SDL_Init(SDL_INIT_EVERYTHING);
	
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);

	

	AVFormatContext *pFmtCtx = avformat_alloc_context();

	//AVInputFormat *fmt = av_find_input_format("vfwcap");
	AVInputFormat *fmt = av_find_input_format("dshow");


	AVDictionary* options = NULL;
	av_dict_set(&options, "list_devices", "true", 0);
	avformat_open_input(&pFmtCtx, "video=dummy", fmt, &options);


	AVDeviceInfoList* devlist = nullptr;
	int rets, w_width, w_height;
	
	static AVCodec *pCodec = NULL;

	if (fmt == NULL) {
		cout << "faild fmt open" << endl;
		return -1;
	}



	AVFormatContext* ctx = avformat_alloc_context();
	//video = XiaoMi USB 2.0 Webcam
	// int ret = avformat_open_input(&ctx,"video = XiaoMi USB 2.0 Webcam", fmt, nullptr);
	//int ret = avformat_open_input(&ctx, "", fmt, nullptr);


	int ret = avformat_open_input(&ctx, "video=XiaoMi USB 2.0 Webcam", fmt, nullptr);
	if (ret != 0) {
		fprintf(stderr, "fail to open input stream: %d\n", ret);
		return -1;
	}

	/*int ret = avformat_open_input(&pFmtCtx, "video=XiaoMi USB 2.0 Webcam", fmt, nullptr);*/

	//int ret = avformat_open_input(&ctx, 0, fmt, nullptr);

	if (ret != 0) {
		fprintf(stderr, "fail to open input stream: %d\n", ret);
		return -1;
	}


	ret = avformat_find_stream_info(ctx, nullptr);
	if (ret < 0) {
		fprintf(stderr, "fail to get stream information: %d\n", ret);
		return -1;
	}
	else {
		cout << "success open stream" << endl;
	}
	w_width = ctx->streams[0]->codecpar->width;
	w_height = ctx->streams[0]->codecpar->height;
	/*cout << "codec is :"<<ctx->streams[0]->codec->codec->name << endl;*/
	cout << "width is :" << w_width << endl;
	cout << "high is :" << w_height << endl;



	pCodecCtx = avcodec_alloc_context3(pCodec);
	pCodecCtx->codec_tag = 0;

	

	avcodec_parameters_to_context(pCodecCtx, ctx->streams[0]->codecpar);

	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	cout << "codec is :"<<pCodec->name<< endl;
	cout << "pCodecCtx->width  is :" << pCodecCtx->width << endl;
	cout << "pCodecCtx->height  is :" << pCodecCtx->height << endl;
	cout << "coded_height is :" << pCodecCtx->coded_height << endl;
	cout << "coded_height is :" << pCodecCtx->coded_width << endl;
	if (pCodec == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unsupported codec!\n");
		return 0;
	}





	//// Open codec
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open decoder!\n");
		return 0;
	}
	AVFrame * pFrame;
	pFrame = av_frame_alloc();






	window = SDL_CreateWindow("getcam",
		200,
		200,
		w_width,
		w_height,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	render = SDL_CreateRenderer(window, -1, 0);

	texture = SDL_CreateTexture(render,
		SDL_PIXELFORMAT_IYUV,
		SDL_TEXTUREACCESS_STREAMING,
		w_width,
		w_height);




	int             frameFinished;


	AVFrame * frame;
	frame = av_frame_alloc();
	//AVPacket *packet;
	//packet = av_packet_alloc();

	screenwith = pCodecCtx->width;
	screenhight = pCodecCtx->height;




	ThreadForRender *pam;
	pam =(ThreadForRender *)av_mallocz(sizeof(ThreadForRender));

	pam->vq = (PacketQueue *)av_mallocz(sizeof(PacketQueue));

	pam->ctx = ctx;
	pam->pCodecCtx = pCodecCtx;
	//pam->render = render;
	//pam->texture = texture;
	packet_queue_init(pam->vq);



	schedule_refresh(pam->vq, 40);


	SDL_Thread* thread_id = SDL_CreateThread(packet_to_render, "Camera get packet thread", pam);

	while (true) {
		SDL_Event event;
	/*	SDL_WaitEvent(&event);*/
		SDL_PollEvent(&event);
		switch (event.type)
		{
	
		case FF_REFRESH_EVENT:
			video_refresh_timer(event.user.data1);
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
				case SDLK_F1:
					SDL_Log("preess f1 !!!");
					break;
				case SDLK_F2:
					SDL_Log("preess f2 !!!");
					break;

				case SDLK_F3:
					SDL_Log("preess f3 !!!");
					break;
				case SDLK_q:
					SDL_Log("presee q ,quit");
					goto __EXIT;
					break;
				default:
					SDL_Log("get other press");
					break;
			
			}
			break;
		case SDL_WINDOWEVENT:
		/*	SDL_Log("event type is SDL_WINDOWEVENT %d \n",event);*/

			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:

				SDL_Log("Window %d resized to %dx%d",
					event.window.windowID, event.window.data1,
					event.window.data2);
				screenwith = event.window.data1;
				screenhight = event.window.data2;
				SDL_Log("event type is SDL_WINDOWEVENT_RESIZED \n");
			break;
			default:
				break;
			}



			break;
		case SDL_SYSWMEVENT:
			SDL_Log("event type is SDL_SYSWMEVENT");
			break;
		case SDL_QUIT:
			fprintf(stderr, "receive a QUIT event: %d\n", event.type);
			SDL_Log("event type is ext %d", event.type);
			quit = 1;
			goto __EXIT;
			break ;
		default:
			/*SDL_Log("event type is %d",event.type);*/
			break;
		}

	}



	//SDL_Delay(10000);
	SDL_DestroyRenderer(render);


	if (!window) {
		cout << "Failed to create window" << endl;
		goto __EXIT;
	}

__EXIT:
	SDL_Quit();
	return 0;
}







int main(int argc, char *argv[]) {
	avdevice_register_all();
	/*listDevicesds();*/
	opencam();
	return 0;
}