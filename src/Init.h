#ifndef INIT_H
#define INIT_H

class Init
{
	public:
		static bool _init;
		static void init();
		static void deInit();
	private:
		static void initOpenSSL();
		static void deInitOpenSSL();
		static void initGcrypt();
};

#endif // INIT_H
