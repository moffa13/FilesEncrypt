#include "Downloader.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>
#include <QObject>
#include <QTimer>

Downloader::Downloader(QString const& url) : m_request{QUrl{url}}{}

Downloader::Downloader(QUrl const& url) : m_request{url}{}

Downloader::Downloader(QNetworkRequest const& request) : m_request{request}{}

QNetworkReply* Downloader::download(){
	QNetworkAccessManager *q = new QNetworkAccessManager(this);
	QObject::connect(q, SIGNAL(finished(QNetworkReply*)), this, SLOT(fileDownloadedSlot(QNetworkReply*)));
    rep = q->get(m_request);
	QObject::connect(rep, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgressSlot(qint64,qint64)));
	return rep;
}

void Downloader::fileDownloadedSlot(QNetworkReply *reply){
    QUrl movedUrl = QUrl{reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl()};
	int responseCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(responseCode != 200 && movedUrl.isEmpty()){
        Q_EMIT error();
        return;
	}

    if(movedUrl.isEmpty()){
		QByteArray res = reply->readAll();
        Q_EMIT downloaded(res);
		return;
	}

    m_request.setUrl(movedUrl);
	download();

}

void Downloader::downloadProgressSlot(qint64 read, qint64 total){
	int r = rep->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	if(r == 200)
        Q_EMIT downloadProgress(read, total);
}

