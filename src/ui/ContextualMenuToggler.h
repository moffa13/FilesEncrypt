#ifndef CONTEXTUALMENUTOGGLER_H
#define CONTEXTUALMENUTOGGLER_H

#include <QObject>

class ContextualMenuToggler : public QObject
{
		Q_OBJECT

	public:
		static void toggleCryptUncryptOptions(bool enable);
	private:
		static void setCryptUncryptOptions();
		static void unsetCryptUncryptOptions();
};

#endif // CONTEXTUALMENUTOGGLER_H
