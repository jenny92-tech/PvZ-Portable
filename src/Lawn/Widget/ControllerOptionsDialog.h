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

#ifndef __CONTROLLEROPTIONSDIALOG_H__
#define __CONTROLLEROPTIONSDIALOG_H__

#include "widget/Dialog.h"
#include "widget/SliderListener.h"
#include "widget/CheckboxListener.h"

class LawnApp;
class NewLawnButton;
namespace Sexy
{
	class Slider;
	class Checkbox;
};

class ControllerOptionsDialog : public Sexy::Dialog, public Sexy::SliderListener, public Sexy::CheckboxListener
{
protected:
	enum
	{
		ControllerOptionsDialog_Sensitivity,
		ControllerOptionsDialog_SunRadius,
		ControllerOptionsDialog_FreeCursor,
		ControllerOptionsDialog_FastForward,
		ControllerOptionsDialog_SwapXY,
	};

public:
	LawnApp*				mApp;
	Sexy::Slider*			mSensitivitySlider;
	Sexy::Slider*			mSunRadiusSlider;
	Sexy::Checkbox*			mFreeCursorCheckbox;
	Sexy::Checkbox*			mFastForwardCheckbox;
	Sexy::Checkbox*			mSwapXYCheckbox;
	NewLawnButton*			mBackButton;

public:
	explicit ControllerOptionsDialog(LawnApp* theApp);
	~ControllerOptionsDialog() override;

	int						GetPreferredHeight(int theWidth) override;
	void					AddedToManager(Sexy::WidgetManager* theWidgetManager) override;
	void					RemovedFromManager(Sexy::WidgetManager* theWidgetManager) override;
	void					Resize(int theX, int theY, int theWidth, int theHeight) override;
	void					Draw(Sexy::Graphics* g) override;
	void					SliderVal(int theId, double theVal) override;
	void					CheckboxChecked(int theId, bool checked) override;
	void					ButtonPress(int theId) override;
	void					ButtonDepress(int theId) override;
	void					KeyDown(Sexy::KeyCode theKey) override;
};

// True when the loaded game data is a localized (non-ASCII) build.
bool IsLocalizedUI(LawnApp* theApp);

#endif
