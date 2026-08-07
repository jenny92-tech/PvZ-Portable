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

#ifndef __CONTROLLERHELPDIALOG_H__
#define __CONTROLLERHELPDIALOG_H__

#include "widget/Dialog.h"

class LawnApp;
class NewLawnButton;
class LawnStoneButton;

// Lists what the gamepad buttons do. Shown once when a pad is first seen, and
// available from the controller settings after that.
class ControllerHelpDialog : public Sexy::Dialog
{
protected:
	enum
	{
		ControllerHelpDialog_Settings = 100,
	};

public:
	LawnApp*				mApp;
	LawnStoneButton*		mSettingsButton;	// only on the card shown by itself
	NewLawnButton*			mBackButton;

public:
	// theOfferSettings adds a way through to the controller settings, for the
	// card that appears on its own -- the one opened from those settings does
	// not, so the two cannot lead back and forth into each other.
	ControllerHelpDialog(LawnApp* theApp, bool theOfferSettings);
	~ControllerHelpDialog() override;

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
