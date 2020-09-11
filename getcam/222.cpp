::CoInitialize(NULL); //调用DirectShow SDK的API需要用到COM库


int iVideoCapDevNum = 0;
int iAudioCapDevNum = 0;

char * DevicesArray[20];
for (int i = 0; i < 20; i++)
{
	DevicesArray[i] = new char[256];
	memset(DevicesArray[i], 0, 256);
}

HRESULT hr;
hr = EnumDevice(DSHOW_VIDEO_DEVICE, DevicesArray, sizeof(DevicesArray) / sizeof(DevicesArray[0]), iVideoCapDevNum);
if (hr == S_OK)
{
	for (int i = 0; i < iVideoCapDevNum; i++)
	{
		CString strDevName = DevicesArray[i];

	}
}

hr = EnumDevice(DSHOW_AUDIO_DEVICE, DevicesArray, sizeof(DevicesArray) / sizeof(DevicesArray[0]), iAudioCapDevNum);
if (hr == S_OK)
{
	for (int i = 0; i < iAudioCapDevNum; i++)
	{
		CString strDevName = DevicesArray[i];

	}
}

for (int i = 0; i < 20; i++)
{
	delete DevicesArray[i];
	DevicesArray[i] = NULL;
}
