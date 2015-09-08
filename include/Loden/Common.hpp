#ifndef LODEN_COMMON_HPP_
#define LODEN_COMMON_HPP_

#include <memory>

#ifdef _WIN32
#define LODEN_EXPORT_SYMBOL __declspec(dllexport)
#define LODEN_IMPORT_SYMBOL __declspec(dllexport)
#else
#define LODEN_EXPORT_SYMBOL __attribute__ ((visibility ("default")))
#define LODEN_IMPORT_SYMBOL __attribute__ ((visibility ("default")))
#endif

#ifdef BUILDING_LODEN_CORE
#define LODEN_CORE_EXPORT LODEN_EXPORT_SYMBOL
#else
#define LODEN_CORE_EXPORT LODEN_IMPORT_SYMBOL
#endif

#define LODEN_EXTERN_C extern "C"

#define LODEN_DECLARE_SMART_POINTERS(className) \
	typedef std::shared_ptr<className> className ## Ptr; \
	typedef std::weak_ptr<className> className ##WeakPtr;
	
#define LODEN_DECLARE_CLASS(className) \
	class className; \
	LODEN_DECLARE_SMART_POINTERS(className)
	
#endif //LODEN_COMMON_HPP_
