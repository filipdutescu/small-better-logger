/*
MIT License

Copyright (c) 2019 Filip Dutescu

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef SMALL_BETTER_LOGGER_H
#define SMALL_BETTER_LOGGER_H

// Log Levels macros to be used with "SBLOGGER_LOG_LEVEL" macro for defining a default level
#define SBLOGGER_LEVEL_TRACE     0
#define SBLOGGER_LEVEL_DEBUG     1
#define SBLOGGER_LEVEL_INFO      2
#define SBLOGGER_LEVEL_WARN      3
#define SBLOGGER_LEVEL_ERROR     4
#define SBLOGGER_LEVEL_CRITICAL  5
#define SBLOGGER_LEVEL_OFF       6
//
// Define your preferred active level (e.g. using the macro bellow), or use the static method Logger::SetLoggingLevel(const LOG_LEVELS& level)
//
//#define SBLOGGER_LOG_LEVEL SBLOGGER_LEVEL_TRACE

//
// Either uncomment or define this macro, should your environment support colours and you wish to use them
//
//#define SBLOGGER_COLORS

#if __cplusplus != 199711L
	#if __cplusplus < 201703L
		// For pre C++17 compilers define the "SBLOGGER_LEGACY" macro, to replace <filesystem> operations with regex and other alternatives
		#define SBLOGGER_LEGACY
	#endif
	#if __cplusplus <= 201703L
		// For formatting dates to string pre C++20
		#define SBLOGGER_OLD_DATES
	#endif
#endif

// Cross-platform newline macros
#ifdef SBLOGGER_NIX
	#define SBLOGGER_NEWLINE '\n'
	#define SBLOGGER_PATH_SEPARATOR '/'
#elif SBLOGGER_OS9
	#define SBLOGGER_NEWLINE '\r'
	#define SBLOGGER_PATH_SEPARATOR '/'
#else
	#define SBLOGGER_NEWLINE "\r\n"
	#define SBLOGGER_PATH_SEPARATOR '\\'
#endif

#ifdef SBLOGGER_LEGACY
	// Raw file path regex as string literal
	#define SBLOGGER_RAW_FILE_PATH_REGEX R"regex(^(((([a-zA-Z]\:|\\)+\\[^\/\\:"'*?<>|\0]+)+|([^\/\\:"'*?<>|\0]+)+)|(((\.\/|\~\/|\/[^\/\\:"'*?~<>|\0]+\/)?[^\/\\:"'*?~<>|\0]+)+))$)regex"
#endif

// Used for writing to output stream
#include <iostream>
#include <fstream>

// Used for formatting and creating the output string
#include <sstream>
#include <vector>

// Used for processing using time such as timed file logs and date formatting (if SBLOGGER_LEGACY is not defined)
#include <chrono>

// Used for asynchronous operations such as changing files for timed file logs
#include <thread>
#include <mutex>
#include <condition_variable>

// For pre C++17 compilers define the "SBLOGGER_LEGACY" macro, to replace <filesystem> operations with regex and other alternatives
#ifdef SBLOGGER_LEGACY
// Used for file path checking
#include <regex>
#else
// Used for file path checking and file manipulation
#include <filesystem>
#endif

// For formatting dates to string pre C++20
#ifdef SBLOGGER_OLD_DATES
// Make use of std::strftime
#include <ctime>
#endif

// For pre C++17 compilers define the "LEGACY" macro, to replace <filesystem> operations with regex and other alternatives
#ifdef SBLOGGER_LEGACY
	// File Path Regex using std::regex
	#define SBLOGGER_FILE_PATH_REGEX std::regex(SBLOGGER_RAW_FILE_PATH_REGEX)
#endif

namespace sblogger
{
	//
	// Loggers' declaration
	//

	// Basic Logger
	// Abstract class which implements basic logger methods and members (ex.: auto flush, format, replace formatters etc)
	class Logger;
	using logger = Logger;

	// Stream Logger
	// Used to log messages to a non-file stream (ex.: STDOUT, STDERR, STDLOG)
	class StreamLogger;
	using stream_logger = StreamLogger;

	// File Logger
	// Used to log messages to a file stream
	class FileLogger;
	using file_logger = FileLogger;

	//
	// Custom exceptions' definition
	//

	// Base exception
	class SBLoggerException;
	using sblogger_exception = SBLoggerException;

	// NullOrEmptyPathException
	// Thrown when the given file path is null or empty
	class NullOrEmptyPathException;
	using null_or_empty_path_exception = NullOrEmptyPathException;
	
	// NullOrWhitespaceNameException
	// Thrown when the given file name is null or whitespace
	class NullOrWhitespaceNameException;
	using null_or_whitespace_name_exception = NullOrWhitespaceNameException;
	
	// InvalidFilePathException
	// Thrown when the specified file could not be opened
	class InvalidFilePathException;
	using invalid_file_path_exception = InvalidFilePathException;
	
	// TimeRangeException  
	// Thrown when a time related value is out of bounds (e.g.: hours not in [0, 23])
	class TimeRangeException;
	using time_range_exception = TimeRangeException;

	//
	// Enum definitions
	//

	// Log level enum. Contains all possible log levels, such as TRACE, ERROR, FATAL etc.
	enum class LOG_LEVELS
	{ 
		TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL, OFF 
	};
	using log_levels = LOG_LEVELS;

	// Stream types to be used by a Logger instance
	enum class STREAM_TYPE
	{
		STDOUT, STDERR, STDLOG
	};
	using stream_type = STREAM_TYPE;

	//
	// Custom exceptions
	//

	// Base exception
	class SBLoggerException : public std::exception
	{
	protected:
		//
		// Protected members
		//

		std::string m_Exception;

		//
		// Protected methods
		//

		// Creates an exception with a given message
		SBLoggerException(std::string& exception);

		// Creates an exception with a given message
		SBLoggerException(std::string&& exception);

		// Creates an exception with a given message
		SBLoggerException(const char* exception);

	public:
		// Throw exception
		~SBLoggerException() throw() = default;

		// Get error message
		inline const char* What() const throw();
	};

	//
	// Constructors and destructors
	//

	// Creates an exception with a given message
	SBLoggerException::SBLoggerException(std::string& exception)
		: std::exception(exception.c_str()), m_Exception(exception)
	{ }

	// Creates an exception with a given message
	SBLoggerException::SBLoggerException(std::string&& exception)
		: std::exception(exception.c_str()), m_Exception(exception)
	{ }

	// Creates an exception with a given message
	SBLoggerException::SBLoggerException(const char* exception)
		: std::exception(exception), m_Exception(exception)
	{ }

	//
	// Public methods
	//

	// Get error message
	inline const char* SBLoggerException::What() const throw() { return m_Exception.c_str(); }

	//
	// NullOrEmptyPathException
	//

	// Thrown when the given file path is null or empty
	class NullOrEmptyPathException : public SBLoggerException
	{
	public:
		// Default constructor
		NullOrEmptyPathException();
	};

	// Default constructor
	NullOrEmptyPathException::NullOrEmptyPathException() 
		: SBLoggerException("File path cannot be null or empty.")
	{ }

	//
	// NullOrWhitespaceNameException
	//

	// Thrown when the given file name is null or whitespace
	class NullOrWhitespaceNameException : public SBLoggerException
	{
	public:
		// Default constructor
		NullOrWhitespaceNameException();
	};

	// Default constructor
	NullOrWhitespaceNameException::NullOrWhitespaceNameException() 
		: SBLoggerException("File name cannot be null or whitespace.")
	{ }

	//
	// InvalidFilePathException
	//

	// Thrown when the specified file could not be opened
	class InvalidFilePathException : public SBLoggerException
	{
	public:
		//
		// Constructors and destructors
		//

		// Default constructor
		InvalidFilePathException();

		// Creates an invalid file path exception with a given message
		InvalidFilePathException(const std::string& filePath);

		// Creates an invalid file path exception with a given message
		InvalidFilePathException(const std::string&& filePath);
	};

	//
	// Constructors and destructors
	//

	// Default constructor
	InvalidFilePathException::InvalidFilePathException() 
		: SBLoggerException("Cannot open log file to write to.")
	{ }

	// Creates an invalid file path exception with a given message
	InvalidFilePathException::InvalidFilePathException(const std::string& filePath)
		: SBLoggerException("Cannot open log file " + filePath + '.')
	{ }

	// Creates an invalid file path exception with a given message
	InvalidFilePathException::InvalidFilePathException(const std::string&& filePath)
		: SBLoggerException("Cannot open log file " + filePath + '.')
	{ }

	//
	// TimeRangeException  
	//

	// Thrown when a time related value is out of bounds (e.g.: hours not in [0, 23])
	class TimeRangeException : public SBLoggerException
	{
	public:
		//
		// Constructors and destructors
		//

		// Default constructor
		TimeRangeException();
	};

	//
	// Constructors and destructors
	//

	// Default constructor
	TimeRangeException::TimeRangeException()
		: SBLoggerException("Time value not in the allowed interval.")
	{ }

	//
	// Classes' definitions
	//

	// Abstract class which implements basic logger methods and members (ex.: auto flush, format, replace formatters etc)
	class Logger
	{
	protected:
		//
		// Protected members
		//

		std::string m_Format;
		bool m_AutoFlush;
		size_t m_IndentCount;
		static LOG_LEVELS s_CurrentLogLevel;

		//
		// Protected constructors
		//

		// Initialize a logger, with a format, auto flush (by default)
		inline Logger(const std::string& format, bool autoFlush);

		// Initialize a logger, with no format, auto flush (by default)
		inline Logger(bool autoFlush) noexcept;

		// Copy constructor
		inline Logger(const Logger& other) noexcept;

		// Move constructor
		inline Logger(Logger&& other) noexcept;

		//
		// Protected methods
		//

		// Writes string to appropriate stream
		inline virtual void writeToStream(const std::string&& message) = 0;

		// Converts a T value to a string to be used in writing a log
		template<typename T>
		inline std::string stringConvert(const T& t) const noexcept;

		// Adds ANSII colour codes if current stream supports them
		// (The Logger base class does not do anything with the message, the method needing implementation from derived classes)
		inline virtual void addColours(std::string& message) const noexcept;

		// Add indent to string (if it is set)
		inline void addIndent(std::string& message) const noexcept;

		// Add padding to string (if padding format exists)
		inline void addPadding(std::string& message) const noexcept;

		// Append format (if it exists) and replace all "{n}" placeholders with their respective values (n=0,...)
		inline std::string replacePlaceholders(std::string message, std::vector<std::string>&& items) const noexcept;

		// Replaces predefined placeholders from the message (e.g. "%er" will be changed to "Error" in the final message)
		inline void replacePredefinedPlaceholders(std::string& message) const noexcept;

		// Replace current logging level in format
		inline void replaceCurrentLevel(std::string& message) const noexcept;

		// Replace other placeholders, such as those for file, line and function related information
		inline void replaceOthers(std::string& message, const char* file, const char* line, const char* function) const noexcept;

		// Replace date format using std::strftime (pre C++20) or std::chrono::format
		inline void replaceDateFormats(std::string& message) const noexcept;

	public:
		// Default destructor
		virtual ~Logger() = default;

		//
		// Public methods
		//

		// Set the current logging level to one of the "LOG_LEVELS" options (ex.: TRACE, DEBUG, INFO etc). 
		static inline void SetLoggingLevel(const LOG_LEVELS& level) noexcept;

		// Get the current logging level (one of the "LOG_LEVELS" options, ex.: TRACE, DEBUG, INFO etc). 
		static inline const LOG_LEVELS GetLoggingLevel() noexcept;

		// Get the current log format
		inline const std::string GetFormat() const noexcept;

		// Set the current log format to "format"
		inline void SetFormat(const std::string& format);

		// Flush appropriate stream
		virtual inline void Flush() noexcept = 0;

		// Indent (prepend '\t') log, returns the number of indents the final message will contain
		inline const size_t Indent() noexcept;

		// Dedent (remove '\t') log, returns the number of indents the final message will contain
		inline const size_t Dedent() noexcept;

		//
		// Generic Methods: Write a TRACE level message (depending on the specified "LOG_LEVEL") to a stream
		//

		// Writes to the stream the newline character, assuming a default log level of TRACE
		inline void WriteLine(LOG_LEVELS logLevel = LOG_LEVELS::TRACE);

		// Writes to the stream a message and inserts values into placeholders (should they exist), assuming a default log level of TRACE
		template<typename ...T>
		inline void Write(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist), assuming a default log level of TRACE
		template<typename ...T>
		inline void Write(const std::string&& message, const T&& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist) and finishes with the newline character, assuming a default log level of TRACE
		template<typename ...T>
		inline void WriteLine(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist) and finishes with the newline character, assuming a default log level of TRACE
		template<typename ...T>
		inline void WriteLine(const std::string&& message, const T&& ...t);

		//
		// Generic Methods: Write a level message (depending on the specified "LOG_LEVEL") to a stream 
		//

		// Writes to the stream a message and inserts values into placeholders (should they exist), of "logLevel" importance
		template<typename ...T>
		inline void Write(LOG_LEVELS logLevel, const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist), of "logLevel" importance
		template<typename ...T>
		inline void Write(LOG_LEVELS logLevel, const std::string&& message, const T&& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist) and finishes with the newline character, of "logLevel" importance
		template<typename ...T>
		inline void WriteLine(LOG_LEVELS logLevel, const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist) and finishes with the newline character, of "logLevel" importance
		template<typename ...T>
		inline void WriteLine(LOG_LEVELS logLevel, const std::string&& message, const T&& ...t);

		//
		// Generic Methods: Write a TRACE level message to a stream
		//

		// Writes to the stream a message and inserts values into placeholders (should they exist), of TRACE importance
		template<typename ...T>
		inline void Trace(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist), of TRACE importance
		template<typename ...T>
		inline void Trace(const std::string&& message, const T&& ...t);

		//
		// Generic Methods: Write a DEBUG level message to a stream 
		//

		// Writes to the stream a message and inserts values into placeholders (should they exist), of DEBUG importance
		template<typename ...T>
		inline void Debug(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist), of DEBUG importance
		template<typename ...T>
		inline void Debug(const std::string&& message, const T&& ...t);

		//
		// Generic Methods: Write a INFO level message to a stream 
		//

		// Writes to the stream a message and inserts values into placeholders (should they exist), of INFO importance
		template<typename ...T>
		inline void Info(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist), of INFO importance
		template<typename ...T>
		inline void Info(const std::string&& message, const T&& ...t);

		//
		// Generic Methods: Write a WARN level message to a stream 
		//

		// Writes to the stream a message and inserts values into placeholders (should they exist), of WARN importance
		template<typename ...T>
		inline void Warn(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist), of WARN importance
		template<typename ...T>
		inline void Warn(const std::string&& message, const T&& ...t);

		//
		// Generic Methods: Write a ERROR level message to a stream 
		//

		// Writes to the stream a message and inserts values into placeholders (should they exist), of ERROR importance
		template<typename ...T>
		inline void Error(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist), with ERROR importance
		template<typename ...T>
		inline void Error(const std::string&& message, const T&& ...t);

		//
		// Generic Methods: Write a CRITICAL level message to a stream 
		//

		// Writes to the stream a message and inserts values into placeholders (should they exist), of CRITICAL importance
		template<typename ...T>
		inline void Critical(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist), with CRITICAL importance
		template<typename ...T>
		inline void Critical(const std::string&& message, const T&& ...t);
	};
	// Static member initialization
	// Check to see what is the current active log level, by default use TRACE
#if SBLOGGER_LOG_LEVEL == SBLOGGER_LEVEL_DEBUG
	LOG_LEVELS Logger::s_CurrentLogLevel = LOG_LEVELS::DEBUG;
#elif SBLOGGER_LOG_LEVEL == SBLOGGER_LEVEL_INFO
	LOG_LEVELS Logger::s_CurrentLogLevel = LOG_LEVELS::INFO;
#elif SBLOGGER_LOG_LEVEL == SBLOGGER_LEVEL_WARN
	LOG_LEVELS Logger::s_CurrentLogLevel = LOG_LEVELS::WARN;
#elif SBLOGGER_LOG_LEVEL == SBLOGGER_LEVEL_ERROR
	LOG_LEVELS Logger::s_CurrentLogLevel = LOG_LEVELS::ERROR;
#elif SBLOGGER_LOG_LEVEL == SBLOGGER_LEVEL_CRITICAL
	LOG_LEVELS Logger::s_CurrentLogLevel = LOG_LEVELS::CRITICAL;
#elif SBLOGGER_LOG_LEVEL == SBLOGGER_LEVEL_OFF
	LOG_LEVELS Logger::s_CurrentLogLevel = LOG_LEVELS::OFF;
#else
	LOG_LEVELS Logger::s_CurrentLogLevel = LOG_LEVELS::TRACE;
#endif

	//
	// Protected constructors
	//

	// Initialize a logger, with a format, auto flush (by default)
	inline Logger::Logger(const std::string& format, bool autoFlush)
		: m_Format(format), m_AutoFlush(autoFlush), m_IndentCount(0u)
	{
		if (!m_Format.empty())
		{
			addPadding(m_Format);
#ifdef SBLOGGER_LEGACY
			std::string placeholder = "tr";
#else
			std::string_view placeholder = "tr";
#endif
			size_t placeholderPosition;
			while ((placeholderPosition = m_Format.find(placeholder)) != std::string::npos && (m_Format[placeholderPosition - 1u] == '%' || m_Format[placeholderPosition - 2u] == '%'))
				m_Format[placeholderPosition - 1u] == '^' ? m_Format.replace(placeholderPosition - 2u, placeholder.size() + 2u, "TRACE") : m_Format.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Trace");

			placeholder = "dbg";
			while ((placeholderPosition = m_Format.find(placeholder)) != std::string::npos && (m_Format[placeholderPosition - 1u] == '%' || m_Format[placeholderPosition - 2u] == '%'))
				m_Format[placeholderPosition - 1u] == '^' ? m_Format.replace(placeholderPosition - 2u, placeholder.size() + 2u, "DEBUG") : m_Format.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Debug");

			placeholder = "inf";
			while ((placeholderPosition = m_Format.find(placeholder)) != std::string::npos && (m_Format[placeholderPosition - 1u] == '%' || m_Format[placeholderPosition - 2u] == '%'))
				m_Format[placeholderPosition - 1u] == '^' ? m_Format.replace(placeholderPosition - 2u, placeholder.size() + 2u, "INFO") : m_Format.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Info");

			placeholder = "wn";
			while ((placeholderPosition = m_Format.find(placeholder)) != std::string::npos && (m_Format[placeholderPosition - 1u] == '%' || m_Format[placeholderPosition - 2u] == '%'))
				m_Format[placeholderPosition - 1u] == '^' ? m_Format.replace(placeholderPosition - 2u, placeholder.size() + 2u, "WARN") : m_Format.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Warn");

			placeholder = "er";
			while ((placeholderPosition = m_Format.find(placeholder)) != std::string::npos && (m_Format[placeholderPosition - 1u] == '%' || m_Format[placeholderPosition - 2u] == '%'))
				m_Format[placeholderPosition - 1u] == '^' ? m_Format.replace(placeholderPosition - 2u, placeholder.size() + 2u, "ERROR") : m_Format.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Error");

			placeholder = "crt";
			while ((placeholderPosition = m_Format.find(placeholder)) != std::string::npos && (m_Format[placeholderPosition - 1u] == '%' || m_Format[placeholderPosition - 2u] == '%'))
				m_Format[placeholderPosition - 1u] == '^' ? m_Format.replace(placeholderPosition - 2u, placeholder.size() + 2u, "CRITICAL") : m_Format.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Critical");
		}
	}

	// Initialize a logger, with no format, auto flush (by default)
	inline Logger::Logger(bool autoFlush) noexcept
		: m_Format(), m_AutoFlush(autoFlush), m_IndentCount(0u)
	{ }

	// Copy constructor
	inline Logger::Logger(const Logger& other) noexcept
		: m_Format(other.m_Format), m_AutoFlush(other.m_AutoFlush), m_IndentCount(other.m_IndentCount)
	{ }

	// Move constructor
	inline Logger::Logger(Logger&& other) noexcept
	{
		m_Format = other.m_Format;
		m_AutoFlush = other.m_AutoFlush;
		m_IndentCount = other.m_IndentCount;
	}

	//
	// Protected methods
	//

	// Converts a T value to a string to be used in writing a log
	template<typename T>
	inline std::string Logger::stringConvert(const T& t) const noexcept
	{
		std::stringstream ss;
		ss << t;
		return ss.str();
	}

	// Adds ANSII colour codes if current stream supports them
	// (The Logger base class does not do anything with the message, the method needing implementation from derived classes)
	inline void Logger::addColours(std::string& message) const noexcept
	{ 
		char colours[][12]{ { "reset" }, { "black" }, { "red" }, { "green" }, { "yellow" }, { "blue" }, { "magenta" }, { "cyan" }, { "white" },
				{ "bg-black" }, { "bg-red" }, { "bg-green" }, { "bg-yellow" }, { "bg-blue" }, { "bg-magenta" }, { "bg-cyan" }, { "bg-white" } };

		size_t placeholderPosition, placeholderSize;
		char currentColour[17]{ '{' };

		for (size_t i = 0u; i < 17u; ++i)
			if ((placeholderPosition = message.find(colours[i])) != std::string::npos
				&& placeholderPosition > 1u && message[placeholderPosition - 1u] == '{'
				&& placeholderPosition < (message.size() - 1u) && message[placeholderPosition + std::strlen(colours[i])] == '}')
			{
				std::strncpy(currentColour + 1, colours[i], std::strlen(colours[i]) + 1u);
				std::strncat(currentColour, "}", 1u);
				while ((placeholderPosition = message.find(currentColour)) != std::string::npos)
				{
					placeholderSize = std::strlen(currentColour);
					message[placeholderPosition - 1u] == '^' ?
						message.replace(placeholderPosition - 2u, placeholderSize + 2u, "")
						: message.replace(placeholderPosition - 1u, placeholderSize + 1u, "");
				}
			}
	}

	// Add indent to string (if it is set)
	inline void Logger::addIndent(std::string& message) const noexcept
	{
		for (size_t i = 0u; i < m_IndentCount; ++i)
			message = '\t' + message;
	}

	// Add padding to string (if padding format exists)
	inline void Logger::addPadding(std::string& message) const noexcept
	{
#ifdef SBLOGGER_LEGACY
		std::string placeholders[] { "msg", "lvl", "tr", "dbg", "inf", "wn", "er", "crt" };
#else
		std::string_view placeholders[] { "msg", "lvl", "tr", "dbg", "inf", "wn", "er", "crt" };
#endif
		std::string digits = "1234567890", floatDigits = "1234567890.", currentPadding;
		size_t placeholderPosition, offset = 0u, noDigits, noDecimals, currentSectionEnd, placeholderSize, noPlaceholders = 8u;
		float noSpacesLeft, noSpacesRight;
		char nextCharacter;

		while ((placeholderPosition = message.find_first_of(digits, offset)) != std::string::npos)
		{
			if (message[placeholderPosition - 1u] != '%' && (placeholderPosition > 1u && message[placeholderPosition - 2u] != '%' && message[placeholderPosition - 1u] != '.'))
				offset += placeholderPosition + 1u;
			else
			{
				if(message[placeholderPosition - 1u] != '%')
					currentPadding = message.substr(placeholderPosition - 1u, message.find_first_not_of(floatDigits, placeholderPosition + 1u) - placeholderPosition + 1u);
				else
					currentPadding = message.substr(placeholderPosition, message.find_first_not_of(floatDigits, placeholderPosition) - placeholderPosition);
				
				noDigits = currentPadding.size();
				noDecimals = (noDecimals = currentPadding.find('.')) != std::string::npos ? (noDigits - noDecimals - 1u) : 0u;
				noSpacesRight = std::modf(std::stof(currentPadding), &noSpacesLeft);
				for (size_t i = 0u; i < noDecimals; ++i)
					noSpacesRight *= 10;

				placeholderSize = 0u;
				for (size_t i = 0u; i < noPlaceholders && !placeholderSize; ++i)
					if ((currentSectionEnd = message.find(placeholders[i], placeholderPosition + noDigits)) != std::string::npos && placeholderPosition + noDigits == currentSectionEnd - 1u)
						placeholderSize = placeholders[i].size();

				if (noSpacesRight && (placeholderSize || (currentSectionEnd = message.find_first_of(" .-,@#(){}[]'\"\\/!`~|;:?><=+-_%&*", placeholderPosition + noDigits + 1u)) != std::string::npos))
				{
					nextCharacter = message[placeholderSize ? (currentSectionEnd + placeholderSize) : currentSectionEnd];
					message.replace(placeholderSize ? (currentSectionEnd + placeholderSize) : currentSectionEnd, 1u, (size_t)noSpacesRight + 1u, ' ');
					if(nextCharacter != ' ')
						message[placeholderSize ? (currentSectionEnd + placeholderSize) : (currentSectionEnd + (size_t)noSpacesRight)] = nextCharacter;
				}

				message.replace(placeholderPosition - 1u, noDigits + 1u, !placeholderSize ? (size_t)noSpacesLeft : ((size_t)noSpacesLeft + 1u) , ' ');
				if (placeholderSize)
					message[placeholderPosition + (size_t)noSpacesLeft] = '%';
			}
		}
	}

	// Append format (if it exists) and replace all "{n}" placeholders with their respective values (n=0,...)
	inline std::string Logger::replacePlaceholders(std::string message, std::vector<std::string>&& items) const noexcept
	{
		bool hasMacros = false;
		std::string placeholder;
		size_t placeholderPosition, placeholderSize, noArguments = items.size();

		if(noArguments > 3u)
			for (size_t i = noArguments - 1u; i >= 0u && !hasMacros; --i)
				if (items[i] == "__MACROS__")
					hasMacros = true;

		for (size_t i = 0u; i < noArguments; ++i)
		{
			placeholderSize = (placeholder = '{' + std::to_string(i) + '}').size();
			while ((placeholderPosition = message.find(placeholder)) != std::string::npos)
				message.replace(placeholderPosition, placeholderSize, items[i]);
		}
		
		if(!m_Format.empty())
			message = (placeholderPosition = m_Format.find("%msg")) != std::string::npos ? std::string(m_Format).replace(placeholderPosition, 4u, message) : (m_Format + ' ' + message);
		
		addIndent(message);
		addPadding(message);
		addColours(message);
		replacePredefinedPlaceholders(message);
		replaceCurrentLevel(message);
		if(hasMacros)
			replaceOthers(message, items[noArguments - 3u].c_str(), items[noArguments - 2u].c_str(), items[noArguments - 1u].c_str());
		else
			replaceOthers(message, nullptr, nullptr, nullptr);
		replaceDateFormats(message);

		return message;
	}

	// Replaces predefined placeholders from the message (e.g. "%er" will be changed to "Error" in the final message)
	inline void Logger::replacePredefinedPlaceholders(std::string& message) const noexcept
	{
#ifdef SBLOGGER_LEGACY
		std::string placeholder = "tr";
#else
		std::string_view placeholder = "tr";
#endif
		size_t placeholderPosition;
		while ((placeholderPosition = message.find(placeholder)) != std::string::npos && (message[placeholderPosition - 1u] == '%' || message[placeholderPosition - 2u] == '%'))
			message[placeholderPosition - 1u] == '^' ? message.replace(placeholderPosition - 2u, placeholder.size() + 2u, "TRACE") : message.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Trace");

		while ((placeholderPosition = message.find(placeholder = "dbg")) != std::string::npos && (message[placeholderPosition - 1u] == '%' || message[placeholderPosition - 2u] == '%'))
			message[placeholderPosition - 1u] == '^' ? message.replace(placeholderPosition - 2u, placeholder.size() + 2u, "DEBUG") : message.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Debug");

		while ((placeholderPosition = message.find(placeholder = "inf")) != std::string::npos && (message[placeholderPosition - 1u] == '%' || message[placeholderPosition - 2u] == '%'))
			message[placeholderPosition - 1u] == '^' ? message.replace(placeholderPosition - 2u, placeholder.size() + 2u, "INFO") : message.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Info");

		while ((placeholderPosition = message.find(placeholder = "wn")) != std::string::npos && (message[placeholderPosition - 1u] == '%' || message[placeholderPosition - 2u] == '%'))
			message[placeholderPosition - 1u] == '^' ? message.replace(placeholderPosition - 2u, placeholder.size() + 2u, "WARN") : message.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Warn");

		while ((placeholderPosition = message.find(placeholder = "er")) != std::string::npos && (message[placeholderPosition - 1u] == '%' || message[placeholderPosition - 2u] == '%'))
			message[placeholderPosition - 1u] == '^' ? message.replace(placeholderPosition - 2u, placeholder.size() + 2u, "ERROR") : message.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Error");

		while ((placeholderPosition = message.find(placeholder = "crt")) != std::string::npos && (message[placeholderPosition - 1u] == '%' || message[placeholderPosition - 2u] == '%'))
			message[placeholderPosition - 1u] == '^' ? message.replace(placeholderPosition - 2u, placeholder.size() + 2u, "CRITICAL") : message.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Critical");
	}

	// Replace current logging level in format
	inline void Logger::replaceCurrentLevel(std::string& message) const noexcept
	{
		const size_t placeholderPosition = message.find("lvl");
		if (placeholderPosition != std::string::npos && (message[placeholderPosition - 1u] == '%' || message[placeholderPosition - 2u] == '%'))
			switch (s_CurrentLogLevel)
			{
			case LOG_LEVELS::TRACE:
				message[placeholderPosition - 1u] == '^' ?
					message.replace(placeholderPosition - 2u, 5u, "TRACE") : message.replace(placeholderPosition - 2u, 4u, "Trace");
				break;
			case LOG_LEVELS::DEBUG:
				message[placeholderPosition - 1u] == '^' ?
					message.replace(placeholderPosition - 2u, 5u, "DEBUG") : message.replace(placeholderPosition - 2u, 4u, "Debug");
				break;
			case LOG_LEVELS::INFO:
				message[placeholderPosition - 1u] == '^' ?
					message.replace(placeholderPosition - 2u, 5u, "INFO") : message.replace(placeholderPosition - 2u, 4u, "Info");
				break;
			case LOG_LEVELS::WARN:
				message[placeholderPosition - 1u] == '^' ?
					message.replace(placeholderPosition - 2u, 5u, "WARN") : message.replace(placeholderPosition - 2u, 4u, "Warn");
				break;
			case LOG_LEVELS::ERROR:
				message[placeholderPosition - 1u] == '^' ?
					message.replace(placeholderPosition - 2u, 5u, "ERROR") : message.replace(placeholderPosition - 2u, 4u, "Error");
				break;
			case LOG_LEVELS::CRITICAL:
				message[placeholderPosition - 1u] == '^' ?
					message.replace(placeholderPosition - 2u, 5u, "CRITICAL") : message.replace(placeholderPosition - 2u, 4u, "Critical");
				break;
			default:
				message = "";
				break;
			}
	}

	// Replace other placeholders, such as those for file, line and function related information
	inline void Logger::replaceOthers(std::string& message, const char* file, const char* line, const char* function) const noexcept
	{
#ifdef SBLOGGER_LEGACY
		std::string placeholder = "src";
#else
		std::string_view placeholder = "src";
#endif
		size_t placeholderPosition = message.find(placeholder);
		while (placeholderPosition != std::string::npos && message[placeholderPosition - 1u] == '%')
			message.replace(placeholderPosition - 1u, 4u, file == nullptr ? "" : (std::strrchr(file, SBLOGGER_PATH_SEPARATOR) + 1));

		placeholder = "fsrc";
		while ((placeholderPosition = message.find(placeholder)) != std::string::npos && message[placeholderPosition - 1u] == '%')
			message.replace(placeholderPosition - 1u, 5u, file == nullptr ? "" : file);

		placeholder = "ln";
		while ((placeholderPosition = message.find(placeholder)) != std::string::npos && message[placeholderPosition - 1u] == '%')
			message.replace(placeholderPosition - 1u, 3u, line == nullptr ? "" : line);

		placeholder = "func";
		while ((placeholderPosition = message.find(placeholder)) != std::string::npos && message[placeholderPosition - 1u] == '%')
			message.replace(placeholderPosition - 1u, 5u, function == nullptr ? "" : function);
	}

	// Replace date format using std::strftime (pre C++20) or std::chrono::format
	inline void Logger::replaceDateFormats(std::string& message) const noexcept
	{
// Replace date format using std::strftime (pre C++20)
#ifdef SBLOGGER_OLD_DATES
		std::time_t currentTime = std::time(nullptr);
		size_t messageLength = message.size();
		char* buffer = new char[messageLength + 101u]{ 0 };

		if (std::strftime(buffer, sizeof(char) * (messageLength + 100u), message.c_str(), std::localtime(&currentTime)))
			message = std::string(buffer);

		delete[] buffer;
// Replace date format using std::chrono::format
#else
		// Replace date format using std::chrono::format
		// Wait for MSVC to catch up
#endif
	}

	//
	// Public methods
	//

	// Indent (prepend '\t') log, returns the number of indents the final message will contain
	inline const size_t Logger::Indent() noexcept
	{
		return ++m_IndentCount;
	}

	// Dedent (prepend '\t') log, returns the number of indents the final message will contain
	inline const size_t Logger::Dedent() noexcept
	{
		return m_IndentCount > 0 ? --m_IndentCount : m_IndentCount;
	}

	// Set the current logging level to one of the "LOG_LEVELS" options (ex.: TRACE, DEBUG, INFO etc).
	inline void Logger::SetLoggingLevel(const LOG_LEVELS& level) noexcept
	{
		s_CurrentLogLevel = level;
	}

	// Get the current logging level (one of the "LOG_LEVELS" options, ex.: TRACE, DEBUG, INFO etc). 
	inline const LOG_LEVELS Logger::GetLoggingLevel() noexcept
	{
		return s_CurrentLogLevel;
	}

	// Get the current log format
	inline const std::string Logger::GetFormat() const noexcept
	{
		return m_Format;
	}

	// Set the current log format to "format"
	inline void Logger::SetFormat(const std::string& format)
	{
		m_Format = format;
	}

	// Writes to the stream the newline character with a log level of TRACE
	inline void Logger::WriteLine(LOG_LEVELS logLevel)
	{
		if(s_CurrentLogLevel <= logLevel)
			writeToStream(SBLOGGER_NEWLINE);
	}

	//
	// Generic write methods to write a TRACE level message to the stream
	//

	// Writes to the stream a message and inserts values into placeholders (should they exist) with a default level of TRACE
	template<typename ...T>
	inline void Logger::Write(const std::string& message, const T& ...t)
	{
		if (s_CurrentLogLevel <= LOG_LEVELS::TRACE)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }));
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist) with a default level of TRACE
	template<typename ...T>
	inline void Logger::Write(const std::string&& message, const T&& ...t)
	{
		if (s_CurrentLogLevel <= LOG_LEVELS::TRACE)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }));
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist) and finishes with the newline character with a default level of TRACE
	template<typename ...T>
	inline void Logger::WriteLine(const std::string& message, const T& ...t)
	{
		if (s_CurrentLogLevel <= LOG_LEVELS::TRACE)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }) + SBLOGGER_NEWLINE);
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist) and finishes with the newline character with a default level of TRACE
	template<typename ...T>
	inline void Logger::WriteLine(const std::string&& message, const T&& ...t)
	{
		if (s_CurrentLogLevel <= LOG_LEVELS::TRACE)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }) + SBLOGGER_NEWLINE);
	}

	//
	// Generic Methods: Write a level message (depending on the specified "LOG_LEVEL") to a stream 
	//

	// Writes to the stream a message and inserts values into placeholders (should they exist), with "logLevel" importance
	template<typename ...T>
	inline void Logger::Write(LOG_LEVELS logLevel, const std::string& message, const T& ...t)
	{
		if (s_CurrentLogLevel <= logLevel)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }));
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist), with "logLevel" importance
	template<typename ...T>
	inline void Logger::Write(LOG_LEVELS logLevel, const std::string&& message, const T&& ...t)
	{
		if (s_CurrentLogLevel <= logLevel)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }));
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist) and finishes with the newline character, with "logLevel" importance
	template<typename ...T>
	inline void Logger::WriteLine(LOG_LEVELS logLevel, const std::string& message, const T& ...t)
	{
		if (s_CurrentLogLevel <= logLevel)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }) + SBLOGGER_NEWLINE);
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist) and finishes with the newline character, with "logLevel" importance
	template<typename ...T>
	inline void Logger::WriteLine(LOG_LEVELS logLevel, const std::string&& message, const T&& ...t)
	{
		if (s_CurrentLogLevel <= logLevel)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }) + SBLOGGER_NEWLINE);
	}

	//
	// Generic Methods: Write a TRACE level message to a stream 
	//

	// Writes to the stream a message and inserts values into placeholders (should they exist), of TRACE importance
	template<typename ...T>
	inline void Logger::Trace(const std::string& message, const T& ...t)
	{
		if (s_CurrentLogLevel <= LOG_LEVELS::TRACE)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }));
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist), of TRACE importance
	template<typename ...T>
	inline void Logger::Trace(const std::string&& message, const T&& ...t)
	{
		if (s_CurrentLogLevel <= LOG_LEVELS::TRACE)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }));
	}

	//
	// Generic Methods: Write a DEBUG level message to a stream 
	//

	// Writes to the stream a message and inserts values into placeholders (should they exist), of DEBUG importance
	template<typename ...T>
	inline void Logger::Debug(const std::string& message, const T& ...t)
	{
		if (s_CurrentLogLevel <= LOG_LEVELS::DEBUG)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }));
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist), of DEBUG importance
	template<typename ...T>
	inline void Logger::Debug(const std::string&& message, const T&& ...t)
	{
		if (s_CurrentLogLevel <= LOG_LEVELS::DEBUG)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }));
	}

	//
	// Generic Methods: Write a INFO level message to a stream 
	//

	// Writes to the stream a message and inserts values into placeholders (should they exist), of INFO importance
	template<typename ...T>
	inline void Logger::Info(const std::string& message, const T& ...t)
	{
		if (s_CurrentLogLevel <= LOG_LEVELS::INFO)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }));
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist), of INFO importance
	template<typename ...T>
	inline void Logger::Info(const std::string&& message, const T&& ...t)
	{
		if (s_CurrentLogLevel <= LOG_LEVELS::INFO)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }));
	}

	//
	// Generic Methods: Write a WARN level message to a stream 
	//

	// Writes to the stream a message and inserts values into placeholders (should they exist), of WARN importance
	template<typename ...T>
	inline void Logger::Warn(const std::string& message, const T& ...t)
	{
		if (s_CurrentLogLevel <= LOG_LEVELS::WARN)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }));
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist), of WARN importance
	template<typename ...T>
	inline void Logger::Warn(const std::string&& message, const T&& ...t)
	{
		if (s_CurrentLogLevel <= LOG_LEVELS::WARN)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }));
	}

	//
	// Generic Methods: Write a ERROR level message to a stream 
	//

	// Writes to the stream a message and inserts values into placeholders (should they exist), of ERROR importance
	template<typename ...T>
	inline void Logger::Error(const std::string& message, const T& ...t)
	{
		if (s_CurrentLogLevel <= LOG_LEVELS::ERROR)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }));
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist), of ERROR importance
	template<typename ...T>
	inline void Logger::Error(const std::string&& message, const T&& ...t)
	{
		if (s_CurrentLogLevel <= LOG_LEVELS::ERROR)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }));
	}

	//
	// Generic Methods: Write a CRITICAL level message to a stream 
	//

	// Writes to the stream a message and inserts values into placeholders (should they exist), of CRITICAL importance
	template<typename ...T>
	inline void Logger::Critical(const std::string& message, const T& ...t)
	{
		if (s_CurrentLogLevel <= LOG_LEVELS::CRITICAL)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }));
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist), of CRITICAL importance
	template<typename ...T>
	inline void Logger::Critical(const std::string&& message, const T&& ...t)
	{
		if (s_CurrentLogLevel <= LOG_LEVELS::CRITICAL)
			writeToStream(replacePlaceholders(message, std::vector<std::string>{ stringConvert(t)... }));
	}

	//
	// StreamLogger class
	//

	// Used to log messages to a non-file stream (ex.: STDOUT, STDERR, STDLOG)
	class StreamLogger : public Logger
	{
	protected:
		//
		// Protected members
		//

		STREAM_TYPE m_StreamType;

		//
		// Protected methods
		//

#if defined SBLOGGER_COLOURS || defined SBLOGGER_COLORS
		// Adds colours where the specific placeholders are found (e.g. %colour_name{...}colour_name%)
		inline void addColours(std::string& message) const noexcept override;
#endif	// Adds colours where the specific placeholders are found (e.g. %colour_name{...}colour_name%)

		// Writes string to appropriate stream based on instance STREAM_TYPE (m_StreamType)
		inline void writeToStream(const std::string&& str) override;

	public:
		//
		// Constructors and destructors
		//

		// Creates an instance of Logger which outputs to a stream chosen from a STREAM_TYPE
		// By default uses STREAM_TYPE::STDOUT and no format or auto flush
		inline StreamLogger(const STREAM_TYPE& type = STREAM_TYPE::STDOUT, const std::string& format = std::string(), bool autoFlush = false);

		// Creates an instance of Logger which outputs to STDOUT. Formats logs and auto flushes based on the parameter "autoFlush"
		inline StreamLogger(const std::string& format, bool autoFlush = false);

		// Creates an instance of Logger which outputs to STDOUT. Formats logs and auto flushes based on the parameter "autoFlush"
		inline StreamLogger(const char* format, bool autoFlush = false);

		// Creates an instance of Logger which outputs to STDOUT. Formats logs and auto flushes based on the parameter "autoFlush"
		inline StreamLogger(bool autoFlush) noexcept;

		// Copy constructor

		// Creates a Logger instance from an already existing one
		inline StreamLogger(const StreamLogger& other) noexcept;

		// Move constructor

		// Creates a Logger instance from another
		inline StreamLogger(StreamLogger&& other) noexcept;

		// Destructor
		
		// Flush stream before deletion
		inline ~StreamLogger() override;

		//
		// Overloaded operators
		//

		// Assignment operator
		inline StreamLogger& operator=(const StreamLogger& other) noexcept;

		//
		// Public methods
		//

		// Flush appropriate stream
		virtual inline void Flush() noexcept override;

		// Change the logger's stream type (to a different "STREAM_TYPE")
		inline void SetStreamType(STREAM_TYPE streamType);
	};

	//
	// Constructors and destructors
	//

	// Creates an instance of Logger which outputs to a stream chosen from a STREAM_TYPE
	// By default uses STREAM_TYPE::STDOUT and no format or auto flush
	inline StreamLogger::StreamLogger(const STREAM_TYPE& type, const std::string& format, bool autoFlush)
		: Logger(format, autoFlush), m_StreamType(type)
	{ }

	// Creates an instance of Logger which outputs to STDOUT. Formats logs and auto flushes based on the parameter "autoFlush"
	inline StreamLogger::StreamLogger(const std::string& format, bool autoFlush)
		: Logger(format, autoFlush), m_StreamType(STREAM_TYPE::STDOUT)
	{ }

	// Creates an instance of Logger which outputs to STDOUT. Formats logs and auto flushes based on the parameter "autoFlush"
	inline StreamLogger::StreamLogger(const char* format, bool autoFlush)
		: Logger(format, autoFlush), m_StreamType(STREAM_TYPE::STDOUT)
	{ }

	// Creates an instance of Logger which outputs to STDOUT. Formats logs and auto flushes based on the parameter "autoFlush"
	inline StreamLogger::StreamLogger(bool autoFlush) noexcept
		: Logger(std::string(), autoFlush), m_StreamType(STREAM_TYPE::STDOUT)
	{ }

	// Copy constructor

	// Creates a Logger instance from an already existing one
	inline StreamLogger::StreamLogger(const StreamLogger& other) noexcept
		: Logger(other), m_StreamType(other.m_StreamType)
	{ }

	// Move constructor

	// Creates a Logger instance from another
	inline StreamLogger::StreamLogger(StreamLogger&& other) noexcept
		: Logger(other), m_StreamType(other.m_StreamType)
	{ }

	// Destructor

	// Flush stream before deletion
	inline StreamLogger::~StreamLogger()
	{
		switch (m_StreamType)
		{
		case STREAM_TYPE::STDERR:   std::cerr.flush();       break;
		case STREAM_TYPE::STDLOG:   std::clog.flush();       break;
		case STREAM_TYPE::STDOUT:   std::cout.flush();       break;
		}
	}

	//
	// Protected methods
	//

#if defined SBLOGGER_COLOURS || defined SBLOGGER_COLORS
	// Adds colours where the specific placeholders are found (e.g. %colour_name{...}colour_name%)
	inline void StreamLogger::addColours(std::string& message) const noexcept
	{
		static struct Colours
		{
			char name[12];
			size_t code;
		}colours[]{ { "reset", 0 }, { "black", 30 }, { "red", 31 }, { "green", 32 }, { "yellow", 33 }, { "blue", 34 }, { "magenta", 35 }, { "cyan", 36 }, { "white", 37 },
				{ "bg-black", 40 }, { "bg-red", 41 }, { "bg-green", 42 }, { "bg-yellow", 43 }, { "bg-blue", 44 }, { "bg-magenta", 45 }, { "bg-cyan", 46 }, { "bg-white", 47 } };

		size_t placeholderPosition, placeholderSize;
		bool isBright;
		char currentColour[17]{ '{' }, colourCode[6]{ '\033', '[' };

		for (size_t i = 0u; i < 17u; ++i)
			if ((placeholderPosition = message.find(colours[i].name)) != std::string::npos
				&& placeholderPosition > 1u && message[placeholderPosition - 1u] == '{'
				&& placeholderPosition < (message.size() - 1u) && message[placeholderPosition + std::strlen(colours[i].name)] == '}')
			{
				std::strncpy(currentColour + 1, colours[i].name, std::strlen(colours[i].name) + 1u);
				std::strncat(currentColour, "}", 1u);
				while ((placeholderPosition = message.find(currentColour)) != std::string::npos)
				{
					placeholderSize = std::strlen(currentColour);
					message[placeholderPosition - 1u] == '^' ?
						message.replace(placeholderPosition - 2u, placeholderSize + 2u, colours[i].code != 0 ? ("\033[" + std::to_string(colours[i].code + 60u) + 'm') : "\033[m")
						: message.replace(placeholderPosition - 1u, placeholderSize + 1u, colours[i].code != 0 ? ("\033[" + std::to_string(colours[i].code) + 'm') : "\033[m");
				}
			}
	}
#endif	// Adds colours where the specific placeholders are found (e.g. %colour_name{...}colour_name%)

	// Writes string to appropriate stream based on instance STREAM_TYPE (m_StreamType)
	inline void StreamLogger::writeToStream(const std::string&& str)
	{
		switch (m_StreamType)
		{
		case STREAM_TYPE::STDERR:   std::cerr << str;       if (m_AutoFlush) std::cerr.flush();       break;
		case STREAM_TYPE::STDLOG:   std::clog << str;       if (m_AutoFlush) std::clog.flush();       break;
		default:                    std::cout << str;       if (m_AutoFlush) std::cout.flush();       break;
		}
	}

	//
	// Overloaded operators
	//

	// Assignment operator
	inline StreamLogger& StreamLogger::operator=(const StreamLogger& other) noexcept
	{
		if (this != &other)
		{
			m_AutoFlush = other.m_AutoFlush;
			m_Format = other.m_Format;
			m_IndentCount = other.m_IndentCount;
			m_StreamType = other.m_StreamType;
		}

		return *this;
	}

	//
	// Public methods
	//

	// Flush appropriate stream
	inline void StreamLogger::Flush() noexcept
	{
		switch (m_StreamType)
		{
		case STREAM_TYPE::STDERR:   std::cerr.flush();       break;
		case STREAM_TYPE::STDLOG:   std::clog.flush();       break;
		default:                    std::cout.flush();       break;
		}
	}

	// Change the logger's stream type (to a different "STREAM_TYPE")
	inline void StreamLogger::SetStreamType(STREAM_TYPE streamType)
	{
		m_StreamType = streamType;
	}

	//
	// FileLogger class
	//

	// Used to log messages to a file stream
	class FileLogger : public Logger
	{
	protected:
		//
		// Protected members
		//

#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		std::string m_FilePath;
#else
		std::filesystem::path m_FilePath;
#endif
		std::fstream m_FileStream;

		//
		// Protected methods
		//

		// Writes string to file stream and flush if auto flush is set
		inline virtual void writeToStream(const std::string&& str) override;

	public:
		//
		// Constructors and destructors
		//

		// Deleted to prevent usage without providing a file path
		inline FileLogger() = delete;

		// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
		// By default there is no formatting and auto flush is set to true
		inline FileLogger(const char* filePath, const char* format = nullptr, bool autoFlush = true);

		// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
		// By default there is no formatting and auto flush is set to true
		inline FileLogger(const char* filePath, const std::string& format, bool autoFlush = true);

		// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
		// By default there is no formatting and auto flush is set to true
		inline FileLogger(const std::string& filePath, const std::string& format = std::string(), bool autoFlush = true);

		// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
		// By default there is no formatting and auto flush is set to true
		inline FileLogger(const std::string&& filePath, const std::string&& format = std::string(), bool autoFlush = true);

		// Copy constructor

		inline FileLogger(const FileLogger& other) = delete;

		// Move constructor

		inline FileLogger(FileLogger&& other) = delete;

		// Destructor

		// Flush and close stream if open
		inline ~FileLogger() override;

		//
		// Overloaded operators
		//

		// Assignment operator (deleted since having two streams for the same file causes certain output not to be written).
		inline FileLogger& operator=(const FileLogger& other) = delete;

		//
		// Public methods
		//

		// Get the file path
#if SBLOGGER_LEGACY
		std::string GetFilePath() const noexcept;
#else
		std::filesystem::path GetFilePath() const noexcept;
#endif

		// Flush file stream
		inline virtual void Flush() noexcept override;

		// Clear log file
		inline virtual void ClearLogs() noexcept;
	};

	//
	// Constructors and destructors
	//

	// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
	// By default there is no formatting and auto flush is set to true
	inline FileLogger::FileLogger(const char* filePath, const char* format, bool autoFlush)
		: Logger(format == nullptr ? "" : format, autoFlush)
	{
		if (filePath == nullptr || filePath[0] == '\0') throw NullOrEmptyPathException();

		std::string formattedFilePath(filePath);
		addPadding(formattedFilePath);
		addColours(formattedFilePath);
		replacePredefinedPlaceholders(formattedFilePath);
		replaceCurrentLevel(formattedFilePath);
		replaceOthers(formattedFilePath, nullptr, nullptr, nullptr);
		replaceDateFormats(formattedFilePath);
#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		m_FilePath = formattedFilePath;
		if (!std::regex_match(m_FilePath, SBLOGGER_FILE_PATH_REGEX)) throw InvalidFilePathException(m_FilePath);
#else
		m_FilePath = std::filesystem::path(formattedFilePath);
		// Check file path for null, empty, inexistent or whitespace only paths and filenames
		if (!m_FilePath.has_filename() || !m_FilePath.has_extension()) throw NullOrEmptyPathException();
		if (m_FilePath.filename().replace_extension().string().find_first_not_of(' ') == std::string::npos) throw NullOrWhitespaceNameException();
		auto parentPath = m_FilePath.parent_path();
		if (!parentPath.empty() && !std::filesystem::directory_entry(parentPath).exists()) throw InvalidFilePathException(formattedFilePath);
#endif
		m_FileStream = std::fstream(m_FilePath, std::fstream::app | std::fstream::out);

#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath);
#else
		if (!m_FileStream.is_open()) throw InvalidFilePathException(formattedFilePath);
#endif
	}

	// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
	// By default there is no formatting and auto flush is set to true
	inline FileLogger::FileLogger(const char* filePath, const std::string& format, bool autoFlush)
		: Logger(format, autoFlush)
	{
		if (filePath == nullptr || filePath[0] == '\0') throw NullOrEmptyPathException();

		std::string formattedFilePath(filePath);
		addPadding(formattedFilePath);
		addColours(formattedFilePath);
		replacePredefinedPlaceholders(formattedFilePath);
		replaceCurrentLevel(formattedFilePath);
		replaceOthers(formattedFilePath, nullptr, nullptr, nullptr);
		replaceDateFormats(formattedFilePath);
#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		m_FilePath = formattedFilePath;
		if (!std::regex_match(m_FilePath, SBLOGGER_FILE_PATH_REGEX)) throw InvalidFilePathException(m_FilePath);
#else
		m_FilePath = std::filesystem::path(formattedFilePath);
		// Check file path for null, empty, inexistent or whitespace only paths and filenames
		if (!m_FilePath.has_filename() || !m_FilePath.has_extension()) throw NullOrEmptyPathException();
		if (m_FilePath.filename().replace_extension().string().find_first_not_of(' ') == std::string::npos) throw NullOrWhitespaceNameException();
		if (!std::filesystem::directory_entry(m_FilePath.parent_path()).exists()) throw InvalidFilePathException(formattedFilePath);
#endif
		m_FileStream = std::fstream(m_FilePath, std::fstream::app | std::fstream::out);

#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath);
#else
		if (!m_FileStream.is_open()) throw InvalidFilePathException(formattedFilePath);
#endif
	}

	// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
	// By default there is no formatting and auto flush is set to true
	inline FileLogger::FileLogger(const std::string& filePath, const std::string& format, bool autoFlush)
		: Logger(format, autoFlush)
	{
		std::string formattedFilePath(filePath);
		addPadding(formattedFilePath);
		addColours(formattedFilePath);
		replacePredefinedPlaceholders(formattedFilePath);
		replaceCurrentLevel(formattedFilePath);
		replaceOthers(formattedFilePath, nullptr, nullptr, nullptr);
		replaceDateFormats(formattedFilePath);
#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		m_FilePath = formattedFilePath;
		if (!std::regex_match(m_FilePath, SBLOGGER_FILE_PATH_REGEX)) throw InvalidFilePathException(m_FilePath);
#else
		m_FilePath = std::filesystem::path(formattedFilePath);
		// Check file path for null, empty, inexistent or whitespace only paths and filenames
		if (!m_FilePath.has_filename() || !m_FilePath.has_extension()) throw NullOrEmptyPathException();
		if (m_FilePath.filename().replace_extension().string().find_first_not_of(' ') == std::string::npos) throw NullOrWhitespaceNameException();
		if (!std::filesystem::directory_entry(m_FilePath.parent_path()).exists()) throw InvalidFilePathException(formattedFilePath);
#endif
		m_FileStream = std::fstream(m_FilePath, std::fstream::app | std::fstream::out);

#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath);
#else
		if (!m_FileStream.is_open()) throw InvalidFilePathException(formattedFilePath);
#endif
	}

	// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
	// By default there is no formatting and auto flush is set to true
	inline FileLogger::FileLogger(const std::string&& filePath, const std::string&& format, bool autoFlush)
		: Logger(format, autoFlush)
	{
		std::string formattedFilePath(filePath);
		addPadding(formattedFilePath);
		addColours(formattedFilePath);
		replacePredefinedPlaceholders(formattedFilePath);
		replaceCurrentLevel(formattedFilePath);
		replaceOthers(formattedFilePath, nullptr, nullptr, nullptr);
		replaceDateFormats(formattedFilePath);
#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		m_FilePath = formattedFilePath;
		if (!std::regex_match(m_FilePath, SBLOGGER_FILE_PATH_REGEX)) throw InvalidFilePathException(m_FilePath);
#else
		m_FilePath = std::filesystem::path(formattedFilePath);
		// Check file path for null, empty, inexistent or whitespace only paths and filenames
		if (!m_FilePath.has_filename() || !m_FilePath.has_extension()) throw NullOrEmptyPathException();
		if (m_FilePath.filename().replace_extension().string().find_first_not_of(' ') == std::string::npos) throw NullOrWhitespaceNameException();
		if (!std::filesystem::directory_entry(m_FilePath.parent_path()).exists()) throw InvalidFilePathException(formattedFilePath);
#endif
		m_FileStream = std::fstream(m_FilePath, std::fstream::app | std::fstream::out);

#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath);
#else
		if (!m_FileStream.is_open()) throw InvalidFilePathException(formattedFilePath);
#endif
	}

	// Destructor

	// Flush and close stream if open
	inline FileLogger::~FileLogger()
	{
		if (m_FileStream.is_open())
		{
			m_FileStream.flush();
			m_FileStream.close();
		}
	}

	//
	// Private methods
	//

	// Writes string to file stream and flush if auto flush is set
	inline void FileLogger::writeToStream(const std::string&& str)
	{
		if (!m_FileStream.is_open())
#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
			std::cerr << "The file stream " + m_FilePath + " is not opened.";
#else
			std::cerr << "The file stream " + m_FilePath.string() + " is not opened.";
#endif
		else
		{
			m_FileStream << str;
			if (m_AutoFlush) 
				m_FileStream.flush();
		}
	}

	//
	// Public methods
	//

	// Get the file path
#if SBLOGGER_LEGACY
	std::string FileLogger::GetFilePath() const noexcept
#else
	std::filesystem::path FileLogger::GetFilePath() const noexcept
#endif
	{
		return m_FilePath;
	}

	// Flush file stream
	inline void FileLogger::Flush() noexcept
	{
		if(m_FileStream.is_open())
			m_FileStream.flush();
	}

	// Clear log file
	inline void FileLogger::ClearLogs() noexcept
	{
		if (m_FileStream.is_open())
		{
#ifdef SBLOGGER_LEGACY
			m_FileStream.close();
			m_FileStream = std::fstream(m_FilePath, std::ofstream::out | std::ofstream::trunc);
#else
			std::filesystem::resize_file(m_FilePath, 0);
			m_FileStream.seekp(0);
#endif
		}
	}

	
	class DailyLogger : public FileLogger
	{
		//
		// Private members
		//

		std::thread m_FileChangeThread;
		std::mutex m_Mutex;
		std::string m_FileNameFormat;
		std::chrono::system_clock::time_point m_NextChangeTime;
		int m_Hours, m_Minutes, m_Seconds;
		bool m_Stop;
		
		//
		// Protected methods
		//

		// Writes string to file stream and flush if auto flush is set
		inline virtual void writeToStream(const std::string&& str) override;

		// Check if it is time to change the current file (closing it) and open the new one, according to the time provided
		inline void changeFile();

	public:
		//
		// Constructors and destructors
		//

		// Deleted to prevent usage without providing a file path and time to change it at
		inline DailyLogger() = delete;

		// Creates an instance of DailyLogger which outputs to a file stream given by the "filePath" parameter, which will be recreated at the specified interval
		// By default there is no formatting and auto flush is set to true
		inline DailyLogger(const char* filePath, const char* format = nullptr, int hour = 0, int minutes = 0, int seconds = 0, bool autoFlush = true);

		// Creates an instance of DailyLogger which outputs to a file stream given by the "filePath" parameter, which will be recreated at the specified interval
		// By default there is no formatting and auto flush is set to true
		inline DailyLogger(const char* filePath, const std::string& format, int hour = 0, int minutes = 0, int seconds = 0, bool autoFlush = true);

		// Creates an instance of DailyLogger which outputs to a file stream given by the "filePath" parameter, which will be recreated at the specified interval
		// By default there is no formatting and auto flush is set to true
		inline DailyLogger(const std::string& filePath, const std::string& format = std::string(), int hour = 0, int minutes = 0, int seconds = 0, bool autoFlush = true);

		// Creates an instance of DailyLogger which outputs to a file stream given by the "filePath" parameter, which will be recreated at the specified interval
		// By default there is no formatting and auto flush is set to true
		inline DailyLogger(const std::string&& filePath, const std::string&& format = std::string(), int hour = 0, int minutes = 0, int seconds = 0, bool autoFlush = true);

		// Copy constructor

		inline DailyLogger(const DailyLogger& other) = delete;

		// Move constructor

		inline DailyLogger(DailyLogger&& other) = delete;

		// Destructor

		// Flush and close stream if open
		inline ~DailyLogger() override;

		//
		// Overloaded operators
		//

		// Assignment operator (deleted since having two streams for the same file causes certain output not to be written).
		inline DailyLogger& operator=(const DailyLogger& other) = delete;

		//
		// Public methods
		//

		// Flush file stream
		inline virtual void Flush() noexcept override;

		// Clear log file
		inline virtual void ClearLogs() noexcept override;
	};

	//
	// Constructors and destructors
	//

	// Creates an instance of DailyLogger which outputs to a file stream given by the "filePath" parameter, which will be recreated at the specified interval
	// By default there is no formatting and auto flush is set to true
	inline DailyLogger::DailyLogger(const char* filePath, const char* format, int hour, int minutes, int seconds, bool autoFlush)
		: FileLogger(filePath, format, autoFlush), m_FileNameFormat(filePath), m_Stop(false)
	{
		if (hour < 0 || hour > 23)			throw TimeRangeException();
		if (minutes < 0 || minutes > 59)	throw TimeRangeException();
		if (seconds < 0 || seconds > 59)	throw TimeRangeException();

		// Set the date using time_t and mktime (pre C++20)
#ifdef SBLOGGER_OLD_DATES
		auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		auto newTime = *std::localtime(&tt);
		newTime.tm_sec = m_Seconds;
		newTime.tm_min = m_Minutes;
		newTime.tm_hour = m_Hours;
		newTime.tm_mday++;
		tt = std::mktime(&newTime);
		m_NextChangeTime = std::chrono::system_clock::from_time_t(tt);
#else
// Set the date using C++20 (or later) methods
#endif
		m_FileChangeThread = std::thread(&DailyLogger::changeFile, this);
	}

	// Creates an instance of DailyLogger which outputs to a file stream given by the "filePath" parameter, which will be recreated at the specified interval
	// By default there is no formatting and auto flush is set to true
	inline DailyLogger::DailyLogger(const char* filePath, const std::string& format, int hour, int minutes, int seconds, bool autoFlush)
		: FileLogger(filePath, format, autoFlush), m_FileNameFormat(filePath), m_Stop(false)
	{
		if (hour < 0 || hour > 23)			throw TimeRangeException();
		if (minutes < 0 || minutes > 59)	throw TimeRangeException();
		if (seconds < 0 || seconds > 59)	throw TimeRangeException();

// Set the date using time_t and mktime (pre C++20)
#ifdef SBLOGGER_OLD_DATES
		auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		auto newTime = *std::localtime(&tt);
		newTime.tm_sec = m_Seconds;
		newTime.tm_min = m_Minutes;
		newTime.tm_hour = m_Hours;
		newTime.tm_mday++;
		tt = std::mktime(&newTime);
		m_NextChangeTime = std::chrono::system_clock::from_time_t(tt);
#else
// Set the date using C++20 (or later) methods
#endif
		m_FileChangeThread = std::thread(&DailyLogger::changeFile, this);
	}

	// Creates an instance of DailyLogger which outputs to a file stream given by the "filePath" parameter, which will be recreated at the specified interval
	// By default there is no formatting and auto flush is set to true
	inline DailyLogger::DailyLogger(const std::string& filePath, const std::string& format, int hour, int minutes, int seconds, bool autoFlush)
		: FileLogger(filePath, format, autoFlush), m_FileNameFormat(filePath), m_Stop(false)
	{
		if (hour < 0 || hour > 23)			throw TimeRangeException();
		if (minutes < 0 || minutes > 59)	throw TimeRangeException();
		if (seconds < 0 || seconds > 59)	throw TimeRangeException();

// Set the date using time_t and mktime (pre C++20)
#ifdef SBLOGGER_OLD_DATES
		auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		auto newTime = *std::localtime(&tt);
		newTime.tm_sec = m_Seconds;
		newTime.tm_min = m_Minutes;
		newTime.tm_hour = m_Hours;
		newTime.tm_mday++;
		tt = std::mktime(&newTime);
		m_NextChangeTime = std::chrono::system_clock::from_time_t(tt);
#else
// Set the date using C++20 (or later) methods
#endif
		m_FileChangeThread = std::thread(&DailyLogger::changeFile, this);
	}

	// Creates an instance of DailyLogger which outputs to a file stream given by the "filePath" parameter, which will be recreated at the specified interval
	// By default there is no formatting and auto flush is set to true
	inline DailyLogger::DailyLogger(const std::string&& filePath, const std::string&& format, int hour, int minutes, int seconds, bool autoFlush)
		: FileLogger(filePath, format, autoFlush), m_FileNameFormat(filePath), m_Stop(false)
	{
		if (hour < 0 || hour > 23)			throw TimeRangeException();
		if (minutes < 0 || minutes > 59)	throw TimeRangeException();
		if (seconds < 0 || seconds > 59)	throw TimeRangeException();

// Set the date using time_t and mktime (pre C++20)
#ifdef SBLOGGER_OLD_DATES
		auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		auto newTime = *std::localtime(&tt);
		newTime.tm_sec = m_Seconds;
		newTime.tm_min = m_Minutes;
		newTime.tm_hour = m_Hours;
		newTime.tm_mday++;
		tt = std::mktime(&newTime);
		m_NextChangeTime = std::chrono::system_clock::from_time_t(tt);
#else
// Set the date using C++20 (or later) methods
#endif
		m_FileChangeThread = std::thread(&DailyLogger::changeFile, this);
	}

	// Destructor

	// Flush and close stream if open
	inline DailyLogger::~DailyLogger()
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_Stop = true;
		lock.unlock();

		if (m_FileChangeThread.joinable())
			m_FileChangeThread.join();

		lock.lock();
		if (m_FileStream.is_open())
		{
			m_FileStream.flush();
			m_FileStream.close();
		}
	}

	//
	// Protected methods
	//

	// Writes string to file stream and flush if auto flush is set
	inline void DailyLogger::writeToStream(const std::string&& str)
	{
		std::unique_lock<std::mutex> lock(m_Mutex);

		if (!m_FileStream.is_open())
#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
			std::cerr << "The file stream " + m_FilePath + " is not opened.";
#else
			std::cerr << "The file stream " + m_FilePath.string() + " is not opened.";
#endif
		else
		{
			m_FileStream << str;
			if (m_AutoFlush)
				m_FileStream.flush();
		}
	}

	// Check if it is time to change the current file (closing it) and open the new one, according to the time provided
	inline void DailyLogger::changeFile()
	{
		auto delay = std::chrono::minutes(1);
		auto now = std::chrono::system_clock::now() + delay;

		while (!m_Stop)
		{
			while (now < m_NextChangeTime)
			{
				std::this_thread::sleep_for(delay);
				now = std::chrono::system_clock::now() + delay;
			}

			if (!m_Stop)
			{
// Set the date using time_t and mktime (pre C++20)
#ifdef SBLOGGER_OLD_DATES
				auto tt = std::chrono::system_clock::to_time_t(m_NextChangeTime);
				auto newTime = *std::localtime(&tt);
				newTime.tm_sec = m_Seconds;
				newTime.tm_min = m_Minutes;
				newTime.tm_hour = m_Hours;
				newTime.tm_mday++;
				tt = std::mktime(&newTime);
				m_NextChangeTime = std::chrono::system_clock::from_time_t(tt);
#else
// Set the date using C++20 (or later) methods
#endif

				std::unique_lock<std::mutex> lock(m_Mutex);
				if (m_FileStream.is_open())
				{
					m_FileStream.flush();
					m_FileStream.close();

					std::string formattedFilePath(m_FileNameFormat);
					addPadding(formattedFilePath);
					addColours(formattedFilePath);
					replacePredefinedPlaceholders(formattedFilePath);
					replaceCurrentLevel(formattedFilePath);
					replaceOthers(formattedFilePath, nullptr, nullptr, nullptr);
					replaceDateFormats(formattedFilePath);
					m_FileStream = std::fstream((m_FilePath = formattedFilePath), std::ios::out | std::ios::trunc);
				}
			}
		}
	}

	//
	// Public methods
	//

	// Flush file stream
	inline void DailyLogger::Flush() noexcept
	{
		std::unique_lock<std::mutex> lock(m_Mutex);

		if (m_FileStream.is_open())
			m_FileStream.flush();
	}

	// Clear log file
	inline void DailyLogger::ClearLogs() noexcept
	{
		std::unique_lock<std::mutex> lock(m_Mutex);

		if (m_FileStream.is_open())
		{
#ifdef SBLOGGER_LEGACY
			m_FileStream.close();
			m_FileStream = std::fstream(m_FilePath, std::ofstream::out | std::ofstream::trunc);
#else
			std::filesystem::resize_file(m_FilePath, 0);
			m_FileStream.seekp(0);
#endif
		}
	}
}

//
// Macros for logging. Adds support for file, line and function info in logs.
//

#if defined SBLOGGER_LOG_LEVEL && SBLOGGER_LOG_LEVEL < SBLOGGER_LEVEL_OFF
	#if SBLOGGER_LOG_LEVEL <= SBLOGGER_LEVEL_TRACE
		#define SBLOGGER_WRITE(x, ...)			x.Write(__VA_ARGS__, "__MACROS__", __FILE__, __LINE__, __func__)
		#define SBLOGGER_WRITELINE(x, ...)		x.WriteLine(__VA_ARGS__, "__MACROS__", __FILE__, __LINE__, __func__)
		#define SBLOGGER_TRACE(x, ...)			x.Trace(__VA_ARGS__, "__MACROS__", __FILE__, __LINE__, __func__)
	#else
		#define SBLOGGER_WRITE(x, ...)
		#define SBLOGGER_WRITELINE(x, ...)
		#define SBLOGGER_TRACE(x, ...)
	#endif

	#if SBLOGGER_LOG_LEVEL <= SBLOGGER_LEVEL_DEBUG
		#define SBLOGGER_DEBUG(x, ...)			x.Debug(__VA_ARGS__, "__MACROS__", __FILE__, __LINE__, __func__)
	#else
		#define SBLOGGER_DEBUG(x, ...)
	#endif

	#if SBLOGGER_LOG_LEVEL <= SBLOGGER_LEVEL_INFO
		#define SBLOGGER_INFO(x, ...)			x.Info(__VA_ARGS__, "__MACROS__", __FILE__, __LINE__, __func__)
	#else
		#define SBLOGGER_INFO(x, ...)
	#endif

	#if SBLOGGER_LOG_LEVEL <= SBLOGGER_LEVEL_WARN
		#define SBLOGGER_WARN(x, ...)			x.Warn(__VA_ARGS__, "__MACROS__", __FILE__, __LINE__, __func__)
	#else
		#define SBLOGGER_WARN(x, ...)
	#endif

	#if SBLOGGER_LOG_LEVEL <= SBLOGGER_LEVEL_ERROR
		#define SBLOGGER_ERROR(x, ...)			x.Error(__VA_ARGS__, "__MACROS__", __FILE__, __LINE__, __func__)
	#else
		#define SBLOGGER_ERROR(x, ...)
	#endif

	#if SBLOGGER_LOG_LEVEL <= SBLOGGER_LEVEL_CRITICAL
		#define SBLOGGER_CRITICAL(x, ...)		x.Critical(__VA_ARGS__, "__MACROS__", __FILE__, __LINE__, __func__)
	#else
		#define SBLOGGER_CRITICAL(x, ...)
	#endif
#else
	#define SBLOGGER_WRITE(x, ...)
	#define SBLOGGER_WRITELINE(x, ...)
	#define SBLOGGER_TRACE(x, ...)
	#define SBLOGGER_DEBUG(x, ...)
	#define SBLOGGER_INFO(x, ...)
	#define SBLOGGER_WARN(x, ...)
	#define SBLOGGER_ERROR(x, ...)
	#define SBLOGGER_CRITICAL(x, ...)
#endif
#endif