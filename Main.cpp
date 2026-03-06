#include "mainmenu.h" // หรือชื่อฟอร์มหน้าแรกของคุณ

using namespace System;
using namespace System::Windows::Forms;

[STAThreadAttribute]
int Main(array<System::String^>^ args) {
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	// เรียกหน้าแรกของโปรเจกต์ Mangkudd
	Mangkudd::mainmenu form;
	Application::Run(% form);
	return 0;
}