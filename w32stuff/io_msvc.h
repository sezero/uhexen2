/* different MSVC names for <io.h> functions
 * new MSVC versions spit deprecation warnings.
 * make SURE to include this AFTER io.h
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#  define read		_read
#  define write		_write
#  define open		_open
#  define close		_close
#  define dup		_dup
#  define access	_access
#  define rmdir		_rmdir
#  define unlink	_unlink
#endif	/* _MSC_VER */

