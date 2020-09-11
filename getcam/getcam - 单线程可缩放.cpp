// getcam.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <string>
#include "DS_AudioVideoDevices.h"
#include <windows.h>


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


#define SDL_EVENT_INTERFACE_FRESH (SDL_USEREVENT+1)

int thread_exit = 0;

int event_handler(void * data) {
	while (!thread_exit) {
		SDL_Event event;
		event.type = SDL_EVENT_INTERFACE_FRESH;
		SDL_PushEvent(&event);
		/*SDL_Log("this is test11");*/
		Sleep(40);
	/*	SDL_Delay(40);*/
	
	}

	return 0;

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
	SDL_Window *window = NULL;
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Renderer *render = NULL;
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

	// AVCodecContext  *pCodecCtxOrig = avcodec_alloc_context3(pCodec); //codec context
	//pCodecCtxOrig = ctx->streams[0]->codec;
	//AVCodecContext * c;
	//c = ctx->streams[0]->codec;


	//cout << "codec is :" << pCodec->name << endl;
	AVCodecContext *pCodecCtx;

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



//#pragma warning(suppress : 4996)
//	if (avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
//		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't copy codec context");
//		return 0;
//	}

	/*AVCodecParameters*/
	//AVCodecContext *codec_ctx = avcodec_alloc_context3(pCodec);
	//ret = avcodec_parameters_to_context(codec_ctx, ctx->streams[0]->codecpar);

	//ret = avcodec_parameters_from_context(pCodecCtx, codec_ctx);




	//// Open codec
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open decoder!\n");
		return 0;
	}
	AVFrame * pFrame;
	pFrame = av_frame_alloc();





	//return 0;

	////window=SDL_CreateWindow("摄像头",
	////	200,
	////	200,
	////	640,
	////	480,
	////	SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);



	window = SDL_CreateWindow("getcam",
		200,
		200,
		w_width,
		w_height,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);



	render = SDL_CreateRenderer(window, -1, 0);


	SDL_Texture *texture;

	texture = SDL_CreateTexture(render,
		SDL_PIXELFORMAT_IYUV,
		SDL_TEXTUREACCESS_STREAMING,
		w_width,
		w_height);


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
	//SDL_SetRenderDrawColor(render, 255, 0, 0, 255);


	int             frameFinished;
	SDL_Rect        rect;
	// Read frames and save first five frames to disk

	SDL_Thread* thread_id = SDL_CreateThread(event_handler,"Camera thread",NULL);


	//cout << "codec is111 :" << pCodec->name << endl;
	AVFrame* frame_yuv = av_frame_alloc();


	

	int bufSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height,1);
	uint8_t* buffer = (uint8_t *)av_malloc(bufSize);
	av_image_fill_arrays(frame_yuv->data,frame_yuv->linesize,(const uint8_t *)buffer,
		AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height,1);


	frame_yuv->format = AV_PIX_FMT_YUV420P;

	AVFrame * frame;
	frame = av_frame_alloc();
	AVPacket *packet;
	packet = av_packet_alloc();

	int screenwith = pCodecCtx->width;
	int screenhight = pCodecCtx->height;

	//pCodecCtx->time_base.den = 1;

	//pCodecCtx->time_base.num = 75;
	while (true) {
		SDL_Event event;
		SDL_WaitEvent(&event);
		switch (event.type)
		{
		case SDL_EVENT_INTERFACE_FRESH:
			
			//SDL_Log("into frame hahaha");
			if (av_read_frame(ctx, packet) >= 0) {
				if (packet->stream_index == 0) {
					/*SDL_Log("av_read_frame frame hahaha,size is %d" , packet->size);*/
				
					int ret = avcodec_send_packet(pCodecCtx, packet);
					if (ret != 0) {
						SDL_Log("Cannot decoder packet. ret is %d\n",ret);
						break;
					}

					if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
						std::cout << "avcodec_send_packet: " << ret << std::endl;
						break;
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
							rect.w = screenwith;
							rect.h = screenhight;

							SDL_RenderClear(render);
							SDL_RenderCopy(render, texture, NULL, &rect);
							SDL_RenderPresent(render);
						}

					}				
				}
				
				av_packet_unref(packet);
			}
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