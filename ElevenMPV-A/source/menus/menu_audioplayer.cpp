#include <audioout.h>
#include <appmgr.h>
#include <kernel.h>
#include <kernel/rng.h>
#include <power.h>
#include <shellsvc.h>
#include <ctrl.h>
#include <shellaudio.h>
#include <paf.h>
#include <stdlib.h>
#include <string.h>
#include <motion.h>

#include "audio.h"
#include "common.h"
#include "config.h"
#include "menu_displayfiles.h"
#include "menu_audioplayer.h"
#include "motion_e.h"
#include "utils.h"
#include "ipc.h"
#include "vitaaudiolib.h"

using namespace paf;

static SceBool s_isBtCbRegistered = SCE_FALSE;
static SceBool s_needBackButtonShow = SCE_FALSE;

static widget::Widget *s_counterText = SCE_NULL;
static widget::Widget *s_playerPlane = SCE_NULL;

static graphics::Texture *s_pauseButtonTex;
static graphics::Texture *s_playButtonTex;
static graphics::Texture *s_shuffleOnButtonTex;
static graphics::Texture *s_shuffleOffButtonTex;
static graphics::Texture *s_repeatOnButtonTex;
static graphics::Texture *s_repeatOnOneButtonTex;
static graphics::Texture *s_repeatOffButtonTex;

typedef enum RepeatState{
	REPEAT_STATE_NONE,
	REPEAT_STATE_ONE,
	REPEAT_STATE_ALL
} RepeatState;

typedef enum ShuffleState {
	SHUFFLE_STATE_OFF,
	SHUFFLE_STATE_ON
} ShuffleState;

static SceUInt32 s_repeatState = REPEAT_STATE_NONE;
static SceUInt32 s_shuffleState = SHUFFLE_STATE_OFF;

static SceUInt32 s_timerPof = 0;

static SceUInt64 s_oldCurrentPosSec = 0;

static String *s_totalLength;

SceVoid menu::audioplayer::Audioplayer::HandleNext(SceBool fromHandlePrev, SceBool fromFfButton)
{
	Resource::Element searchParam;
	SceUInt32 i = 0;
	SceInt32 originalIdx = 0;
	String text8;
	WString text16;

	if (g_currentPlayerInstance->core->GetDecoder()->IsPaused()) {
		searchParam.hash = EMPVAUtils::GetHash("player_play_button");
		widget::Widget *playButton = s_playerPlane->GetChildByHash(&searchParam, 0);
		playButton->SetTextureBase(s_pauseButtonTex);
		EMPVAUtils::IPC::SendInfo(SCE_NULL, SCE_NULL, SCE_NULL, 0);
	}

	delete g_currentPlayerInstance->core;
	g_currentPlayerInstance->core = SCE_NULL;

	if (s_repeatState == REPEAT_STATE_ONE && !fromHandlePrev && !fromFfButton) {
		g_currentPlayerInstance->core = new AudioplayerCore(g_currentPlayerInstance->playlist.path[g_currentPlayerInstance->playlistIdx]->data);
		return;
	}

	if (s_shuffleState == SHUFFLE_STATE_ON) {

		if ((g_currentPlayerInstance->totalConsumedIdx == g_currentPlayerInstance->totalIdx) && (s_repeatState != REPEAT_STATE_ALL)) {
			delete g_currentPlayerInstance;
			return;
		}
		else if (g_currentPlayerInstance->totalConsumedIdx == g_currentPlayerInstance->totalIdx) {
			g_currentPlayerInstance->totalConsumedIdx = 0;

			while (g_currentPlayerInstance->playlist.path[i] != SCE_NULL) {
				g_currentPlayerInstance->playlist.isConsumed[i] = 0;
				i++;
			}
		}

		if (!fromHandlePrev && !fromFfButton) {
			while (g_currentPlayerInstance->playlist.isConsumed[g_currentPlayerInstance->playlistIdx]) {
				sceKernelGetRandomNumber(&g_currentPlayerInstance->playlistIdx, sizeof(SceUInt32));
				g_currentPlayerInstance->playlistIdx = g_currentPlayerInstance->playlistIdx % (g_currentPlayerInstance->totalIdx);
			}
		}
		else {
			originalIdx = g_currentPlayerInstance->playlistIdx;
			while (g_currentPlayerInstance->playlistIdx == originalIdx) {
				sceKernelGetRandomNumber(&g_currentPlayerInstance->playlistIdx, sizeof(SceUInt32));
				g_currentPlayerInstance->playlistIdx = g_currentPlayerInstance->playlistIdx % (g_currentPlayerInstance->totalIdx);
			}
		}
	}
	else {
		g_currentPlayerInstance->playlistIdx++;
		if (g_currentPlayerInstance->playlistIdx >= g_currentPlayerInstance->totalIdx)
			g_currentPlayerInstance->playlistIdx = 0;

		if ((g_currentPlayerInstance->playlistIdx == g_currentPlayerInstance->startIdx) && (s_repeatState != REPEAT_STATE_ALL) && !fromHandlePrev && !fromFfButton) {
			delete g_currentPlayerInstance;
			return;
		}
	}
	g_currentPlayerInstance->core = new AudioplayerCore(g_currentPlayerInstance->playlist.path[g_currentPlayerInstance->playlistIdx]->data);

	s_totalLength->Clear();
	ConvertSecondsToString(s_totalLength, g_currentPlayerInstance->core->GetDecoder()->GetLength() / g_currentPlayerInstance->core->GetDecoder()->GetSampleRate(), SCE_FALSE);

	searchParam.hash = EMPVAUtils::GetHash("text_player_number");
	widget::Widget *numText = g_player_page->GetChildByHash(&searchParam, 0);
	text8.Setf("%u / %u", g_currentPlayerInstance->playlistIdx + 1, g_currentPlayerInstance->totalIdx);
	text8.ToWString(&text16);
	numText->SetLabel(&text16);
	text8.Clear();
	text16.Clear();

	// Handle cover
	if (g_currentPlayerInstance->core->GetDecoder()->coverLoader == SCE_NULL && !g_currentPlayerInstance->core->GetDecoder()->GetMetadataLocation()->hasCover) {
		g_currentPlayerInstance->core->GetDecoder()->coverLoader = new audio::PlayerCoverLoaderThread(SCE_KERNEL_COMMON_QUEUE_HIGHEST_PRIORITY, SCE_KERNEL_4KiB, "EMPVA::PlayerCoverLoader");
		g_currentPlayerInstance->core->GetDecoder()->coverLoader->workptr = SCE_NULL;
		g_currentPlayerInstance->core->GetDecoder()->coverLoader->Start();
	}
}

SceVoid menu::audioplayer::Audioplayer::HandlePrev()
{
	if (s_shuffleState == SHUFFLE_STATE_ON) {
		HandleNext(SCE_TRUE, SCE_FALSE);
		return;
	}

	g_currentPlayerInstance->playlistIdx--;
	if (g_currentPlayerInstance->playlistIdx < 0)
		g_currentPlayerInstance->playlistIdx = g_currentPlayerInstance->totalIdx - 1;

	g_currentPlayerInstance->playlist.isConsumed[g_currentPlayerInstance->playlistIdx] = 0;

	g_currentPlayerInstance->playlistIdx--;
	if (g_currentPlayerInstance->playlistIdx < 0)
		g_currentPlayerInstance->playlistIdx = g_currentPlayerInstance->totalIdx - 1;

	g_currentPlayerInstance->totalConsumedIdx--;

	HandleNext(SCE_TRUE, SCE_FALSE);
}

SceVoid menu::audioplayer::Audioplayer::ConvertSecondsToString(String *string, SceUInt64 seconds, SceBool needSeparator)
{
	SceInt32 h = 0, m = 0, s = 0;
	h = (seconds / 3600);
	m = (seconds - (3600 * h)) / 60;
	s = (seconds - (3600 * h) - (m * 60));

	if (needSeparator) {
		if (h > 0)
			string->Setf("%02d:%02d:%02d / ", h, m, s);
		else
			string->Setf("%02d:%02d / ", m, s);
	}
	else {
		if (h > 0)
			string->Setf("%02d:%02d:%02d", h, m, s);
		else
			string->Setf("%02d:%02d", m, s);
	}
}

SceVoid menu::audioplayer::Audioplayer::RegularTask(ScePVoid pUserData)
{
	String text8;
	WString text16;
	WString text16fake;
	SceUInt64 currentPos = 0;
	SceUInt64 currentPosSec = 0;
	SceUInt64 length = 0;
	SceInt32 motionCom = 0;
	SceUInt32 ipcCom = 0;
	SceCtrlData ctrlData;
	Plugin::TemplateInitParam tmpParam;
	widget::Widget *commonWidget;
	Resource::Element searchParam;
	audio::GenericDecoder *currentDecoder = g_currentPlayerInstance->GetCore()->GetDecoder();
	config::Config::EMPVAConfig *config = g_config->GetConfigLocation();

	// Set progressbar value
	if (!sceKernelPollEventFlag(g_eventFlagUid, FLAG_ELEVENMPVA_IS_FG, SCE_KERNEL_EVF_WAITMODE_AND, SCE_NULL)) {
		searchParam.hash = EMPVAUtils::GetHash("progressbar_player");
		widget::ProgressBarTouch *playerProgBar = (widget::ProgressBarTouch *)g_player_page->GetChildByHash(&searchParam, 0);

		currentPos = currentDecoder->GetPosition();
		length = currentDecoder->GetLength();
		currentPosSec = (SceUInt64)((SceFloat32)currentPos / (SceFloat32)currentDecoder->GetSampleRate());

		SceFloat32 progress = (SceFloat32)currentPos * 100.0f / (SceFloat32)length;

		playerProgBar->SetProgress(progress, 0, 0);

		if (s_oldCurrentPosSec != currentPosSec || currentPosSec == 0) {

			searchParam.hash = EMPVAUtils::GetHash("text_player_counter");
			commonWidget = g_player_page->GetChildByHash(&searchParam, 0);

			ConvertSecondsToString(&text8, currentPosSec, SCE_TRUE);

			text8.Append(s_totalLength->data, s_totalLength->length);
			text8.ToWString(&text16);

			commonWidget->SetLabel(&text16);

			text8.Clear();
			text16.Clear();
		}

		s_oldCurrentPosSec = currentPosSec;
	}

	if (!currentDecoder->IsPaused()) {
		sceDisplayWaitVblankStart();
	}

	// Check auto suspend feature
	if (g_currentPlayerInstance != SCE_NULL) {
		if (currentDecoder->IsPaused() && config->power_saving) {
			if ((sceKernelGetProcessTimeLow() - s_timerPof) > 60000000 * config->power_timer)
				delete g_currentPlayerInstance;
		}
	}

	// Handle next file
	if (!currentDecoder->isPlaying) {
		if (s_repeatState != REPEAT_STATE_ONE) {
			g_currentPlayerInstance->playlist.isConsumed[g_currentPlayerInstance->playlistIdx] = 1;
			g_currentPlayerInstance->totalConsumedIdx++;
		}
		HandleNext(SCE_FALSE, SCE_FALSE);
		return;
	}
	int ret = 0;
	// Check Shell IPC
	if (!EMPVAUtils::IsSleep()) {
		ipcCom = EMPVAUtils::IPC::PeekTx();
		switch (ipcCom) {
		case EMPVA_IPC_PLAY:

			searchParam.hash = EMPVAUtils::GetHash("player_play_button");
			commonWidget = g_player_page->GetChildByHash(&searchParam, 0);

			currentDecoder->Pause();

			if (currentDecoder->IsPaused()) {
				commonWidget->SetTextureBase(s_playButtonTex);
				EMPVAUtils::IPC::SendInfo(SCE_NULL, SCE_NULL, SCE_NULL, 1);
			}
			else {
				commonWidget->SetTextureBase(s_pauseButtonTex);
				EMPVAUtils::IPC::SendInfo(SCE_NULL, SCE_NULL, SCE_NULL, 0);
			}

			s_timerPof = sceKernelGetProcessTimeLow();

			return;

			break;
		case EMPVA_IPC_FF:

			menu::audioplayer::Audioplayer::HandleNext(SCE_FALSE, SCE_TRUE);
			return;

			break;
		case EMPVA_IPC_REW:

			menu::audioplayer::Audioplayer::HandlePrev();
			return;

			break;
		}
	}

	// Check analog stick
	if (EMPVAUtils::IsSleep() && !Misc::IsDolce() && config->stick_skip) {

		sce_paf_memset(&ctrlData, 0, sizeof(SceCtrlData));
		sceCtrlPeekBufferPositive(0, &ctrlData, 1);

		if (ctrlData.rx < 0x10) {
			menu::audioplayer::Audioplayer::HandlePrev();
			return;
		}
		else if (ctrlData.rx > 0xEF) {
			menu::audioplayer::Audioplayer::HandleNext(SCE_FALSE, SCE_TRUE);
			return;
		}
	}

	// Check motion feature
	if (!Misc::IsDolce() && config->motion_mode) {
		motionCom = motion::Motion::GetCommand();
		switch (motionCom) {
		case motion::Motion::MOTION_STOP:

			searchParam.hash = EMPVAUtils::GetHash("player_play_button");
			commonWidget = g_player_page->GetChildByHash(&searchParam, 0);

			currentDecoder->Pause();

			if (currentDecoder->IsPaused()) {
				commonWidget->SetTextureBase(s_playButtonTex);
				EMPVAUtils::IPC::SendInfo(SCE_NULL, SCE_NULL, SCE_NULL, 1);
			}
			else {
				commonWidget->SetTextureBase(s_pauseButtonTex);
				EMPVAUtils::IPC::SendInfo(SCE_NULL, SCE_NULL, SCE_NULL, 0);
			}

			s_timerPof = sceKernelGetProcessTimeLow();

			break;
		case motion::Motion::MOTION_NEXT:

			menu::audioplayer::Audioplayer::HandleNext(SCE_FALSE, SCE_TRUE);

			break;
		case motion::Motion::MOTION_PREVIOUS:

			menu::audioplayer::Audioplayer::HandlePrev();

			break;
		}
	}
}

SceVoid menu::audioplayer::BackButtonCB::BackButtonCBFun(SceInt32 eventId, widget::Widget *self, SceInt32 a3, ScePVoid pUserData)
{
	delete g_currentPlayerInstance;
}

SceVoid menu::audioplayer::PlayerButtonCB::PlayerButtonCBFun(SceInt32 eventId, widget::Widget *self, SceInt32 a3, ScePVoid pUserData)
{
	Resource::Element searchParam;
	SceBool isPreSeekPaused = SCE_FALSE;
	audio::GenericDecoder *currentDecoder = g_currentPlayerInstance->GetCore()->GetDecoder();

	switch (self->hash) {
	case ButtonHash_Play:

		currentDecoder->Pause();

		if (currentDecoder->IsPaused()) {
			self->SetTextureBase(s_playButtonTex);
			EMPVAUtils::IPC::SendInfo(SCE_NULL, SCE_NULL, SCE_NULL, 1);
		}
		else {
			self->SetTextureBase(s_pauseButtonTex);
			EMPVAUtils::IPC::SendInfo(SCE_NULL, SCE_NULL, SCE_NULL, 0);
		}

		s_timerPof = sceKernelGetProcessTimeLow();

		break;
	case ButtonHash_Rew:

		menu::audioplayer::Audioplayer::HandlePrev();

		break;
	case ButtonHash_Ff:

		menu::audioplayer::Audioplayer::HandleNext(SCE_FALSE, SCE_TRUE);

		break;
	case ButtonHash_Repeat:

		if (s_repeatState == REPEAT_STATE_ONE) {
			s_repeatState = REPEAT_STATE_NONE;
			self->SetTextureBase(s_repeatOffButtonTex);
		}
		else if (s_repeatState == REPEAT_STATE_ALL) {
			s_repeatState = REPEAT_STATE_ONE;
			self->SetTextureBase(s_repeatOnOneButtonTex);
		}
		else {
			s_repeatState = REPEAT_STATE_ALL;
			self->SetTextureBase(s_repeatOnButtonTex);
		}

		break;
	case ButtonHash_Shuffle:

		if (s_shuffleState == SHUFFLE_STATE_ON) {
			s_shuffleState = SHUFFLE_STATE_OFF;
			self->SetTextureBase(s_shuffleOffButtonTex);
		}
		else {
			s_shuffleState = SHUFFLE_STATE_ON;
			self->SetTextureBase(s_shuffleOnButtonTex);
		}

		break;
	case ButtonHash_Progressbar:

		widget::ProgressBarTouch *bar = (widget::ProgressBarTouch *)self;

		isPreSeekPaused = currentDecoder->IsPaused();

		if (!isPreSeekPaused && EMPVAUtils::IsDecoderUsed())
			currentDecoder->Pause();

		currentDecoder->Seek(bar->currentValue);

		if (currentDecoder->IsPaused() && EMPVAUtils::IsDecoderUsed() && !isPreSeekPaused)
			currentDecoder->Pause();

		break;
	}
}

menu::audioplayer::Audioplayer::Audioplayer(const char *cwd, menu::displayfiles::File *startFile)
{
	String text8;
	WString text16;
	String fullPath;
	Resource::Element searchParam;
	Plugin::SceneInitParam rwiParam;
	graphics::Texture coverTex;
	widget::Widget *playerCover;
	audio::GenericDecoder::Metadata *meta;
	config::Config::EMPVAConfig *config = g_config->GetConfigLocation();
	playlistIdx = 0;
	s_timerPof = 0;

	// Hide (disable) back/settings displayfiles buttons
	searchParam.hash = EMPVAUtils::GetHash("displayfiles_settings_button");
	widget::Widget *settingsButton = (widget::Button *)g_root_page->GetChildByHash(&searchParam, 0);
	settingsButton->PlayAnimationReverse(0.0f, widget::Widget::Animation_Fadein1, SCE_NULL);

	searchParam.hash = EMPVAUtils::GetHash("displayfiles_back_button");
	widget::Widget *backButton = (widget::Button *)g_root_page->GetChildByHash(&searchParam, 0);
	if (backButton->animationStatus != 0x10) { // Check if back button is actually on screen
		backButton->PlayAnimationReverse(0.0f, widget::Widget::Animation_Fadein1, SCE_NULL);
		s_needBackButtonShow = SCE_TRUE;
	}

	searchParam.hash = EMPVAUtils::GetHash("plane_common_bg");
	widget::Widget *rootPlane = g_root_page->GetChildByHash(&searchParam, 0);
	rootPlane->PlayAnimationReverse(0.0f, widget::Widget::Animation_Fadein1, SCE_NULL);

	// Create player scene
	searchParam.hash = EMPVAUtils::GetHash("page_player");
	g_player_page = g_empvaPlugin->CreateScene(&searchParam, &rwiParam);

	searchParam.hash = EMPVAUtils::GetHash("plane_player_cover");
	playerCover = g_player_page->GetChildByHash(&searchParam, 0);

	g_isPlayerActive = SCE_TRUE;

	// Get player widgets
	searchParam.hash = EMPVAUtils::GetHash("plane_player_bg");
	s_playerPlane = g_player_page->GetChildByHash(&searchParam, 0);
	s_playerPlane->PlayAnimation(0.0f, widget::Widget::Animation_Fadein1, SCE_NULL);

	if (!s_isBtCbRegistered) { // Register player plane button callbacks (one time per app lifetime)

		widget::Widget *commonWidget;

		searchParam.hash = EMPVAUtils::GetHash("player_settings_button");
		commonWidget = s_playerPlane->GetChildByHash(&searchParam, 0);
		commonWidget->RegisterEventCallback(0x10000008, g_settingsButtonCB, 0);

		auto backButtonCb = new menu::audioplayer::BackButtonCB();
		searchParam.hash = EMPVAUtils::GetHash("player_back_button");
		commonWidget = s_playerPlane->GetChildByHash(&searchParam, 0);
		commonWidget->RegisterEventCallback(0x10000008, backButtonCb, 0);

		auto playerButtonCb = new menu::audioplayer::PlayerButtonCB();
		searchParam.hash = EMPVAUtils::GetHash("player_play_button");
		commonWidget = s_playerPlane->GetChildByHash(&searchParam, 0);
		commonWidget->RegisterEventCallback(0x10000008, playerButtonCb, 0);

		searchParam.hash = EMPVAUtils::GetHash("player_rew_button");
		commonWidget = s_playerPlane->GetChildByHash(&searchParam, 0);
		commonWidget->RegisterEventCallback(0x10000008, playerButtonCb, 0);
		commonWidget->AssignButton(SCE_CTRL_L1);

		searchParam.hash = EMPVAUtils::GetHash("player_ff_button");
		commonWidget = s_playerPlane->GetChildByHash(&searchParam, 0);
		commonWidget->RegisterEventCallback(0x10000008, playerButtonCb, 0);
		commonWidget->AssignButton(SCE_CTRL_R1);

		searchParam.hash = EMPVAUtils::GetHash("player_shuffle_button");
		commonWidget = s_playerPlane->GetChildByHash(&searchParam, 0);
		commonWidget->RegisterEventCallback(0x10000008, playerButtonCb, 0);

		searchParam.hash = EMPVAUtils::GetHash("player_repeat_button");
		commonWidget = s_playerPlane->GetChildByHash(&searchParam, 0);
		commonWidget->RegisterEventCallback(0x10000008, playerButtonCb, 0);

		searchParam.hash = EMPVAUtils::GetHash("progressbar_player");
		commonWidget = s_playerPlane->GetChildByHash(&searchParam, 0);
		commonWidget->RegisterEventCallback(0x10000003, playerButtonCb, 0);

		s_pauseButtonTex = new graphics::Texture();
		searchParam.hash = EMPVAUtils::GetHash("tex_button_pause");
		Plugin::LoadTexture(s_pauseButtonTex, g_empvaPlugin, &searchParam);

		s_playButtonTex = new graphics::Texture();
		searchParam.hash = EMPVAUtils::GetHash("tex_button_play");
		Plugin::LoadTexture(s_playButtonTex, g_empvaPlugin, &searchParam);

		s_shuffleOffButtonTex = new graphics::Texture();
		searchParam.hash = EMPVAUtils::GetHash("tex_button_shuffle");
		Plugin::LoadTexture(s_shuffleOffButtonTex, g_empvaPlugin, &searchParam);

		s_shuffleOnButtonTex = new graphics::Texture();
		searchParam.hash = EMPVAUtils::GetHash("tex_button_shuffle_glow");
		Plugin::LoadTexture(s_shuffleOnButtonTex, g_empvaPlugin, &searchParam);

		s_repeatOffButtonTex = new graphics::Texture();
		searchParam.hash = EMPVAUtils::GetHash("tex_button_repeat");
		Plugin::LoadTexture(s_repeatOffButtonTex, g_empvaPlugin, &searchParam);

		s_repeatOnButtonTex = new graphics::Texture();
		searchParam.hash = EMPVAUtils::GetHash("tex_button_repeat_glow");
		Plugin::LoadTexture(s_repeatOnButtonTex, g_empvaPlugin, &searchParam);

		s_repeatOnOneButtonTex = new graphics::Texture();
		searchParam.hash = EMPVAUtils::GetHash("tex_button_repeat_glow_one");
		Plugin::LoadTexture(s_repeatOnOneButtonTex, g_empvaPlugin, &searchParam);

		s_isBtCbRegistered = SCE_TRUE;
	}

	// Player init
	*(SceUInt32 *)g_settingsButtonCB->pUserData = menu::settings::SettingsButtonCB::Parent_Player;

	sceAppMgrAcquireBgmPortWithPriority(0x81);

	GetMusicList(startFile);

	EMPVAUtils::SetPowerTickTask(SCE_TRUE);

	fullPath.Set(cwd);
	fullPath.Append(startFile->name->string.data, startFile->name->string.length);

	core = new AudioplayerCore(fullPath.data);
	fullPath.Clear();

	searchParam.hash = EMPVAUtils::GetHash("text_player_number");
	widget::Widget *numText = g_player_page->GetChildByHash(&searchParam, 0);
	text8.Setf("%u / %u", playlistIdx + 1, totalIdx);
	text8.ToWString(&text16);
	numText->SetLabel(&text16);
	text8.Clear();
	text16.Clear();

	s_totalLength = new String();
	ConvertSecondsToString(s_totalLength, core->GetDecoder()->GetLength() / core->GetDecoder()->GetSampleRate(), SCE_FALSE);

	// Handle cover
	meta = core->GetDecoder()->GetMetadataLocation();
	if (core->GetDecoder()->coverLoader == SCE_NULL && !meta->hasCover) {
		core->GetDecoder()->coverLoader = new audio::PlayerCoverLoaderThread(SCE_KERNEL_COMMON_QUEUE_HIGHEST_PRIORITY, SCE_KERNEL_4KiB, "EMPVA::PlayerCoverLoader");
		core->GetDecoder()->coverLoader->workptr = SCE_NULL;
		core->GetDecoder()->coverLoader->Start();
	}

	searchParam.hash = EMPVAUtils::GetHash("player_play_button");
	widget::Widget *playButton = s_playerPlane->GetChildByHash(&searchParam, 0);
	playButton->SetTextureBase(s_pauseButtonTex);

	if (!Misc::IsDolce() && config->motion_mode) {
		motion::Motion::SetState(SCE_TRUE);
		motion::Motion::SetReleaseTimer(config->motion_timer);
		motion::Motion::SetAngleThreshold(config->motion_degree);
	}

	g_currentPlayerInstance = this;

	common::Utils::AddMainThreadTask(RegularTask, SCE_NULL);
}

menu::audioplayer::Audioplayer::~Audioplayer()
{
	Resource::Element searchParam;
	SceInt32 i = 0;

	*(SceUInt32 *)g_settingsButtonCB->pUserData = menu::settings::SettingsButtonCB::Parent_Displayfiles;
	common::Utils::RemoveMainThreadTask(RegularTask, SCE_NULL);
	if (core != SCE_NULL)
		delete core;

	g_currentPlayerInstance = SCE_NULL;

	// Show (enable) back/settings displayfiles buttons
	searchParam.hash = EMPVAUtils::GetHash("displayfiles_settings_button");
	widget::Button *settingsButton = (widget::Button *)g_root_page->GetChildByHash(&searchParam, 0);
	settingsButton->PlayAnimation(0.0f, widget::Widget::Animation_Fadein1, SCE_NULL);

	if (s_needBackButtonShow) {
		searchParam.hash = EMPVAUtils::GetHash("displayfiles_back_button");
		widget::Button *backButton = (widget::Button *)g_root_page->GetChildByHash(&searchParam, 0);
		backButton->PlayAnimation(0.0f, widget::Widget::Animation_Fadein1, SCE_NULL);
		s_needBackButtonShow = SCE_FALSE;
	}

	// Get hashes for animations and play them in reverse
	searchParam.hash = EMPVAUtils::GetHash("plane_common_bg");
	widget::Widget *rootPlane = g_root_page->GetChildByHash(&searchParam, 0);
	rootPlane->PlayAnimation(-1000.0f, widget::Widget::Animation_Fadein1, SCE_NULL);

	searchParam.hash = EMPVAUtils::GetHash("plane_player_bg");
	widget::Widget *playerPlane = g_player_page->GetChildByHash(&searchParam, 0);
	playerPlane->PlayAnimationReverse(0.0f, widget::Widget::Animation_Fadein1, SCE_NULL);

	EMPVAUtils::SetPowerTickTask(SCE_FALSE);
	sceAppMgrReleaseBgmPort();
	audio::Utils::ResetBgmMode();

	while (playlist.path[i] != SCE_NULL) {
		playlist.path[i]->Clear();
		delete playlist.path[i];
		i++;
	}

	if (!Misc::IsDolce())
		motion::Motion::SetState(SCE_FALSE);

	s_totalLength->Clear();
	delete s_totalLength;

	g_isPlayerActive = SCE_FALSE;
}

SceVoid menu::audioplayer::Audioplayer::GetMusicList(menu::displayfiles::File *startFile)
{
	SceInt32 i = 0;
	menu::displayfiles::File *file = g_currentDispFilePage->files;

	while (file != SCE_NULL) {

		if (file->type == menu::displayfiles::File::Type_Music) {
			playlist.path[i] = new String(g_currentDispFilePage->cwd->data, g_currentDispFilePage->cwd->length);
			playlist.path[i]->Append(file->name->string.data, file->name->string.length);
			playlist.isConsumed[i] = 0;

			if (startFile == file) {
				playlistIdx = i;
				startIdx = i;
			}

			i++;
		}

		file = file->next;
	}

	totalIdx = i;
	playlist.path[i] = SCE_NULL;
	totalConsumedIdx = 0;
}

menu::audioplayer::AudioplayerCore *menu::audioplayer::Audioplayer::GetCore()
{
	return core;
}

menu::audioplayer::AudioplayerCore::AudioplayerCore(const char *file)
{
	SceUInt32 decoderType = EMPVAUtils::GetDecoderType(file);

	switch (decoderType) {
	case 0:
	case 1:
	case 2:
	case 3:
		decoder = new audio::XmDecoder(file, SCE_TRUE);
		break;
	case 4:
	case 5:
	case 6:
		decoder = new audio::At3Decoder(file, SCE_TRUE);
		break;
	case 7:
		decoder = new audio::OggDecoder(file, SCE_TRUE);
		break;
	case 8:
		decoder = new audio::Mp3Decoder(file, SCE_FALSE);
		break;
	case 9:
		decoder = new audio::OpusDecoder(file, SCE_TRUE);
		break;
	case 10:
		decoder = new audio::FlacDecoder(file, SCE_TRUE);
		break;
	case 11:
	case 12:
	case 13:
	case 14:
		decoder = new audio::ShellCommonDecoder(file, SCE_FALSE);
		break;
	}

	SetInitialParams();

	SetMetadata(file);

	EMPVAUtils::IPC::Enable();
}

menu::audioplayer::AudioplayerCore::~AudioplayerCore()
{
	delete decoder;
	decoder = SCE_NULL;

	EMPVAUtils::IPC::Disable();
}

audio::GenericDecoder *menu::audioplayer::AudioplayerCore::GetDecoder()
{
	return decoder;
}

SceVoid menu::audioplayer::AudioplayerCore::SetInitialParams()
{
	SceBool isDecoderUsed = EMPVAUtils::IsDecoderUsed();
	config::Config::EMPVAConfig *config = g_config->GetConfigLocation();

	if (isDecoderUsed) {
		sceAudioOutSetEffectType(config->eq_mode);
		sceAudioOutSetAlcMode(config->alc_mode);
	}
	else {
		sceMusicPlayerServiceSetEQ(config->eq_mode);
		sceMusicPlayerServiceSetALC(config->alc_mode);
	}
}

SceVoid menu::audioplayer::AudioplayerCore::SetMetadata(const char *file)
{
	String text8;
	WString text16;
	Resource::Element searchParam;
	char *name = sce_paf_strrchr(file, '/');
	name = name + 1;
	audio::GenericDecoder::Metadata *metadata = decoder->GetMetadataLocation();

	searchParam.hash = EMPVAUtils::GetHash("text_player_title");
	widget::Widget *textTitle = g_player_page->GetChildByHash(&searchParam, 0);
	searchParam.hash = EMPVAUtils::GetHash("text_player_album");
	widget::Widget *textAlbum = g_player_page->GetChildByHash(&searchParam, 0);
	searchParam.hash = EMPVAUtils::GetHash("text_player_artist");
	widget::Widget *textArtist = g_player_page->GetChildByHash(&searchParam, 0);

	textAlbum->SetLabel(&text16);
	textArtist->SetLabel(&text16);
	textTitle->SetLabel(&text16);

	if (metadata->hasMeta) {
		textAlbum->SetLabel(&metadata->album);
		textArtist->SetLabel(&metadata->artist);

		if (metadata->title.length == 0) {
			text8.Set(name);
			text8.ToWString(&metadata->title);
			text8.Clear();
		}
	}

	if (!metadata->hasMeta) {
		text8.Set(name);
		text8.ToWString(&metadata->title);
		text8.Clear();
	}

	textTitle->SetLabel(&metadata->title);

	EMPVAUtils::IPC::SendInfo(&metadata->title, &metadata->artist, &metadata->album, -1);
}