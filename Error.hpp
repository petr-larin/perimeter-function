#pragma once
#include <typeinfo>

// Base class for error reporting

struct GeneralError;

class GeneralErrorHandler {
public:

	// Error codes

	enum GenErrCode {

		OK = 0,
		Unknown, // for debug
		
		// Fatal system errors
		NoCommCtl, ScrnRes, GDI, System, OutOfMem,

		// Non-fatal system errors
		FileOpen, FileClose, FileWrite, NoRichEdit, NoHelpFile,
		
		// Non-fatal application errors
		AppWrongFileFormat,
		AppNotEnoughVertices,
		AppOutOfRangeFile,
		AppOutOfRangeW,
		AppOutOfRangeR
	};

protected:

	explicit GeneralErrorHandler(GenErrCode, GeneralError* = nullptr);
	GeneralErrorHandler() = delete;

	virtual ~GeneralErrorHandler() = default;

	// On error, throw an exception (mostly for fatal errors)
	virtual void Throw(unsigned where) const; // with default error code
	virtual void Throw(unsigned where, GenErrCode what) const;
	virtual void Throw(GeneralError gen_err) const;

	// On error, post info about it to an external pointer
	virtual void Post(unsigned where) const; // with default error code
	virtual void Post(unsigned where, GenErrCode what) const;

	// On error, do something defined in a derived class
	// By default - do nothing
	virtual void OnError(unsigned where); // with default error code
	virtual void OnError(unsigned where, GenErrCode what);

	// Deduce the type name where the error happened
	virtual const char* my_name() const;

	// Default error code
	const GenErrCode default_err_code;

	// External pointer for Post, can be nullptr in which case - no posting
	GeneralError* const external_err_ptr;
};

struct GeneralError {

	const unsigned where;
	const GeneralErrorHandler::GenErrCode what;
	const char* class_name;

	GeneralError() = delete;
	GeneralError(
		unsigned where,
		GeneralErrorHandler::GenErrCode what,
		const char* class_name)
			
		: where(where), what(what), class_name(class_name) {}
};

inline GeneralErrorHandler::GeneralErrorHandler(
	GenErrCode default_err_code, GeneralError* external_err_ptr)
	: external_err_ptr{ external_err_ptr }, default_err_code{ default_err_code }
		{
			if (external_err_ptr != nullptr)
				external_err_ptr->GeneralError::GeneralError(0, OK, nullptr);
		}