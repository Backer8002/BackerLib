#include "Regex.h"
#include <BackerLibTypes.h>
#include <BackerLibTextprocessing.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_AMOUNT_OF_NODES (SIZE_MAX-2)
#define INVALID_END_NODE (SIZE_MAX-1)
#define VALID_END_NODE (SIZE_MAX-2)

struct TransitionConstraint {
	BL_Unicodepoint beginRange;
	BL_Unicodepoint endRange;
	size_t transition;
};

struct DFANode {
	union {
		BL_DataTypeFlags flags;
		BL_Container constaints;
		struct {
			BL_DataTypeFlags flags;
			struct TransitionConstraint constraint;
		} constraint;
	};
	size_t defualtTransition;
};

enum RegexASTOperation {
	RegexOpConcat,
	RegexOpUnion,
	RegexOpRepeat,
	RegexOpUnionRange,
	RegexOpOptional
};

struct RegexASTNode;

struct RegexASTOp {
	bool isAtom;
	union {
		struct RegexASTNode* node;
		BL_Unicodepoint codepoint;
	};
};

struct RegexASTNode {
	enum RegexASTOperation operation;
	struct RegexASTOp firstOp;
	union {
		BL_DynamicContainer ranges;
		struct RegexASTOp secondOp;
	};
};

static BL_DynamicContainer internal_make_regex_ast(BL_UnicodeView expr) {
	
}


BL_Regex bl_regex_create(BL_UnicodeView expr) {
	BL_DynamicContainer tokens = internal_make_regex_ast(expr);
	if (!bl_container_dynamic_is_valid(&tokens))
		return (BL_Regex){0};


	BL_Regex regex = {.dfaNodes = bl_container_dynamic_create_stack(0, sizeof(struct DFANode))};

	return regex;
}

