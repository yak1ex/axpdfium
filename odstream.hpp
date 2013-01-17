/***********************************************************************/
/*                                                                     */
/* odstream.hpp: Header file for stream class using OutputDebugString  */
/*                                                                     */
/*     Copyright (C) 2011 Yak! / Yasutaka ATARASHI                     */
/*                                                                     */
/*     This software is distributed under the terms of a zlib/libpng   */
/*     License.                                                        */
/*                                                                     */
/*     $Id: 3637c03a3f2adb2e627120e488208f73bb995977 $                 */
/*                                                                     */
/***********************************************************************/

#ifndef ODSTREAM_HPP
#define ODSTREAM_HPP

#include <iosfwd>

namespace yak {

  namespace debug_yes {
		extern std::ostream ods;
	} // namespace debug_yes

	namespace debug_no {

		class PseudoNullStream;
		extern PseudoNullStream ods;

		extern std::ostream ns;

		class PseudoNullStream
		{
		public:
			template<typename T>
			const PseudoNullStream& operator << (T) const { return ods; }
			const PseudoNullStream& operator << (std::ostream& (*)(std::ostream&)) const { return ods; }
			const PseudoNullStream& operator << (std::ios_base& (*)(std::ios_base&)) const { return ods; }
			const PseudoNullStream& operator << (std::basic_ios<char>& (*)(std::basic_ios<char>&)) const { return ods; }
			operator std::ostream& () { return ns; }
		};

	} // namespace debug_no

#ifndef YAK_DEBUG_NO_DECLARE_NAMESPACE
#ifdef DEBUG
	namespace debug = debug_yes;
#else
	namespace debug = debug_no;
#endif
#endif

} // namespace yak

#endif
