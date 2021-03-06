/*
Copyright (c) 2012, Esteban Pellegrino
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TESTCOMMON_HPP_
#define TESTCOMMON_HPP_

#include "../../Common/Common.hpp"
#include "../../Environment/McEnvironment.hpp"

class InputPath {
	static InputPath inputpath;
	std::string path;
	InputPath() : path("./") {/* */}
public:
	static InputPath& access() {return inputpath;}
	std::string getPath() const {return path;}
	void setPath(const std::string& newpath) {path = newpath;}
	~InputPath() {/* */}
};

template<class T>
inline std::vector<T> genVector(T min, T max) {
	std::vector<T> v;
	for(T i = min ; i <= max ; i++) {
		v.push_back(i);
	}
	return v;
}

template<>
inline std::vector<std::string> genVector(std::string min, std::string max) {
	std::vector<std::string> v;
	for(int i = Helios::fromString<int>(min) ; i <= Helios::fromString<int>(max) ; i++) {
		v.push_back(Helios::toString(i));
	}
	return v;
}
#endif /* TESTCOMMON_HPP_ */
