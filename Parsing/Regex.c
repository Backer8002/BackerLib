#include "Regex.h"
#include <BackerLibTypes.h>
#include <stdint.h>
#include <stddef.h>

#define MAX_AMOUNT_OF_NODES (SIZE_MAX >> 2)
#define END_NODE_FLAG (0x1 << (SIZE_WIDTH - 1))
#define VALID_END_FLAG (0x1 << (SIZE_WIDTH - 2))

struct DFANode {
	BL_Hashmap transitions;
	size_t defualtTransition;
};

BL_DynamicContainer internal_tokenize_regex(BL_UnicodeView expr) {

}

BL_Regex bl_regex_create(BL_UnicodeView expr) {
	BL_DynamicContainer tokens = internal_tokenize_regex(expr);
	if (!bl_container_dynamic_is_valid(&tokens))
		return (BL_Regex){0};


	BL_Regex regex = {bl_container_dynamic_create_stack(0, sizeof(struct DFANode))};

	return regex;
}

