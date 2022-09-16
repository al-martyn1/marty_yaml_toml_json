#pragma once

#include <string>

namespace marty{
namespace yaml2json{






class FastSimpleStringStream
{
    std::string m_str;

public:

    FastSimpleStringStream() : m_str() {} // = delete;
    //FastSimpleStringStream( std::string &s ) : m_str(s) {}
    FastSimpleStringStream( const FastSimpleStringStream &ss ) : m_str(ss.m_str) {}

    FastSimpleStringStream( FastSimpleStringStream &&) = delete;
    FastSimpleStringStream& operator=( const FastSimpleStringStream &ss ) = delete;

    const std::string& str() const
    {
        return m_str;
    }

    FastSimpleStringStream& operator<<( const char *str )
    {
        m_str.append(str);
        return *this;
    }

    FastSimpleStringStream& operator<<( char ch )
    {
        m_str.append(1, ch);
        return *this;
    }

    FastSimpleStringStream& operator<<( const std::string &str )
    {
        m_str.append(str);
        return *this;
    }

    FastSimpleStringStream& operator<<( bool b )
    {
        m_str.append(b ? "true" : "false");
        return *this;
    }

    template<typename PrimType>
    FastSimpleStringStream& inserterImpl( PrimType t )
    {
        std::ostringstream os;
        os<<t;
        m_str.append(os.str());
        return *this;
    }

    FastSimpleStringStream& operator<<( double d )
    {
        return inserterImpl(d);
    }

    FastSimpleStringStream& operator<<( float f )
    {
        return inserterImpl(f);
    }

    FastSimpleStringStream& operator<<( std::int32_t i )
    {
        return inserterImpl(i);
    }

    FastSimpleStringStream& operator<<( std::uint32_t u )
    {
        return inserterImpl(u);
    }

    FastSimpleStringStream& operator<<( std::int64_t i )
    {
        return inserterImpl(i);
    }

    FastSimpleStringStream& operator<<( std::uint64_t u )
    {
        return inserterImpl(u);
    }

};


} // namespace yaml2json
} // namespace marty




