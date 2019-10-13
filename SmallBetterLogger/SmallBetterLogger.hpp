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

// Used for writing to output stream
#include <iostream>
#include <fstream>

// Used for truncating log files
#include <filesystem>

// Used for formatting and creating the output string
#include <sstream>
#include <vector>
#include <regex>

// Cross-platform macros
#ifdef SBLOGGER_UNIX
#define NEWLINE '\n'
#elif SBLOGGER_OS9
#define NEWLINE '\r'
#else
#define NEWLINE "\r\n"
#endif

// File Path Regex
#define FILE_PATH_REGEX std::regex(R"regex(^(((([a-zA-Z]\:|\\)+\\[^\/\\:"'*?<>|\0]+)+|([^\/\\:"'*?<>|\0]+)+)|(((\.\/|\~\/|\/[^\/\\:"'*?~<>|\0]+\/)?[^\/\\:"'*?~<>|\0]+)+)))regex")

namespace sblogger
{
	// Loggers' declaration

	// Basic Logger
	// Used to log messages to a non-file stream (ex.: STDOUT, STDERR, STDLOG)
	class Logger;
	// Used to log messages to a non-file stream (ex.: STDOUT, STDERR, STDLOG)
	using logger = Logger;

	// File Logger
	// Used to log messages to a file stream
	class FileLogger;
	// Used to log messages to a file stream
	using filelogger = FileLogger;
	// Used to log messages to a file stream
	using file_logger = FileLogger;

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

	// Used to log messages to a non-file stream (ex.: STDOUT, STDERR, STDLOG)
	class Logger
	{
	protected:
		// Protected members
		STREAM_TYPE m_StreamType;
		bool m_AutoFlush;

		// Protected methods

		// Converts a T value to a string to be used in writing a log
		template<typename T>
		inline std::string stringConvert(const T& t);

		// Replace all "{n}" placeholders with their respective values (n=0,...)
		inline std::string replacePlaceholders(std::string message, std::vector<std::string>& items);

		// Writes string to appropriate stream based on instance STREAM_TYPE (m_StreamType)
		virtual inline void writeToStream(const std::string&& str);

	public:
		// Constructors and destructors

		// Creates an instance of Logger which outputs to a stream chosen from a STREAM_TYPE
		// By default uses STREAM_TYPE::STDOUT and no auto flush
		Logger(STREAM_TYPE type = STREAM_TYPE::STDOUT, bool autoFlush = false)
			: m_StreamType(type), m_AutoFlush(autoFlush)
		{ }

		// Creates an instance of Logger which outputs to STDOUT and auto flushes based on the parameter "autoFlush"
		Logger(bool autoFlush)
			: m_StreamType(STREAM_TYPE::STDOUT), m_AutoFlush(autoFlush)
		{ }

		// Copy constructor
		// Creates a Logger instance from an already existing one
		Logger(const Logger& other)
			: m_StreamType(other.m_StreamType), m_AutoFlush(other.m_AutoFlush)
		{ }

		// Move constructor
		// Creates a Logger instance from another
		Logger(Logger&& other) noexcept
		{
			m_AutoFlush = other.m_AutoFlush;
			m_StreamType = other.m_StreamType;
		}

		// Destructor
		// Flush stream before deletion
		~Logger()
		{ 
			switch (m_StreamType)
			{
			case STREAM_TYPE::STDERR:   std::cerr.flush();       break;
			case STREAM_TYPE::STDLOG:   std::clog.flush();       break;
			case STREAM_TYPE::STDOUT:   std::cout.flush();       break;
			}
		}

		// Public Methods

		// Writes to the stream a message and inserts values into placeholders (should they exist)
		template<typename ...T>
		inline void Write(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist)
		template<typename ...T>
		inline void Write(const std::string&& message, const T&& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist) and finishes with the newline character
		template<typename ...T>
		inline void WriteLine(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist) and finishes with the newline character
		template<typename ...T>
		inline void WriteLine(const std::string&& message, const T&& ...t);

		// Flush appropriate stream
		virtual inline void Flush();
	};

	// Writes to the stream a message and inserts values into placeholders (should they exist)
	template<typename ...T>
	inline void Logger::Write(const std::string& message, const T& ...t)
	{
		std::vector<std::string> printValues { stringConvert(t)... };
		writeToStream(replacePlaceholders(message, printValues));
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist)
	template<typename ...T>
	inline void Logger::Write(const std::string&& message, const T&& ...t)
	{
		std::vector<std::string> printValues{ stringConvert(t)... };
		writeToStream(replacePlaceholders(message, printValues));
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist) and finishes with the newline character
	template<typename ...T>
	inline void Logger::WriteLine(const std::string& message, const T& ...t)
	{
		std::vector<std::string> printValues{ stringConvert(t)... };
		writeToStream(replacePlaceholders(message, printValues) + NEWLINE);
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist) and finishes with the newline character
	template<typename ...T>
	inline void Logger::WriteLine(const std::string&& message, const T&& ...t)
	{
		std::vector<std::string> printValues{ stringConvert(t)... };
		writeToStream(replacePlaceholders(message, printValues) + NEWLINE);
	}

	template<typename T>
	inline std::string Logger::stringConvert(const T& t)
	{
		std::stringstream ss;
		ss << t;
		return ss.str();
	}

	inline std::string sblogger::Logger::replacePlaceholders(std::string message, std::vector<std::string>& items)
	{
		std::regex placeholder;

		for (unsigned int i = 0; i < items.size(); i++)
		{
			placeholder = R"(\{)" + std::to_string(i) + R"(\})";
			message = std::regex_replace(message, placeholder, items[i]);
		}

		return message;
	}

	inline void Logger::writeToStream(const std::string&& str)
	{
		switch (m_StreamType)
		{
		case STREAM_TYPE::STDERR:   std::cerr << str;       if (m_AutoFlush) std::cerr.flush();       break;
		case STREAM_TYPE::STDLOG:   std::clog << str;       if (m_AutoFlush) std::clog.flush();       break;
		default:                    std::cout << str;       if (m_AutoFlush) std::cout.flush();       break;
		}
	}

	// Flush appropriate stream
	inline void Logger::Flush()
	{
		switch (m_StreamType)
		{
		case STREAM_TYPE::STDERR:   std::cerr.flush();       break;
		case STREAM_TYPE::STDLOG:   std::clog.flush();       break;
		default:                    std::cout.flush();       break;
		}
	}

	// Used to log messages to a file stream
	class FileLogger : public Logger
	{
		// Private members
		std::string m_FilePath;
		std::fstream m_FileStream;

		// Writes string to file stream and flush if auto flush is set
		inline void writeToStream(const std::string&& str) override;

	public:
		// Constructors

		// Deleted to prevent usage without providing a file path
		FileLogger() = delete;

		// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
		// By default auto flush is set to true
		FileLogger(const char* filePath, bool autoFlush = true) : Logger(autoFlush)
		{
			if (filePath == nullptr || filePath[0] == '\0') throw NullOrEmptyPathException();
			if (!std::regex_match(filePath, FILE_PATH_REGEX)) throw InvalidFilePathException(filePath);

			m_FilePath = std::string(filePath);

			m_FileStream = std::fstream(filePath, std::fstream::app | std::fstream::out);
			if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath);
		}

		// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
		// By default auto flush is set to true
		FileLogger(const std::string& filePath, bool autoFlush = true) : Logger(autoFlush)
		{
			if (filePath.empty() || filePath == " ") throw NullOrEmptyPathException();
			if (!std::regex_match(filePath, FILE_PATH_REGEX)) throw InvalidFilePathException(filePath);

			m_FilePath = filePath;

			m_FileStream = std::fstream(filePath, std::fstream::app | std::fstream::out);
			if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath);
		}

		// Creates an instance of FileLogger which outputs to a file stream given by the "filePath" parameter
		// By default auto flush is set to true
		FileLogger(const std::string&& filePath, bool autoFlush = true) : Logger(autoFlush)
		{
			if (filePath.empty() || filePath == " ") throw NullOrEmptyPathException();
			if (!std::regex_match(filePath, FILE_PATH_REGEX)) throw InvalidFilePathException(filePath);

			m_FilePath = filePath;

			m_FileStream = std::fstream(filePath, std::fstream::app | std::fstream::out);
			if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath);
		}

		// Copy constructor
		// Creates a FileLogger instance from an already existing one
		FileLogger(const FileLogger& other)
		{
			if (other.m_FilePath.empty() || other.m_FilePath == " ") throw NullOrEmptyPathException();
			if(!std::regex_match(other.m_FilePath, FILE_PATH_REGEX)) throw InvalidFilePathException(other.m_FilePath);

			m_FilePath = other.m_FilePath;
			
			m_FileStream = std::fstream(other.m_FilePath, std::fstream::app | std::fstream::out);
			if (!m_FileStream.is_open()) throw InvalidFilePathException(m_FilePath);
		}

		// Move constructor
		// Creates a FileLogger instance from another one
		FileLogger(FileLogger&& other) noexcept
		{
			m_FilePath = std::move(other.m_FilePath);
			m_FileStream = std::fstream(other.m_FilePath, std::fstream::app | std::fstream::out);
		}

		// Destructor
		// Flush and close stream if open
		~FileLogger()
		{
			if (m_FileStream.is_open())
			{
				m_FileStream.flush();
				m_FileStream.close();
			}
		}

		// Public Methods

		// Writes to the stream a message and inserts values into placeholders (should they exist)
		template<typename ...T>
		inline void Write(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist)
		template<typename ...T>
		inline void Write(const std::string&& message, const T&& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist) and finishes with the newline character
		template<typename ...T>
		inline void WriteLine(const std::string& message, const T& ...t);

		// Writes to the stream a message and inserts values into placeholders (should they exist) and finishes with the newline character
		template<typename ...T>
		inline void WriteLine(const std::string&& message, const T&& ...t);

		// Flush file stream
		inline void Flush() override;

		// Clear log file
		inline void ClearLogs();
	};

	// Writes to the stream a message and inserts values into placeholders (should they exist)
	template<typename ...T>
	inline void FileLogger::Write(const std::string& message, const T& ...t)
	{
		std::vector<std::string> printValues{ stringConvert(t)... };
		writeToStream(replacePlaceholders(message, printValues));
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist)
	template<typename ...T>
	inline void FileLogger::Write(const std::string&& message, const T&& ...t)
	{
		std::vector<std::string> printValues{ stringConvert(t)... };
		writeToStream(replacePlaceholders(message, printValues));
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist) and finishes with the newline character
	template<typename ...T>
	inline void FileLogger::WriteLine(const std::string& message, const T& ...t)
	{
		std::vector<std::string> printValues{ stringConvert(t)... };
		writeToStream(replacePlaceholders(message, printValues) + NEWLINE);
	}

	// Writes to the stream a message and inserts values into placeholders (should they exist) and finishes with the newline character
	template<typename ...T>
	inline void FileLogger::WriteLine(const std::string&& message, const T&& ...t)
	{
		std::vector<std::string> printValues{ stringConvert(t)... };
		writeToStream(replacePlaceholders(message, printValues) + NEWLINE);
	}

	// Writes string to file stream and flush if auto flush is set
	inline void FileLogger::writeToStream(const std::string&& str)
	{
		m_FileStream << str;
		if (m_AutoFlush)
			m_FileStream.flush();
	}

	// Flush file stream
	inline void FileLogger::Flush()
	{
		m_FileStream.flush();
	}

	// Clear log file
	inline void FileLogger::ClearLogs()
	{
		std::filesystem::resize_file(m_FilePath, 0);
		m_FileStream.seekp(0);
	}
}
#endif