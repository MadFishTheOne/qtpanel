#include "panelapplication.h"

int main(int argc, char** argv)
{
	PanelApplication app(argc, argv);
	app.init();
	return app.exec();
}
