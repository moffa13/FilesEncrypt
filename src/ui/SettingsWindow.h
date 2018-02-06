#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QPointer>
#include <QSettings>

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QDialog
{
	Q_OBJECT

public:
	explicit SettingsWindow(QWidget *parent = 0);
	static QVariant getDefaultSetting(QString const& name);
	~SettingsWindow();
private Q_SLOTS:

Q_SIGNALS:
	void closed();
private:
	Ui::SettingsWindow *ui;
	QPointer<QSettings> m_settings;
	static QMap<QString, QPair<QString, QVariant>> checkNames;
	static bool defaultValuesInit;
	bool m_refuseClose;
	static void init();
	bool action(const QString &param, bool value);
	protected:
	void closeEvent(QCloseEvent*);
};

#endif // SETTINGSWINDOW_H
