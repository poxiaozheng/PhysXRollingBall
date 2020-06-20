#include "SoundUtils.h"

using namespace std;

int Sound::playSound()
{
	PlaySound(TEXT("D:\\test.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	return 0;
}
