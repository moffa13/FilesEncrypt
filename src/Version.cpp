#include "Version.h"

#include <QRegExp>
#include <QtDebug>

Version::Version(unsigned major, unsigned minor, unsigned patch) : _major{major}, _minor{minor}, _patch{patch}{}

Version::Version(QString const& version){

	QRegExp version_regex{"^(([\\d]+)(?:\\.([\\d]+))?(?:\\.([\\d]+))?)$"};
	version_regex.indexIn(version, 0);

	if(version_regex.cap(0).isEmpty()){
		throw std::runtime_error{"Unknown version format"};
	}

	_major = version_regex.cap(2).isEmpty() ? 0 : version_regex.cap(2).toUInt();
	_minor = version_regex.cap(3).isEmpty() ? 0 : version_regex.cap(3).toUInt();
	_patch = version_regex.cap(4).isEmpty() ? 0 : version_regex.cap(4).toUInt();

}

unsigned Version::getMajor() const{
	return _major;
}

unsigned Version::getMinor() const{
	return _minor;
}

unsigned Version::getPatch() const{
	return _patch;
}

std::string Version::getVersionStr() const{
	return std::string{std::to_string(_major)} + "." + std::to_string(_minor) + "." + std::to_string(_patch);
}

unsigned Version::getVersionInt() const{
	return _major * 100 + _minor * 10 + _patch;
}

bool Version::operator <(const Version &lhs) const{
	return getVersionInt() < lhs.getVersionInt();
}

bool Version::operator >(const Version &lhs) const{
	return !operator <(lhs);
}

bool Version::operator >=(const Version &lhs) const{
	return operator>(lhs) || operator ==(lhs);
}

bool Version::operator <=(const Version &lhs) const{
	return operator<(lhs) || operator ==(lhs);
}


bool Version::operator ==(const Version &lhs) const{
	return _major == lhs._major && _minor == lhs._minor && _patch == lhs._patch;
}

bool Version::operator !=(const Version &lhs) const{
	return !operator==(lhs);
}


