/*
 * strUtils.h
 *
 *  Created on: Jun 12, 2013
 *      Author: user
 */

#ifndef STRUTILS_H_
#define STRUTILS_H_

#define int_to_str(i,buf,len) 	snprintf(buf,len,"%d",i)
#define long_to_str(i,buf,len) 	snprintf(buf,len,"%ld",i)
#define long_to_hex(i,buf,len) 	snprintf(buf,len,"%lx",i)



#endif /* STRUTILS_H_ */
