/***********************************************************************/
/*                                                                     */
/* odstream.cpp: Source file for stream class using OutputDebugString  */
/*                                                                     */
/*     Copyright (C) 2011 Yak! / Yasutaka ATARASHI                     */
/*                                                                     */
/*     This software is distributed under the terms of a zlib/libpng   */
/*     License.                                                        */
/*                                                                     */
/*     $Id: 6513ed3a9f23aca93b18470eacbced77a34566dd $                 */
/*                                                                     */
/***********************************************************************/

#include <sstream>

#include <windows.h>

#include "odstream.hpp"

namespace yak {

    namespace debug_yes {

		class odstringbuf : public std::stringbuf
		{
		protected:
			virtual int sync(void) {
				OutputDebugString(str().c_str());
				str("");
				return 0;
			}
		};

		odstringbuf odsbuf;
		std::ostream ods(&odsbuf);

	} // namespace debug_yes

	namespace debug_no {

		PseudoNullStream ods;

		class nullstreambuf : public std::streambuf {};

		nullstreambuf nullbuf;
		std::ostream ns(&nullbuf);

	} // namespace debug_no

} // namespace yak
