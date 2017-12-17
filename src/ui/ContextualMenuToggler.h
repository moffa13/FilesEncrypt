#ifndef CONTEXTUALMENUTOGGLER_H
#define CONTEXTUALMENUTOGGLER_H

#include <QObject>

class ContextualMenuToggler : public QObject
{
		Q_OBJECT

	public:
		static bool toggleCryptUncryptOptions(bool enable);
#ifdef Q_OS_WIN
		static bool regsvr(bool unregister = false);
#endif
};

#endif // CONTEXTUALMENUTOGGLER_H
