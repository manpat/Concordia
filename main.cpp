#include "bluebear.hpp"
#include <iostream>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <squirrel.h>

int main() {
	HSQUIRRELVM sqvm = sq_open(INITIAL_SQVM_STACK_SIZE);

	sq_setprintfunc(sqvm, SquirrelExamples::squirrel_print_function, NULL);
	const SQChar *program = "print(\"Hello World!\\n\");";

	if (
		SQ_FAILED(
			sq_compilebuffer(
				sqvm,
				program,
				sizeof(SQChar) * strlen(program),
				"program",
				SQFalse
			)
		)
	) {
        SquirrelExamples::squirrel_print_last_error(sqvm);
        return 1;
    }

	sq_pushroottable(sqvm);
	if (
		SQ_FAILED(
			sq_call(sqvm, 1, SQFalse, SQFalse)
		)
	) {
		 SquirrelExamples::squirrel_print_last_error(sqvm);
		 return 1;
	}

	sq_close(sqvm);
	return 0;
}
