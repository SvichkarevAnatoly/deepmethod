#include <Rinternals.h>
#include <R_ext/Parse.h>
//#include <Rembedded.h>

int main(int argc, char **argv) {
	SEXP x;
	ParseStatus status;
	const char* expr = "2 + 2";

	char *localArgs[] = {"R", "--no-save","--silent"};
	Rf_initEmbeddedR(3, localArgs);

	x = R_ParseVector(mkString(expr), 1, &status, R_NilValue);
	if (TYPEOF(x) == EXPRSXP) {
        /* parse returns an expr vector, you want the first */
		x = eval(VECTOR_ELT(x, 0), R_GlobalEnv);
		PrintValue(x);
        printf("%f\n",REAL(x)[0]);
	}

	Rf_endEmbeddedR(0);

	return 0;
}
