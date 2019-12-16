#pragma once

#include <utki/config.hpp>
#include <utki/exception.hpp>



namespace papki{

/**
 * @brief Basic exception class.
 */
class exception : public utki::exception{
public:
	/**
	 * @brief Constructor.
	 * @param descr - human readable description of the error.
	 */
	exception(const std::string& descr) :
			utki::exception(std::string("[papki::exception]: ") + descr)
	{}
};

}