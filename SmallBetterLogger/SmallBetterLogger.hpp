#ifndef SMALL_BETTER_LOGGER_H
#define SMALL_BETTER_LOGGER_H

// Used for writing to output stream
#include <iostream>
#include <fstream>

// Used for formatting and creating the output string
#include <sstream>
#include <vector>
#include <regex>

namespace sbl
{
	// Logger declaration
	class Logger;
	using logger = Logger;

	// Stream types to be used by a Logger instance
	enum class STREAM_TYPE
	{
		STDOUT, STDERR, STDLOG, FILE
	};
	using stream_type = STREAM_TYPE;

	// Logger class: The main part of the library, used to log messages to a stream (ex.: STDOUT, STDERR, file etc.)
	class Logger
	{
	public:
		// Creates an instance of Logger which outputs to a stream chosen from a STREAM_TYPE (except STREAM_TYPE::FILE)
		// Defaults to STREAM_TYPE::STDOUT
		Logger(STREAM_TYPE type = STREAM_TYPE::STDOUT)
			: m_StreamType(type)
		{
			if (m_StreamType == STREAM_TYPE::FILE) throw std::exception("Cannot make logger with STREAM_TYPE::FILE with no file provided.");
		}

		// Creates an instance of Logger which outputs to a stream chosen from a STREAM_TYPE (except STREAM_TYPE::FILE)
		Logger(const char* filepath)
			: m_StreamType(STREAM_TYPE::FILE)
		{
			if (!filepath || strlen(filepath) < 1 || *filepath == 0) throw std::exception("No file path provided. Cannot create output stream.");

			m_FileStream.open(filepath);
			if (!m_FileStream.is_open()) throw std::exception("Cannot open file to output to.");
		}

		// Creates an instance of Logger which outputs to a stream chosen from a STREAM_TYPE (except STREAM_TYPE::FILE)
		Logger(const std::string& filepath)
			: m_StreamType(STREAM_TYPE::FILE)
		{
			if (filepath.empty() 
				|| filepath.size() < 1 
				|| std::regex_match(filepath, std::regex(R"(^[ ]+$/g)"))) 
				throw std::exception("No file path provided. Cannot create output stream.");

			m_FileStream.open(filepath);
			if (!m_FileStream.is_open()) throw std::exception("Cannot open file to output to.");
		}

		// Close file stream if open
		~Logger()
		{
			if (m_FileStream.is_open())
				m_FileStream.close();
		}

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

	private:
		STREAM_TYPE m_StreamType;
		std::fstream m_FileStream;

		// Converts a T value to a string to be used in writing a log
		template<typename T>
		inline std::string stringConvert(const T& t);

		// Replace all "{n}" placeholders with their respective values (n=0,...)
		inline std::string replacePlaceholders(std::string message, std::vector<std::string>& items);

		// Writes string to appropriate stream based on instance STREAM_TYPE (m_StreamType)
		inline void writeToStream(const std::string&& str);
	};

	template<typename ...T>
	inline void Logger::Write(const std::string& message, const T& ...t)
	{
		std::vector<std::string> printValues { stringConvert(t)... };
		writeToStream(replacePlaceholders(message, printValues));
	}

	template<typename ...T>
	inline void Logger::Write(const std::string&& message, const T&& ...t)
	{
		std::vector<std::string> printValues{ stringConvert(t)... };
		writeToStream(replacePlaceholders(message, printValues));
	}

	template<typename ...T>
	inline void Logger::WriteLine(const std::string& message, const T& ...t)
	{
		std::vector<std::string> printValues{ stringConvert(t)... };
		writeToStream(replacePlaceholders(message, printValues) + "\n");
	}

	template<typename ...T>
	inline void Logger::WriteLine(const std::string&& message, const T&& ...t)
	{
		std::vector<std::string> printValues{ stringConvert(t)... };
		writeToStream(replacePlaceholders(message, printValues) + "\n");
	}

	template<typename T>
	inline std::string Logger::stringConvert(const T& t)
	{
		std::stringstream ss;
		ss << t;
		return ss.str();
	}

	inline std::string sbl::Logger::replacePlaceholders(std::string message, std::vector<std::string>& items)
	{
		std::regex placeholder;

		for (int i = 0; i < items.size(); i++)
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
		case STREAM_TYPE::STDERR:   std::cerr << str;       break;
		case STREAM_TYPE::STDLOG:   std::clog << str;       break;      
		case STREAM_TYPE::FILE:     m_FileStream << str;    break;
		default:                    std::cout << str;       break;
		}
	}
}
#endif