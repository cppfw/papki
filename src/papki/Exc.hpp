#pragma once

#include <utki/config.hpp>
#include <utki/Exc.hpp>



namespace papki{

/**
 * @brief Basic exception class.
 */
class Exc : public utki::Exc{
public:
	/**
	 * @brief Constructor.
	 * @param descr - human readable description of the error.
	 */
	Exc(const std::string& descr) :
			utki::Exc(std::string("[papki::Exc]: ") + descr)
	{}
};

/**
 * @brief Illegal state exception.
 * This exception is usually thrown when trying to perform some operation on the
 * object while the object is in inappropriate state for that operation. For example,
 * when trying to read from file while it is not opened.
 */
class IllegalStateExc : public Exc{
public:
	/**
	 * @brief Constructor.
	 * @param descr - human readable description of the error.
	 */
	IllegalStateExc(const std::string& descr = "Illegal opened/closed state") :
			Exc(descr)
	{}
};

}