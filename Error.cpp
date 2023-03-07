#include "error.hpp"

void GeneralErrorHandler::Throw(unsigned where) const
{
	Throw(where, default_err_code);
}

void GeneralErrorHandler::Throw(unsigned where, GenErrCode what) const
{
	throw GeneralError(where, what, my_name());
}

void GeneralErrorHandler::Throw(GeneralError gen_err) const
{
	throw gen_err;
}

void GeneralErrorHandler::Post(unsigned where) const
{
	Post(where, default_err_code);
}

void GeneralErrorHandler::Post(unsigned where, GenErrCode what) const
{
	if (external_err_ptr != nullptr)
		external_err_ptr->GeneralError::GeneralError(where, what, my_name());
}

void GeneralErrorHandler::OnError(unsigned where)
{
	OnError(where, default_err_code);
}

void GeneralErrorHandler::OnError(unsigned where, GenErrCode what)
{
	// Default behavior - do nothing
	// If a derived class uses this member it should redefine it
}

const char* GeneralErrorHandler::my_name() const 
{ 
	return typeid(*this).name(); 
}