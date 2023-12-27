#pragma once


// #include "yaml_utils.h"

#include <string>
#include <iostream>
#include <sstream>
#include <iterator>

#include <nlohmann/json.hpp>


//----------------------------------------------------------------------------
#ifndef MARTY_ARG_USED

    //! Подавление варнинга о неиспользованном аргументе
    #define MARTY_ARG_USED(x)                   (void)(x)

#endif

//----------------------------------------------------------------------------



#include "simple_formatter_json_inserter.h"

#include "umba/regex_helpers.h"

// #include "yaml_utils.h"

#include "fsss.h"





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
        case JsonNodeType::unknown        : [[fallthrough]]  ;
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
// detail/string_escape.hpp
// nlohmann::detail
// inline std::string escape(std::string s)

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
// detail/string_escape.hpp
// nlohmann::detail
// static void unescape(std::string& s)

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
inline
void removePaths( nlohmann::json &jNode
                , const std::basic_regex<char> &r
                , std::regex_constants::match_flag_type flags = std::regex_constants::match_default
                , std::string path = ""
                )
{
    typedef nlohmann::json  jnode_type;

    // auto nodeType = marty::json_utils::nodeType(jNode);

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


    //if (marty::json_utils::isArrayNode(nodeType))
    if (jNode.type()==jnode_type::value_t::array)
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
    //else if (marty::json_utils::isObjectNode(nodeType))
    else if (jNode.type()==jnode_type::value_t::object)
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
inline
void removePaths( nlohmann::json &jNode
                , const std::vector< std::basic_regex<char> > &r
                , std::regex_constants::match_flag_type flags = std::regex_constants::match_default
                , std::string path = ""
                )
{
    typedef nlohmann::json  jnode_type;

    //auto nodeType = marty::json_utils::nodeType(jNode);

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


    //if (marty::json_utils::isArrayNode(nodeType))
    if (jNode.type()==jnode_type::value_t::array)
    {
        unsigned idx = 0;
        for (nlohmann::json::iterator it=jNode.begin(); it!=jNode.end(); ++it, ++idx)
        {
            auto childPath = path + "/" + jsonNameEscape(std::to_string(idx));
            if (umba::regex_helpers::regexMatch(childPath, r, 0, flags))
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
    //else if (marty::json_utils::isObjectNode(nodeType))
    else if (jNode.type()==jnode_type::value_t::object)
    {
        for (nlohmann::json::iterator it=jNode.begin(); it!=jNode.end(); ++it)
        {
            auto childPath = path + "/" + jsonNameEscape(it.key());
            if (umba::regex_helpers::regexMatch(childPath, r, 0, flags))
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
inline
void findPathMatches( nlohmann::json               &jNode
                    , std::vector<std::string>     &pathMatches
                    , const std::basic_regex<char> &r
                    , bool                         recurse = true // lookup recursively in matched nodes
                    , std::regex_constants::match_flag_type flags = std::regex_constants::match_default
                    , std::string path = ""
                    )
{
    typedef nlohmann::json  jnode_type;

    //auto nodeType = marty::json_utils::nodeType(jNode);

    //if (marty::json_utils::isArrayNode(nodeType))
    if (jNode.type()==jnode_type::value_t::array)
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
    //else if (marty::json_utils::isObjectNode(nodeType))
    else if (jNode.type()==jnode_type::value_t::object)
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
inline
void findPathMatches( nlohmann::json               &jNode
                    , std::vector<std::string>     &pathMatches
                    , const std::vector< std::basic_regex<char> > &r
                    , bool                         recurse = true
                    , std::regex_constants::match_flag_type flags = std::regex_constants::match_default
                    , std::string path = ""
                    )
{
    typedef nlohmann::json  jnode_type;

    //auto nodeType = marty::json_utils::nodeType(jNode);

    //if (marty::json_utils::isArrayNode(nodeType))
    if (jNode.type()==jnode_type::value_t::array)
    {
        unsigned idx = 0;
        for (nlohmann::json::iterator it=jNode.begin(); it!=jNode.end(); ++it, ++idx)
        {
            auto childPath = path + "/" + std::to_string(idx);
            if (umba::regex_helpers::regexMatch(childPath, r, 0, flags))
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
    //else if (marty::json_utils::isObjectNode(nodeType))
    else if (jNode.type()==jnode_type::value_t::object)
    {
        for (nlohmann::json::iterator it=jNode.begin(); it!=jNode.end(); ++it)
        {
            auto childPath = path + "/" + jsonNameEscape(it.key());
            if (umba::regex_helpers::regexMatch(childPath, r, 0, flags))
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
inline
void simpleUpdateNode( nlohmann::json  &jNodeTo, const nlohmann::json  &jNodeFrom, bool allowOverride=true )
{
    typedef nlohmann::json  jnode_type;

    if (jNodeTo == jNodeFrom)
        return;


    if (jNodeTo.type()!=jnode_type::value_t::object || jNodeFrom.type()!=jnode_type::value_t::object)
    {
        jNodeTo = jNodeFrom;
        return;
    }
    // if (jNodeTo.type()!=jnode_type::value_t::object)
    //     return;
    //  
    // if (jNodeFrom.type()!=jnode_type::value_t::object)
    //     return;

    for (nlohmann::json::const_iterator it=jNodeFrom.cbegin(); it!=jNodeFrom.cend(); ++it)
    {
        auto iterTo = jNodeTo.find(it.key());
        if (iterTo==jNodeTo.end())
        {
            // target not exist, assign allowed
            jNodeTo[it.key()] = it.value();
        }
        else
        {
            if ((iterTo.value()==jnode_type::value_t::object && it.value()==jnode_type::value_t::object) || allowOverride)
            {
                simpleUpdateNode(iterTo.value(),it.value(),allowOverride);
            }
        }
    }

}

//----------------------------------------------------------------------------



} // namespace json_utils
} // namespace marty

// marty::json_utils::

