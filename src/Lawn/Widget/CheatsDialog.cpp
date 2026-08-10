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
#include "../Board.h"
#include "../System/PlayerInfo.h"

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
	mSlowMoCheckbox = MakeNewCheckbox(
		CheatsDialog::CheatsDialog_SlowMo, this, theApp->mSlowMoCheat);

	bool aCN = IsLocalizedUI(theApp);
	mCoinsButton = MakeButton(CheatsDialog::CheatsDialog_Coins, this,
		aCN ? "增加金币" : "Add coins");
	// Red: this one advances adventure progress and cannot be taken back.
	mWinLevelButton = MakeButton(CheatsDialog::CheatsDialog_WinLevel, this,
		aCN ? "立即过关" : "Win level");
	mWinLevelButton->mLabelColor = Color(255, 120, 120);

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
	delete mSlowMoCheckbox;
	delete mCoinsButton;
	delete mWinLevelButton;
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
	AddWidget(mSlowMoCheckbox);
	AddWidget(mCoinsButton);
	AddWidget(mWinLevelButton);
	AddWidget(mBackButton);
}

void CheatsDialog::RemovedFromManager(Sexy::WidgetManager* theWidgetManager)
{
	Dialog::RemovedFromManager(theWidgetManager);
	RemoveWidget(mFreePlantingCheckbox);
	RemoveWidget(mInfiniteSunCheckbox);
	RemoveWidget(mSlowMoCheckbox);
	RemoveWidget(mCoinsButton);
	RemoveWidget(mWinLevelButton);
	RemoveWidget(mBackButton);
}

void CheatsDialog::Resize(int theX, int theY, int theWidth, int theHeight)
{
	Dialog::Resize(theX, theY, theWidth, theHeight);
	mFreePlantingCheckbox->Resize(284, 132, 46, 45);
	mInfiniteSunCheckbox->Resize(284, 172, 46, 45);
	mSlowMoCheckbox->Resize(284, 212, 46, 45);
	mCoinsButton->Resize(107, 262, 209, 46);
	mWinLevelButton->Resize(107, 318, 209, 46);
	mBackButton->Resize(30, 381, mBackButton->mWidth, mBackButton->mHeight);
}

void CheatsDialog::Draw(Sexy::Graphics* g)
{
	g->DrawImage(IMAGE_OPTIONS_MENUBACK, 0, 0);

	Sexy::Color aTextColor(107, 109, 145);
	bool aCN = IsLocalizedUI(mApp);
	PvzpDrawString(g, aCN ? "免费种植" : "Free planting", 274, 156, FONT_DWARVENTODCRAFT18,
				   aTextColor, DrawStringJustification::DS_ALIGN_RIGHT);
	PvzpDrawString(g, aCN ? "无限阳光" : "Infinite sun", 274, 196, FONT_DWARVENTODCRAFT18,
				   aTextColor, DrawStringJustification::DS_ALIGN_RIGHT);
	PvzpDrawString(g, aCN ? "慢动作" : "Slow motion", 274, 236, FONT_DWARVENTODCRAFT18,
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
	case CheatsDialog::CheatsDialog_SlowMo:
		mApp->mSlowMoCheat = checked;
		mApp->mSlowMoCheatCounter = 0;
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

void CheatsDialog::ButtonDepress(int theId)
{
	switch (theId)
	{
	case CheatsDialog::CheatsDialog_Coins:
		mApp->mPlayerInfo->AddCoins(100);
		if (mApp->mBoard != nullptr)
			mApp->mBoard->ShowCoinBank();
		return;
	case CheatsDialog::CheatsDialog_WinLevel:
		// Only ask for it here; Board::Update ends the level once every dialog
		// has closed, so nothing is left holding a board that is going away.
		if (mApp->mBoard != nullptr && mApp->mGameScene == GameScenes::SCENE_PLAYING)
			mApp->mWantWinLevel = true;
		Dialog::ButtonDepress(Dialog::ID_OK);
		return;
	}

	Dialog::ButtonDepress(theId);
}

void CheatsDialog::ButtonPress(int theId)
{
	(void)theId;
	mApp->PlaySample(SOUND_GRAVEBUTTON);
}
