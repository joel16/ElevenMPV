#ifndef _ELEVENMPV_MENU_DISPLAYFILES_H_
#define _ELEVENMPV_MENU_DISPLAYFILES_H_

#include <paf.h>

using namespace paf;

namespace menu {
	namespace displayfiles {

		class Page;
		class File;

		class CoverLoaderThread : public thread::Thread
		{
		public:

			using thread::Thread::Thread;

			SceVoid EntryFunction();

			Page *workPage;
			File *workFile;
		};

		class BackButtonCB : public widget::Widget::EventCallback
		{
		public:

			BackButtonCB()
			{
				eventHandler = BackButtonCBFun;
			};

			virtual ~BackButtonCB()
			{

			};

			static SceVoid BackButtonCBFun(SceInt32 eventId, widget::Widget *self, SceInt32 a3, ScePVoid pUserData);

		};

		class ButtonCB : public widget::Widget::EventCallback
		{
		public:

			ButtonCB()
			{
				eventHandler = ButtonCBFun;
			};

			virtual ~ButtonCB()
			{

			};

			static SceVoid ButtonCBFun(SceInt32 eventId, widget::Widget *self, SceInt32 a3, ScePVoid pUserData);

		};

		class BusyCB : public widget::Widget::EventCallback
		{
		public:

			BusyCB()
			{
				eventHandler = BusyCBFun;
			};

			virtual ~BusyCB()
			{

			};

			static SceVoid BusyCBFun(SceInt32 eventId, widget::Widget *self, SceInt32 a3, ScePVoid pUserData);

		};

		class File
		{
		public:

			enum Type 
			{
				Type_Unsupported,
				Type_Dir,
				Type_Music
			};

			File() :
				next(SCE_NULL),
				name(SCE_NULL),
				type(Type_Unsupported),
				button(SCE_NULL)
			{

			}

			~File()
			{
				delete name;
			}

			File *next;
			SWString *name;
			char ext[5];
			Type type;
			widget::ImageButton *button;
			ButtonCB *buttonCB;
		};

		class Page
		{
		public:

			Page(const char *path);

			~Page();

			static SceVoid ResetBgPlaneTex();

			static SceVoid Init();

			widget::Plane *root;
			widget::Box *box;
			String *cwd;
			Page *prev;
			File *files;
			BusyCB *busyCB;
			CoverLoaderThread *coverLoader;
			SceUInt32 fileNum;
			SceBool coverState;
			File *coverWork;

		private:

			static SceInt32 Cmpstringp(const ScePVoid p1, const ScePVoid p2);

			SceInt32 PopulateFiles(const char *rootPath);
			SceVoid ClearFiles(File *file);

			const SceUInt32 k_busyIndicatorFileLimit = 50;
		};
	}
}

#endif
