/************************************************************************************************************************************************
* MOSP TESTS MANAGER																															*
/************************************************************************************************************************************************
* AUTHORS: MARCO ANTONIO MOREIRA DE CARVALHO (marco.opt@gmail.com) AND NEI YOSHIHIRO SOMA (soma@ita.br)											*
* AERONAUTICS TECHNOLOGICAL INSTITUTE - BRAZIL																									*
* DATE OF CREATION: 01/06/2008																													*
* LAST MODIFICATION: 02/06/2019																													*
/************************************************************************************************************************************************
* TERMS OF USE																																	*
*																																				*
* This code available for free public use.																										*
* The code in this archive is licensed gratis to all third parties under the terms of this paragraph.											*
* Copying and distribution of the files in this archive is unrestricted if and only if the files are not modified.								*
* Modification of the files is encouraged, but the distribution of modifications of the files in this archive is unrestricted only				*
* if you meet the following conditions: modified files must carry a prominent notice stating (i) the author and date,							*
* (ii) the new author and the date of release of the modification, (iii) that the work is licensed at no charge to all parties.					*
*																																				*
* If you use the code extensively in your research, you are requested to provide appropriate attribution and thanks to the author of the code.  *
/************************************************************************************************************************************************
* DISCLAIMER OF WARRANTY																														*
*																																				*
* This source code is provided "as is" and without warranties as to performance or merchantability.												*
* The authors and/or distributors of this source code may have made statements about this source code.											*
* Any such statements do not constitute warranties and shall not be relied on by the user in deciding whether to use this source code.			*
*																																				*
* This source code is provided without any express or implied warranties whatsoever. Because of the diversity of conditions and hardware under	*
*  which this source code may be used, no warranty of fitness for a particular purpose is offered.												*
* The user is advised to test the source code thoroughly before relying on it. The user must assume the entire risk of using the source code.	*
*																																				*
************************************************************************************************************************************************/

#include <cstdio>
using namespace std;
#include "Lookahead.hpp"						//includes the heuristic

int main()
{
	int LookaheadSolutionValue;					//stores the result of the constructive heuristic
	int i = 0;

	double LookaheadRunningTime;				//stores the time of the constructive heuristic
	double LookaheadSolutionValueAccumulated = 0;

	char inputFileName[256];					//stores the name of the input file

	FILE *fpOut;								//output file
	//FILE *fpIndex = fopen("indice_GRIDS.txt", "r");
	//FILE *fpIndex = fopen("indice_SMALL.txt", "r");
	//FILE *fpIndex = fopen("indice_GMPLAN.txt", "r");
	//FILE *fpIndex = fopen("indice_SCOOP.txt", "r");
	//FILE *fpIndex = fopen("indice_CHALLENGE.txt", "r");
	//FILE *fpIndex = fopen("indice_FAGGIOLIBENTIVOGLIO.txt", "r");
	//FILE *fpIndex = fopen("indice_chustuckey.txt", "r");
	//FILE *fpIndex = fopen("indice_lorena.txt", "r");
	FILE *fpIndex = fopen("index.txt", "r");

	printf("/***************************************************************************\n");
	printf("* LOOKAHEAD HEURISTIC FOR THE MINIMIZATION OF OPEN STACKS PROBLEM  *\n");
	printf("/***************************************************************************\n");
	printf("* IMPLEMENTATION: MARCO ANTONIO MOREIRA DE CARVALHO (marco.opt@gmail.com)\n");
	printf("* AND NEI YOSHIHIRO SOMA (soma@ita.br)\n");
	printf("* AERONAUTICS TECHNOLOGICAL INSTITUTE - BRAZIL\n");
	printf("* DATE OF CREATION: 13/02/2014\n");
	printf("* LAST MODIFICATION: 23/04/2015\n");
	printf("/********************************************************************\n");
	printf("* TERMS OF USE\n");
	printf("*\n");
	printf("* This code available for free public use.\n");
	printf("* The code in this archive is licensed gratis to all third parties\n");
	printf("* under the terms of this paragraph.\n");
	printf("* Copying and distribution of the files in this archive is\n");
	printf("* unrestricted if and only if the files are not modified.\n");
	printf("* Modification of the files is encouraged, but the distribution of\n");
	printf("* modifications of the files in this archive is unrestricted only\n");
	printf("* if you meet the following conditions: modified files must carry a\n");
	printf("* prominent notice stating\n");
	printf("* (i) the author and date,\n");
	printf("* (ii) the new author and the date of release of the modification,\n");
	printf("* (iii) that the work is licensed at no charge to all parties.\n");
	printf("*\n");
	printf("* If you use the code extensively in your research, you are\n");
	printf("* requested to provide appropriate attribution and thanks to the\n");
	printf("* author of the code.\n");
	printf("********************************************************************/\n");

	printf("* DISCLAIMER OF WARRANTY\n");
	printf("*\n");
	printf("* This source code is provided \"as is\" and without warranties as to\n");
	printf("* performance or merchantability.\n");
	printf("* The authors and/or distributors of this source code may have made\n");
	printf("* statements about this source code.\n");
	printf("* Any such statements do not constitute warranties and shall not be\n");
	printf("* relied on by the user in deciding whether to use this source code.\n");
	printf("*\n");
	printf("* This source code is provided without any express or implied warranties\n");
	printf("* whatsoever. Because of the diversity of conditions and hardware under\n");
	printf("* which this source code may be used, no warranty of fitness for a\n");
	printf("* particular purpose is offered.\n");
	printf("* The user is advised to test the source code thoroughly before relying\n");
	printf("* on it. The user must assume the entire risk of using the source code.\n");
	printf("*\n");
	printf("********************************************************************/\n");

	fpOut = fopen("RESULTS_SUMMARY.txt", "w");

	while(fscanf(fpIndex, "%s", inputFileName) != -1)
	{
		i++;
		//mainMethod(&LookaheadSolutionValue, &LookaheadRunningTime, inputFileName);
		localSearch(&LookaheadSolutionValue, &LookaheadRunningTime, inputFileName, i-1);
		printf("PROBLEM %d: %s %d\n", i, inputFileName, LookaheadSolutionValue);
		fprintf(fpOut, "%s %d %.10f\n", inputFileName, LookaheadSolutionValue, LookaheadRunningTime);
		LookaheadSolutionValueAccumulated += LookaheadSolutionValue;
	}

	fclose(fpIndex);
	fclose(fpOut);
}
