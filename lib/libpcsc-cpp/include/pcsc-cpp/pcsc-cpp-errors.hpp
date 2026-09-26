// SPDX-FileCopyrightText: Estonian Information System Authority
// SPDX-License-Identifier: MIT

#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace pcsc_cpp
{

/** Remove absolute path prefix until 'src' from the given path, '/path/to/src/main.cpp' becomes
 * 'src/main.cpp'. */
constexpr const char* removeAbsolutePathPrefix(std::string_view filePath)
{
    const auto lastSrc = filePath.rfind("src");
    return lastSrc == std::string::npos ? filePath.data() : filePath.substr(lastSrc).data();
}

/** Base class for all pcsc-cpp errors. */
class Error : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

/** Programming or system errors. */
class SystemError : public Error
{
public:
    using Error::Error;
};

/** Base class for all SCard API errors. */
class ScardError : public Error
{
public:
    using Error::Error;
};

/** Thrown when the PC/SC service is not running. */
class ScardServiceNotRunningError : public ScardError
{
public:
    using ScardError::ScardError;
};

/** Thrown when no card readers are connected to the system. */
class ScardNoReadersError : public ScardError
{
public:
    using ScardError::ScardError;
};

/** Thrown when no card is connected to the selected reader. */
class ScardNoCardError : public ScardError
{
public:
    using ScardError::ScardError;
};

/** Thrown when communication with the card or reader fails. */
class ScardCardCommunicationFailedError : public ScardError
{
public:
    using ScardError::ScardError;
};

/** Thrown when the card is removed from the selected reader. */
class ScardCardRemovedError : public ScardError
{
public:
    using ScardError::ScardError;
};

/** Thrown when the card transaction fails. */
class ScardTransactionFailedError : public ScardError
{
public:
    using ScardError::ScardError;
};

} // namespace pcsc_cpp

#define THROW_WITH_CALLER_INFO(ExceptionType, message, file, line, func)                           \
    throw ExceptionType(std::string(message) + " in " + pcsc_cpp::removeAbsolutePathPrefix(file)   \
                        + ':' + std::to_string(line) + ':' + (func))

#define THROW(ExceptionType, message)                                                              \
    THROW_WITH_CALLER_INFO(ExceptionType, message, __FILE__, __LINE__, __func__)

#define REQUIRE_NON_NULL(val)                                                                      \
    if (!(val)) {                                                                                  \
        THROW(std::logic_error, "Null " #val);                                                     \
    }
