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

using namespace std;
//
//int main(int argc, char **argv) {
//	cout << "hellp world" << endl;
//	int ret;
//	int videoindex = -1;
//	AVFormatContext *fmtCTX=
//
//
//}