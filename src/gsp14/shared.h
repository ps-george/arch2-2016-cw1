/*
 * shared.h
 *
 *  Created on: 15 Oct 2016
 *      Author: big-g
 */

#ifndef SRC_GSP14_SHARED_H_
#define SRC_GSP14_SHARED_H_

#include <map>

/*!
 * J-type if 2 or 3.
 * R-type if 0.
 * I-type if anything else.
 */
const std::map<uint8_t, std::string> opcode_to_str;

#endif /* SRC_GSP14_SHARED_H_ */
