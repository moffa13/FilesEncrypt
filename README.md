# FilesEncrypt

[![Build Status](https://travis-ci.org/moffa13/FilesEncrypt.svg?branch=master)](https://travis-ci.org/moffa13/FilesEncrypt) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/ead1c31ffd0347f7a50e4a2724309fba)](https://www.codacy.com/app/moffa13/FilesEncrypt?utm_source=github.com&utm_medium=referral&utm_content=moffa13/FilesEncrypt&utm_campaign=badger)

Application for files/dir encrypting, RSA-4096 & AES-256 CBC

***

## Installation

If you are using linux system, you might have to install some extra libraries and write the following in a terminal.

	sudo apt-get install openssl libssl-dev libgl1-mesa-dev gnome-keyring libglib2.0-dev libsecret-1-dev libgcr-3-dev -y
	sudo cp /usr/lib/x86_64-linux-gnu/glib-2.0/include/glibconfig.h /usr/include/glib-2.0/glibconfig.h

Then, simply run

	qmake src/FilesEncrypt.pro

    
then run

	nmake
 
or
 
	make

Depending if you are on Windows or Linux.


### Donations

Even a little contribution is well appreciated :)

You can donate bitcoins at this address : 1LAB8UjmNBLB75pFhFxL6JTPo3vFWSoCG2

Or you can send with paypal at moffajeremy@outlook.com
