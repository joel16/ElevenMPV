#include <kernel.h>
#include <appmgr.h>
#include <stdlib.h>
#include <string.h>
#include <shellaudio.h>
#include <audioout.h>

#include "config.h"
#include "motion_e.h"
#include "common.h"
#include "menu_settings.h"
#include "utils.h"

using namespace paf;

static SceUInt32 s_callerMode = 0;

static SceBool s_isBtCbRegistered = SCE_FALSE;
static SceBool s_needPageReload = SCE_FALSE;
static SceBool s_needCwdReload = SCE_FALSE;
static SceBool s_needBackButtonShow = SCE_FALSE;

static widget::Widget *s_currPlane = SCE_NULL;
static widget::Widget *s_prevPlane = SCE_NULL;

static widget::Widget *s_settingsButtonDevice = SCE_NULL;
static widget::Widget *s_settingsButtonSort = SCE_NULL;
static widget::Widget *s_settingsButtonAudio = SCE_NULL;
static widget::Widget *s_settingsButtonPower = SCE_NULL;
static widget::Widget *s_settingsButtonControl = SCE_NULL;

static widget::Widget *s_settingsButtonAudioEq = SCE_NULL;
static widget::CheckBox *s_settingsCheckboxAudioAlc = SCE_NULL;
static widget::CheckBox *s_settingsCheckboxAudioLimit = SCE_NULL;
static widget::CheckBox *s_settingsCheckboxPowerSave = SCE_NULL;
static widget::Widget *s_settingsImePowerTime = SCE_NULL;
static widget::CheckBox *s_settingsCheckboxControlStick = SCE_NULL;
static widget::CheckBox *s_settingsCheckboxControlMotion = SCE_NULL;
static widget::Widget *s_settingsImeControlTime = SCE_NULL;
static widget::Widget *s_settingsImeControlAngle = SCE_NULL;

static SceUInt32 s_callerButtonHash = 0;

const SceUInt32 k_deviceItemNum = 6;
const SceUInt32 k_sortItemNum = 4;
const SceUInt32 k_audioEqItemNum = 5;

SceVoid menu::settings::SettingsOptionSelectionButtonCB::SettingsOptionSelectionButtonCBFun(SceInt32 eventId, paf::widget::Widget *self, SceInt32 a3, ScePVoid pUserData)
{
	SceBool needBgButtonReset = SCE_TRUE;
	String *text8 = SCE_NULL;
	Resource::Element searchParam;
	config::Config::EMPVAConfig *config = g_config->GetConfigLocation();

	searchParam.hash = EMPVAUtils::GetHash("settings_option_box");
	widget::Widget *settingsOptionBox = g_settings_option->GetChildByHash(&searchParam, 0);

	widget::Widget *targetOption = settingsOptionBox->GetChildByNum(self->hash - 1);
	targetOption->SetTextureBase(g_texCheckMark);

	for (int i = 0; i < settingsOptionBox->childNum; i++) {
		if (i != self->hash - 1) {
			targetOption = settingsOptionBox->GetChildByNum(i);
			targetOption->SetTextureBase(g_texTransparent);
		}
	}

	switch (s_callerButtonHash) {
	case menu::settings::SettingsOptionButtonCB::ButtonHash_Device:

		text8 = String::WCharToNewString(EMPVAUtils::GetStringWithNum("msg_option_device_", self->hash - 1), text8);

		if (io::Misc::Exists(text8->data)) {
			if (config->device != self->hash - 1) {
				s_needPageReload = SCE_TRUE;
				s_needCwdReload = SCE_TRUE;
			}
			config->device = self->hash - 1;
		}

		text8->Clear();
		delete text8;

		break;
	case menu::settings::SettingsOptionButtonCB::ButtonHash_Sort:

		if (config->sort != self->hash - 1)
			s_needPageReload = SCE_TRUE;
		config->sort = self->hash - 1;

		break;
	case menu::settings::SettingsOptionButtonCB::ButtonHash_Audio_Eq:

		config->eq_mode = self->hash - 1;

		if (g_isPlayerActive) {
			if (EMPVAUtils::IsDecoderUsed())
				sceAudioOutSetEffectType(config->eq_mode);
			else
				sceMusicPlayerServiceSetEQ(config->eq_mode);
		}

		needBgButtonReset = SCE_FALSE;

		break;
	default:
		break;
	}

	g_config->Save();

	g_commonOptionDialog->PlayAnimationReverse(0.0f, widget::Widget::Animation_3D_SlideFromFront, SCE_NULL);
	common::Utils::WidgetStateTransition(0.0f, settingsOptionBox, widget::Widget::Animation_Reset, SCE_TRUE, SCE_TRUE); // TODO find nice looking delay

	if (needBgButtonReset) {
		searchParam.hash = EMPVAUtils::GetHash("settings_scroll_view");
		widget::Widget *settingsScrollView = g_settings_page->GetChildByHash(&searchParam, 0);
		settingsScrollView->SetAlpha(1.0f);

		searchParam.hash = EMPVAUtils::GetHash("settings_close_button");
		widget::Widget *settingsCloseButton = g_settings_page->GetChildByHash(&searchParam, 0);
		settingsCloseButton->SetAlpha(1.0f);
	}
	else {
		searchParam.hash = EMPVAUtils::GetHash("plane_settings_audio_bg");
		widget::Widget *settingsAudioPlane = g_settings_page->GetChildByHash(&searchParam, 0);

		searchParam.hash = EMPVAUtils::GetHash("settings_audio_scroll_view");
		widget::Widget *settingsScrollView = settingsAudioPlane->GetChildByHash(&searchParam, 0);
		settingsScrollView->SetAlpha(1.0f);

		searchParam.hash = EMPVAUtils::GetHash("settings_back_button");
		widget::Widget *settingsBackButton = g_settings_page->GetChildByHash(&searchParam, 0);
		settingsBackButton->SetAlpha(1.0f);
	}

	menu::settings::SettingsButtonCB::RefreshButtonText();
}

SceVoid menu::settings::SettingsOptionButtonCB::SettingsOptionButtonCBFun(SceInt32 eventId, paf::widget::Widget *self, SceInt32 a3, ScePVoid pUserData)
{
	SceBool needOptionDialog = SCE_FALSE;
	WString text16;
	Resource::Element searchParam;
	Plugin::TemplateInitParam tmpParam;
	config::Config::EMPVAConfig *config = g_config->GetConfigLocation();
	widget::ImageButton *currButton;
	menu::settings::SettingsOptionSelectionButtonCB *selectionBtCb;
	widget::Widget *settingsOptionBox;
	widget::Widget *settingsAudioPlane;
	widget::Widget *settingsPowerPlane;
	widget::Widget *settingsControlPlane;
	widget::Widget *settingsBackButton;
	widget::Widget *settingsScrollView;

	switch (self->hash) {
	case ButtonHash_Device:
	case ButtonHash_Sort:
	case ButtonHash_Audio_Eq:
		needOptionDialog = SCE_TRUE;
		break;
	default:
		break;
	}

	searchParam.hash = EMPVAUtils::GetHash("settings_scroll_view");
	settingsScrollView = g_settings_page->GetChildByHash(&searchParam, 0);
	settingsScrollView->SetAlpha(0.39f);

	searchParam.hash = EMPVAUtils::GetHash("settings_close_button");
	widget::Widget *settingsCloseButton = g_settings_page->GetChildByHash(&searchParam, 0);
	settingsCloseButton->SetAlpha(0.39f);
	
	if (needOptionDialog) {
		g_commonOptionDialog->PlayAnimation(0.0f, widget::Widget::Animation_3D_SlideFromFront, SCE_NULL);

		searchParam.hash = EMPVAUtils::GetHash("dialog_settings_option");
		widget::Widget *settingsOptionDialog = g_settings_option->GetChildByHash(&searchParam, 0);

		searchParam.hash = EMPVAUtils::GetHash("settings_template_option_box");
		g_empvaPlugin->AddWidgetFromTemplate(settingsOptionDialog, &searchParam, &tmpParam);

		searchParam.hash = EMPVAUtils::GetHash("settings_option_box");
		settingsOptionBox = settingsOptionDialog->GetChildByHash(&searchParam, 0);

		selectionBtCb = new menu::settings::SettingsOptionSelectionButtonCB();

		searchParam.hash = EMPVAUtils::GetHash("settings_template_option_button");
	}

	switch (self->hash) {
	case ButtonHash_Device:
		
		for (int i = 0; i < k_deviceItemNum; i++) {
			g_empvaPlugin->AddWidgetFromTemplate(settingsOptionBox, &searchParam, &tmpParam);

			currButton = (widget::ImageButton *)settingsOptionBox->GetChildByNum(settingsOptionBox->childNum - 1);
			currButton->hash = i + 1;
			currButton->RegisterEventCallback(0x10000008, selectionBtCb, 0);

			text16.Set(EMPVAUtils::GetStringWithNum("msg_option_device_", i));
			currButton->SetLabel(&text16);
			text16.Clear();

			if (i == config->device)
				currButton->SetTextureBase(g_texCheckMark);
		}

		break;
	case ButtonHash_Sort:

		for (int i = 0; i < k_sortItemNum; i++) {
			g_empvaPlugin->AddWidgetFromTemplate(settingsOptionBox, &searchParam, &tmpParam);

			currButton = (widget::ImageButton *)settingsOptionBox->GetChildByNum(settingsOptionBox->childNum - 1);
			currButton->hash = i + 1;
			currButton->RegisterEventCallback(0x10000008, selectionBtCb, 0);

			text16.Set(EMPVAUtils::GetStringWithNum("msg_option_sort_", i));
			currButton->SetLabel(&text16);
			text16.Clear();

			if (i == config->sort)
				currButton->SetTextureBase(g_texCheckMark);
		}

		break;
	case ButtonHash_Audio:
		
		searchParam.hash = EMPVAUtils::GetHash("plane_settings_audio_bg");
		settingsAudioPlane = g_settings_page->GetChildByHash(&searchParam, 0);
		settingsAudioPlane->PlayAnimation(0.0f, widget::Widget::Animation_3D_SlideFromFront, SCE_NULL);

		searchParam.hash = EMPVAUtils::GetHash("settings_back_button");
		settingsBackButton = g_settings_page->GetChildByHash(&searchParam, 0);
		settingsBackButton->PlayAnimation(0.0f, widget::Widget::Animation_Reset, SCE_NULL);

		s_prevPlane = s_currPlane;
		s_currPlane = settingsAudioPlane;

		break;
	case ButtonHash_Power:

		searchParam.hash = EMPVAUtils::GetHash("plane_settings_power_bg");
		settingsPowerPlane = g_settings_page->GetChildByHash(&searchParam, 0);
		settingsPowerPlane->PlayAnimation(0.0f, widget::Widget::Animation_3D_SlideFromFront, SCE_NULL);

		searchParam.hash = EMPVAUtils::GetHash("settings_back_button");
		settingsBackButton = g_settings_page->GetChildByHash(&searchParam, 0);
		settingsBackButton->PlayAnimation(0.0f, widget::Widget::Animation_Reset, SCE_NULL);

		s_prevPlane = s_currPlane;
		s_currPlane = settingsPowerPlane;

		break;
	case ButtonHash_Control:

		searchParam.hash = EMPVAUtils::GetHash("plane_settings_control_bg");
		settingsControlPlane = g_settings_page->GetChildByHash(&searchParam, 0);
		settingsControlPlane->PlayAnimation(0.0f, widget::Widget::Animation_3D_SlideFromFront, SCE_NULL);

		searchParam.hash = EMPVAUtils::GetHash("settings_back_button");
		settingsBackButton = g_settings_page->GetChildByHash(&searchParam, 0);
		settingsBackButton->PlayAnimation(0.0f, widget::Widget::Animation_Reset, SCE_NULL);

		s_prevPlane = s_currPlane;
		s_currPlane = settingsControlPlane;

		break;

	case ButtonHash_Audio_Eq:

		for (int i = 0; i < k_audioEqItemNum; i++) {
			g_empvaPlugin->AddWidgetFromTemplate(settingsOptionBox, &searchParam, &tmpParam);

			currButton = (widget::ImageButton *)settingsOptionBox->GetChildByNum(settingsOptionBox->childNum - 1);
			currButton->hash = i + 1;
			currButton->RegisterEventCallback(0x10000008, selectionBtCb, 0);

			text16.Set(EMPVAUtils::GetStringWithNum("msg_option_audio_eq_", i));
			currButton->SetLabel(&text16);
			text16.Clear();

			if (i == config->eq_mode)
				currButton->SetTextureBase(g_texCheckMark);
		}

		searchParam.hash = EMPVAUtils::GetHash("settings_back_button");
		settingsBackButton = g_settings_page->GetChildByHash(&searchParam, 0);
		settingsBackButton->SetAlpha(0.39f);

		searchParam.hash = EMPVAUtils::GetHash("settings_audio_scroll_view");
		settingsScrollView = s_currPlane->GetChildByHash(&searchParam, 0);
		settingsScrollView->SetAlpha(0.39f);

		break;
	case ButtonHash_Audio_Alc:

		config->alc_mode = s_settingsCheckboxAudioAlc->checked;
		g_config->Save();

		if (g_isPlayerActive) {
			if (EMPVAUtils::IsDecoderUsed())
				sceAudioOutSetAlcMode(config->alc_mode);
			else
				sceMusicPlayerServiceSetALC(config->alc_mode);
		}

		break;
	case ButtonHash_Audio_Limit:

		config->eq_volume = s_settingsCheckboxAudioLimit->checked;
		g_config->Save();

		break;
	case ButtonHash_Power_Save:

		config->power_saving = s_settingsCheckboxPowerSave->checked;
		g_config->Save();

		if (config->power_saving)
			s_settingsImePowerTime->SetAlpha(1.0f);
		else
			s_settingsImePowerTime->SetAlpha(0.39f);

		break;
	case ButtonHash_Control_Stick:

		config->stick_skip = s_settingsCheckboxControlStick->checked;
		g_config->Save();

		break;
	case ButtonHash_Control_Motion:

		config->motion_mode = s_settingsCheckboxControlMotion->checked;
		g_config->Save();

		if (config->motion_mode) {
			s_settingsImeControlTime->SetAlpha(1.0f);
			s_settingsImeControlAngle->SetAlpha(1.0f);
		}
		else {
			s_settingsImeControlTime->SetAlpha(0.39f);
			s_settingsImeControlAngle->SetAlpha(0.39f);
		}

		if (g_isPlayerActive)
			motion::Motion::SetState(SCE_TRUE);

		break;
	case ButtonHash_Power_Time:

		s_settingsImePowerTime->GetLabel(&text16);
		if (text16.data[0] == 0) {
			text16.Clear();
			text16.Set((SceWChar16 *)L"1");
			s_settingsImePowerTime->SetLabel(&text16);
		}
		if (text16.length > 1) {
			char *tmpRef = (char *)text16.data;
			tmpRef[1] = tmpRef[2];
			tmpRef[2] = 0;
		}
		config->power_timer = sce_paf_atoi((char *)text16.data);
		text16.Clear();
		g_config->Save();

		break;
	case ButtonHash_Control_Time:

		s_settingsImeControlTime->GetLabel(&text16);
		if (text16.data[0] == 0) {
			text16.Clear();
			text16.Set((SceWChar16 *)L"1");
			s_settingsImeControlTime->SetLabel(&text16);
		}
		if (text16.length > 1) {
			char *tmpRef = (char *)text16.data;
			tmpRef[1] = tmpRef[2];
			tmpRef[2] = 0;
		}
		config->motion_timer = sce_paf_atoi((char *)text16.data);
		text16.Clear();
		g_config->Save();

		if (g_isPlayerActive)
			motion::Motion::SetReleaseTimer(config->motion_timer);

		break;
	case ButtonHash_Control_Angle:

		s_settingsImeControlAngle->GetLabel(&text16);
		if (text16.data[0] == 0) {
			text16.Clear();
			text16.Set((SceWChar16 *)L"1");
			s_settingsImeControlAngle->SetLabel(&text16);
		}
		if (text16.length > 1) {
			char *tmpRef = (char *)text16.data;
			tmpRef[1] = tmpRef[2];
			tmpRef[2] = 0;
		}
		config->motion_degree = sce_paf_atoi((char *)text16.data);
		text16.Clear();
		g_config->Save();

		if (g_isPlayerActive)
			motion::Motion::SetAngleThreshold(config->motion_degree);

		break;
	}

	s_callerButtonHash = self->hash;
}

SceVoid menu::settings::SettingsButtonCB::RefreshButtonText()
{
	SceWChar16 buf16[10];
	WString text16;
	Resource::Element searchParam;
	widget::Widget *buttonValue;
	config::Config::EMPVAConfig *config = g_config->GetConfigLocation();

	searchParam.hash = EMPVAUtils::GetHash("settings_device_button_text_value");
	buttonValue = s_settingsButtonDevice->GetChildByHash(&searchParam, 0);
	text16.Set(EMPVAUtils::GetStringWithNum("msg_option_device_", config->device));
	buttonValue->SetLabel(&text16);
	text16.Clear();

	searchParam.hash = EMPVAUtils::GetHash("settings_sort_button_text_value");
	buttonValue = s_settingsButtonSort->GetChildByHash(&searchParam, 0);
	text16.Set(EMPVAUtils::GetStringWithNum("msg_option_sort_", config->sort));
	buttonValue->SetLabel(&text16);
	text16.Clear();

	searchParam.hash = EMPVAUtils::GetHash("settings_audio_eq_button_text_value");
	buttonValue = s_settingsButtonAudioEq->GetChildByHash(&searchParam, 0);
	text16.Set(EMPVAUtils::GetStringWithNum("msg_option_audio_eq_", config->eq_mode));
	buttonValue->SetLabel(&text16);
	text16.Clear();

	s_settingsCheckboxAudioAlc->SetChecked(0.0f, config->alc_mode, 0);

	s_settingsCheckboxAudioLimit->SetChecked(0.0f, config->eq_volume, 0);

	s_settingsCheckboxPowerSave->SetChecked(0.0f, config->power_saving, 0);

	s_settingsCheckboxControlStick->SetChecked(0.0f, config->stick_skip, 0);

	s_settingsCheckboxControlMotion->SetChecked(0.0f, config->motion_mode, 0);

	sce_paf_swprintf((wchar_t *)buf16, 10, L"%u", config->power_timer);
	text16.Set(buf16);
	s_settingsImePowerTime->SetLabel(&text16);
	text16.Clear();

	sce_paf_swprintf((wchar_t *)buf16, 10, L"%u", config->motion_timer);
	text16.Set(buf16);
	s_settingsImeControlTime->SetLabel(&text16);
	text16.Clear();

	sce_paf_swprintf((wchar_t *)buf16, 10, L"%u", config->motion_degree);
	text16.Set(buf16);
	s_settingsImeControlAngle->SetLabel(&text16);
	text16.Clear();
}

SceVoid menu::settings::SettingsBackButtonCB::SettingsBackButtonCBFun(SceInt32 eventId, paf::widget::Widget *self, SceInt32 a3, ScePVoid pUserData)
{
	Resource::Element searchParam;

	s_currPlane->PlayAnimationReverse(0.0f, widget::Widget::Animation_3D_SlideFromFront, SCE_NULL);
	s_prevPlane->PlayAnimation(0.0f, widget::Widget::Animation_3D_SlideFromFront, SCE_NULL);
	s_currPlane = s_prevPlane;

	searchParam.hash = EMPVAUtils::GetHash("settings_back_button");
	widget::Widget *settingsBackButton = g_settings_page->GetChildByHash(&searchParam, 0);
	settingsBackButton->PlayAnimationReverse(0.0f, widget::Widget::Animation_Reset, SCE_NULL);

	searchParam.hash = EMPVAUtils::GetHash("settings_scroll_view");
	widget::Widget *settingsScrollView = g_settings_page->GetChildByHash(&searchParam, 0);
	settingsScrollView->SetAlpha(1.0f);

	searchParam.hash = EMPVAUtils::GetHash("settings_close_button");
	widget::Widget *settingsCloseButton = g_settings_page->GetChildByHash(&searchParam, 0);
	settingsCloseButton->SetAlpha(1.0f);
}

SceVoid menu::settings::SettingsButtonCB::SettingsButtonCBFun(SceInt32 eventId, paf::widget::Widget *self, SceInt32 a3, ScePVoid pUserData)
{
	SceFVector4 pos;
	Resource::Element searchParam;
	Plugin::SceneInitParam rwiParam;
	widget::Button *settingsButton;
	widget::Button *backButton;
	widget::Widget *rootPlane;
	widget::Widget *settingsScrollView;
	config::Config::EMPVAConfig *config = g_config->GetConfigLocation();

	s_callerMode = *(SceUInt32 *)pUserData;

	switch (s_callerMode) {
	case Parent_Displayfiles:

		// Hide (disable) back/settings displayfiles buttons
		searchParam.hash = EMPVAUtils::GetHash("displayfiles_settings_button");
		settingsButton = (widget::Button *)g_root_page->GetChildByHash(&searchParam, 0);
		settingsButton->PlayAnimationReverse(1000.0f, widget::Widget::Animation_Fadein1, SCE_NULL);

		searchParam.hash = EMPVAUtils::GetHash("displayfiles_back_button");
		backButton = (widget::Button *)g_root_page->GetChildByHash(&searchParam, 0);
		if (backButton->animationStatus != 0x10) { // Check if back button is actually on screen
			backButton->PlayAnimationReverse(1000.0f, widget::Widget::Animation_Fadein1, SCE_NULL);
			s_needBackButtonShow = SCE_TRUE;
		}

		searchParam.hash = EMPVAUtils::GetHash("plane_common_bg");
		rootPlane = g_root_page->GetChildByHash(&searchParam, 0);
		rootPlane->PlayAnimationReverse(1000.0f, widget::Widget::Animation_Fadein1, SCE_NULL);

		break;
	case Parent_Player:
		
		// Hide (disable) player plane
		searchParam.hash = EMPVAUtils::GetHash("plane_player_bg");
		rootPlane = g_player_page->GetChildByHash(&searchParam, 0);
		rootPlane->PlayAnimationReverse(1000.0f, widget::Widget::Animation_Fadein1, SCE_NULL);

		break;
	}

	// Get hashes for animations and play them
	searchParam.hash = EMPVAUtils::GetHash("page_settings");
	g_settings_page = g_empvaPlugin->CreateScene(&searchParam, &rwiParam);

	searchParam.hash = EMPVAUtils::GetHash("plane_settings_audio_bg");
	widget::Widget *settingsAudioPlane = g_settings_page->GetChildByHash(&searchParam, 0);
	settingsAudioPlane->PlayAnimationReverse(0.0f, widget::Widget::Animation_Reset, SCE_NULL);

	searchParam.hash = EMPVAUtils::GetHash("plane_settings_power_bg");
	widget::Widget *settingsPowerPlane = g_settings_page->GetChildByHash(&searchParam, 0);
	settingsPowerPlane->PlayAnimationReverse(0.0f, widget::Widget::Animation_Reset, SCE_NULL);

	searchParam.hash = EMPVAUtils::GetHash("plane_settings_control_bg");
	widget::Widget *settingsControlPlane = g_settings_page->GetChildByHash(&searchParam, 0);
	settingsControlPlane->PlayAnimationReverse(0.0f, widget::Widget::Animation_Reset, SCE_NULL);

	searchParam.hash = EMPVAUtils::GetHash("settings_back_button");
	widget::Widget *settingsBackButton = g_settings_page->GetChildByHash(&searchParam, 0);
	settingsBackButton->PlayAnimationReverse(0.0f, widget::Widget::Animation_Reset, SCE_NULL);

	searchParam.hash = EMPVAUtils::GetHash("plane_settings_bg");
	widget::Widget *settingsPlane = g_settings_page->GetChildByHash(&searchParam, 0);
	settingsPlane->PlayAnimation(0.0f, widget::Widget::Animation_SlideFromBottom1, SCE_NULL);

	// Register settings plane button callbacks (one time per app lifetime)
	if (!s_isBtCbRegistered) {

		auto settingsCloseButtonCB = new CloseButtonCB();
		searchParam.hash = EMPVAUtils::GetHash("settings_close_button");
		widget::Widget *settingsCloseButton = g_settings_page->GetChildByHash(&searchParam, 0);
		settingsCloseButton->RegisterEventCallback(0x10000008, settingsCloseButtonCB, 0);

		auto settingsBackButtonCB = new SettingsBackButtonCB();
		settingsBackButton->RegisterEventCallback(0x10000008, settingsBackButtonCB, 0);

		auto settingsOptionButtonCB = new SettingsOptionButtonCB();
		searchParam.hash = EMPVAUtils::GetHash("settings_device_button");
		s_settingsButtonDevice = settingsPlane->GetChildByHash(&searchParam, 0);
		s_settingsButtonDevice->RegisterEventCallback(0x10000008, settingsOptionButtonCB, 0);

		searchParam.hash = EMPVAUtils::GetHash("settings_sort_button");
		s_settingsButtonSort = settingsPlane->GetChildByHash(&searchParam, 0);
		s_settingsButtonSort->RegisterEventCallback(0x10000008, settingsOptionButtonCB, 0);

		searchParam.hash = EMPVAUtils::GetHash("settings_audio_button");
		s_settingsButtonAudio = settingsPlane->GetChildByHash(&searchParam, 0);
		s_settingsButtonAudio->RegisterEventCallback(0x10000008, settingsOptionButtonCB, 0);

		searchParam.hash = EMPVAUtils::GetHash("settings_power_button");
		s_settingsButtonPower = settingsPlane->GetChildByHash(&searchParam, 0);

		searchParam.hash = EMPVAUtils::GetHash("settings_control_button");
		s_settingsButtonControl = settingsPlane->GetChildByHash(&searchParam, 0);
		// Check if PSTV to disable unsupported settings buttons
#ifdef NDEBUG
		if (Misc::IsDolce()) {
			common::Utils::WidgetStateTransition(0.0f, s_settingsButtonPower, widget::Widget::Animation_Reset, SCE_TRUE, SCE_TRUE);
			common::Utils::WidgetStateTransition(0.0f, s_settingsButtonControl, widget::Widget::Animation_Reset, SCE_TRUE, SCE_TRUE);

			pos.x = 0.0f;
			pos.y = -14.0f;
			pos.z = 0.0f;
			pos.w = 0.0f;

			searchParam.hash = EMPVAUtils::GetHash("settings_scroll_view");
			settingsScrollView = g_settings_page->GetChildByHash(&searchParam, 0);
			settingsScrollView->SetPosition(&pos);
		}
		else
#endif
		{
			s_settingsButtonPower->RegisterEventCallback(0x10000008, settingsOptionButtonCB, 0);
			s_settingsButtonControl->RegisterEventCallback(0x10000008, settingsOptionButtonCB, 0);
		}

		searchParam.hash = EMPVAUtils::GetHash("settings_audio_eq_button");
		s_settingsButtonAudioEq = settingsAudioPlane->GetChildByHash(&searchParam, 0);
		s_settingsButtonAudioEq->RegisterEventCallback(0x10000008, settingsOptionButtonCB, 0);

		searchParam.hash = EMPVAUtils::GetHash("settings_audio_eq_checkbox");
		s_settingsCheckboxAudioAlc = (widget::CheckBox *)settingsAudioPlane->GetChildByHash(&searchParam, 0);
		s_settingsCheckboxAudioAlc->RegisterEventCallback(0x10000008, settingsOptionButtonCB, 0);

		searchParam.hash = EMPVAUtils::GetHash("settings_audio_limit_checkbox");
		s_settingsCheckboxAudioLimit = (widget::CheckBox *)settingsAudioPlane->GetChildByHash(&searchParam, 0);
		s_settingsCheckboxAudioLimit->RegisterEventCallback(0x10000008, settingsOptionButtonCB, 0);

		searchParam.hash = EMPVAUtils::GetHash("settings_power_save_checkbox");
		s_settingsCheckboxPowerSave = (widget::CheckBox *)settingsPowerPlane->GetChildByHash(&searchParam, 0);
		s_settingsCheckboxPowerSave->RegisterEventCallback(0x10000008, settingsOptionButtonCB, 0);

		searchParam.hash = EMPVAUtils::GetHash("settings_power_limit_ime");
		s_settingsImePowerTime = settingsPowerPlane->GetChildByHash(&searchParam, 0);
		s_settingsImePowerTime->RegisterEventCallback(0x10000008, settingsOptionButtonCB, 0);

		searchParam.hash = EMPVAUtils::GetHash("settings_control_stick_checkbox");
		s_settingsCheckboxControlStick = (widget::CheckBox *)settingsControlPlane->GetChildByHash(&searchParam, 0);
		s_settingsCheckboxControlStick->RegisterEventCallback(0x10000008, settingsOptionButtonCB, 0);

		searchParam.hash = EMPVAUtils::GetHash("settings_control_motion_checkbox");
		s_settingsCheckboxControlMotion = (widget::CheckBox *)settingsControlPlane->GetChildByHash(&searchParam, 0);
		s_settingsCheckboxControlMotion->RegisterEventCallback(0x10000008, settingsOptionButtonCB, 0);

		searchParam.hash = EMPVAUtils::GetHash("settings_control_time_ime");
		s_settingsImeControlTime = settingsControlPlane->GetChildByHash(&searchParam, 0);
		s_settingsImeControlTime->RegisterEventCallback(0x10000008, settingsOptionButtonCB, 0);

		searchParam.hash = EMPVAUtils::GetHash("settings_control_angle_ime");
		s_settingsImeControlAngle = settingsControlPlane->GetChildByHash(&searchParam, 0);
		s_settingsImeControlAngle->RegisterEventCallback(0x10000008, settingsOptionButtonCB, 0);

		s_isBtCbRegistered = SCE_TRUE;
	}

	if (config->motion_mode) {
		s_settingsImeControlTime->SetAlpha(1.0f);
		s_settingsImeControlAngle->SetAlpha(1.0f);
	}
	else {
		s_settingsImeControlTime->SetAlpha(0.39f);
		s_settingsImeControlAngle->SetAlpha(0.39f);
	}

	if (config->power_saving)
		s_settingsImePowerTime->SetAlpha(1.0f);
	else
		s_settingsImePowerTime->SetAlpha(0.39f);

	if (s_callerMode == Parent_Player) {
		s_settingsButtonDevice->Disable(SCE_FALSE);
		s_settingsButtonSort->Disable(SCE_FALSE);
	}

	RefreshButtonText();

	s_currPlane = settingsPlane;
}

SceVoid menu::settings::CloseButtonCB::CloseButtonCBFun(SceInt32 eventId, paf::widget::Widget *self, SceInt32 a3, ScePVoid pUserData)
{
	String *text8 = SCE_NULL;
	Resource::Element searchParam;
	widget::Widget *rootPlane;
	widget::Button *settingsButton;

	switch (s_callerMode) {
	case menu::settings::SettingsButtonCB::Parent_Displayfiles:
		// Show (enable) back/settings displayfiles buttons
		searchParam.hash = EMPVAUtils::GetHash("displayfiles_settings_button");
		settingsButton = (widget::Button *)g_root_page->GetChildByHash(&searchParam, 0);
		settingsButton->PlayAnimation(0.0f, widget::Widget::Animation_Fadein1, SCE_NULL);

		if (s_needBackButtonShow) {
			searchParam.hash = EMPVAUtils::GetHash("displayfiles_back_button");
			widget::Button *backButton = (widget::Button *)g_root_page->GetChildByHash(&searchParam, 0);
			backButton->PlayAnimation(0.0f, widget::Widget::Animation_Fadein1, SCE_NULL);
			s_needBackButtonShow = SCE_FALSE;
		}

		// Get hashes for animations and play them in reverse
		searchParam.hash = EMPVAUtils::GetHash("plane_common_bg");
		rootPlane = g_root_page->GetChildByHash(&searchParam, 0);
		rootPlane->PlayAnimation(-1000.0f, widget::Widget::Animation_Fadein1, SCE_NULL);

		break;
	case menu::settings::SettingsButtonCB::Parent_Player:

		s_settingsButtonDevice->Enable(SCE_FALSE);
		s_settingsButtonSort->Enable(SCE_FALSE);

		// Get hashes for animations and play them in reverse
		searchParam.hash = EMPVAUtils::GetHash("plane_player_bg");
		rootPlane = g_player_page->GetChildByHash(&searchParam, 0);
		rootPlane->PlayAnimation(-1000.0f, widget::Widget::Animation_Fadein1, SCE_NULL);

		break;
	}

	searchParam.hash = EMPVAUtils::GetHash("plane_settings_bg");
	widget::Widget *settingsPlane = g_settings_page->GetChildByHash(&searchParam, 0);
	settingsPlane->PlayAnimationReverse(0.0f, widget::Widget::Animation_SlideFromBottom1, SCE_NULL);

	// Reset file browser pages if needed
	if (s_needPageReload) {

		menu::displayfiles::Page *tmpCurr;

		if (s_needCwdReload) {

			while (g_currentDispFilePage->prev != SCE_NULL) {
				tmpCurr = g_currentDispFilePage;
				g_currentDispFilePage = g_currentDispFilePage->prev;
				delete tmpCurr;
			}

			text8 = String::WCharToNewString(EMPVAUtils::GetStringWithNum("msg_option_device_", g_config->GetConfigLocation()->device), text8);

			menu::displayfiles::Page *newPage = new menu::displayfiles::Page(text8->data);

			text8->Clear();
			delete text8;

			s_needCwdReload = SCE_FALSE;
		}
		else {
			text8 = new String(g_currentDispFilePage->cwd->data);

			if (g_currentDispFilePage->prev != SCE_NULL) {
				tmpCurr = g_currentDispFilePage;
				g_currentDispFilePage = g_currentDispFilePage->prev;
				delete tmpCurr;
			}

			menu::displayfiles::Page *newPage = new menu::displayfiles::Page(text8->data);

			text8->Clear();
			delete text8;
		}

		s_needPageReload = SCE_FALSE;
	}
}