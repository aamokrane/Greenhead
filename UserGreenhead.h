/*
 * UserGreenhead.h
 *
 *  Created on: Apr 14, 2014
 *      Author: ahmedamokrane
 */

#ifndef USERGREENHEAD_H_
#define USERGREENHEAD_H_

#include <string>
#include <iostream>


class UserGreenhead {
public:

	int id;
	std::string session;

	UserGreenhead();
	virtual ~UserGreenhead();
};

#endif /* USERGREENHEAD_H_ */
