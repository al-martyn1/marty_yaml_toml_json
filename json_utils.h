#pragma once


#include "yaml_utils.h"

#include <string>
#include <iostream>
#include <sstream>
#include <iterator>

#include "nlohmann/json.hpp"
#include "simple_formatter_json_inserter.h"

#include "umba/regex_helpers.h"

#include "yaml_utils.h"


// marty::json_utils::

namespace marty{
namespace json_utils{

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
enum class JsonNodeType
{
    unknown        ,

    // scalar node types
    null           ,
    boolean        ,
    numberUnsigned ,
    numberInteger  ,
    numberFloat    ,
    string         ,

    array          ,

    object

}; // enum class JsonNodeType

//------------------------------
inline
JsonNodeType nodeType(const nlohmann::json &jNode)
{
    if (jNode.is_null())
        return JsonNodeType::null;

    else if (jNode.is_boolean())
        return JsonNodeType::boolean;

    else if (jNode.is_number_integer() && jNode.is_number_unsigned())
        return JsonNodeType::numberUnsigned;

    else if (jNode.is_number_integer())
        return JsonNodeType::numberInteger;

    else if (jNode.is_number_float())
        return JsonNodeType::numberFloat;

    else if (jNode.is_string())
        return JsonNodeType::string;

    else if (jNode.is_array())
        return JsonNodeType::array;

    else if (jNode.is_object())
        return JsonNodeType::object;

    else 
        return JsonNodeType::unknown;

}

//------------------------------
inline
std::string nodeTypeName( JsonNodeType nt )
{
    switch(nt)
    {
        case JsonNodeType::null           : return "null"    ;
        case JsonNodeType::boolean        : return "boolean" ;
        case JsonNodeType::numberUnsigned : return "unsigned";
        case JsonNodeType::numberInteger  : return "integer" ;
        case JsonNodeType::numberFloat    : return "float"   ;
        case JsonNodeType::string         : return "string"  ;
        case JsonNodeType::array          : return "array"   ;
        case JsonNodeType::object         : return "object"  ;
        default                           : return "unknown" ;
    }
}

//------------------------------
inline
bool isScalarNode( JsonNodeType nodeType )
{
    if (nodeType==JsonNodeType::array || nodeType==JsonNodeType::object)
        return false;
    return true;
}

//------------------------------
inline
bool isArrayNode( JsonNodeType nodeType )
{
    if (nodeType==JsonNodeType::array)
        return true;
    return false;
}

//------------------------------
inline
bool isObjectNode( JsonNodeType nodeType )
{
    if (nodeType==JsonNodeType::object)
        return true;
    return false;
}

//------------------------------
inline
std::string nodeTypeName( const nlohmann::json &jNode )
{
    return nodeTypeName(nodeType(jNode));
}

//------------------------------
inline
bool isScalarNode( const nlohmann::json &jNode )
{
    return isScalarNode(nodeType(jNode));
}

//------------------------------
inline
bool isArrayNode( const nlohmann::json &jNode )
{
    return isArrayNode(nodeType(jNode));
}

//------------------------------
inline
bool isObjectNode( const nlohmann::json &jNode )
{
    return isObjectNode(nodeType(jNode));
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
nlohmann::json_pointer<nlohmann::json> makeJsonPointer(const std::string &path)
{
    if (!path.empty() && path[0]!='/')
        return nlohmann::json_pointer<nlohmann::json>(std::string("/")+path);
    return nlohmann::json_pointer<nlohmann::json>(path);
}

inline
nlohmann::json_pointer<nlohmann::json> makeJsonPointer(const char* path)
{
    if (!path)
        //return nlohmann::json_pointer<json>();
        makeJsonPointer(std::string());

    return makeJsonPointer(std::string(path));
}
//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
enum class FileFormat
{
    unknown = 0,
    json,
    yaml
};

// 
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
    data.append(buffer, in.gcount());
    
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




//----------------------------------------------------------------------------
inline
std::string jsonNameEscape( const std::string &str )
{
    std::string res; res.reserve(str.size());

    for( auto ch : str )
    {
        switch(ch)
        {
            case '~': res.append("~0"); break;
            case '/': res.append("~1"); break;
            default : res.append(1,ch);
        }
    }

    return res;
}

//------------------------------
inline
std::string jsonNameUnescape( const std::string &str )
{
    std::string res; res.reserve(str.size());

    bool prevTilde = false;

    for( auto ch : str )
    {
        if (prevTilde)
        {
            switch(ch)
            {
                case '0': res.append(1,'~'); break;
                case '1': res.append(1,'/'); break;
                default : res.append(1,'~'); res.append(1,ch); // не знаю, что это, просто игнорим
            }

            prevTilde = false;
        }
        else
        {
            switch(ch)
            {
                case '~': prevTilde = true; break;
                default : res.append(1,ch);
            }
        }
    }

    if (prevTilde)
    {
        res.append(1,'~');
    }

    return res;
}

//----------------------------------------------------------------------------
/*  Notes for iterator

    https://json.nlohmann.me/api/basic_json/erase/#exceptions

    - Invalidates iterators and references at or after the point of the erase, including the end() iterator.
    - References and iterators to the erased elements are invalidated. Other references and iterators are not affected.

 */
void removePaths( nlohmann::json &jNode
                , const std::basic_regex<char> &r
                , std::regex_constants::match_flag_type flags = std::regex_constants::match_default
                , std::string path = ""
                )
{
    auto nodeType = marty::json_utils::nodeType(jNode);

    std::vector< nlohmann::json::iterator > removeNodeIterators;

    auto eraseChilds = [&]( nlohmann::json &j )
    {
        std::vector< nlohmann::json::iterator >::const_reverse_iterator rit = removeNodeIterators.rbegin();
        for(; rit!=removeNodeIterators.rend(); ++rit )
        {
            nlohmann::json::iterator eit = *rit;
            j.erase(eit);
        }
    };


    if (marty::json_utils::isArrayNode(nodeType))
    {
        unsigned idx = 0;
        for (nlohmann::json::iterator it=jNode.begin(); it!=jNode.end(); ++it, ++idx)
        {
            auto childPath = path + "/" + std::to_string(idx);
            if (umba::regex_helpers::regexMatch(childPath, r, flags))
            {
                removeNodeIterators.emplace_back(it);
            }
        }

        eraseChilds(jNode);

        for (nlohmann::json::iterator it=jNode.begin(); it!=jNode.end(); ++it, ++idx)
        {
            removePaths( *it, r );
        }
    
    }
    else if (marty::json_utils::isObjectNode(nodeType))
    {
        for (nlohmann::json::iterator it=jNode.begin(); it!=jNode.end(); ++it)
        {
            auto childPath = path + "/" + jsonNameEscape(it.key());
            if (umba::regex_helpers::regexMatch(childPath, r, flags))
            {
                removeNodeIterators.emplace_back(it);
            }
        }

        eraseChilds(jNode);

        for (nlohmann::json::iterator it=jNode.begin(); it!=jNode.end(); ++it)
        {
            removePaths( it.value(), r );
        }

    }
}

//----------------------------------------------------------------------------
void removePaths( nlohmann::json &jNode
                , const std::vector< std::basic_regex<char> > &r
                , std::regex_constants::match_flag_type flags = std::regex_constants::match_default
                , std::string path = ""
                )
{
    auto nodeType = marty::json_utils::nodeType(jNode);

    std::vector< nlohmann::json::iterator > removeNodeIterators;

    auto eraseChilds = [&]( nlohmann::json &j )
    {
        std::vector< nlohmann::json::iterator >::const_reverse_iterator rit = removeNodeIterators.rbegin();
        for(; rit!=removeNodeIterators.rend(); ++rit )
        {
            nlohmann::json::iterator eit = *rit;
            j.erase(eit);
        }
    };


    if (marty::json_utils::isArrayNode(nodeType))
    {
        unsigned idx = 0;
        for (nlohmann::json::iterator it=jNode.begin(); it!=jNode.end(); ++it, ++idx)
        {
            auto childPath = path + "/" + jsonNameEscape(std::to_string(idx));
            if (umba::regex_helpers::regexMatch(childPath, r, flags))
            {
                removeNodeIterators.emplace_back(it);
            }
        }

        eraseChilds(jNode);

        for (nlohmann::json::iterator it=jNode.begin(); it!=jNode.end(); ++it, ++idx)
        {
            auto childPath = path + "/" + std::to_string(idx);
            removePaths( *it, r, flags, childPath );
        }
    
    }
    else if (marty::json_utils::isObjectNode(nodeType))
    {
        for (nlohmann::json::iterator it=jNode.begin(); it!=jNode.end(); ++it)
        {
            auto childPath = path + "/" + jsonNameEscape(it.key());
            if (umba::regex_helpers::regexMatch(childPath, r, flags))
            {
                removeNodeIterators.emplace_back(it);
            }
        }

        eraseChilds(jNode);

        for (nlohmann::json::iterator it=jNode.begin(); it!=jNode.end(); ++it)
        {
            auto childPath = path + "/" + jsonNameEscape(it.key());
            removePaths( it.value(), r, flags, childPath );
        }

    }
}

//----------------------------------------------------------------------------
void findPathMatches( nlohmann::json               &jNode
                    , std::vector<std::string>     &pathMatches
                    , const std::basic_regex<char> &r
                    , bool                         recurse = true // lookup recursively in matched nodes
                    , std::regex_constants::match_flag_type flags = std::regex_constants::match_default
                    , std::string path = ""
                    )
{
    auto nodeType = marty::json_utils::nodeType(jNode);

    if (marty::json_utils::isArrayNode(nodeType))
    {
        unsigned idx = 0;
        for (nlohmann::json::iterator it=jNode.begin(); it!=jNode.end(); ++it, ++idx)
        {
            auto childPath = path + "/" + std::to_string(idx);
            if (umba::regex_helpers::regexMatch(childPath, r, flags))
            {
                pathMatches.emplace_back(childPath);
                if (recurse)
                    findPathMatches(*it, pathMatches, r, recurse, flags, childPath);
            }
            else
            {
                findPathMatches(*it, pathMatches, r, recurse, flags, childPath);
            }
        }
    }
    else if (marty::json_utils::isObjectNode(nodeType))
    {
        for (nlohmann::json::iterator it=jNode.begin(); it!=jNode.end(); ++it)
        {
            auto childPath = path + "/" + jsonNameEscape(it.key());
            if (umba::regex_helpers::regexMatch(childPath, r, flags))
            {
                pathMatches.emplace_back(childPath);
                if (recurse)
                    findPathMatches(it.value(), pathMatches, r, recurse, flags, childPath);
            }
            else
            {
                findPathMatches(it.value(), pathMatches, r, recurse, flags, childPath);
            }
        }
    }
}

//----------------------------------------------------------------------------
void findPathMatches( nlohmann::json               &jNode
                    , std::vector<std::string>     &pathMatches
                    , const std::vector< std::basic_regex<char> > &r
                    , bool                         recurse = true
                    , std::regex_constants::match_flag_type flags = std::regex_constants::match_default
                    , std::string path = ""
                    )
{
    auto nodeType = marty::json_utils::nodeType(jNode);

    if (marty::json_utils::isArrayNode(nodeType))
    {
        unsigned idx = 0;
        for (nlohmann::json::iterator it=jNode.begin(); it!=jNode.end(); ++it, ++idx)
        {
            auto childPath = path + "/" + std::to_string(idx);
            if (umba::regex_helpers::regexMatch(childPath, r, flags))
            {
                pathMatches.emplace_back(childPath);
                if (recurse)
                    findPathMatches(*it, pathMatches, r, recurse, flags, childPath);
            }
            else
            {
                findPathMatches(*it, pathMatches, r, recurse, flags, childPath);
            }
        }
    }
    else if (marty::json_utils::isObjectNode(nodeType))
    {
        for (nlohmann::json::iterator it=jNode.begin(); it!=jNode.end(); ++it)
        {
            auto childPath = path + "/" + jsonNameEscape(it.key());
            if (umba::regex_helpers::regexMatch(childPath, r, flags))
            {
                pathMatches.emplace_back(childPath);
                if (recurse)
                    findPathMatches(it.value(), pathMatches, r, recurse, flags, childPath);
            }
            else
            {
                findPathMatches(it.value(), pathMatches, r, recurse, flags, childPath);
            }
        }
    }
}

//----------------------------------------------------------------------------


} // namespace json_utils
} // namespace marty

// marty::json_utils::

