#ifndef __ASSERT_H__
#define __ASSERT_H__


#ifdef _DEBUG
#define Assert( _exp ) _assert( (_exp), "Assertion Failed:\n\tFile: %s\n\tLine: %d\n", __FILE__, __LINE__)
#else
#define Assert( _exp ) (void*)0
#endif

void _assert( int condition, const char* fmt, ... );
void _assert( void* ptr, const char* fmt, ... );



#endif
