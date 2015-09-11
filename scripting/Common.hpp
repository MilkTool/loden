#ifndef LODTALK_COMMON_HPP
#define LODTALK_COMMON_HPP

#define LODTALK_UNIMPLEMENTED() \
	fprintf(stderr, "The method %s is unimplmented in %s at line %d\n", __PRETTY_FUNCTION__, __FILE__, __LINE__); \
	abort();

#endif // LODTALK_COMMON_HPP
