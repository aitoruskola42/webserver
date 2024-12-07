/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger_config.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xamayuel <xamayuel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:32:56 by xamayuel          #+#    #+#             */
/*   Updated: 2024/09/01 18:21:21 by xamayuel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_CONFIG_HPP
#define LOGGER_CONFIG_HPP

// -----------------------------------------------------------------------------
// LOG FORMAT CONFIG
// -----------------------------------------------------------------------------
// Default log file name
#define LOG_FILE "log.txt"
// Width of the function column
#define FUNCTION_WIDTH 26
// Width of the file name column
#define FILE_NAME_WIDTH 30
// Width of the PID column
#define PID_WIDTH 8
// Width of the log level column
#define LOG_LEVEL_WIDTH 10

// -----------------------------------------------------------------------------
// COLORS CONFIG
// -----------------------------------------------------------------------------
// COLORS
#define KRED "\033[0;31m"
#define KGRE "\033[0;32m"
#define KYEL "\033[0;33m"
#define LBLU "\033[0;34m"
#define KPUR "\033[0;35m"
#define KNOR "\033[0;37m"
#define KCYA "\033[0;36m"

#define KBLA "\033[0;30m"
#define KBLU "\x1B[34m"
#define SUBR "\033[0;31m\033[4m"
#define NEGR "\033[0;31m\033[1m"

// Colores adicionales
#define KORA "\033[0;33m"
#define KPIN "\033[0;35m"
#define KGRA "\033[0;90m"
#define KBRO "\033[0;33m"
#define KLIM "\033[0;92m"

// Colores brillantes
#define KBRED "\033[1;31m"
#define KBGRE "\033[1;32m"
#define KBYEL "\033[1;33m"
#define KBBLU "\033[1;34m"
#define KBPUR "\033[1;35m"
#define KBCYA "\033[1;36m"
#define KBWHI "\033[1;37m"

// Colores de fondo
#define BGRED "\033[41m"
#define BGGRE "\033[42m"
#define BGYEL "\033[43m"
#define BGBLU "\033[44m"
#define BGPUR "\033[45m"
#define BGCYA "\033[46m"
#define BGWHI "\033[47m"
#endif