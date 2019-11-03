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

// For pre C++17 compilers define the "SBLOGGER_LEGACY" macro, to replace <filesystem> operations with regex and other alternatives
#ifndef SBLOGGER_LEGACY
// Used for truncating log files
#include <filesystem>
#endif

// For formatting dates to string pre C++20
#ifndef SBLOGGER_OLD_DATES
#include <chrono>
#else
#include <iomanip>
#include <ctime>
#endif

// Used for formatting and creating the output string
#include <sstream>
#include <vector>
#include <regex>

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
		SBLoggerException(std::string& exception) 
			: std::exception(exception.c_str()), m_Exception(exception)
		{ }

		// Creates an exception with a given message
		SBLoggerException(std::string&& exception)
			: std::exception(exception.c_str()), m_Exception(exception)
		{ }

		// Creates an exception with a given message
		SBLoggerException(const char* exception)
			: std::exception(exception), m_Exception(exception)
		{ }

	public:
		// Throw exception
		~SBLoggerException() throw() 
		{ }

		const char* What() const throw() { return m_Exception.c_str(); }
	};

	// Thrown when the given file path is null or empty
	class NullOrEmptyPathException : public SBLoggerException
	{
	public:
		NullOrEmptyPathException() : SBLoggerException("File path cannot be null or empty.")
		{ }
	};

	// Thrown when the given file name is null or whitespace
	class NullOrWhitespaceNameException : public SBLoggerException
	{
	public:
		NullOrWhitespaceNameException() : SBLoggerException("File name cannot be null or whitespace.")
		{ }
	};

	// Thrown when the specified file could not be openned
	class InvalidFilePathException : public SBLoggerException
	{
	public:
		InvalidFilePathException() : SBLoggerException("Cannot open log file to write to.")
		{ }

		InvalidFilePathException(const std::string& filePath) : SBLoggerException("Cannot open log file " + filePath + '.')
		{ }

		InvalidFilePathException(const std::string&& filePath) : SBLoggerException("Cannot open log file " + filePath + '.')
		{ }
	};

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

		// Initialize a logger, with no format, auto flush (by default)
		Logger(const std::string& format, bool autoFlush)
			: m_Format(format), m_AutoFlush(autoFlush), m_IndentCount(0)
		{ }

		// Initialize a logger, with no format, auto flush (by default)
		Logger(bool autoFlush)
			: m_Format(""), m_AutoFlush(autoFlush), m_IndentCount(0)
		{ }

		// Copy constructor
		Logger(const Logger& other)
			: m_Format(other.m_Format), m_AutoFlush(other.m_AutoFlush), m_IndentCount(other.m_IndentCount)
		{ }

		// Move constructor
		Logger(Logger&& other) noexcept
		{
			m_Format = other.m_Format;
			m_AutoFlush = other.m_AutoFlush;
			m_IndentCount = other.m_IndentCount;
		}

		// Protected methods

		// Converts a T value to a string to be used in writing a log
		template<typename T>
		inline std::string stringConvert(const T& t);

		// Append format (if it exists) and replace all "{n}" placeholders with their respective values (n=0,...)
		inline std::string replacePlaceholders(std::string message, std::vector<std::string>&& items);

		// Add indent to string (if it is set)
		inline std::string addIndent(std::string message);

		// Writes string to appropriate stream
		inline virtual void writeToStream(const std::string&& message) = 0;

	public:
		// Default destructor
		virtual ~Logger() = default;

		// Public Methods

		// Set the current logging level to one of the "LOG_LEVELS" options (ex.: TRACE, DEBUG, INFO etc). 
		static inline void SetLoggingLevel(const LOG_LEVELS& level) noexcept;

		// Get the current logging level (one of the "LOG_LEVELS" options, ex.: TRACE, DEBUG, INFO etc). 
		static inline const LOG_LEVELS GetLoggingLevel() noexcept;

		// Get the current log format
		inline const std::string GetFormat() const;

		// Set the current log format to "format"
		inline void SetFormat(const std::string& format);

		// Flush appropriate stream
		virtual inline void Flush() = 0;

		// Indent (prepend '\t') log, returns the number of indents the final message will contain
		inline const int Indent();

		// Dedent (remove '\t') log, returns the number of indents the final message will contain
		inline const int Dedent();

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

	// Converts a T value to a string to be used in writing a log
	template<typename T>
	inline std::string Logger::stringConvert(const T& t)
	{
		std::stringstream ss;
		ss << t;
		return ss.str();
	}

	// Append format (if it exists) and replace all "{n}" placeholders with their respective values (n=0,...)
	inline std::string Logger::replacePlaceholders(std::string message, std::vector<std::string>&& items)
	{
		std::regex placeholder;

		for (unsigned int i = 0; i < items.size(); i++)
		{
			placeholder = R"(\{)" + std::to_string(i) + R"(\})";
			message = std::regex_replace(message, placeholder, items[i]);
		}

		return addIndent(m_Format.empty() ? message : m_Format + " " + message);
	}

	// Add indent to string (if it is set)
	inline std::string Logger::addIndent(std::string message)
	{
		for (int i = 0; i < m_IndentCount; i++)
			message = '\t' + message;

		return  message;
	}

	// Indent (prepend '\t') log, returns the number of indents the final message will contain
	inline const int Logger::Indent()
	{
		return ++m_IndentCount;
	}

	// Dedent (prepend '\t') log, returns the number of indents the final message will contain
	inline const int Logger::Dedent()
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
	inline const std::string Logger::GetFormat() const
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
		StreamLogger(const STREAM_TYPE& type = STREAM_TYPE::STDOUT, const std::string& format = std::string(), bool autoFlush = false)
			: Logger(format, autoFlush), m_StreamType(type)
		{ }

		// Creates an instance of Logger which outputs to STDOUT. Formats logs and auto flushes based on the parameter "autoFlush"
		StreamLogger(const std::string& format, bool autoFlush = false)
			: Logger(format, autoFlush), m_StreamType(STREAM_TYPE::STDOUT)
		{ }

		// Creates an instance of Logger which outputs to STDOUT. Formats logs and auto flushes based on the parameter "autoFlush"
		StreamLogger(const char* format, bool autoFlush = false)
			: Logger(format, autoFlush), m_StreamType(STREAM_TYPE::STDOUT)
		{ }

		// Creates an instance of Logger which outputs to STDOUT. Formats logs and auto flushes based on the parameter "autoFlush"
		StreamLogger(bool autoFlush)
			: Logger(std::string(), autoFlush), m_StreamType(STREAM_TYPE::STDOUT)
		{ }

		// Copy constructor

		// Creates a Logger instance from an already existing one
		StreamLogger(const StreamLogger& other)
			: Logger(other), m_StreamType(other.m_StreamType)
		{ }

		// Move constructor

		// Creates a Logger instance from another
		StreamLogger(StreamLogger&& other) noexcept
			: Logger(other)
		{
			m_AutoFlush = other.m_AutoFlush;
			m_StreamType = other.m_StreamType;
		}

		// Destructor
		
		// Flush stream before deletion
		~StreamLogger() override
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
		StreamLogger& operator=(const StreamLogger& other) noexcept
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

		// Public Methods

		// Flush appropriate stream
		virtual inline void Flush() override;

		// Change the logger's stream type (to a different "STREAM_TYPE")
		inline void SetStreamType(STREAM_TYPE streamType);
	};

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
		FileLogger(const char* filePath, const std::string& format = std::string(), bool autoFlush = true) 
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
			if(!std::filesystem::directory_entry(m_FilePath.parent_path()).exists()) throw InvalidFilePathException(filePath);
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
		FileLogger(const std::string& filePath, const std::string& format = std::string(), bool autoFlush = true) 
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
		FileLogger(const std::string&& filePath, const std::string& format = std::string(), bool autoFlush = true) 
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
		FileLogger(const FileLogger& other)
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
		FileLogger(FileLogger&& other) noexcept
			: Logger(other)
		{
			m_FilePath = std::move(other.m_FilePath);
			m_FileStream = std::fstream(other.m_FilePath, std::fstream::app | std::fstream::out);
		}

		// Destructor

		// Flush and close stream if open
		~FileLogger() override
		{
			if (m_FileStream.is_open())
			{
				m_FileStream.flush();
				m_FileStream.close();
			}
		}

		// Overloaded Operators

		// Assignment Operator
		FileLogger& operator=(const FileLogger& other)
		{
			if (this != &other)
			{
#ifdef SBLOGGER_LEGACY // Pre C++17 Compilers
				if (!std::regex_match(other.m_FilePath, SBLOGGER_FILE_PATH_REGEX)) throw InvalidFilePathException(other.m_FilePath);
#else
				// Check file path for null, empty, inexistent or whitespace only paths and filenames
				if (!other.m_FilePath.has_filename() || !other.m_FilePath.has_extension()) throw NullOrEmptyPathException();
				if (other.m_FilePath.filename().replace_extension().string().find_first_not_of(' ') == std::string::npos) throw NullOrWhitespaceNameException();
				if (!std::filesystem::directory_entry(other.m_FilePath.parent_path()).exists()) throw InvalidFilePathException(other.m_FilePath.string());
#endif
				m_FilePath = other.m_FilePath;
				m_AutoFlush = other.m_AutoFlush;
				m_Format = other.m_Format;
				m_IndentCount = other.m_IndentCount;
			}

			return *this;
		}

		// Public Methods

		// Flush file stream
		inline void Flush() override;

		// Clear log file
		inline void ClearLogs();
	};

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
#endif