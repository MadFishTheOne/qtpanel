#include <QtGui/QWidget>

class PanelWindow: public QWidget
{
	Q_OBJECT
public:
	PanelWindow();
	~PanelWindow();

	void setScreen(int screen);
private:
	int m_screen;
};
