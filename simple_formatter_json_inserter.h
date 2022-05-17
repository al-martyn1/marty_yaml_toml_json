#pragma once


#include "umba/simple_formatter.h"


#include "nlohmann/json.hpp"

// using nlohmann::json;

/*
    NLOHMANN_BASIC_JSON_TPL_DECLARATION
    NLOHMANN_BASIC_JSON_TPL

NLOHMANN_BASIC_JSON_TPL_DECLARATION
inline void swap(nlohmann::NLOHMANN_BASIC_JSON_TPL& j1, nlohmann::NLOHMANN_BASIC_JSON_TPL& j2) noexcept(  // NOLINT(readability-inconsistent-declaration-parameter-name)

*/


/*

// How to detect nlohmann::json
#ifndef INCLUDE_NLOHMANN_JSON_HPP_
#define INCLUDE_NLOHMANN_JSON_HPP_

#define NLOHMANN_JSON_VERSION_MAJOR 3
#define NLOHMANN_JSON_VERSION_MINOR 10
#define NLOHMANN_JSON_VERSION_PATCH 5


nlohmann/macro_scope.hpp:188

#define NLOHMANN_BASIC_JSON_TPL_DECLARATION                                \
    template<template<typename, typename, typename...> class ObjectType,   \
             template<typename, typename...> class ArrayType,              \
             class StringType, class BooleanType, class NumberIntegerType, \
             class NumberUnsignedType, class NumberFloatType,              \
             template<typename> class AllocatorType,                       \
             template<typename, typename = void> class JSONSerializer,     \
             class BinaryType>

#define NLOHMANN_BASIC_JSON_TPL                                            \
    basic_json<ObjectType, ArrayType, StringType, BooleanType,             \
    NumberIntegerType, NumberUnsignedType, NumberFloatType,                \
    AllocatorType, JSONSerializer, BinaryType>


#if defined(UMBA_COMPILE_VERBOSE)

    #if defined(_MSC_VER) || defined(__GNUC__) || defined(__clang__)
        #pragma message("Note: umba/simple_formatter.h included")
    #elif defined(__CC_ARM)
        #warning        "Note: umba/simple_formatter.h included"
    #endif

#endif


*/


//#if defined(USE_SIMPLE_FORMATTER_JSON_INSERTER)

    // //NOTE: !!! NLOHMANN_BASIC_JSON_TPL* macros can be changed in future versions of nlohmann json library

    #if !defined(NLOHMANN_BASIC_JSON_TPL_DECLARATION)

        // From nlohmann/detail/macro_scope.hpp:188
        #define NLOHMANN_BASIC_JSON_TPL_DECLARATION                                \
            template<template<typename, typename, typename...> class ObjectType,   \
                     template<typename, typename...> class ArrayType,              \
                     class StringType, class BooleanType, class NumberIntegerType, \
                     class NumberUnsignedType, class NumberFloatType,              \
                     template<typename> class AllocatorType,                       \
                     template<typename, typename = void> class JSONSerializer,     \
                     class BinaryType>
        

    #endif

    #if !defined(NLOHMANN_BASIC_JSON_TPL)

        // From nlohmann/detail/macro_scope.hpp:197
        #define NLOHMANN_BASIC_JSON_TPL                                            \
            basic_json<ObjectType, ArrayType, StringType, BooleanType,             \
            NumberIntegerType, NumberUnsignedType, NumberFloatType,                \
            AllocatorType, JSONSerializer, BinaryType>

    #endif


    #if !defined(NLOHMANN_BASIC_JSON_TPL_DECLARATION) || !defined(NLOHMANN_BASIC_JSON_TPL)

        /*
        #if defined(UMBA_COMPILE_VERBOSE)
         
            #if defined(_MSC_VER) || defined(__GNUC__) || defined(__clang__)
                #pragma message("Note: NLOHMANN_BASIC_JSON_TPL_DECLARATION or NLOHMANN_BASIC_JSON_TPL are not defined - including 'nlohmann/detail/macro_scope.hpp'")
            #elif defined(__CC_ARM)
                #warning        "Note: NLOHMANN_BASIC_JSON_TPL_DECLARATION or NLOHMANN_BASIC_JSON_TPL are not defined - including 'nlohmann/detail/macro_scope.hpp'"
            #endif
         
        #endif

        
        #include "nlohmann/detail/macro_scope.hpp"


        #if !defined(NLOHMANN_BASIC_JSON_TPL_DECLARATION)

            #if defined(UMBA_COMPILE_VERBOSE)
             
                #if defined(_MSC_VER) || defined(__GNUC__) || defined(__clang__)
                    #pragma message("Note: NLOHMANN_BASIC_JSON_TPL_DECLARATION not defined")
                #elif defined(__CC_ARM)
                    #warning        "Note: NLOHMANN_BASIC_JSON_TPL_DECLARATION not defined"
                #endif
             
            #endif

            //#error "NLOHMANN_* (1) not defined"

        #endif

        #if !defined(NLOHMANN_BASIC_JSON_TPL)

            #if defined(UMBA_COMPILE_VERBOSE)
             
                #if defined(_MSC_VER) || defined(__GNUC__) || defined(__clang__)
                    #pragma message("Note: NLOHMANN_BASIC_JSON_TPL not defined")
                #elif defined(__CC_ARM)
                    #warning        "Note: NLOHMANN_BASIC_JSON_TPL not defined"
                #endif
             
            #endif

            //#error "NLOHMANN_* (2) not defined"

        #endif
        */

    #endif

    #if 0
    //template<typename CharType>
    class NlohmannJsonSimpleFormatterOutputAdapter : public ::nlohmann::detail::output_adapter_protocol< char /* CharType */ >
    {

    public:

        explicit NlohmannJsonSimpleFormatterOutputAdapter( umba::SimpleFormatter &fmt) noexcept
            : m_fmt(fmt)
        {}
       
        void write_character(char c) override
        {
            //v.push_back(c);
            m_fmt.writeBuf( &c, 1 );
        }
       
        void write_characters(const char* s, std::size_t length) override
        {
            //std::copy(s, s + length, std::back_inserter(v));
            m_fmt.writeBuf( s, length );
        }
   
    private:

        umba::SimpleFormatter &m_fmt;

    };
    #endif

    #include <sstream>
    #include <iomanip>


    NLOHMANN_BASIC_JSON_TPL_DECLARATION
    umba::SimpleFormatter& operator<<( umba::SimpleFormatter& o, const nlohmann::NLOHMANN_BASIC_JSON_TPL& j )
    {

        #if 0

        // read width member and use it as indentation parameter if nonzero

        auto savedStreamWidth = o.width();
    
        const bool pretty_print = savedStreamWidth > 0;
        const auto indentation = pretty_print ? savedStreamWidth : 0;
    
        // reset width to 0 for subsequent calls to this stream
        o.width(0);
    
        // do the actual serialization
        using namespace nlohmann;
    
        // typedef nlohmann::NLOHMANN_BASIC_JSON_TPL :: serializer    serializer;

        typedef nlohmann::NLOHMANN_BASIC_JSON_TPL                basic_json_t;
        typedef  ::nlohmann::detail::serializer<basic_json_t>    serializer  ;

        //serializer s(nlohmann::detail::output_adapter<char>(o), o.fill());
        serializer s(NlohmannJsonSimpleFormatterOutputAdapter(o) /* , o.fill() */ );
        
        s.dump(j, pretty_print, false, static_cast<unsigned int>(indentation));
    
        o.width(savedStreamWidth);

        #else

        std::ostringstream oss;
        oss << std::setw(o.width()) << j;

        o << oss.str();

        #endif



        return o;
    }

    #if defined(NLOHMANN_BASIC_JSON_TPL_DECLARATION)
        #undef NLOHMANN_BASIC_JSON_TPL_DECLARATION
    #endif

    #if defined(NLOHMANN_BASIC_JSON_TPL)
        #undef NLOHMANN_BASIC_JSON_TPL
    #endif


    #if defined(NLOHMANN_BASIC_JSON_TPL_DECLARATION) || defined(NLOHMANN_BASIC_JSON_TPL)

        /*
        #if defined(UMBA_COMPILE_VERBOSE)
         
            #if defined(_MSC_VER) || defined(__GNUC__) || defined(__clang__)
                #pragma message("Note: Undef NLOHMANN_* macros - including 'nlohmann/detail/macro_unscope.hpp'")
            #elif defined(__CC_ARM)
                #warning        "Note: Undef NLOHMANN_* macros - including 'nlohmann/detail/macro_unscope.hpp'"
            #endif
         
        #endif

        #include "nlohmann/detail/macro_unscope.hpp"
        */

    #endif

//#endif // USE_SIMPLE_FORMATTER_JSON_INSERTER



