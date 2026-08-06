/*
 * Portions of this file are based on the PopCap Games Framework
 * Copyright (C) 2005-2009 PopCap Games, Inc.
 *
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * This file is part of PvZ-Portable.
 *
 * PvZ-Portable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PvZ-Portable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PvZ-Portable. If not, see <https://www.gnu.org/licenses/>.
 */

#include "Common.h"
#include "ImageLib.h"
#include <math.h>
#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <string_view>
#include <mutex>
#include <unordered_set>
#include "paklib/PakInterface.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#include "stb_image.h"

using namespace ImageLib;
using namespace std::string_view_literals;

Image::Image()
{
	mWidth = 0;
	mHeight = 0;
	mBits = nullptr;
}

Image::~Image()
{
	if (mStbLoaded)
		stbi_image_free(mBits);
	else
		delete[] mBits;
}

int	Image::GetWidth()
{
	return mWidth;
}

int	Image::GetHeight()
{
	return mHeight;
}

uint32_t* Image::GetBits()
{
	return mBits;
}

// stb_image reading through the pak interface, so images decode straight from
// the (possibly memory-mapped) archive without an intermediate file copy.

static int PakStbRead(void* theUser, char* theData, int theSize)
{
	return static_cast<int>(p_fread(theData, 1, theSize, static_cast<PFILE*>(theUser)));
}

static void PakStbSkip(void* theUser, int theCount)
{
	p_fseek(static_cast<PFILE*>(theUser), theCount, SEEK_CUR);
}

static int PakStbEof(void* theUser)
{
	return p_feof(static_cast<PFILE*>(theUser));
}

// stb decodes to RGBA byte order; the engine expects 0xAARRGGBB words.
static void SwizzleToARGB(uint32_t* theBits, size_t thePixelCount)
{
	if constexpr (std::endian::native == std::endian::little)
	{
		for (size_t i = 0; i < thePixelCount; i++)
		{
			uint32_t aPixel = theBits[i];
			theBits[i] = (aPixel & 0xFF00FF00) |
						 ((aPixel & 0x000000FF) << 16) |
						 ((aPixel & 0x00FF0000) >> 16);
		}
	}
}

static Image* GetStbImage(const std::string& theFileName)
{
	PFILE* aFP = p_fopen(theFileName.c_str(), "rb");
	if (aFP == nullptr)
		return nullptr;

	stbi_io_callbacks aCallbacks = { PakStbRead, PakStbSkip, PakStbEof };
	int aWidth = 0, aHeight = 0, aComponents = 0;
	unsigned char* aPixels = stbi_load_from_callbacks(&aCallbacks, aFP, &aWidth, &aHeight, &aComponents, 4);
	p_fclose(aFP);

	if (aPixels == nullptr)
		return nullptr;

	uint32_t* aBits = reinterpret_cast<uint32_t*>(aPixels);
	SwizzleToARGB(aBits, static_cast<size_t>(aWidth) * aHeight);

	Image* anImage = new Image();
	anImage->mWidth = aWidth;
	anImage->mHeight = aHeight;
	anImage->mBits = aBits;
	anImage->mStbLoaded = true;
	return anImage;
}

Image* GetPNGImage(const std::string& theFileName)
{
	return GetStbImage(theFileName);
}

Image* GetTGAImage(const std::string& theFileName)
{
	return GetStbImage(theFileName);
}

Image* GetGIFImage(const std::string& theFileName)
{
	return GetStbImage(theFileName);
}

Image* GetJPEGImage(const std::string& theFileName)
{
	return GetStbImage(theFileName);
}

int ImageLib::gAlphaComposeColor = 0xFFFFFF;
bool ImageLib::gAutoLoadAlpha = true;
bool ImageLib::gIgnoreJPEG2000Alpha = true;

using ImageLoader = Image* (*)(const std::string&);
using ImageExtEntry = std::pair<std::string_view, ImageLoader>;
static constexpr std::array<ImageExtEntry, 4> kImageExts = {
	ImageExtEntry{ ".png"sv, GetPNGImage },
	ImageExtEntry{ ".jpg"sv, GetJPEGImage },
	ImageExtEntry{ ".gif"sv, GetGIFImage },
	ImageExtEntry{ ".tga"sv, GetTGAImage },
};

static bool EqualsIgnoreCase(std::string_view theLeft, std::string_view theRight)
{
	if (theLeft.size() != theRight.size())
		return false;

	for (size_t i = 0; i < theLeft.size(); i++)
	{
		const unsigned char aLeftChar = static_cast<unsigned char>(theLeft[i]);
		const unsigned char aRightChar = static_cast<unsigned char>(theRight[i]);
		if (std::tolower(aLeftChar) != std::tolower(aRightChar))
			return false;
	}

	return true;
}

static std::mutex gMissingDirMutex;
static std::unordered_set<std::string> gMissingDirCache;

static bool CheckSinglePath(std::string_view thePath)
{
	if (thePath.empty())
		return false;

	if (gPakInterface)
	{
		if (gPakInterface->mPakRecordMap.contains(PakInterface::NormalizePakPath(thePath)))
			return true;
	}

	const std::string aPathString(thePath);
	if (!Sexy::IsPathRooted(aPathString))
	{
		const auto& aResourceBase = Sexy::GetResourceFolder();
		if (!aResourceBase.empty())
		{
			const std::string aDir = Sexy::GetFileDir(aPathString);
			if (!aDir.empty())
			{
				std::scoped_lock lock(gMissingDirMutex);
				if (gMissingDirCache.contains(aDir))
					return false;
			}

			if (Sexy::FileExists(Sexy::GetResourcePath(aPathString)))
				return true;

			if (!aDir.empty())
			{
				if (!Sexy::FileExists(Sexy::GetResourcePath(aDir)))
				{
					std::scoped_lock lock(gMissingDirMutex);
					gMissingDirCache.insert(aDir);
				}
			}

			return false;
		}
	}

	return Sexy::FileExists(aPathString);
}

static bool FastFileExists(std::string_view thePath)
{
	if (thePath.empty())
		return false;

	const auto aFilePath = Sexy::PathFromU8(thePath);
	if (aFilePath.has_extension())
		return CheckSinglePath(thePath);

	std::string aCandidate(thePath);
	const auto aBaseLen = aCandidate.size();
	for (const auto& [aExt, _] : kImageExts)
	{
		aCandidate.resize(aBaseLen);
		aCandidate.append(aExt);
		if (CheckSinglePath(aCandidate))
			return true;
	}

	return false;
}

static Image* TryLoadByExt(const std::string& theBaseName, std::string_view theExt)
{
	for (const auto& [aKnownExt, aLoader] : kImageExts)
	{
		if (theExt.empty() || EqualsIgnoreCase(theExt, aKnownExt))
		{
			if (Image* aImage = aLoader(theBaseName + std::string(aKnownExt)))
				return aImage;
		}
	}
	return nullptr;
}

static void ComposeAlpha(Image* theImage, Image* theAlphaImage)
{
	if (theImage->mWidth != theAlphaImage->mWidth ||
		theImage->mHeight != theAlphaImage->mHeight)
		return;

	uint32_t* aDstBits = theImage->mBits;
	const uint32_t* aSrcBits = theAlphaImage->mBits;
	const int aSize = theImage->mWidth * theImage->mHeight;

	for (int i = 0; i < aSize; i++)
		aDstBits[i] = (aDstBits[i] & 0x00FFFFFF) | ((aSrcBits[i] & 0xFF) << 24);
}

static void ApplyAlphaAsImage(Image* theImage, uint32_t theBaseColor)
{
	uint32_t* aBits = theImage->mBits;
	const int aSize = theImage->mWidth * theImage->mHeight;

	for (int i = 0; i < aSize; i++)
		aBits[i] = theBaseColor | ((aBits[i] & 0xFF) << 24);
}

Image* ImageLib::GetImage(const std::string& theFilename, bool lookForAlphaImage)
{
	if (!gAutoLoadAlpha)
		lookForAlphaImage = false;

	const auto aLastSlashPos = theFilename.rfind('/');
	const auto aLastDotPos = theFilename.rfind('.');

	std::string_view anExt;
	std::string aFilename;

	if (aLastDotPos != std::string::npos &&
		(aLastSlashPos == std::string::npos || aLastDotPos > aLastSlashPos))
	{
		anExt = std::string_view(theFilename).substr(aLastDotPos);
		aFilename = theFilename.substr(0, aLastDotPos);
	}
	else
		aFilename = theFilename;

	// Load image, trying each supported format
	Image* anImage = TryLoadByExt(aFilename, anExt);

	// Probe alpha images with fast existence check
	Image* anAlphaImage = nullptr;
	if (lookForAlphaImage)
	{
		const auto slashEnd = (aLastSlashPos != std::string::npos) ? aLastSlashPos + 1 : 0;
		const std::string alphaPath1 = theFilename.substr(0, slashEnd) + "_" +
			theFilename.substr(slashEnd);

		if (FastFileExists(alphaPath1))
			anAlphaImage = GetImage(alphaPath1, false);

		if (!anAlphaImage)
		{
			const std::string alphaPath2 = theFilename + "_";
			if (FastFileExists(alphaPath2))
				anAlphaImage = GetImage(alphaPath2, false);
		}
	}

	// Compose alpha channel with image
	if (anAlphaImage)
	{
		if (anImage)
		{
			ComposeAlpha(anImage, anAlphaImage);
			delete anAlphaImage;
		}
		else
		{
			anImage = anAlphaImage;
			ApplyAlphaAsImage(anImage, static_cast<uint32_t>(gAlphaComposeColor));
		}
	}

	return anImage;
}
