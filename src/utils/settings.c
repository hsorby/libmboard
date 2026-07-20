/* $Id: settings.c 2176 2009-10-01 10:58:55Z lsc $ */
/*!
 * \file settings.c
 * 
 * \code
 *      Author: Lee-Shawn Chin 
 *      Date  : Aug 2009
 *      Copyright (c) 2009 STFC Rutherford Appleton Laboratory
 * \endcode
 * 
 * \brief Defines and populates global settings
 * 
 */
#include "mb_settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _EXTRA_CHECKS
#define PRINT_CONFIG
#endif

#ifdef PRINT_CONFIG
#define P_CONFIG(n,v,e)   printf("[libmboard] <settings> %s = %u %s\n", n, v, e)
#define P_CONFIG_S(n,v,e) printf("[libmboard] <settings> %s = %s %s\n", n, v, e)
#else
#define P_CONFIG(n,v,e)
#define P_CONFIG_S(n,v,e)
#endif
    
#define DEFAULT_OFF 0
#define DEFAULT_ON  1

/*! \brief global variable to store library-wide settings */
struct MBIt_config_data MBI_CONFIG;

/* internal function */
static unsigned int _read_env_bool(const char *varname, unsigned int default_value);
static unsigned int _read_env_uint(const char *varname, 
                                    unsigned int minimum,
                                    unsigned int maximum,
                                    unsigned int default_value);
#ifdef _PARALLEL
static enum MBIt_config_protocols _read_env_protocol(const char *varname);
#endif

/*! \brief sets default configuration parameters and reads environment
 * variables to apply necessary changes
 */
void MBI_update_settings(void) {
    
    /* boolean config vals */
    MBI_CONFIG.mempool_recycle = _read_env_bool("MBOARD_MEMPOOL_RECYCLE", DEFAULT_OFF);
    
    /* uint config vals */
    MBI_CONFIG.mempool_blocksize = _read_env_uint("MBOARD_MEMPOOL_BLOCKSIZE", 
                                                    10,     /* minimum */
                                                    1000000,  /* maximum */
                                                    512);   /* default */

#ifdef _PARALLEL
    /* communication protocol to use */
    MBI_CONFIG.comm_protocol = _read_env_protocol("MBOARD_COMM_PROTOCOL");
#endif
}

/*! \brief check environment variable for boolean-based settings 
 * \param[in] varname Name of environment variable to check for
 * \param[in] default_value Default value to set if no valid environment variable found
 * \return Value to update settings with, 0 or 1
 * 
 * If the given environment variable is set, we inspect its value and
 * interpret "1", "ON", "YES" or "TRUE" as a request to enable, and
 * "0", "OFF", "NO" or "FALSE" as a request to disable.
 * 
 * If the environment variable is not defined, or if it is assigned an 
 * invalid value, the \c default_value is used.
 * 
 */
static unsigned int _read_env_bool(const char *varname, unsigned int default_value) {
    
    char *value;
    
    value = getenv(varname);
    if (value != NULL)
    {
        if ( 
                strcmp(value, "1")    == 0   ||
                strcmp(value, "ON")   == 0   ||
                strcmp(value, "YES")  == 0   ||
                strcmp(value, "TRUE") == 0 )
        {
            P_CONFIG(varname, (unsigned int)1, "");
            return 1; /* enable */
        }
        else if (
                strcmp(value, "0")     == 0  ||
                strcmp(value, "OFF")   == 0  ||
                strcmp(value, "NO")    == 0  ||
                strcmp(value, "FALSE") == 0 )
        {
            P_CONFIG(varname, (unsigned int)0, "");
            return 0; /* disable */
        }
    }
    
    P_CONFIG(varname, default_value, "(default)");
    return default_value; /* default value */
}

/*! \brief check environment variable for uint-based settings 
 * \param[in] varname Name of environment variable to check for
 * \param[in] minimum Minimum value that can be requested
 * \param[in] maximum Maximum value that can be requested
 * \param[in] default_value Default value to set if no valid environment variable found
 * \return Value to update settings with
 * 
 * If the given environment variable is set, we inspect its value and
 * return the value if it is a valid integer and not below \c minimum.
 * 
 */
static unsigned int _read_env_uint(const char *varname, 
                                    unsigned int minimum, 
                                    unsigned int maximum, 
                                    unsigned int default_value) {
    
    unsigned int value;
    char *str;
    
    str = getenv(varname);
    if (str != NULL) /* env var defined */
    {
        value = (unsigned int) atoi(str);
        if (value >= minimum && value <= maximum)
        {
            P_CONFIG(varname, value, "");
            return value;
        }

    }
    
    P_CONFIG(varname, default_value, "(default)");
    return default_value; /* default value */
}

#ifdef _PARALLEL
/*! \brief check environment variable for which communication protocol to use
 * \param[in] varname Name of environment variable to check for
 * \return Value to update settings with
 * 
 */
static enum MBIt_config_protocols _read_env_protocol(const char *varname) {

    char *str;
    
    str = getenv(varname);
    if (str != NULL) /* env var defined */
    {
        if (strcmp(str, "HANDSHAKE") == 0)
        {
            P_CONFIG_S(varname, str, "");
            return MB_CONFIG_PROTO_HANDSHAKE;
        }
        else if (strcmp(str, "OLD") == 0)
        {

            P_CONFIG_S(varname, str, "");
            return MB_CONFIG_PROTO_OLD;
        }
    }
    
    /* default value */
    P_CONFIG_S(varname, "HANDSHAKE", "(default)");
    return MB_CONFIG_PROTO_HANDSHAKE;
}
#endif
