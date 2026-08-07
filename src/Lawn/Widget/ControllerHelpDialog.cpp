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
#include "ControllerHelpDialog.h"
#include "ControllerOptionsDialog.h"	// IsLocalizedUI
#include "../../ConstEnums.h"
#include "../../PvzpLib/PvzpCommon.h"

using namespace Sexy;

// The pak fonts only carry glyphs the stock game uses, so the Chinese wording
// is built from characters that appear in the game's own strings -- checked
// against a localized LawnStrings.txt -- and the buttons stay as letters.
struct HelpRow { const char* mButton; const char* mActionCN; const char* mActionEN; };
static const HelpRow kRows[] = {
	{ "D-PAD",  "移动光标",    "Move cursor"     },
	{ "STICK",  "移动光标",    "Move cursor"     },
	{ "A",      "种植 / 收集", "Plant / collect" },
	{ "B",      "铲子",        "Shovel"          },
	{ "X",      "商店 / 锤子", "Store / hammer"  },
	{ "Y",      "花园",        "Zen Garden"      },
	{ "L1  R1", "选择卡片",    "Seed packets"    },
	{ "R2  L3", "加速移动",    "Move faster"     },
	{ "START",  "暂停",        "Pause"           },
};

ControllerHelpDialog::ControllerHelpDialog(LawnApp* theApp) :
	Dialog(nullptr, nullptr, Dialogs::DIALOG_CONTROLLER_HELP, true, "Controls", "", "", Dialog::BUTTONS_NONE)
{
	mApp = theApp;
	SetColor(Dialog::COLOR_BUTTON_TEXT, Color(255, 255, 100));

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

ControllerHelpDialog::~ControllerHelpDialog()
{
	delete mBackButton;
}

int ControllerHelpDialog::GetPreferredHeight(int theWidth)
{
	(void)theWidth;
	return IMAGE_OPTIONS_MENUBACK->mWidth;
}

void ControllerHelpDialog::AddedToManager(Sexy::WidgetManager* theWidgetManager)
{
	Dialog::AddedToManager(theWidgetManager);
	AddWidget(mBackButton);
}

void ControllerHelpDialog::RemovedFromManager(Sexy::WidgetManager* theWidgetManager)
{
	Dialog::RemovedFromManager(theWidgetManager);
	RemoveWidget(mBackButton);
}

void ControllerHelpDialog::Resize(int theX, int theY, int theWidth, int theHeight)
{
	Dialog::Resize(theX, theY, theWidth, theHeight);
	mBackButton->Resize(30, 381, mBackButton->mWidth, mBackButton->mHeight);
}

void ControllerHelpDialog::Draw(Sexy::Graphics* g)
{
	g->DrawImage(IMAGE_OPTIONS_MENUBACK, 0, 0);

	Sexy::Color aButtonColor(70, 74, 110);
	Sexy::Color aTextColor(107, 109, 145);

	bool aCN = IsLocalizedUI(mApp);
	int aY = 116;
	for (const auto& aRow : kRows)
	{
		PvzpDrawString(g, aRow.mButton, 175, aY, FONT_DWARVENTODCRAFT18, aButtonColor,
					   DrawStringJustification::DS_ALIGN_RIGHT);
		PvzpDrawString(g, aCN ? aRow.mActionCN : aRow.mActionEN, 190, aY,
					   FONT_DWARVENTODCRAFT18, aTextColor,
					   DrawStringJustification::DS_ALIGN_LEFT);
		aY += 27;
	}
}

void ControllerHelpDialog::KeyDown(Sexy::KeyCode theKey)
{
	if (theKey == KeyCode::KEYCODE_SPACE || theKey == KeyCode::KEYCODE_RETURN ||
		theKey == KeyCode::KEYCODE_ESCAPE)
	{
		Dialog::ButtonDepress(Dialog::ID_OK);
	}
}

void ControllerHelpDialog::ButtonPress(int theId)
{
	(void)theId;
	mApp->PlaySample(SOUND_GRAVEBUTTON);
}

void ControllerHelpDialog::ButtonDepress(int theId)
{
	Dialog::ButtonDepress(theId);
	if (theId == Dialog::ID_OK)
		mApp->WriteToRegistry();	// remember that the controls have been shown
}
