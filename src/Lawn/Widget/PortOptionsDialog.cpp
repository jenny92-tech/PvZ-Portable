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
#include "PortOptionsDialog.h"
#include "ControllerOptionsDialog.h"
#include "ControllerHelpDialog.h"
#include "CheatsDialog.h"
#include "../../ConstEnums.h"
#include "../../PvzpLib/PvzpCommon.h"

using namespace Sexy;

PortOptionsDialog::PortOptionsDialog(LawnApp* theApp) :
	Dialog(nullptr, nullptr, Dialogs::DIALOG_PORT_OPTIONS, true, "Port", "", "", Dialog::BUTTONS_NONE)
{
	mApp = theApp;
	SetColor(Dialog::COLOR_BUTTON_TEXT, Color(255, 255, 100));

	bool aCN = IsLocalizedUI(theApp);
	mControllerButton = MakeButton(PortOptionsDialog::PortOptionsDialog_Controller, this,
		aCN ? "手柄设置" : "Controller");
	mCheatsButton = MakeButton(PortOptionsDialog::PortOptionsDialog_Cheats, this,
		aCN ? "作弊设置" : "Cheats");
	mControlsButton = MakeButton(PortOptionsDialog::PortOptionsDialog_Controls, this,
		aCN ? "操作说明" : "Controls");

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

PortOptionsDialog::~PortOptionsDialog()
{
	delete mControllerButton;
	delete mCheatsButton;
	delete mControlsButton;
	delete mBackButton;
}

int PortOptionsDialog::GetPreferredHeight(int theWidth)
{
	(void)theWidth;
	return IMAGE_OPTIONS_MENUBACK->mWidth;
}

void PortOptionsDialog::AddedToManager(Sexy::WidgetManager* theWidgetManager)
{
	Dialog::AddedToManager(theWidgetManager);
	AddWidget(mControllerButton);
	AddWidget(mCheatsButton);
	AddWidget(mControlsButton);
	AddWidget(mBackButton);
}

void PortOptionsDialog::RemovedFromManager(Sexy::WidgetManager* theWidgetManager)
{
	Dialog::RemovedFromManager(theWidgetManager);
	RemoveWidget(mControllerButton);
	RemoveWidget(mCheatsButton);
	RemoveWidget(mControlsButton);
	RemoveWidget(mBackButton);
}

void PortOptionsDialog::Resize(int theX, int theY, int theWidth, int theHeight)
{
	Dialog::Resize(theX, theY, theWidth, theHeight);
	mControllerButton->Resize(107, 170, 209, 46);
	mCheatsButton->Resize(107, 225, 209, 46);
	mControlsButton->Resize(107, 280, 209, 46);
	mBackButton->Resize(30, 381, mBackButton->mWidth, mBackButton->mHeight);
}

void PortOptionsDialog::Draw(Sexy::Graphics* g)
{
	g->DrawImage(IMAGE_OPTIONS_MENUBACK, 0, 0);
}

void PortOptionsDialog::KeyDown(Sexy::KeyCode theKey)
{
	if (theKey == KeyCode::KEYCODE_SPACE || theKey == KeyCode::KEYCODE_RETURN ||
		theKey == KeyCode::KEYCODE_ESCAPE)
	{
		Dialog::ButtonDepress(Dialog::ID_OK);
	}
}

void PortOptionsDialog::ButtonPress(int theId)
{
	(void)theId;
	mApp->PlaySample(SOUND_GRAVEBUTTON);
}

void PortOptionsDialog::ButtonDepress(int theId)
{
	switch (theId)
	{
	case PortOptionsDialog::PortOptionsDialog_Controller:
	{
		ControllerOptionsDialog* aDialog = mApp->DoControllerOptionsDialog();
		aDialog->WaitForResult(true);
		mApp->KillDialog(Dialogs::DIALOG_CONTROLLER_OPTIONS);
		return;
	}
	case PortOptionsDialog::PortOptionsDialog_Cheats:
	{
		CheatsDialog* aDialog = mApp->DoCheatsDialog();
		aDialog->WaitForResult(true);
		mApp->KillDialog(Dialogs::DIALOG_CHEATS);
		return;
	}
	case PortOptionsDialog::PortOptionsDialog_Controls:
	{
		ControllerHelpDialog* aDialog = mApp->DoControllerHelpDialog(false);
		aDialog->WaitForResult(true);
		mApp->KillDialog(Dialogs::DIALOG_CONTROLLER_HELP);
		return;
	}
	}

	Dialog::ButtonDepress(theId);
	if (theId == Dialog::ID_OK)
		mApp->WriteToRegistry();	// persist whatever the sub-screens changed
}
