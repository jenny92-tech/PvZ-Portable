/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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

#include "PakMemoryMap.h"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

bool PakMemoryMap::MapFile(FILE* theFile, size_t theSize)
{
	if (theFile == nullptr || theSize == 0)
		return false;

#ifdef _WIN32
	HANDLE aFileHandle = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(theFile)));
	if (aFileHandle == INVALID_HANDLE_VALUE)
		return false;

	// The view keeps the file referenced, so neither handle needs to outlive this call.
	HANDLE aMapping = CreateFileMappingA(aFileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
	if (aMapping == nullptr)
		return false;

	void* aView = MapViewOfFile(aMapping, FILE_MAP_READ, 0, 0, theSize);
	CloseHandle(aMapping);
	if (aView == nullptr)
		return false;

	mDataPtr = static_cast<const uint8_t*>(aView);
	mSize = theSize;
	return true;
#else
	int aFd = dup(fileno(theFile));
	if (aFd < 0)
		return false;

	void* aView = mmap(nullptr, theSize, PROT_READ, MAP_PRIVATE, aFd, 0);
	if (aView == MAP_FAILED)
	{
		close(aFd);
		return false;
	}

	// Best-effort shared lock so the archive isn't modified from under the
	// mapping (a truncated file would SIGBUS on access). Not fatal if the
	// platform doesn't support it (e.g. Emscripten).
	struct flock aLock = {};
	aLock.l_type = F_RDLCK;
	aLock.l_whence = SEEK_SET;
	fcntl(aFd, F_SETLK, &aLock);

	mDataPtr = static_cast<const uint8_t*>(aView);
	mSize = theSize;
	mFd = aFd;
	return true;
#endif
}

void PakMemoryMap::Unmap()
{
	if (mDataPtr == nullptr)
		return;

#ifdef _WIN32
	UnmapViewOfFile(const_cast<uint8_t*>(mDataPtr));
#else
	munmap(const_cast<uint8_t*>(mDataPtr), mSize);
	close(mFd);
	mFd = -1;
#endif

	mDataPtr = nullptr;
	mSize = 0;
}
