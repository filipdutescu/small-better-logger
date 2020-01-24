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

// Cross-platform macros
#ifdef SBLOGGER_NIX
	#define SBLOGGER_NEWLINE '\n'
#elif SBLOGGER_OS9
	#define SBLOGGER_NEWLINE '\r'
#else
	#define SBLOGGER_NEWLINE "\r\n"
#endif

#ifdef SBLOGGER_LEGACY
	// Raw file path regex as string literal
	#define SBLOGGER_RAW_FILE_PATH_REGEX R"regex(^(((([a-zA-Z]\:|\\)+\\[^\/\\:"'*?<>|\0]+)+|([^\/\\:"'*?<>|\0]+)+)|(((\.\/|\~\/|\/[^\/\\:"'*?~<>|\0]+\/)?[^\/\\:"'*?~<>|\0]+)+))$)regex"
#endif

// Log Levels macros to be used with "SBLOGGER_LOG_LEVEL" macro for defining a default level
#define SBLOGGER_LEVEL_TRACE     0
#define SBLOGGER_LEVEL_DEBUG     1
#define SBLOGGER_LEVEL_INFO      2
#define SBLOGGER_LEVEL_WARN      3
#define SBLOGGER_LEVEL_ERROR     4
#define SBLOGGER_LEVEL_CRITICAL  5
#define SBLOGGER_LEVEL_OFF       6
// Define your prefered active level using the macro bellow, or use the static method Logger::SetLoggingLevel(const LOG_LEVELS& level)
//#define SBLOGGER_LOG_LEVEL SBLOGGER_LEVEL_TRACE

// Used for writing to output stream
#include <iostream>
#include <fstream>

// Used for formatting and creating the output string
#include <sstream>
#include <vector>
#include <regex>

// For pre C++17 compilers define the "SBLOGGER_LEGACY" macro, to replace <filesystem> operations with regex and other alternatives
#ifndef SBLOGGER_LEGACY
// Used for truncating log files
#include <filesystem>
#endif

// For formatting dates to string pre C++20
#ifndef SBLOGGER_OLD_DATES
// Make use of std::chrono::format
#include <chrono>
#else
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
	// Loggers' declaration

	// Basic Logger
	// Abstract class which implements basic logger methods and members (ex.: auto flush, format, replace formatters etc)
	class Logger;
	using logger = Logger;

	// Stream Logger
	// Used to log messages to a non-file stream (ex.: STDOUT, STDERR, STDLOG)
	class StreamLogger;
	// Used to log messages to a non-file stream (ex.: STDOUT, STDERR, STDLOG)
	using stream_logger = StreamLogger;

	// File Logger
	// Used to log messages to a file stream
	class FileLogger;
	// Used to log messages to a file stream
	using file_logger = FileLogger;

	// Enum Definitions

	// Log level enum. Contains all possible log levels, such as TRACE, ERROR, FATAL etc.
	enum LOG_LEVELS
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

	// Custom Exceptions

	// Base Exception
	class SBLoggerException : public std::exception
	{
	protected:
		// Protected Members
		std::string m_Exception;

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

	// Get error message
	inline const char* SBLoggerException::What() const throw() { return m_Exception.c_str(); }

	// NullOrEmptyPathException

	// Thrown when the given file path is null or empty
	class NullOrEmptyPathException : public SBLoggerException
	{
	public:
		NullOrEmptyPathException();
	};

	NullOrEmptyPathException::NullOrEmptyPathException() 
		: SBLoggerException("File path cannot be null or empty.")
	{ }

	// NullOrWhitespaceNameException

	// Thrown when the given file name is null or whitespace
	class NullOrWhitespaceNameException : public SBLoggerException
	{
	public:
		NullOrWhitespaceNameException();
	};

	NullOrWhitespaceNameException::NullOrWhitespaceNameException() 
		: SBLoggerException("File name cannot be null or whitespace.")
	{ }

	// InvalidFilePathException

	// Thrown when the specified file could not be openned
	class InvalidFilePathException : public SBLoggerException
	{
	public:
		InvalidFilePathException();
		InvalidFilePathException(const std::string& filePath);
		InvalidFilePathException(const std::string&& filePath);
	};

	InvalidFilePathException::InvalidFilePathException() 
		: SBLoggerException("Cannot open log file to write to.")
	{ }

	InvalidFilePathException::InvalidFilePathException(const std::string& filePath)
		: SBLoggerException("Cannot open log file " + filePath + '.')
	{ }

	InvalidFilePathException::InvalidFilePathException(const std::string&& filePath)
		: SBLoggerException("Cannot open log file " + filePath + '.')
	{ }

	// Classes' Definitions

	// Abstract class which implements basic logger methods and members (ex.: auto flush, format, replace formatters etc)
	class Logger
	{
	protected:
		// Protected Members
		std::string m_Format;
		bool m_AutoFlush;
		int m_IndentCount;
		static LOG_LEVELS s_CurrentLogLevel;

		// Protected constructors

		// Initialize a logger, with a format, auto flush (by default)
		inline Logger(const std::string& format, bool autoFlush);

		// Initialize a logger, with no format, auto flush (by default)
		inline Logger(bool autoFlush) noexcept;

		// Copy constructor
		inline Logger(const Logger& other) noexcept;

		// Move constructor
		inline Logger(Logger&& other) noexcept;

		// Protected methods

		// Writes string to appropriate stream
		inline virtual void writeToStream(const std::string&& message) = 0;

		// Converts a T value to a string to be used in writing a log
		template<typename T>
		inline std::string stringConvert(const T& t) const noexcept;

		// Add indent to string (if it is set)
		inline void addIndent(std::string& message) const noexcept;

		// Add padding to string (if padding format exists)
		inline void addPadding(std::string& message) const noexcept;

		// Append format (if it exists) and replace all "{n}" placeholders with their respective values (n=0,...)
		inline std::string replacePlaceholders(std::string message, std::vector<std::string>&& items) const noexcept;

		// Replace current logging level in format
		inline void replaceCurrentLevel(std::string& message) const noexcept;

		// Replace date format using std::strftime (pre C++20) or std::chrono::format
		inline void replaceDateFormats(std::string& message) const noexcept;


	public:
		// Default destructor
		virtual ~Logger() = default;

		// Public Methods

		// Set the current logging level to one of the "LOG_LEVELS" options (ex.: TRACE, DEBUG, INFO etc). 
		static inline void SetLoggingLevel(const LOG_LEVELS& level) noexcept;

		// Get the current logging level (one of the "LOG_LEVELS" options, ex.: TRACE, DEBUG, INFO etc). 
		static inline const LOG_LEVELS GetLoggingLevel() noexcept;

		// Get the current log format
		inline const std::string GetFormat() const noexcept;

		// Set the current log format to "format"
		inline void SetFormat(const std::string& format);

		// Flush appropriate stream
		virtual inline void Flush() = 0;

		// Indent (prepend '\t') log, returns the number of indents the final message will contain
		inline const int Indent() noexcept;

		// Dedent (remove '\t') log, returns the number of indents the final message will contain
		inline const int Dedent() noexcept;

		// Generic Methods: Write a TRACE level message (depending on the specified "LOG_LEVEL") to a stream

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

		// Generic Methods: Write a level message (depending on the specified "LOG_LEVEL") to a stream 

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

		// Generic Methods: Write a TRACE level message to a stream 

		// Writes to the stream a message and inserts values into placeholders (should they exist), of TRACE importance
		template<typename ...T>
		inline void Trace(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist), of TRACE importance
		template<typename ...T>
		inline void Trace(const std::string&& message, const T&& ...t);

		// Generic Methods: Write a DEBUG level message to a stream 

		// Writes to the stream a message and inserts values into placeholders (should they exist), of DEBUG importance
		template<typename ...T>
		inline void Debug(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist), of DEBUG importance
		template<typename ...T>
		inline void Debug(const std::string&& message, const T&& ...t);

		// Generic Methods: Write a INFO level message to a stream 

		// Writes to the stream a message and inserts values into placeholders (should they exist), of INFO importance
		template<typename ...T>
		inline void Info(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist), of INFO importance
		template<typename ...T>
		inline void Info(const std::string&& message, const T&& ...t);

		// Generic Methods: Write a WARN level message to a stream 

		// Writes to the stream a message and inserts values into placeholders (should they exist), of WARN importance
		template<typename ...T>
		inline void Warn(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist), of WARN importance
		template<typename ...T>
		inline void Warn(const std::string&& message, const T&& ...t);

		// Generic Methods: Write a ERROR level message to a stream 

		// Writes to the stream a message and inserts values into placeholders (should they exist), of ERROR importance
		template<typename ...T>
		inline void Error(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist), with ERROR importance
		template<typename ...T>
		inline void Error(const std::string&& message, const T&& ...t);

		// Generic Methods: Write a CRITICAL level message to a stream 

		// Writes to the stream a message and inserts values into placeholders (should they exist), of CRITICAL importance
		template<typename ...T>
		inline void Critical(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist), with CRITICAL importance
		template<typename ...T>
		inline void Critical(const std::string&& message, const T&& ...t);
	};
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

	// Initialize a logger, with a format, auto flush (by default)
	inline Logger::Logger(const std::string& format, bool autoFlush)
		: m_Format(format), m_AutoFlush(autoFlush), m_IndentCount(0)
	{
#ifdef SBLOGGER_LEGACY
		std::string placeholder = "tr";
#else
		std::string_view placeholder = "tr";
#endif
		std::size_t placeholderPosition = m_Format.find(placeholder);
		if (placeholderPosition != std::string::npos && (m_Format[placeholderPosition - 1u] == '%' || m_Format[placeholderPosition - 2u] == '%'))
			m_Format[placeholderPosition - 1u] == '^' ? m_Format.replace(placeholderPosition - 2u, placeholder.size() + 2u, "TRACE") : m_Format.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Trace");
		
		placeholderPosition = m_Format.find(placeholder = "dbg");
		if (placeholderPosition != std::string::npos && (m_Format[placeholderPosition - 1u] == '%' || m_Format[placeholderPosition - 2u] == '%'))
			m_Format[placeholderPosition - 1u] == '^' ? m_Format.replace(placeholderPosition - 2u, placeholder.size() + 2u, "DEBUG") : m_Format.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Debug");

		placeholderPosition = m_Format.find(placeholder = "inf");
		if (placeholderPosition != std::string::npos && (m_Format[placeholderPosition - 1u] == '%' || m_Format[placeholderPosition - 2u] == '%'))
			m_Format[placeholderPosition - 1u] == '^' ? m_Format.replace(placeholderPosition - 2u, placeholder.size() + 2u, "INFO") : m_Format.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Info");

		placeholderPosition = m_Format.find(placeholder = "wn");
		if (placeholderPosition != std::string::npos && (m_Format[placeholderPosition - 1u] == '%' || m_Format[placeholderPosition - 2u] == '%'))
			m_Format[placeholderPosition - 1u] == '^' ? m_Format.replace(placeholderPosition - 2u, placeholder.size() + 2u, "WARN") : m_Format.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Warn");

		placeholderPosition = m_Format.find(placeholder = "er");
		if (placeholderPosition != std::string::npos && (m_Format[placeholderPosition - 1u] == '%' || m_Format[placeholderPosition - 2u] == '%'))
			m_Format[placeholderPosition - 1u] == '^' ? m_Format.replace(placeholderPosition - 2u, placeholder.size() + 2u, "ERROR") : m_Format.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Error");

		placeholderPosition = m_Format.find(placeholder = "crt");
		if (placeholderPosition != std::string::npos && (m_Format[placeholderPosition - 1u] == '%' || m_Format[placeholderPosition - 2u] == '%'))
			m_Format[placeholderPosition - 1u] == '^' ? m_Format.replace(placeholderPosition - 2u, placeholder.size() + 2u, "CRITICAL") : m_Format.replace(placeholderPosition - 1u, placeholder.size() + 2u, "Critical");
		}

	// Initialize a logger, with no format, auto flush (by default)
	inline Logger::Logger(bool autoFlush) noexcept
		: m_Format(), m_AutoFlush(autoFlush), m_IndentCount(0)
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

	// Converts a T value to a string to be used in writing a log
	template<typename T>
	inline std::string Logger::stringConvert(const T& t) const noexcept
	{
		std::stringstream ss;
		ss << t;
		return ss.str();
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
		std::string placeholders[]{ "msg", "lvl", "tr", "dbg", "inf", "wrn", "er", "crt" };
#else
		std::string_view placeholders[] { "msg", "lvl", "tr", "dbg", "inf", "wrn", "er", "crt" };
#endif
		std::string digits = "1234567890", floatDigits = "1234567890.", currentPadding;
		size_t placeholderPosition, offset = 0u, noDigits, noDecimals, currentSectionEnd;
		float noSpacesLeft, noSpacesRight;
		bool isNextCharacterSpace;

		while ((placeholderPosition = message.find_first_of(digits, offset)) != std::string::npos)
		{
			if (message[placeholderPosition - 1u] != '%' && (placeholderPosition > 1u && message[placeholderPosition - 2u] != '%' && message[placeholderPosition - 1u] != '.'))
				offset += placeholderPosition + 1u;
			else
			{
				if(message[placeholderPosition - 1] != '%')
					currentPadding = message.substr(placeholderPosition - 1u, message.find_first_not_of(floatDigits, placeholderPosition + 1u) - placeholderPosition + 1u);
				else
					currentPadding = message.substr(placeholderPosition, message.find_first_not_of(floatDigits, placeholderPosition) - placeholderPosition);
				
				noDigits = currentPadding.size();
				noDecimals = (noDecimals = currentPadding.find('.')) != std::string::npos ? (noDigits - noDecimals - 1u) : 0u;
				noSpacesRight = std::modf(std::stof(currentPadding), &noSpacesLeft);
				for (size_t i = 0u; i < noDecimals; ++i)
					noSpacesRight *= 10;

				if (noSpacesRight)
				{
					if ((currentSectionEnd = message.find_first_of(" ."), placeholderPosition + noDigits + 1u) != std::string::npos)
					{
						isNextCharacterSpace = message[currentSectionEnd + placeholderPosition + noDigits + 1u] == ' ';
						message.replace(currentSectionEnd + placeholderPosition + noDigits + 1u, 1, (size_t)noSpacesRight + 1u, ' ');
						if(!isNextCharacterSpace)
							message[currentSectionEnd + placeholderPosition + noDigits + 1u + (size_t)noSpacesRight] = '.';
					}
				}

				message.replace(placeholderPosition - 1u, noDigits + 1u, (size_t)noSpacesLeft, ' ');
			}
		}
	}

	// Append format (if it exists) and replace all "{n}" placeholders with their respective values (n=0,...)
	inline std::string Logger::replacePlaceholders(std::string message, std::vector<std::string>&& items) const noexcept
	{
		std::string placeholder;
		std::size_t placeholderPosition;
		for (size_t i = 0u; i < items.size(); ++i)
		{
			placeholder = "{" + std::to_string(i) + "}";
			while ((placeholderPosition = message.find(placeholder)) != std::string::npos)
				message.replace(placeholderPosition, placeholder.size(), items[i]);
		}
		
		if (m_Format.empty())
		{
			addIndent(message);
			return message;
		}
		
		message = (placeholderPosition = m_Format.find("%msg")) != std::string::npos ? std::string(m_Format).replace(placeholderPosition, 4u, message) : (m_Format + ' ' + message);
		addIndent(message);
		addPadding(message);
		replaceCurrentLevel(message);
		replaceDateFormats(message);

		return message;
	}

	// Replace current logging level in format
	inline void Logger::replaceCurrentLevel(std::string& message) const noexcept
	{
		const std::size_t placeholderPosition = message.find("lvl");
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

	// Replace date format using std::strftime (pre C++20) or std::chrono::format
	inline void Logger::replaceDateFormats(std::string& message) const noexcept
	{
		// Replace date format using std::strftime (pre C++20)
#ifdef SBLOGGER_OLD_DATES
		std::time_t t = std::time(nullptr);
		std::size_t messageLength = message.size();
		char* buffer = new char[messageLength + 100u]{ 0 };
		std::string result;

		if (std::strftime(buffer, sizeof(char) * (messageLength + 100u), message.c_str(), std::localtime(&t)))
			message = std::string(buffer);

		delete[] buffer;
#else
		// Replace date format using std::chrono::format
		// Wait for MSVC to catch up
#endif
	}

	// Indent (prepend '\t') log, returns the number of indents the final message will contain
	inline const int Logger::Indent() noexcept
	{
		return ++m_IndentCount;
	}

	// Dedent (prepend '\t') log, returns the number of indents the final message will contain
	inline const int Logger::Dedent() noexcept
	{
		return --m_IndentCount;
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

	// Generic write methods to write a TRACE level message to the stream

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

	// Generic Methods: Write a level message (depending on the specified "LOG_LEVEL") to a stream 

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

	// Generic Methods: Write a TRACE level message to a stream 

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

	// Generic Methods: Write a DEBUG level message to a stream 

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

	// Generic Methods: Write a INFO level message to a stream 

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

	// Generic Methods: Write a WARN level message to a stream 

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

	// Generic Methods: Write a ERROR level message to a stream 

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

	// Generic Methods: Write a CRITICAL level message to a stream 

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

	// StreamLogger class

	// Used to log messages to a non-file stream (ex.: STDOUT, STDERR, STDLOG)
	class StreamLogger : public Logger
	{
	protected:
		// Protected members
		STREAM_TYPE m_StreamType;

		// Protected methods

		// Writes string to appropriate stream based on instance STREAM_TYPE (m_StreamType)
		inline void writeToStream(const std::string&& str) override;

	public:
		// Constructors and destructors

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

		// Overloaded Operators

		// Assignment Operator
		inline StreamLogger& operator=(const StreamLogger& other) noexcept;

		// Public Methods

		// Flush appropriate stream
		virtual inline void Flush() override;

		// Change the logger's stream type (to a different "STREAM_TYPE")
		inline void SetStreamType(STREAM_TYPE streamType);
	};

	// Constructors and destructors

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
		: Logger(other)
	{
		m_AutoFlush = other.m_AutoFlush;
		m_StreamType = other.m_StreamType;
	}

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

	// Overloaded Operators

	// Assignment Operator
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

	// Flush appropriate stream
	inline void StreamLogger::Flush()
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

	// Used to log messages to a file stream
	class FileLogger : public Logger
	{
		// Private members
#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		std::string m_FilePath;
#else
		std::filesystem::path m_FilePath;
#endif
		std::fstream m_FileStream;

		// Writes string to file stream and flush if auto flush is set
		inline void writeToStream(const std::string&& str) override;

	public:
		// Constructors

		// Deleted to prevent usage without providing a file path
		FileLogger() = delete;

		// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
		// By default there is no formatting and auto flush is set to true
		inline FileLogger(const char* filePath, const char* format = nullptr, bool autoFlush = true);

		// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
		// By default there is no formatting and auto flush is set to true
		inline FileLogger(const char* filePath, const std::string& format = std::string(), bool autoFlush = true);

		// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
		// By default there is no formatting and auto flush is set to true
		inline FileLogger(const std::string& filePath, const std::string& format = std::string(), bool autoFlush = true);

		// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
		// By default there is no formatting and auto flush is set to true
		inline FileLogger(const std::string&& filePath, const std::string&& format = std::string(), bool autoFlush = true);

		// Copy constructor

		// Creates a FileLogger instance from an already existing one
		inline FileLogger(const FileLogger& other);

		// Move constructor

		// Creates a FileLogger instance from another one
		inline FileLogger(FileLogger&& other) noexcept;

		// Destructor

		// Flush and close stream if open
		inline ~FileLogger() override;

		// Overloaded Operators

		// Assignment Operator (deleted since having two streams for the same file causes certain output not to be writen).
		inline FileLogger& operator=(const FileLogger& other) = delete;

		// Public Methods

		// Flush file stream
		inline void Flush() override;

		// Clear log file
		inline void ClearLogs();
	};

	// Constructors

	// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
	// By default there is no formatting and auto flush is set to true
	inline FileLogger::FileLogger(const char* filePath, const char* format, bool autoFlush)
		: Logger(format == nullptr ? "" : format, autoFlush)
	{
		if (filePath == nullptr || filePath[0] == '\0') throw NullOrEmptyPathException();

#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		m_FilePath = filePath;
		if (!std::regex_match(m_FilePath, SBLOGGER_FILE_PATH_REGEX)) throw InvalidFilePathException(m_FilePath);
#else
		m_FilePath = std::filesystem::path(filePath);
		// Check file path for null, empty, inexistent or whitespace only paths and filenames
		if (!m_FilePath.has_filename() || !m_FilePath.has_extension()) throw NullOrEmptyPathException();
		if (m_FilePath.filename().replace_extension().string().find_first_not_of(' ') == std::string::npos) throw NullOrWhitespaceNameException();
		auto parentPath = m_FilePath.parent_path();
		if (!parentPath.empty() && !std::filesystem::directory_entry(parentPath).exists()) throw InvalidFilePathException(filePath);
#endif
		m_FileStream = std::fstream(filePath, std::fstream::app | std::fstream::out);

#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath);
#else
		if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath.string());
#endif
	}

	// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
	// By default there is no formatting and auto flush is set to true
	inline FileLogger::FileLogger(const char* filePath, const std::string& format, bool autoFlush)
		: Logger(format, autoFlush)
	{
		if (filePath == nullptr || filePath[0] == '\0') throw NullOrEmptyPathException();

#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		m_FilePath = filePath;
		if (!std::regex_match(m_FilePath, SBLOGGER_FILE_PATH_REGEX)) throw InvalidFilePathException(m_FilePath);
#else
		m_FilePath = std::filesystem::path(filePath);
		// Check file path for null, empty, inexistent or whitespace only paths and filenames
		if (!m_FilePath.has_filename() || !m_FilePath.has_extension()) throw NullOrEmptyPathException();
		if (m_FilePath.filename().replace_extension().string().find_first_not_of(' ') == std::string::npos) throw NullOrWhitespaceNameException();
		if (!std::filesystem::directory_entry(m_FilePath.parent_path()).exists()) throw InvalidFilePathException(filePath);
#endif
		m_FileStream = std::fstream(filePath, std::fstream::app | std::fstream::out);

#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath);
#else
		if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath.string());
#endif
	}

	// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
	// By default there is no formatting and auto flush is set to true
	inline FileLogger::FileLogger(const std::string& filePath, const std::string& format, bool autoFlush)
		: Logger(format, autoFlush)
	{
#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		m_FilePath = filePath;
		if (!std::regex_match(m_FilePath, SBLOGGER_FILE_PATH_REGEX)) throw InvalidFilePathException(m_FilePath);
#else
		m_FilePath = std::filesystem::path(filePath);
		// Check file path for null, empty, inexistent or whitespace only paths and filenames
		if (!m_FilePath.has_filename() || !m_FilePath.has_extension()) throw NullOrEmptyPathException();
		if (m_FilePath.filename().replace_extension().string().find_first_not_of(' ') == std::string::npos) throw NullOrWhitespaceNameException();
		if (!std::filesystem::directory_entry(m_FilePath.parent_path()).exists()) throw InvalidFilePathException(filePath);
#endif
		m_FileStream = std::fstream(filePath, std::fstream::app | std::fstream::out);

#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath);
#else
		if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath.string());
#endif
	}

	// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
	// By default there is no formatting and auto flush is set to true
	inline FileLogger::FileLogger(const std::string&& filePath, const std::string&& format, bool autoFlush)
		: Logger(format, autoFlush)
	{
#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		m_FilePath = filePath;
		if (!std::regex_match(m_FilePath, SBLOGGER_FILE_PATH_REGEX)) throw InvalidFilePathException(m_FilePath);
#else
		m_FilePath = std::filesystem::path(filePath);
		// Check file path for null, empty, inexistent or whitespace only paths and filenames
		if (!m_FilePath.has_filename() || !m_FilePath.has_extension()) throw NullOrEmptyPathException();
		if (m_FilePath.filename().replace_extension().string().find_first_not_of(' ') == std::string::npos) throw NullOrWhitespaceNameException();
		if (!std::filesystem::directory_entry(m_FilePath.parent_path()).exists()) throw InvalidFilePathException(filePath);
#endif
		m_FileStream = std::fstream(filePath, std::fstream::app | std::fstream::out);

#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath);
#else
		if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath.string());
#endif
	}

	// Copy constructor
	// Creates a FileLogger instance from an already existing one
	inline FileLogger::FileLogger(const FileLogger& other)
		: Logger(other)
	{
#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		m_FilePath = other.m_FilePath;
		if (!std::regex_match(m_FilePath, SBLOGGER_FILE_PATH_REGEX)) throw InvalidFilePathException(m_FilePath);
#else
		m_FilePath = std::filesystem::path(other.m_FilePath);
		// Check file path for null, empty, inexistent or whitespace only paths and filenames
		if (!m_FilePath.has_filename() || !m_FilePath.has_extension()) throw NullOrEmptyPathException();
		if (m_FilePath.filename().replace_extension().string().find_first_not_of(' ') == std::string::npos) throw NullOrWhitespaceNameException();
		if (!std::filesystem::directory_entry(m_FilePath.parent_path()).exists()) throw InvalidFilePathException(other.m_FilePath.string());
#endif
		m_FileStream = std::fstream(other.m_FilePath, std::fstream::app | std::fstream::out);

#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
		if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath);
#else
		if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath.string());
#endif
	}

	// Move constructor

	// Creates a FileLogger instance from another one
	inline FileLogger::FileLogger(FileLogger&& other) noexcept
		: Logger(other)
	{
		m_FilePath = std::move(other.m_FilePath);
		m_FileStream = std::fstream(other.m_FilePath, std::fstream::app | std::fstream::out);
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

	// Flush file stream
	inline void FileLogger::Flush()
	{
		if(m_FileStream.is_open())
			m_FileStream.flush();
	}

	// Clear log file
	inline void FileLogger::ClearLogs()
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
}

// TODO: Macros for logging. Adds support for file, line and function info in logs. 
//#define TEST __FILE__
#endif