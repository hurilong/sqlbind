#pragma once 
#include <stdint.h>

#ifdef WIN32 
#include <mysql.h>
#else 
#include <mysql/mysql.h>
#endif 

#include <string>
#include <time.h>

using namespace std ;

class mysqlex_stmt
{
private:
	MYSQL_STMT * stmt_; 
public :
	mysqlex_stmt(MYSQL_STMT * stmt = NULL )
		: stmt_(stmt)
	{
	}
	
	inline operator MYSQL_STMT * ()
	{
		return stmt_;
	}
	
	inline void operator = (MYSQL_STMT *stmt)
	{
		if ( stmt_ && stmt_ != stmt ) {
			mysql_stmt_close(stmt_) ;
		}
		stmt_ = stmt;
	}
	
	~mysqlex_stmt()
	{
		if ( stmt_ ) { 
			mysql_stmt_close(stmt_);
		}
	}
};

template<class T, class E> void sql_extend(std::ostream & oss, const T<E> & container, bool escape )
{
	const T<E>::iterator iter, end; 
	bool first = false; 
	for (iter = container.begin(), container.end(); iter != end; ++iter) {
		if (first) {
			oss << ","; 
		}
		first = true; 
		sql_extend(oss, *iter , escape ); 
	}
}

inline void sql_extend( std::ostream & oss , int val )
{
	oss << val; 
}

void sql_extend( std::ostream & oss, short val)
{
	oss << val; 
}

inline void sql_extend( std::ostream & oss , int64_t val)
{
	oss << val;
}


inline void sql_extend( std::ostream & oss, time_t t)
{
	if (t < 0) {
		oss << "null"; 
	}
	else {
		struct tm tmp = *localtime(&t);
		oss << "'" << tmp.tm_year + 1900 << "-" << tmp.tm_mon + 1 << "-" << tmp.tm_mday
			<< " " << tmp.tm_hour << ":" << tmp.tm_min << ":" << tmp.tm_sec << "'";
	}
}

inline void sql_extend( std::ostream & oss , const string & src , bool escape )
{
	if (escape) {
		oss << "'";
		for (const char* s = src.c_str(); *s; s++) {
			if (*s == '\'') {
				oss << "''";
			}
			else {
				oss.write(s, 1);
			}
		}
		oss << "'";
	}
	else {
		oss << src; 
	}
}

inline void sql_bind(MYSQL_BIND * bind , void * buffer , int buffer_length , enum_field_types type )
{
	if ( bind->length == NULL) { 
		bind->length = & bind->length_value;
	}
	if ( bind->is_null == NULL ) {
		bind->is_null = & bind->is_null_value;
	}
	if ( bind->error == NULL ) {
		bind->error = & bind->error_value ;
	}
	bind->buffer = buffer;
	bind->buffer_length = buffer_length;
	bind->buffer_type = type;
}

inline int sql_fetch( MYSQL_STMT * stmt )
{
	switch(r = mysql_stmt_fetch(stmt)) {
	case 0 :
	case MYSQL_DATA_TRUNCATED:
		return 1;
	case MYSQL_NO_DATA : 
		return 0 ;
	default:
		return -1 ;
	}
}

inline bool sql_convert_time (MYSQL_TIME * myt, time_t inter_time)
{
	struct tm tm = *localtime(&inter_time);
	myt->year = tm.tm_year + 1900;
	myt->month = tm.tm_mon + 1;
	myt->day = tm.tm_mday;
	myt->hour = tm.tm_hour;
	myt->minute = tm.tm_min;
	myt->second = tm.tm_sec;
	return true; 
}

inline time_t sql_convert_time(MYSQL_BIND * bind)
{
	if (bind->is_null_value || bind->buffer_type == MYSQL_TYPE_NULL) {
		return -1;
	}

	MYSQL_TIME * my_time = (MYSQL_TIME*)bind->buffer; 

	struct tm t = { 0 };
	t.tm_year = my_time->year - 1900;
	t.tm_mon = my_time->month - 1;
	t.tm_mday = my_time->day;
	t.tm_hour = my_time->hour;
	t.tm_min = my_time->minute;
	t.tm_sec = my_time->second;

	return mktime(&t);
}

inline bool sql_copystr ( MYSQL_STMT * stmt , MYSQL_BIND * bind , string & str, int column ) 
{
	str.clear () ;
	if ( bind->is_null_value || bind->buffer_type == MYSQL_TYPE_NULL) {
		return true ;
	}
	
	if( bind->error_value ) { 
		MYSQL_BIND my_bind ; 
		auto offset = str.size () ; 
		str.resize ( * bind->length ) ; 
		my_bind = * bind ;
		bind->buffer = (char*)str.data() + offset;
		bind->buffer_length =  * bind->length - offset  ; 
		mysql_stmt_fetch_column(stmt, bind, column, offset);
		* bind =my_bind ; 
	}
	else { 
		str.append ((char*)bind->buffer , bind->length_value) ;
	}
	return true ;
}

inline bool sql_bind_time(MYSQL_BIND * bind, time_t t, enum_field_types type)
{
	if (t < 0) {
		bind->buffer_type = MYSQL_TYPE_NULL;
	}
	else {
		sql_convert_time(reinterpret_cast<MYSQL_TIME*>(bind->buffer), t);
		bind->buffer_type = type; 
	}
	return true; 
}