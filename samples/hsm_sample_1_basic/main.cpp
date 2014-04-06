#include "Player.h"

int main()
{
	Player player;
	player.Init();

	for (int i = 0; i < 10; ++i)
	{
		player.FrameUpdate(0.1f);
	}

	player.Shutdown();
}
