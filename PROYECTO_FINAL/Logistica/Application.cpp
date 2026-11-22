#include "Application.h"
#include "MainWindow.h"

IMPLEMENT_APP(mxApplication)
	
	bool mxApplication::OnInit() {
	MainWindow* ventanaPrincipal = new MainWindow();
	ventanaPrincipal->Show(true);
	return true;
}
