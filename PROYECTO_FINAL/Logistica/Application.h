#ifndef MX_APPLICATION_H
#define MX_APPLICATION_H

#include <wx/app.h>
#include "Grafo.h"

class mxApplication : public wxApp {
private:
	grafo miGrafo;
	
public:
	virtual bool OnInit();
};

#endif
