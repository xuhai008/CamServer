/*
 * BaseRespond.h
 *
 *  Created on: 2016年3月11日
 *      Author: max
 */

#ifndef REQUEST_BASERESPOND_H_
#define REQUEST_BASERESPOND_H_

#include "IRespond.h"
class BaseRespond : public IRespond {
public:
	BaseRespond();
	virtual ~BaseRespond();
};

#endif /* REQUEST_BASERESPOND_H_ */