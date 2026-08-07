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
 */

#ifndef __PORTOPTIONSDIALOG_H__
#define __PORTOPTIONSDIALOG_H__

#include "widget/Dialog.h"

class LawnApp;
class NewLawnButton;
class LawnStoneButton;

// Everything this port adds lives behind one entry in the game's options, so
// the original screen gains a single button no matter how much is added here.
class PortOptionsDialog : public Sexy::Dialog
{
protected:
	enum
	{
		PortOptionsDialog_Controller = 100,
		PortOptionsDialog_Cheats,
		PortOptionsDialog_Controls,
	};

public:
	LawnApp*				mApp;
	LawnStoneButton*		mControllerButton;
	LawnStoneButton*		mCheatsButton;
	LawnStoneButton*		mControlsButton;
	NewLawnButton*			mBackButton;

public:
	explicit PortOptionsDialog(LawnApp* theApp);
	~PortOptionsDialog() override;

	int						GetPreferredHeight(int theWidth) override;
	void					AddedToManager(Sexy::WidgetManager* theWidgetManager) override;
	void					RemovedFromManager(Sexy::WidgetManager* theWidgetManager) override;
	void					Resize(int theX, int theY, int theWidth, int theHeight) override;
	void					Draw(Sexy::Graphics* g) override;
	void					ButtonPress(int theId) override;
	void					ButtonDepress(int theId) override;
	void					KeyDown(Sexy::KeyCode theKey) override;
};

#endif
