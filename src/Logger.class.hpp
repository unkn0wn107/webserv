/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.class.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 13:21:59 by mchenava          #+#    #+#             */
/*   Updated: 2024/04/24 17:16:22 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_CLASS_HPP
#define LOGGER_CLASS_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

#define LOG_FILE_PATH "/home/mchenava/log/"
#define LOG_FILE_NAME "webserv"

class Logger {
public:

	static Logger& getInstance() {
		static Logger instance;
		return instance;
	}

	void info(const std::string& message) const;
	void warning(const std::string& message) const;
	void error(const std::string& message) const;
	void setLogFileName(const std::string& logFileName);

private :
	Logger();
	~Logger();

	std::ofstream* _logFile;
	std::string _logFileName;

	std::string _getCurrentTime() const;
	std::string _generateLogFileName();
};

#endif /* LOGGER_CLASS_HPP */
