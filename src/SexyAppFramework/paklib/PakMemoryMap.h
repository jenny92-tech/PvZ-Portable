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

#ifndef __PAKMEMORYMAP_H__
#define __PAKMEMORYMAP_H__

#include <cstdint>
#include <cstdio>

// Read-only memory mapping of an open file. Lets pak archives be accessed
// straight from the page cache instead of being copied into the heap, which
// matters on low-memory devices (PortMaster handhelds).
class PakMemoryMap
{
public:
	PakMemoryMap() {}
	~PakMemoryMap() { Unmap(); }

	PakMemoryMap(const PakMemoryMap&) = delete;
	PakMemoryMap& operator=(const PakMemoryMap&) = delete;

	// Maps theSize bytes of theFile. The caller keeps ownership of theFile and
	// may close it afterwards. Returns false if mapping is unavailable; the
	// caller is expected to fall back to reading the file into memory.
	bool					MapFile(FILE* theFile, size_t theSize);
	void					Unmap();

	const uint8_t*			GetDataPtr() const { return mDataPtr; }
	size_t					GetSize() const { return mSize; }

private:
	const uint8_t*			mDataPtr = nullptr;
	size_t					mSize = 0;
#ifndef _WIN32
	int						mFd = -1;				// dup'd descriptor, held to keep the advisory lock alive
#endif
};

#endif //__PAKMEMORYMAP_H__
