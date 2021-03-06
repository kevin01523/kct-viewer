#include "KVScreenshooter.h"

#include <QWidget>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>
#include <QBuffer>
#include <QPainter>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QClipboard>

#include <QDebug>

KVScreenshooter& KVScreenshooter::instance()
{
	static KVScreenshooter _instance;
	return _instance;
}

KVScreenshooter::KVScreenshooter(QObject *parent) :
	QObject(parent)
{
	manager = new QNetworkAccessManager(this);
}

KVScreenshooter::~KVScreenshooter()
{
	delete manager;
}

QImage KVScreenshooter::captureScreenshot(QWidget *widget)
{
	QImage image(widget->size(), QImage::Format_ARGB32);
	image.fill(Qt::transparent);
	QPainter painter(&image);

	widget->render(&painter);
	painter.end();

	return image;
}

void KVScreenshooter::saveScreenshot(QImage image)
{
	QDateTime currentTime = currentTime.currentDateTime();
	QString currentTimeString = currentTime.toString("yyyy-MM-dd hh.mm.ss");

	QString picName = QString("KCTViewer %1.%2").arg(currentTimeString, screenshotsFormat.toLower());
	int counter = 0;
	while(QFile(QDir(screenshotsPath).filePath(picName)).exists())
	{
		counter++;
		picName = QString("KCTViewer %1 (%2).%3").arg(currentTimeString, QString::number(counter), screenshotsFormat.toLower());
	}

	QString fullPath = QDir(screenshotsPath).filePath(picName);
	image.save(fullPath, screenshotsFormat.toUtf8().constData());
}

void KVScreenshooter::uploadScreenshot(QImage image)
{
	QByteArray rawData;
	QBuffer buffer(&rawData);
	buffer.open(QBuffer::WriteOnly);

	image.save(&buffer, screenshotsFormat.toUtf8().constData());
	buffer.close();

	QNetworkRequest request(QUrl("https://api.imgur.com/3/image.json"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	request.setRawHeader("Authorization", "Client-ID ef6bd901726e8b7");

	QByteArray requestBody;
	requestBody.append(QString("image=").toUtf8());
	requestBody.append(QUrl::toPercentEncoding(rawData.toBase64()));

	QNetworkReply *reply = manager->post(request, requestBody);
	connect(reply, SIGNAL(finished()), this, SLOT(uploadingFinished()));
}

void KVScreenshooter::uploadingFinished()
{
	QNetworkReply *reply(qobject_cast<QNetworkReply*>(QObject::sender()));

	if(reply->error() == QNetworkReply::NoError) {
		QString textData = QString::fromUtf8(reply->readAll());

		QJsonDocument jsonDocument = QJsonDocument::fromJson(textData.toUtf8());
		QJsonObject jsonObject = jsonDocument.object();

		QString link = jsonObject["data"].toObject()["link"].toString();

		QApplication::clipboard()->setText(link);
		qWarning() << link;
	} else {
		qWarning() << "Couldn't upload screenshot" << reply->errorString();
	}

	reply->deleteLater();
}

void KVScreenshooter::takeScreenshot(QWidget *widget)
{
	QImage image = KVScreenshooter::captureScreenshot(widget);

	(uploadScreenshots)? KVScreenshooter::uploadScreenshot(image) : KVScreenshooter::saveScreenshot(image);
}
