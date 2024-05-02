#pragma once


#include "yaml_utils.h"
#include "json_utils.h"
#include "fsss.h"

//#include "nlohmann/json.hpp"




// marty::json_utils::

namespace marty{
namespace json_utils{


//----------------------------------------------------------------------------
enum class FileFormat
{
    unknown = 0,
    json,
    yaml
};

inline
nlohmann::json parseJsonOrYaml( const std::string &data
                              , bool allowComments = true
                              , std::string *pErrMsg = 0
                              , std::string *pTmpJson = 0
                              , FileFormat *pDetectedFormat = 0
                              )
{
    nlohmann::json jRes;

    FileFormat detectedFormat = FileFormat::unknown;

    if (pDetectedFormat)
        *pDetectedFormat = detectedFormat;

    bool jsonStartFound = false;
    std::string::size_type pos = 0;
    for(; pos!=data.size(); ++pos)
    {
        if (data[pos]==' ')
            continue;

        if (data[pos]=='{')
            jsonStartFound = true;
            
        break;
    }

    if (jsonStartFound)
    {
        try
        {
            jRes = nlohmann::json::parse( data
                                        , nullptr        // parser_callback_t
                                        , true           // allow_exceptions
                                        , allowComments  // ignore_comments
                                        );
            detectedFormat = FileFormat::json;
        }
        catch(const std::exception &e)
        {
            if (pErrMsg)
               *pErrMsg = e.what();
        }
    
        if (pDetectedFormat)
            *pDetectedFormat = detectedFormat;
        return jRes;
    }



    try
    {
        YAML::Node yamlNode = YAML::Load(data);

        marty::yaml2json::FastSimpleStringStream fssm;
        //marty::yaml2json::writeJson(fssm, yamlNode, -1);
        marty::yaml2json::writeJson(fssm, yamlNode, 2);

        if (pTmpJson)
           *pTmpJson = fssm.str();
        
        jRes = nlohmann::json::parse( fssm.str()
                                    , nullptr        // parser_callback_t
                                    , true           // allow_exceptions
                                    , allowComments  // ignore_comments
                                    );
        if (pDetectedFormat)
            *pDetectedFormat = FileFormat::yaml;


    }
    catch (const YAML::Exception& e)
    {
        if (pErrMsg)
           *pErrMsg = e.what();
    }
    catch (const std::exception& e)
    {
        if (pErrMsg)
           *pErrMsg = e.what();
    }
    catch (...)
    {
        if (pErrMsg)
           *pErrMsg = "unknown error";
    }

    return jRes;
    
}

//------------------------------
// file_input_adapter
// input_stream_adapter
// iterator_input_adapter


inline
nlohmann::json parseJsonOrYaml( std::istream &in
                              , bool allowComments = true
                              , std::string *pErrMsg = 0
                              , std::string *pTmpJson = 0
                              , FileFormat *pDetectedFormat = 0
                              )
{
    std::string data;
    char buffer[4096];
    while (in.read(buffer, sizeof(buffer)))
        data.append(buffer, sizeof(buffer));
    data.append(buffer, (unsigned)in.gcount());
    
    return parseJsonOrYaml( data, allowComments, pErrMsg, pTmpJson, pDetectedFormat );

#if 0
    nlohmann::json jRes;

    try
    {
        // https://habr.com/ru/post/122283/
        // https://en.cppreference.com/w/cpp/iterator/istream_iterator
        jRes = nlohmann::json::parse( std::istream_iterator<char>(in)
                                    , std::istream_iterator<char>()
                                    , nullptr        // parser_callback_t
                                    , true           // allow_exceptions
                                    , allowComments  // ignore_comments
                                    );
    }
#endif    
}

//----------------------------------------------------------------------------
inline
nlohmann::json parseJsonOrYamlFromFile( const std::string &fileName
                              , bool allowComments = true
                              , std::string *pErrMsg = 0
                              , std::string *pTmpJson = 0
                              , FileFormat *pDetectedFormat = 0
                              )
{
    std::ifstream in(fileName.c_str());

    if (!in)
    {
        if (pDetectedFormat)
            *pDetectedFormat = FileFormat::unknown;

        if (pErrMsg)
            *pErrMsg = "Failed to open file '" + fileName + "'";

        return nlohmann::json{};
    }

    return parseJsonOrYaml( in, allowComments, pErrMsg, pTmpJson, pDetectedFormat );
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
std::string makeIndentStr( int indent )
{
    if (indent>=0)
        return std::string( (std::string::size_type)indent, ' ' );

    //return std::string(" ");
    return std::string();
}

//----------------------------------------------------------------------------
inline
bool isScalar( nlohmann::json &j )
{
    if (j.is_null() || j.is_boolean() || j.is_number() || j.is_string())
        return true;
    return false;
}

inline
bool isNeedToBeQuotedImpl(const std::string & str)
{
    // if (!str.empty() && (str[0]=='<' || str[0]=='\''))
    // {
    //     std::cout << "";
    // }

    for( auto ch : str )
    {
        switch(ch)
        {
            case ' ':
            case '\\':
            case '\'':
            case '\"':
            case '[':
            case ']':
            case '*':
            case '#':
                 return true;
        };
    }

    return false;
}

inline
bool isNeedToBeQuoted(const std::string & str)
{
/*
    if (!str.empty() && str[0]=='[')
    {
        std::cout << "NEED_TO_BE_QUOTED: " << isNeedToBeQuotedImpl(str) << "\n";
    }
*/
    return isNeedToBeQuotedImpl(str);
}

inline
std::string escapeSingleQuotes( const std::string &s)
{
    std::string res; res.reserve(s.size());
    for( auto ch : s )
    {
        if (ch=='\'')
            res.append(1,ch);
        res.append(1,ch);
    }

    return res;
}

template<typename StreamType> inline
bool writeScalar( StreamType &s, nlohmann::json &j )
{
    if (j.is_null())
    {
        s << "null";
    }
    else if (j.is_boolean())
    {
        auto val = j.get<bool>();
        s << (val?"true":"false");
    }
    else if (j.is_number_integer() && j.is_number_unsigned())
    {
        auto val = j.get<std::uint64_t>();
        s << val;
    }
    else if (j.is_number_integer())
    {
        auto val = j.get<std::int64_t>();
        s << val;
    }
    else if (j.is_number_float())
    {
        auto val = j.get<double>();
        s << val;
    }
    else if (j.is_string())
    {
        auto val = j.get<std::string>();
        if (val.empty() || val=="null" || isNeedToBeQuoted(val))
            s << '\'' << escapeSingleQuotes(val) << '\'';
        else
            s << val;
    }
    else // j.is_object() || j.is_array()
    {
        return false;;
    }

    return true;
}

//----------------------------------------------------------------------------
inline
bool writeScalar( std::string &str, nlohmann::json &j )
{
    marty::yaml2json::FastSimpleStringStream fssm;
    writeScalar(fssm, j);
    str = fssm.str();
    return true;
}

//----------------------------------------------------------------------------
inline
std::string getScalarStr( nlohmann::json &j )
{
    marty::yaml2json::FastSimpleStringStream fssm;
    writeScalar(fssm, j);
    return fssm.str();
}

//----------------------------------------------------------------------------
template<typename StreamType> inline
void writeNodeImpl( StreamType &s, nlohmann::json &j // j - не меняется, просто нет константной версии begin/end
                  , int indent, int indentInc, bool noFirstIndent = false
                  ) 
{
    if (indent<0)
        indent = 0;

    if (indentInc<1)
        indentInc = 1;

    if (writeScalar( s, j ))
    {
        return;
    }
    else if (j.is_object())
    {
        bool bFirst = true;
        for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it)
        {
            if (!(bFirst && noFirstIndent))
            {
                s << makeIndentStr(indent);
            }

            // if (it.key()=="x-pattern")
            // {
            //     s << "XPATTERN" << "\n";
            // }

            s << it.key() << ":";
            auto val = it.value();
            if (isScalar(val))
            {
                s << " "; 
                writeScalar(s,val);
                s << "\n";
            }
            else
            {
                s << "\n";
                writeNodeImpl( s, val, indent+indentInc, indentInc );
            }

            bFirst = false;
        }

    }
    else if (j.is_array())
    {
        bool bFirst = true;
        for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it)
        {
            s << makeIndentStr(indent) << "- ";
            if (isScalar(*it))
            {
                writeScalar(s,*it);
                s << "\n";
            }
            else
            {
                writeNodeImpl( s, *it, indent+indentInc, indentInc, true /* noFirstIndent */  );
            }

            bFirst = false;
        }
    }

}

//----------------------------------------------------------------------------
template<typename StreamType> inline
void writeYaml( StreamType &s, nlohmann::json &j // j - не меняется, просто нет константной версии begin/end
              )
{
    writeNodeImpl( s, j, 0, 2, false );
}

//----------------------------------------------------------------------------






} // namespace json_utils
} // namespace marty

// marty::json_utils::

