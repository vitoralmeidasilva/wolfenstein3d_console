// MODIFICATION TO LOAD WOLFENSTEIN 3D MAPS
// WORKS WITH SHAREWARE VERSION ONLY!!!

// HEAVILY BASED AND INSPIRED BY WORK DONE BY JAVIDX9:
// FIRST PERSON SHOOTER... ...on the command line?!
// https://www.youtube.com/watch?v=xW8skO7MFYw


// TODO: split the project into other files if necessary

// TODO: implement keys (N = next level, P = previous level)

// TODO: bonus = understand all the process (MATH) of the raycasting
// TODO: correct tile rendering order, player positioning and initial rotation angle
// TODO: draw a compass
// TODO: implement strafing
// TODO: implement mouse control

// TODO: shrink data sizes where possible
// TODO: color code wall data based on tile index (instead of sampling textures)? or make a mode where the wall texture is shown (sprites?)
// TODO: migrate to olcPixelGameEngine (extract and sample textures)

#include <iostream>
#include <fstream> // ifstream
#include <sstream> // stringstream
#include <chrono> // time control
#include <utility> // pair
#include <algorithm> // sort, find
#include <vector> // vector
#include <tuple> // tuple
#include <cstdio> // swprintf_s
#include <cstdint> // types

using namespace std;

#include "olcConsoleGameEngine.h"

class ConFPSWolf : public olcConsoleGameEngine
{
public:
	ConFPSWolf()
	{
		m_sAppName = L"Wolfenstein 3D Console Version";
	}

	virtual bool OnUserCreate()
	{
		bIsMinimapVisible = true;

		map = LoadWolf3dMap(1, 1, fPlayerX, fPlayerY, fPlayerA, sLevelName); // E1M1
		//map = LoadWolf3dMap(1, 2, fPlayerX, fPlayerY, fPlayerA, sLevelName); // E1M2
		//map = LoadWolf3dMap(1, 3, fPlayerX, fPlayerY, fPlayerA, sLevelName); // E1M3
		//map = LoadWolf3dMap(1, 4, fPlayerX, fPlayerY, fPlayerA, sLevelName); // E1M4
		//map = LoadWolf3dMap(1, 5, fPlayerX, fPlayerY, fPlayerA, sLevelName); // E1M5
		//map = LoadWolf3dMap(1, 6, fPlayerX, fPlayerY, fPlayerA, sLevelName); // E1M6
		//map = LoadWolf3dMap(1, 7, fPlayerX, fPlayerY, fPlayerA, sLevelName); // E1M7
		//map = LoadWolf3dMap(1, 8, fPlayerX, fPlayerY, fPlayerA, sLevelName); // E1M8
		//map = LoadWolf3dMap(1, 9, fPlayerX, fPlayerY, fPlayerA, sLevelName); // E1M9
		//map = LoadWolf3dMap(1, 10, fPlayerX, fPlayerY, fPlayerA, sLevelName); // E1M10
		return true;
	}

	virtual bool OnUserUpdate(float fElapsedTime)
	{
		// Controls
		// Minimap
		if (m_keys[L'M'].bPressed)
			bIsMinimapVisible = !bIsMinimapVisible;
		/*
		// Next level
		if (m_keys[L'N'].bPressed)
			// next level
		// Previous level
		if (m_keys[L'P'].bPressed)
			// previous level
		*/

		// Handle CCW Rotation
		if (m_keys[L'A'].bHeld)
			fPlayerA -= (0.8f) * fElapsedTime; // 0.1 radians per frame update

		if (m_keys[L'D'].bHeld)
			fPlayerA += (0.8f) * fElapsedTime;

		if (m_keys[L'W'].bHeld)
		{
			// project the player forwards
			fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime; // WHY SIN here instead of COS??? Is it because coordinates are inverted?? 
			fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				// if collision, move backwards same amount
				fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
				fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;
			}
		}

		if (m_keys[L'S'].bHeld)
		{
			fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				// if collision, move forwards same amount
				fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;
				fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;
			}
		}



		for (int x = 0; x < ScreenWidth(); x++)
		{
			// For each column, calculate the projected ray angle into world space
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)ScreenWidth()) * fFOV; // calculate what is the starting angle for the field of view
			// (fPlayerA - fFOV / 2.0f)						-> half the field of view
			// ((float)x / (float)ScreenWidth()) * fFOV		-> chopping it up into little bits

			float fDistanceToWall = 0; // distance from the player to the wall for that given angle
			bool bHitWall = false;
			bool bBoundary = false; // Is it the edge of a cell??

			// unit vector representing the direction the player is looking (cartesian coordinates from polar coordinates)
			float fEyeX = sinf(fRayAngle); // Unit vector for ray in player space
			float fEyeY = cosf(fRayAngle);

			while (!bHitWall && fDistanceToWall < fDepth)
			{
				fDistanceToWall += 0.1f;

				// line of a given distance (remember, line equation = y=mx+b where m is the slope and b is the y - intercept)
				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				// Test if ray is out of bounds
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
				{
					// out of bounds of our map
					bHitWall = false; // Just set distance to maximum depth
					fDistanceToWall = fDepth;
				}
				else
				{
					// within bounds of our map (we want to check cells individually in our map)

					// Ray is inbounds so test to see if the ray cell is a wall block
					if (map[nTestY * nMapWidth + nTestX] == '#')
					{
						bHitWall = true;

						std::vector<std::pair<float, float>> p; // distance, dot

						for (int tx = 0; tx < 2; tx++)
							for (int ty = 0; ty < 2; ty++)
							{
								// we are getting the perfect integer corners offset from our player position
								// this will give us just a vector from the player to the perfect corner
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx * vx + vy * vy); // magnitude of that vector (how far away the corner is from the player)
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d); // dot product
								p.push_back(std::make_pair(d, dot));
							}

						// Sort pairs from closest to farthest
						sort(p.begin(), p.end(), [](const std::pair<float, float>& left, const std::pair<float, float>& right) { return left.first < right.first; });

						float fBound = 0.01f;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
						//if (acos(p.at(2).second) < fBound) bBoundary = true;
					}
				}
			}

			// Calculate distance to ceiling and floor
			// for the ceiling, we want to take the midpoint and from that we are subtracting a proportion of the screen height relative to distance to wall
			// as distance to wall gets larger, this subtraction (- ScreenWidth() / ((float)fDistanceToWall)) gets smaller
			// and therefore we have a higher ceiling
			int nCeiling = (int)((float)(ScreenHeight() / 2.0) - ScreenHeight() / ((float)fDistanceToWall));

			// the floor in our case is just a mirror of the ceiling
			int nFloor = ScreenHeight() - nCeiling;

			short nShade = ' ';
			short nShadeFloor = ' ';

			if (fDistanceToWall <= fDepth / 4.0f)		nShade = 0x2588;	// █	full block = Very close
			else if (fDistanceToWall < fDepth / 3.0f)	nShade = 0x2593;	// ▓	dark shade
			else if (fDistanceToWall < fDepth / 2.0f)	nShade = 0x2592;	// ▒	medium shade
			else if (fDistanceToWall < fDepth)			nShade = 0x2591;	// ░	light shade
			else										nShade = ' ';		// Too far away

			if (bBoundary) nShade = ' '; // Black it out

			// now we know where our floor begins, our ceiling begins and our wall ends
			for (int y = 0; y < ScreenHeight(); y++)
			{
				if (y < nCeiling) // the cell must be part of the ceiling
					Draw(x, y, L' '); // sky
				else if (y > nCeiling && y <= nFloor)
					Draw(x, y, nShade); // wall
				else
				{
					// Shade floor based on distance
					float b = 1.0f - (((float)y - ScreenHeight() / 2.0f) / ((float)ScreenHeight() / 2.0f)); // portion of how far the floor can be seen...
					if (b < 0.25)		nShadeFloor = '#';
					else if (b < 0.5)	nShadeFloor = 'x';
					else if (b < 0.75)	nShadeFloor = '.';
					else if (b < 0.9)	nShadeFloor = '-';
					else				nShadeFloor = ' ';
					Draw(x, y, nShadeFloor); // floor
				}
			}

		}

		// Display stats
		// TODO: do I put this into a lambda function?
		//swprintf_s(screen, 80, L"X=%3.2f, Y=%3.2f, A=%3.2f, FPS=%3.2f, LEVEL=%hs", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime, sLevelName.c_str());
		std::stringstream ss; ss << "X=" << fPlayerX << ", Y=" << fPlayerY << ", A=" << fPlayerA << ", FPS=" << (1.0f / fElapsedTime) << ", LEVEL=" << sLevelName.c_str();
		std::string sHud = ss.str(); // convert std::stringstream to std::string
		std::wstring wsHud(sHud.begin(), sHud.end()); // convert std::string to std::wstring
		DrawString(0, 0, wsHud);


		if (bIsMinimapVisible)
		{
			// Display Map (only displaying half)
			for (int nx = 0; nx < nMapWidth; nx++)
				for (int ny = 0; ny < nMapHeight; ny++)
				{
					// + 1 here to not overwrite the stats
					Draw(nx, ny + 1, map[ny * nMapWidth + nx]);
				}

			// marker to show where the player is
			Draw((int)fPlayerX, (int)fPlayerY + 1, L'P');
		}

		return true;
	}

private:
	float fPlayerX = 0.0f;
	float fPlayerY = 0.0f;

	float fPlayerA = 0.0f; // in radians
	float fSpeed = 5.0f; // walking speed

	int nMapHeight = 64, nMapWidth = 64;
	std::wstring map;

	float fFOV = 3.14159f / 4.0f; // something like PI / 4 (quite a narrow field of view) [45 degrees]
	float fDepth = 16.0f;

	std::string sLevelName;

	bool bIsMinimapVisible;

	std::wstring LoadWolf3dMap(const uint8_t nEpisode, const uint8_t nLevel, float& fPlayerPosX, float& fPlayerPosY, float& fPlayerAng, std::string& levelName)
	{
		constexpr int nTotalPlanes = 3; // number of planes available for each level
		const uint8_t nMaxMaps = 100, nLevelsPerEpisode = 10;
		const uint8_t nAbsLevelNumber = (nEpisode == 1) ? nLevel : ((nEpisode * nLevelsPerEpisode) - nLevelsPerEpisode) + nLevel;

		// Helper Utilities ====================

		// Reads nLength bytes from file stream, and constructs a text string
		auto ReadString = [](std::ifstream& ifs, const uint32_t nLength)
		{
			std::string s;
			for (uint32_t i = 0; i < nLength; i++) s += ifs.get();
			return s;
		};

		// Reads nLength bytes from file stream, and constructs a std::vector
		auto ReadBuffer2Vector = [](std::ifstream& ifs, const uint32_t nLength)
		{
			std::vector<uint8_t> vecTmp;
			for (uint32_t i = 0; i < nLength; i++)
			{
				uint8_t n8 = 0;
				ifs.read((char*)&n8, sizeof(uint8_t));

				vecTmp.push_back(n8);
			}
			return vecTmp;
		};

		// Expands the content from "vecSource" vector to "nLength" bytes (length of expanded data) using Carmack's compression algorithm
		auto CarmackExpand = [](const std::vector<uint8_t>& vecSource, uint32_t nLength)
		{
			// nLength = number of bytes (not words) in decompressed data
			// vecSource is in bytes

			const uint32_t nNearTag = 0xA7, nFarTag = 0xA8;
			int32_t nChhigh = 0, nOffset = 0, nCopyptr = 0, nOutptr = 0, nInptr = 0, nCh = 0, nCount = 0;

			nLength /= 2; // length in words (instead of bytes)

			std::vector<uint32_t> vecDest(nLength);

			// while we have words to read in the compressed source...
			while (nLength)
			{
				// Each word is uncompressed data, unless its high byte is 0xA7 or 0xA8 (in that case the word is probably the begining of a pointer)
				// However, there's an exception made for words that unfortunately ends with a pointer signature
				nCh = (vecSource[nInptr] + (vecSource[nInptr + 1] << 8)) & 0xFFFF;

				nInptr += 2;
				nChhigh = nCh >> 8; // high byte of the word being read

				if (nChhigh == nNearTag)
				{
					nCount = nCh & 0xFF; // Number of words to copy
					if (!nCount)
					{
						// have to insert a word containing the tag byte
						nCh |= vecSource[nInptr++];
						vecDest[nOutptr++] = nCh;
						nLength--;
					}
					else
					{
						nOffset = vecSource[nInptr++]; // Relative offset of the first word to copy
						nCopyptr = nOutptr - nOffset;
						nLength -= nCount;
						while (nCount)
						{
							vecDest[nOutptr++] = vecDest[nCopyptr++];
							nCount--;
						}
					}
				}
				else if (nChhigh == nFarTag)
				{
					nCount = nCh & 0xFF;
					if (!nCount)
					{
						// have to insert a word containing the tag byte
						nCh |= vecSource[nInptr++];
						vecDest[nOutptr++] = nCh;
						nLength--;
					}
					else
					{
						nOffset = (vecSource[nInptr] + (vecSource[nInptr + 1] << 8)) & 0xFFFF;
						nInptr += 2;
						nCopyptr = nOffset;
						nLength -= nCount;
						while (nCount)
						{
							vecDest[nOutptr++] = vecDest[nCopyptr++];
							nCount--;
						}
					}
				}
				else
				{
					vecDest[nOutptr++] = nCh; // word
					nLength--;
				}
			}

			return vecDest;
		};

		// Expands the content from "vecSource" array to "nLength" bytes (length of expanded data) compressed with RLE - run - length encoding (RLEW in this case) to a new buffer
		auto RLEWExpand = [](const std::vector<uint32_t>& vecSource, const uint32_t nLength, const uint16_t nRLE)
		{
			uint32_t nValue = 0, nCount = 0, nInptr = 0, nOutptr = 0, nEnd = 0;

			nEnd = nOutptr + (nLength >> 1); // end = number of words to read ("length >> 1" == "length / 2")

			std::vector<uint8_t> vecDest(nEnd);

			do
			{
				nValue = vecSource[nInptr++];
				if (nValue != nRLE)
				{
					// uncompressed
					vecDest[nOutptr++] = nValue;
				}
				else
				{
					// compressed string
					nCount = vecSource[nInptr++];
					nValue = vecSource[nInptr++];
					for (uint32_t i = 0; i < nCount; i++)
					{
						vecDest[nOutptr++] = nValue;
					}
				}
			} while (nOutptr < nEnd);

			return vecDest;
		};

		// Reads plane data from map file
		auto ReadPlaneData = [&](std::ifstream& ifs, const uint32_t nOffset, const uint16_t nLength, const uint16_t nRLE)
		{
			// For each plane:

			// 1 - go to a specified offset
			ifs.seekg(nOffset);

			// 2 - read the specified size of the plane,
			uint16_t nExpandedLength = 0;
			ifs.read((char*)&nExpandedLength, sizeof(uint16_t)); // number of bytes (not words) in decompressed data

			// 3 - then decompresses the chunk of data with Carmack's Algorithm
			// https://1drv.ms/u/s!Ao1E4OCcZiFLn8MQ77qdfT-jaoEBVg
			std::vector<uint8_t> vecCarmack = ReadBuffer2Vector(ifs, nLength - 2);
			std::vector<uint32_t> vecExpandedData = CarmackExpand(vecCarmack, nExpandedLength); // each entry in vecExpandedData is a word

			// first word is the size in bytes (remove it from decompressed data)
			// [number of bytes to read (64 * 64 * 2) = (width * height * bytes_per_word)]
			uint32_t nExpandedDataSize = vecExpandedData[0];
			vecExpandedData.erase(vecExpandedData.begin());

			// 4 - and decompress the result with the RLEW algorithm. 
			return RLEWExpand(vecExpandedData, nExpandedDataSize, nRLE);
		};

		// Load Map Geometry ====================

		// The map header file (MAPHEAD) is of varying length and contains 
		// three main types of data.
		struct sHeader
		{
			uint16_t nMagicWord = 0; // The first is the magic word or flag used for RLEW compression, which is always $ABCD.
			uint32_t nMaps[100]{}; // The second is 100 pointers to start of level 0-99 data in the GAMEMAPS file (relative to the start of that file); Where the data for the level is located
			uint8_t nTotalMaps = 0; // number of maps available [1-100]

			const uint16_t nFlag = 0xABCD; // flag used for RLEW compression (interpreted as big endian 0xABCD but stored as little endian: 0xCDAB)
		} header;


		std::ifstream headFile;
		headFile.open("./resources/MAPHEAD.WL1", std::ifstream::binary);
		if (!headFile.is_open())
			return L""; // Failed to open HEADER file


		headFile.read((char*)&header.nMagicWord, sizeof(uint16_t));
		if (header.nMagicWord != header.nFlag)
			return L""; // Invalid header file. Magic word not supported.


		headFile.read((char*)&header.nMaps, nMaxMaps * sizeof(uint32_t));
		for (uint8_t i = 0; i < nMaxMaps; i++) if (header.nMaps[i] > 0) header.nTotalMaps++;

		headFile.close();

		if (nAbsLevelNumber > header.nTotalMaps)
			return L""; // Invalid level number


		// The header for each level inside the GAMEMAPS (or MAPTEMP) file (which is pointed to by MAPHEAD) is 42 bytes longand
		// RLEW compressed, though this is difficult to see since the data is so shortand non - repetitive.
		// For the offsets to level planes, a value of 0 indicates the plane does not exist.
		// - Plane 0 is background using unmasked tiles (Walls) (this is what we are interested for now!!!)
		// - Plane 1 is foreground and uses masked tiles (Objects)
		// - Plane 2 is sprite/info (Extra)
		// Levels must contain a background plane and usually an infoplane
		using plane_t = std::tuple<uint32_t, uint16_t, std::vector<uint8_t>>;
		struct sLevelHeader
		{
			// 3 Planes available for the map. Each plane has three attributes:
			//  - first: Offset in GAMEMAPS to beginning of compressed plane 0 data (or 0 if plane is not present);
			//  - second: Length of compressed plane data (in bytes)
			//  - third: Decompressed plane data; must have (4096 items (64 * 64); 2 bytes per entry = 2 * 4096 = 8192 bytes)
			// Each plane represents:
			//  - plane 0: The first grid is used to describe the level itself. It describes the walls on the level and the location of the secrets and the doors.Each grid "cell" describes the texture that would cover the four(exterior and interior) walls of that cell.
			//  - plane 1: The second grid is used to show where all the game objects are (including the entrances to secrets).
			//  - plane 2: The third grid is used to contain operational and logical information about the level. It holds the "turning points" for the game's enemies. And other information used by the game engine.
			plane_t planes[nTotalPlanes];

			uint16_t nWidth{}; // Width of level (in tiles)
			uint16_t nHeight{}; // Height of level (in tiles)

			std::string sName; // Internal name for level (used only by editor, not displayed in-game. null-terminated)
		} levelHeader;

		std::ifstream gamemapFile;
		gamemapFile.open("./resources/GAMEMAPS.WL1", std::ifstream::binary);
		if (!gamemapFile.is_open())
			return L""; // Failed to open GAMEMAPS file

		if (ReadString(gamemapFile, 8) != "TED5v1.0")
			return L""; // Invalid header file. Magic word not supported.


		// Each level in the file will have from two to four chunks (usually four)
		// depending on the game, with all levels in a given game having the same number of
		// chunks. These are the level header and 1 - 3 planes (foreground, background
		// and sprite / info.) The chunks are in no particular order and it is
		// possible to read through the entire file decompressing chunks as they're found.
		gamemapFile.seekg(header.nMaps[nAbsLevelNumber - 1]);

		uint32_t nOffsets[nTotalPlanes];
		gamemapFile.read((char*)&nOffsets, nTotalPlanes * sizeof(uint32_t));
		uint16_t nLengths[nTotalPlanes];
		gamemapFile.read((char*)&nLengths, nTotalPlanes * sizeof(uint16_t));
		for (uint8_t i = 0; i < nTotalPlanes; i++)
		{
			std::get<0>(levelHeader.planes[i]) = nOffsets[i];
			std::get<1>(levelHeader.planes[i]) = nLengths[i];
		}

		gamemapFile.read((char*)&levelHeader.nWidth, sizeof(uint16_t));
		gamemapFile.read((char*)&levelHeader.nHeight, sizeof(uint16_t));

		levelHeader.sName = ReadString(gamemapFile, 16);
		levelName = levelHeader.sName;


		// Note that for Wolfenstein 3D, a 4 - byte signature string("!ID!") will normally be present directly after the level name.
		// The signature does not appear to be used anywhere, but is useful for distinguishing between
		// v1.0 files (the signature string is missing),
		// and files for v1.1 and later (includes the signature string).
		const std::string sVersion = ReadString(gamemapFile, 4); // !ID! == v1.1

		// read plane data
		for (uint8_t i = 0; i < nTotalPlanes; i++)
			std::get<2>(levelHeader.planes[i]) = ReadPlaneData(gamemapFile, std::get<0>(levelHeader.planes[i]), std::get<1>(levelHeader.planes[i]), header.nFlag);

		gamemapFile.close();

		std::vector<uint8_t> vecTiles = std::get<2>(levelHeader.planes[0]);
		std::vector<uint8_t> vecObjects = std::get<2>(levelHeader.planes[1]);

		std::vector<uint8_t> vecRefWallTiles = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 51, 85 };
		std::vector<uint8_t> vecRefPlayerTiles = { 19, 20, 21, 22 };
		std::vector<uint8_t> vecRefDoorTiles = { 50, 53, 90, 91, 92, 93, 94, 95, 100, 101 };

		std::wstring sMap = L"";

		//std::vector<std::tuple<uint32_t, uint32_t>> vecPlayerStartingPositions;

		uint32_t nTi = 0, nTiX = 0, nTiY = 0;
		for (const auto& nTile : vecTiles)
		{
			uint8_t nObject = vecObjects[nTi]; // object in current position

			bool bIsWall = std::find(vecRefWallTiles.begin(), vecRefWallTiles.end(), nTile) != vecRefWallTiles.end();
			//bool bIsDoor = std::find(vecDoorTiles.begin(), vecDoorTiles.end(), nTile) != vecDoorTiles.end();

			auto it = std::find(vecRefPlayerTiles.begin(), vecRefPlayerTiles.end(), nObject);
			int orientation = std::distance(vecRefPlayerTiles.begin(), it); // each offset position in vecRefPlayerTiles vector is equivalent to 90 degrees of rotation on y axis
			bool bIsPlayer = (it != vecRefPlayerTiles.end());

			if (bIsWall)	sMap.append(L"#");
			else			sMap.append(L".");

			if (bIsPlayer)
			{
				//vecPlayerStartingPositions.push_back(std::make_tuple(nTiX, nTiY));
				fPlayerPosX = (float)nTiX;
				fPlayerPosY = (float)nTiY;

				//const float fStartPlayerAngles[] = { 0.0f, 1.5708f, 3.14159f, 4.71239f }; // startup player angle in radians (0 = north, 90 = east, 180 = south, 270 = west)
				//const float fStartPlayerAngles[] = { 
				//	0.0f, 
				//	1.5708f, 
				//	3.14159f, 
				//	4.71239f 
				//};
				//fPlayerAng -= fStartPlayerAngles[orientation];
			}

			nTiX++;

			if ((nTiX % levelHeader.nWidth) == 0)
			{
				nTiX = 0;
				nTiY++;
			}

			nTi++;
		}

		return sMap;
	}
};


int main()
{
	ConFPSWolf game;
	game.ConstructConsole(120, 80, 8, 8);
	game.Start();

	return 0;
}
