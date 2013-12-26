#ifndef KVMAINWINDOW_H
#define KVMAINWINDOW_H

#include <QMainWindow>
#include <QWebView>

class KVMainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	KVMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	
protected:
	void loadBundledIndex();
	void loadAPILink();
	QString askForAPILink();
	
private slots:
	void onLoadStarted();
	void onLoadFinished(bool ok);
	
protected:
	QWebView *webView;
	QString server, apiToken;
	QUrl apiLink;
};

#endif
