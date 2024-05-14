/**
 * @file       TinyGsmATDefines.h
 * @author     Sara Damiano
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2024 Volodymyr Shymanskyy
 * @date       Nov 2016
 *
 * @note There are no header guards in this file.
 */

#ifdef AT_NL
#undef AT_NL
#endif
#define AT_NL "\r\n"

#ifdef AT_OK
#undef AT_OK
#endif
#define AT_OK "OK"

#ifdef AT_ERROR
#undef AT_ERROR
#endif
#define AT_ERROR "ERROR"

#if defined TINY_GSM_DEBUG

#ifdef AT_VERBOSE
#undef AT_VERBOSE
#endif
#define AT_VERBOSE "+CME ERROR:"

#endif
