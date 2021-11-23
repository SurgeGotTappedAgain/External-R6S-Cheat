#include <Windows.h>
#include <iostream>
#include "../Game.hpp"
#include "../Overlay.hpp"
#include <xmmintrin.h>
#include "../Defines.hpp"
#include "../globals.hpp"
#include <random>
HWND hwnd = nullptr;
// Credits: https://github.com/fir3z (Base + Overlay), InsideExploit (For The Help & Decryption), UghYes (For Operator Name List) 

auto UpdateEnts()
{
	while (true)
	{
		std::vector<entity_t> tmp = { {} };
		for (int i = 0; i < globals.entityCount; i++)
		{
			uint64_t player = driver::Read<uint64_t>(globals.entityList + (i * sizeof(uint64_t)));
			if (!player) continue;

			uint64_t pawn = Game::Pawn(player);
			if (!pawn) continue;

			if (globals.teamcheck)
			{
				globals.TeamID = Game::get_team_id(globals.replicationinfo);
				uint64_t replicationInfo = Game::ReplicationInfo(player);
				if (Game::get_team_id(replicationInfo) == globals.TeamID) continue;
			}

			entity_t entity{ {} };
			entity.controller = player;
			entity.pawn = pawn;
			tmp.push_back(entity);
		}
		Visuals::entities = tmp;
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
}


int main()
{

	while (!hwnd)
	{
		hwnd = FindWindow(NULL, "Rainbow Six");
	}

	std::cout << "[+] Found Rainbow Six" << std::endl;

	// Dont worry about this messy shit

	driver::getProcessID("RainbowSix.exe");
	Base = (uintptr_t)driver::getModuleBase("RainbowSix.exe");

	// Updated
	globals.gameManager = Game::GameManager();
	globals.gameProfile = Game::GamerProfile();
	globals.cam = Game::GetCamera();
	globals.entityList = Game::EntityManager(globals.gameManager).first;
	globals.entityCount = Game::EntityManager(globals.gameManager).second;
	globals.localPlayer = driver::Read<uint64_t>(globals.entityList);
	globals.replicationinfo = Game::ReplicationInfo(globals.localPlayer);
	globals.localPawn = Game::Pawn(globals.localPlayer);

	std::thread(UpdateEnts).detach();

	setup_window();
	setup_directx(MyWnd);

	while (true)
	{
	   main_loop();
	}

	std::getchar();
}
