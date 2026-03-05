#include "mainmenu.h"
using namespace Mangkudd;
[STAThreadAttribute]
int Main(array<System::String^>^ args)
{
	// Enabling Windows XP visual effects first
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	// Create the main window and run it
	Application::Run(gcnew mainmenu());
	return 0;
}