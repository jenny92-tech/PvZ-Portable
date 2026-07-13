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

#include "GameButton.h"
#include "../LawnCommon.h"
#include "../../LawnApp.h"
#include "../../Resources.h"
#include "ControllerOptionsDialog.h"
#include "../../ConstEnums.h"
#include "widget/Slider.h"
#include "widget/Checkbox.h"
#include "../../PvzpLib/PvzpStringFile.h"
#include "../../PvzpLib/PvzpCommon.h"

using namespace Sexy;

static const double kSensMin = 0.5, kSensMax = 2.0;
static const double kRadMin = 60.0, kRadMax = 640.0;

ControllerOptionsDialog::ControllerOptionsDialog(LawnApp* theApp) :
	Dialog(nullptr, nullptr, Dialogs::DIALOG_CONTROLLER_OPTIONS, true, "Controller", "", "", Dialog::BUTTONS_NONE)
{
	mApp = theApp;
	SetColor(Dialog::COLOR_BUTTON_TEXT, Color(255, 255, 100));

	mSensitivitySlider = new Slider(IMAGE_OPTIONS_SLIDERSLOT, IMAGE_OPTIONS_SLIDERKNOB2,
		ControllerOptionsDialog::ControllerOptionsDialog_Sensitivity, this);
	mSensitivitySlider->SetValue((theApp->GetControllerSensitivity() - kSensMin) / (kSensMax - kSensMin));

	mSunRadiusSlider = new Slider(IMAGE_OPTIONS_SLIDERSLOT, IMAGE_OPTIONS_SLIDERKNOB2,
		ControllerOptionsDialog::ControllerOptionsDialog_SunRadius, this);
	mSunRadiusSlider->SetValue((theApp->GetControllerSunRadius() - kRadMin) / (kRadMax - kRadMin));

	mFreeCursorCheckbox = MakeNewCheckbox(
		ControllerOptionsDialog::ControllerOptionsDialog_FreeCursor, this, theApp->GetControllerFreeCursor());
	mFastForwardCheckbox = MakeNewCheckbox(
		ControllerOptionsDialog::ControllerOptionsDialog_FastForward, this, theApp->GetControllerCursorBoostEnabled());

	mBackButton = MakeNewButton(
		Dialog::ID_OK,
		this,
		"[DIALOG_BUTTON_OK]",
		nullptr,
		IMAGE_OPTIONS_BACKTOGAMEBUTTON0,
		IMAGE_OPTIONS_BACKTOGAMEBUTTON0,
		IMAGE_OPTIONS_BACKTOGAMEBUTTON2
	);
	mBackButton->mTextOffsetX = -2;
	mBackButton->mTextOffsetY = -5;
	mBackButton->mTextDownOffsetX = 0;
	mBackButton->mTextDownOffsetY = 1;
	mBackButton->SetFont(FONT_DWARVENTODCRAFT36GREENINSET);
	mBackButton->SetColor(ButtonWidget::COLOR_LABEL, Color::White);
	mBackButton->SetColor(ButtonWidget::COLOR_LABEL_HILITE, Color::White);
	mBackButton->mHiliteFont = FONT_DWARVENTODCRAFT36BRIGHTGREENINSET;
}

ControllerOptionsDialog::~ControllerOptionsDialog()
{
	delete mSensitivitySlider;
	delete mSunRadiusSlider;
	delete mFreeCursorCheckbox;
	delete mFastForwardCheckbox;
	delete mBackButton;
}

int ControllerOptionsDialog::GetPreferredHeight(int theWidth)
{
	(void)theWidth;
	return IMAGE_OPTIONS_MENUBACK->mWidth;
}

void ControllerOptionsDialog::AddedToManager(Sexy::WidgetManager* theWidgetManager)
{
	Dialog::AddedToManager(theWidgetManager);
	AddWidget(mSensitivitySlider);
	AddWidget(mSunRadiusSlider);
	AddWidget(mFreeCursorCheckbox);
	AddWidget(mFastForwardCheckbox);
	AddWidget(mBackButton);
}

void ControllerOptionsDialog::RemovedFromManager(Sexy::WidgetManager* theWidgetManager)
{
	Dialog::RemovedFromManager(theWidgetManager);
	RemoveWidget(mSensitivitySlider);
	RemoveWidget(mSunRadiusSlider);
	RemoveWidget(mFreeCursorCheckbox);
	RemoveWidget(mFastForwardCheckbox);
	RemoveWidget(mBackButton);
}

void ControllerOptionsDialog::Resize(int theX, int theY, int theWidth, int theHeight)
{
	Dialog::Resize(theX, theY, theWidth, theHeight);
	mSensitivitySlider->Resize(199, 116, 135, 40);
	mSunRadiusSlider->Resize(199, 143, 135, 40);
	mFreeCursorCheckbox->Resize(283, 175, 46, 45);
	mFastForwardCheckbox->Resize(284, 206, 46, 45);
	mBackButton->Resize(30, 381, mBackButton->mWidth, mBackButton->mHeight);
}

// True when the loaded game data is a localized (non-ASCII, e.g. Chinese) build,
// detected from an existing translated menu string.
bool IsLocalizedUI(LawnApp* theApp)
{
	std::string aMusic = theApp->GetString("OPTIONS_MUSIC_LABEL", "Music");
	for (unsigned char c : aMusic)
		if (c >= 0x80)
			return true;
	return false;
}

void ControllerOptionsDialog::Draw(Sexy::Graphics* g)
{
	g->DrawImage(IMAGE_OPTIONS_MENUBACK, 0, 0);
	Sexy::Color aTextColor(107, 109, 145);
	bool aCN = IsLocalizedUI(mApp);
	// The Chinese wording avoids 灵/敏/滑/键 -- the stock game never uses those
	// four glyphs so no pak font carries them; every other label glyph is present.
	PvzpDrawString(g, aCN ? "光标速度" : "Cursor Speed", 186, 140, FONT_DWARVENTODCRAFT18, aTextColor, DrawStringJustification::DS_ALIGN_RIGHT);
	PvzpDrawString(g, aCN ? "收集范围" : "Sun Range", 186, 167, FONT_DWARVENTODCRAFT18, aTextColor, DrawStringJustification::DS_ALIGN_RIGHT);
	PvzpDrawString(g, aCN ? "自由移动" : "Free Cursor", 274, 197, FONT_DWARVENTODCRAFT18, aTextColor, DrawStringJustification::DS_ALIGN_RIGHT);
	PvzpDrawString(g, aCN ? "R2/L3 二倍速" : "R2/L3 = 2x Cursor", 274, 229, FONT_DWARVENTODCRAFT18, aTextColor, DrawStringJustification::DS_ALIGN_RIGHT);
}

void ControllerOptionsDialog::SliderVal(int theId, double theVal)
{
	switch (theId)
	{
	case ControllerOptionsDialog::ControllerOptionsDialog_Sensitivity:
		mApp->SetControllerSensitivity((float)(kSensMin + theVal * (kSensMax - kSensMin)));
		break;
	case ControllerOptionsDialog::ControllerOptionsDialog_SunRadius:
		mApp->SetControllerSunRadius((float)(kRadMin + theVal * (kRadMax - kRadMin)));
		break;
	}
}

void ControllerOptionsDialog::CheckboxChecked(int theId, bool checked)
{
	switch (theId)
	{
	case ControllerOptionsDialog::ControllerOptionsDialog_FreeCursor:
		mApp->SetControllerFreeCursor(checked);
		break;
	case ControllerOptionsDialog::ControllerOptionsDialog_FastForward:
		mApp->SetControllerCursorBoostEnabled(checked);
		break;
	}
}

void ControllerOptionsDialog::KeyDown(Sexy::KeyCode theKey)
{
	if (theKey == KeyCode::KEYCODE_SPACE || theKey == KeyCode::KEYCODE_RETURN ||
		theKey == KeyCode::KEYCODE_ESCAPE)
	{
		Dialog::ButtonDepress(Dialog::ID_OK);
	}
}

void ControllerOptionsDialog::ButtonPress(int theId)
{
	(void)theId;
	mApp->PlaySample(SOUND_GRAVEBUTTON);
}

void ControllerOptionsDialog::ButtonDepress(int theId)
{
	Dialog::ButtonDepress(theId);
	if (theId == Dialog::ID_OK)
		mApp->WriteToRegistry();	// persist the controller settings on close
}
