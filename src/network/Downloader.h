#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QString>
#include <QObject>
#include <QNetworkReply>
#include <QUrl>

class Downloader : public QNetworkAccessManager
{

	Q_OBJECT

	public:
		Downloader(const QString &url);
		Downloader(const QUrl &url);
		Downloader(QNetworkRequest const& request);
		QNetworkReply *download();
		QNetworkReply *rep = nullptr;
	private:
		QNetworkRequest m_request;
    public Q_SLOTS:
		void fileDownloadedSlot(QNetworkReply*);
		void downloadProgressSlot(qint64 read, qint64 total);
    Q_SIGNALS:
		void downloaded(QByteArray const& res);
		void error();
		void downloadProgress(qint64 read, qint64 total);
};

#endif // DOWNLOADER_H
