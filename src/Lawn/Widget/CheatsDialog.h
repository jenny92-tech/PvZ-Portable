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

#ifndef __CHEATSDIALOG_H__
#define __CHEATSDIALOG_H__

#include "widget/Dialog.h"
#include "widget/CheckboxListener.h"

class LawnApp;
class NewLawnButton;
namespace Sexy
{
	class Checkbox;
};

// Cheats, kept out of the controller settings so neither screen crowds the
// other. Deliberately not saved: they last until the game is closed.
class CheatsDialog : public Sexy::Dialog, public Sexy::CheckboxListener
{
protected:
	enum
	{
		CheatsDialog_FreePlanting,
		CheatsDialog_InfiniteSun,
	};

public:
	LawnApp*				mApp;
	Sexy::Checkbox*			mFreePlantingCheckbox;
	Sexy::Checkbox*			mInfiniteSunCheckbox;
	NewLawnButton*			mBackButton;

public:
	explicit CheatsDialog(LawnApp* theApp);
	~CheatsDialog() override;

	int						GetPreferredHeight(int theWidth) override;
	void					AddedToManager(Sexy::WidgetManager* theWidgetManager) override;
	void					RemovedFromManager(Sexy::WidgetManager* theWidgetManager) override;
	void					Resize(int theX, int theY, int theWidth, int theHeight) override;
	void					Draw(Sexy::Graphics* g) override;
	void					CheckboxChecked(int theId, bool checked) override;
	void					ButtonPress(int theId) override;
	void					KeyDown(Sexy::KeyCode theKey) override;
};

#endif
