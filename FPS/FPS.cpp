/*
Window properties : 
Screen width and height adjust to what is put into program
Font should be consolas
Font size should be 16
*/

#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
using namespace std;

#include <stdio.h>
#include <Windows.h>

//Screen height/width
int nScreenWidth = 120;
int nScreenHeight = 40;

//Player stored 
float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

//Map
int nMapHeight = 16;
int nMapWidth = 16;

//FOV
float fFOV = 3.14159f / 4.0f;

//Depth to the wall
float fDepth = 16.0f;

//Main//
int main()
{
	//Screen buffer (I'm still understanding this)
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	//Establishing Map
	wstring map;

	map += L"################";
	map += L"#..##..........#";
	map += L"#..##..........#";
	map += L"#..##..........#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#.........#....#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#.......#......#";
	map += L"#.....###......#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#......#########";
	map += L"#..............#";
	map += L"################";

	//Time points for frames
	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();


	//Game loop
	while(1)
	{
		//Grab current system time
		tp2 = chrono::system_clock::now();
		//Calculate the duration between the current time and then previous time
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		//Player controls and rotation
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) //left
			fPlayerA -= (0.8f) * fElapsedTime;

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) //right
			fPlayerA += (0.8f) * fElapsedTime;

		//Go forward
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			/*Notes: Takes the unit vector established earlier, multiples to give magnitude by the elapsed time*/
			fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;;
			fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;;

			//Collision
			if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
			{
				fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;;
				fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;;
			}
		}

		//Go backwards
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{	
			fPlayerX -= sinf(fPlayerA) * fElapsedTime;;
			fPlayerY -= cosf(fPlayerA) * fElapsedTime;;

			//Collision
			if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
			{
				fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;;
				fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;;
			}
		}

		for (int x=0; x < nScreenWidth; x++)
		{
			//Projected ray angle into space
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			//Tracks the distance between the player and wall
			float fDistanceToWall = 0.0f;
			bool bHitWall = false;

			//Unit vector to determine where the player is looking (Ill look up what a unit vector is later)
			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);

			//When not hitting the wall we want to keep incrementing our distance to the wall
			while (!bHitWall && fDistanceToWall < fDepth)
			{
				//Increases by a step
				fDistanceToWall += 0.1f;

				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				//Checks to see if out of bounds
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
				{
					bHitWall = true;
					fDistanceToWall = fDepth;
				}
				else 
				{
					//Inbounds so test to see if the ray cell is a wall block
					/*Notes: If a # symbol is contained then weve hit a wall*/
					if (map[nTestY * nMapWidth + nTestX] == '#')
					{
						bHitWall = true;
					}
				
				}
			}

			// Calculate distance to ceiling and floor
			/*Notes: As distance to wall get larger subtraction gets smaller*/
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			//Mirror of the ceiling
			int nFloor = nScreenHeight - nCeiling;

			//Shading :D
			short nShade = ' ';

			if (fDistanceToWall <= fDepth / 4.0f) //Really close
				nShade = 0x2588;
			else if (fDistanceToWall < fDepth / 3.0f)
				nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)
				nShade = 0x2592;
			else if (fDistanceToWall < fDepth)
				nShade = 0x2591;
			else
				nShade = ' '; //Far away

			//Draws floor and ceiling
			for(int y = 0; y < nScreenHeight; y++)
			{
				if (y <= nCeiling) //Ceiling
					screen[y * nScreenWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor) //Wall
					screen[y * nScreenWidth + x] = nShade;
				else
				{
					//Shading floor
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));

					/*How far the floor can be seen and then shaded accordingly*/
					if (b < 0.25)		
						nShade = '#';
					else if (b < 0.5)	
						nShade = 'x';
					else if (b < 0.75)	
						nShade = '.';
					else if (b < 0.9)	
						nShade = '-';
					else				
						nShade = ' ';

					screen[y * nScreenWidth + x] = nShade;
				}
					
			}
		}

		// Display Frame
		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);	
	}

	

	return 0;
}
