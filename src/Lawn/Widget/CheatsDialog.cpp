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
#include "CheatsDialog.h"
#include "ControllerOptionsDialog.h"	// IsLocalizedUI
#include "../../ConstEnums.h"
#include "widget/Checkbox.h"
#include "../../PvzpLib/PvzpCommon.h"

using namespace Sexy;

CheatsDialog::CheatsDialog(LawnApp* theApp) :
	Dialog(nullptr, nullptr, Dialogs::DIALOG_CHEATS, true, "Cheats", "", "", Dialog::BUTTONS_NONE)
{
	mApp = theApp;
	SetColor(Dialog::COLOR_BUTTON_TEXT, Color(255, 255, 100));

	mFreePlantingCheckbox = MakeNewCheckbox(
		CheatsDialog::CheatsDialog_FreePlanting, this, theApp->mEasyPlantingCheat);
	mInfiniteSunCheckbox = MakeNewCheckbox(
		CheatsDialog::CheatsDialog_InfiniteSun, this, theApp->mInfiniteSunCheat);

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

CheatsDialog::~CheatsDialog()
{
	delete mFreePlantingCheckbox;
	delete mInfiniteSunCheckbox;
	delete mBackButton;
}

int CheatsDialog::GetPreferredHeight(int theWidth)
{
	(void)theWidth;
	return IMAGE_OPTIONS_MENUBACK->mWidth;
}

void CheatsDialog::AddedToManager(Sexy::WidgetManager* theWidgetManager)
{
	Dialog::AddedToManager(theWidgetManager);
	AddWidget(mFreePlantingCheckbox);
	AddWidget(mInfiniteSunCheckbox);
	AddWidget(mBackButton);
}

void CheatsDialog::RemovedFromManager(Sexy::WidgetManager* theWidgetManager)
{
	Dialog::RemovedFromManager(theWidgetManager);
	RemoveWidget(mFreePlantingCheckbox);
	RemoveWidget(mInfiniteSunCheckbox);
	RemoveWidget(mBackButton);
}

void CheatsDialog::Resize(int theX, int theY, int theWidth, int theHeight)
{
	Dialog::Resize(theX, theY, theWidth, theHeight);
	mFreePlantingCheckbox->Resize(284, 150, 46, 45);
	mInfiniteSunCheckbox->Resize(284, 200, 46, 45);
	mBackButton->Resize(30, 381, mBackButton->mWidth, mBackButton->mHeight);
}

void CheatsDialog::Draw(Sexy::Graphics* g)
{
	g->DrawImage(IMAGE_OPTIONS_MENUBACK, 0, 0);

	Sexy::Color aTextColor(107, 109, 145);
	bool aCN = IsLocalizedUI(mApp);
	PvzpDrawString(g, aCN ? "免费种植" : "Free planting", 274, 174, FONT_DWARVENTODCRAFT18,
				   aTextColor, DrawStringJustification::DS_ALIGN_RIGHT);
	PvzpDrawString(g, aCN ? "无限阳光" : "Infinite sun", 274, 224, FONT_DWARVENTODCRAFT18,
				   aTextColor, DrawStringJustification::DS_ALIGN_RIGHT);
}

void CheatsDialog::CheckboxChecked(int theId, bool checked)
{
	switch (theId)
	{
	case CheatsDialog::CheatsDialog_FreePlanting:
		mApp->mEasyPlantingCheat = checked;	// the game's own cheat: no cost, no cooldown
		break;
	case CheatsDialog::CheatsDialog_InfiniteSun:
		mApp->mInfiniteSunCheat = checked;
		break;
	}
}

void CheatsDialog::KeyDown(Sexy::KeyCode theKey)
{
	if (theKey == KeyCode::KEYCODE_SPACE || theKey == KeyCode::KEYCODE_RETURN ||
		theKey == KeyCode::KEYCODE_ESCAPE)
	{
		Dialog::ButtonDepress(Dialog::ID_OK);
	}
}

void CheatsDialog::ButtonPress(int theId)
{
	(void)theId;
	mApp->PlaySample(SOUND_GRAVEBUTTON);
}
