/**
 * @file logger.h
 * Definition of a simplistic logging interface.
 */
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <chrono>
#include <functional>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

namespace meta {

/**
 * Namespace which contains all of the logging interface classes.
 */
namespace logging {

/**
 * logger: Main logging class. Keeps track of a list of sinks to write
 * lines to---these can be of any std::ostream-derived type.
 */
class logger {
    public:
        /**
         * severity_level: A demarcation of how severe a given message
         * is. Can be used to filter out messages below a certain
         * threshold at the sink-specific level.
         */
        enum class severity_level {
            progress,
            trace,
            debug,
            info,
            warning,
            error,
            fatal
        };

        /**
         * Determines the string form of a given severity_level.
         *
         * @param sev The severity level to get a string for.
         */
        static std::string severity_string( severity_level sev ) {
            switch( sev ) {
                case severity_level::progress:
                    return "progress";
                case severity_level::trace:
                    return "trace";
                case severity_level::debug:
                    return "debug";
                case severity_level::info:
                    return "info";
                case severity_level::warning:
                    return "warning";
                case severity_level::error:
                    return "error";
                case severity_level::fatal:
                    return "fatal";
                default:
                    return "unknown";
            }
        }

        /**
         * log_line: Represents a single message to be written to all
         * sinks.
         */
        class log_line {
            public:
                /**
                 * Constructs a new log line for the given logger.
                 *
                 * @param log The logger this message was created for.
                 * @param sev The severity of this message.
                 * @param line The line number for this message.
                 * @param file The file for this message.
                 */
                log_line( logger & log, severity_level sev,
                        size_t line,
                        const std::string & file )
                    : _log( log ), _sev( sev ), _line( line ),
                        _file( file ) {
                    // nothing
                }

                /**
                 * Simulates a std::endl, but for log entries. Flushes
                 * all internal streams and then writes the log_line to
                 * all sinks of the log object it was created with.
                 */
                static log_line & endlg( log_line & logline ) {
                    logline._stream.flush();
                    logline.write_to_sinks();
                    return logline;
                }

                /**
                 * Overloaded operator<<(), used to determine when a
                 * manipulator is sent to the log_line stream.
                 *
                 * @param fn A function pointer sent to the stream.
                 */
                log_line & operator<<( log_line & ( *fn )( log_line & ) ) {
                    return ( *fn )( *this );
                }

                /**
                 * Generic operator<<(). Simply writes the given data
                 * to the internal stream.
                 *
                 * @param to_write The data to be written.
                 */
                template <class T>
                log_line & operator<<( const T & to_write ) {
                    _stream << to_write;
                    return *this;
                }

                /**
                 * Writes the current log line to all sinks of the
                 * logger it was created with.
                 */
                void write_to_sinks() {
                    _log.write_to_sinks( *this );
                }

                /**
                 * Converts the internal stream to a string.
                 */
                std::string str() const {
                    return _stream.str();
                }

                /**
                 * Gets the severity of this log_line.
                 */
                severity_level severity() const {
                    return _sev;
                }

                /**
                 * Gets the file for this log_line.
                 */
                const std::string & file() const {
                    return _file;
                }

                /**
                 * Gets the line number for this log_line.
                 */
                size_t line() const {
                    return _line;
                }


            private:
                /**
                 * Internal stream.
                 */
                std::stringstream _stream;

                /**
                 * The logger this log_line is to be written on.
                 */
                logger & _log;

                /**
                 * The severity of this message.
                 */
                severity_level _sev;

                /**
                 * The line number for this message.
                 */
                size_t _line;

                /**
                 * The file for this message.
                 */
                std::string _file;
        };

        /**
         * sink: A wrapper for a stream that a logger should write to.
         */
        class sink {
            public:
                typedef std::function<std::string( const log_line & )> formatter_func;
                typedef std::function<bool( const log_line & )> filter_func;

                /**
                 * Creates a new sink with the given formatting function
                 * and filtering function. A filtering function should take
                 * a log_line by const-reference and determine if it should
                 * or should not be written to the stream. If a formatting
                 * function is not provided, use a sane default formatter.
                 *
                 * @param stream The stream this sink will write to.
                 * @param formatter The formatting function object to
                 *  use to format the log_lines written to the stream.
                 * @param filter The filtering function used to
                 *  determine if a given log_line should be written to
                 *  the stream or not.
                 */
                sink( std::ostream & stream,
                      const filter_func & filter =
                        []( const log_line & ) { return true; },
                      const formatter_func & formatter = &default_formatter )
                    : _stream( stream), _formatter( formatter ),
                        _filter( filter ) {
                    // nothing
                }

                /**
                 * Creates a new sink on the given stream, filtering out
                 * all results that are greater than or equal to the
                 * specified severity, using the provided formatting
                 * function. If no formatting function is provided, use a
                 * sane default.
                 *
                 * @param stream The stream this sink will write to.
                 * @param sev The severity level at or above which log
                 *  lines will be kept.
                 * @param formatter The optional formatting function.
                 */
                sink( std::ostream & stream,
                      logger::severity_level sev,
                      const formatter_func & formatter = &default_formatter )
                    : _stream( stream ), _formatter( formatter ),
                      _filter( [sev](const log_line & ll) {
                          return ll.severity() >= sev;
                      } ) {
                    // nothing
                }

                /**
                 * Writes the given log_line to the stream, formatting
                 * and filtering it as necessary.
                 *
                 * @param line The log_line to be written.
                 */
                void write( const log_line & line ) {
                    if( _filter )
                        if( !_filter( line ) )
                            return;
                    if( _formatter )
                        _stream << _formatter( line );
                    else
                        _stream << line.str();
                    _stream << std::flush;
                }

                /**
                 * The default formatting function.
                 *
                 * @param line The log_line to format.
                 * @return A string representation of the log_line
                 *  suitable for writing to a std::ostream.
                 */
                static std::string default_formatter( const log_line & line ) {
                    namespace sc = std::chrono;

                    auto time = sc::system_clock::now();
                    auto since_epoch = time.time_since_epoch();
                    auto unix_time = sc::duration_cast<sc::seconds>(since_epoch);

                    std::stringstream ss;
                    ss << unix_time.count();
                    ss << ": ";

                    std::stringstream sev;
                    sev << "[" << severity_string( line.severity() ) << "]";
                    ss << std::setw( 10 ) << std::left << sev.str();

                    ss << " ";
                    ss << line.str();
                    ss << " ";

                    ss << "(" << line.file() << ":" << line.line() << ")";
                    ss << std::endl;
                    return ss.str();
                }

            private:
                /**
                 * Internal stream.
                 */
                std::ostream & _stream;

                /**
                 * The formatting functor.
                 */
                formatter_func _formatter;

                /**
                 * The filtering functor.
                 */
                filter_func _filter;
        };

        /**
         * Adds a sink to the given logger.
         *
         * @param s The sink to add.
         */
        void add_sink( const sink & s ) {
            _sinks.push_back( s );
        }

        /**
         * Adds a sink to the given logger.
         *
         * @param s The sink to add.
         */
        void add_sink( sink && s ) {
            _sinks.emplace_back( std::move( s ) );
        }

        /**
         * Writes the given log_line to all sinks.
         *
         * @param line The log_line to write.
         */
        void write_to_sinks( const log_line & line ) {
            for( sink & s : _sinks )
                s.write( line );
        }
    private:
        /**
         * The list of sinks to write to.
         */
        std::vector<sink> _sinks;
};

/**
 * Gets a static instance of a logger.
 */
inline logger & get_logger() {
    static logger log;
    return log;
}

/**
 * Adds a sink to a static instance of a logger.
 */
inline void add_sink( const logger::sink & s ) {
    get_logger().add_sink( s );
}

/**
 * Adds a sink to a static instance of a logger.
 */
inline void add_sink( logger::sink && s ) {
    get_logger().add_sink( std::move( s ) );
}

}
}

#define LOG( sev ) logging::logger::log_line( logging::get_logger(), logging::logger::severity_level::sev, __LINE__, __FILE__ )
#define ENDLG logging::logger::log_line::endlg
#define LOG_FUNCTION_START() LOG( trace ) << "entering " << __func__ << "()" << ENDLG
#endif
