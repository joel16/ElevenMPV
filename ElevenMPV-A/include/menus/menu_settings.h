#ifndef _ELEVENMPV_MENU_SETTINGS_H_
#define _ELEVENMPV_MENU_SETTINGS_H_

#include <paf.h>

using namespace paf;

namespace menu {
	namespace settings {

		class SettingsBackButtonCB : public widget::Widget::EventCallback
		{
		public:

			SettingsBackButtonCB()
			{
				eventHandler = SettingsBackButtonCBFun;
			};

			virtual ~SettingsBackButtonCB()
			{

			};

			static SceVoid SettingsBackButtonCBFun(SceInt32 eventId, widget::Widget *self, SceInt32 a3, ScePVoid pUserData);

		};

		class SettingsOptionSelectionButtonCB : public widget::Widget::EventCallback
		{
		public:

			SettingsOptionSelectionButtonCB()
			{
				eventHandler = SettingsOptionSelectionButtonCBFun;
			};

			virtual ~SettingsOptionSelectionButtonCB()
			{

			};

			static SceVoid SettingsOptionSelectionButtonCBFun(SceInt32 eventId, widget::Widget *self, SceInt32 a3, ScePVoid pUserData);

		};

		class SettingsOptionButtonCB : public widget::Widget::EventCallback
		{
		public:

			enum ButtonHash 
			{
				ButtonHash_Device = 0x24b457f2,
				ButtonHash_Sort = 0x2557bb59,
				ButtonHash_Audio = 0x71d038c4,
				ButtonHash_Audio_Eq = 0xa9e40b09,
				ButtonHash_Audio_Alc = 0x38b41306,
				ButtonHash_Audio_Limit = 0xed27a668,
				ButtonHash_Power = 0xb43153d8,
				ButtonHash_Power_Save = 0x96e4b893,
				ButtonHash_Power_Time = 0x45a66e04,
				ButtonHash_Control = 0x8fbd8abf,
				ButtonHash_Control_Stick = 0x4cfd1a03,
				ButtonHash_Control_Motion = 0x4263f274,
				ButtonHash_Control_Time = 0xd0c5b691,
				ButtonHash_Control_Angle = 0xe255baed
			};

			SettingsOptionButtonCB()
			{
				eventHandler = SettingsOptionButtonCBFun;
			};

			virtual ~SettingsOptionButtonCB()
			{

			};

			static SceVoid SettingsOptionButtonCBFun(SceInt32 eventId, widget::Widget *self, SceInt32 a3, ScePVoid pUserData);

		};

		class SettingsButtonCB : public widget::Widget::EventCallback
		{
		public:

			enum Parent
			{
				Parent_Displayfiles,
				Parent_Player
			};

			SettingsButtonCB()
			{
				eventHandler = SettingsButtonCBFun;
			};

			virtual ~SettingsButtonCB()
			{

			};

			static SceVoid RefreshButtonText();

			static SceVoid SettingsButtonCBFun(SceInt32 eventId, widget::Widget *self, SceInt32 a3, ScePVoid pUserData);
		};

		class CloseButtonCB : public widget::Widget::EventCallback
		{
		public:

			CloseButtonCB()
			{
				eventHandler = CloseButtonCBFun;
			};

			virtual ~CloseButtonCB()
			{

			};

			static SceVoid CloseButtonCBFun(SceInt32 eventId, widget::Widget *self, SceInt32 a3, ScePVoid pUserData);

		};
	}
}

#endif
