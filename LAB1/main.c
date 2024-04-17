#include "return_codes.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Numbers
{
	int8_t sign;
	int8_t flagInf;
	int8_t flagOutBit;
	int8_t lastOutBit;
	int16_t exponent;
	uint64_t mantissa;
} Number;

Number normals(Number Result, int8_t op, int8_t mant)
{
	int8_t cnt = 0;
	while (Result.mantissa >> mant >= 2)
	{
		Result.lastOutBit = Result.mantissa & 1;
		if (Result.lastOutBit == 1)
		{
			cnt++;
		}
		Result.exponent = Result.exponent + 1 * op;
		Result.mantissa >>= 1;
	}
	while (Result.mantissa >> mant < 1 && Result.mantissa != 0)
	{
		Result.lastOutBit = Result.mantissa % (1 << mant);
		if (Result.lastOutBit == 1)
		{
			cnt++;
		}
		Result.exponent = Result.exponent - 1 * op;
		Result.mantissa <<= 1;
	}
	Result.flagOutBit = cnt - Result.lastOutBit > 0 ? 1 : 0;
	Result.flagInf = cnt != 0 ? 1 : 0;
	return Result;
}

Number createNumber(uint64_t argument, int8_t exp, int8_t mant)
{
	Number Result;
	Result.mantissa = argument % (1 << mant);
	argument >>= mant;
	Result.exponent = (argument % (1 << exp));
	Result.mantissa = Result.exponent != 0 ? Result.mantissa + (1 << mant) : Result.mantissa;
	Result.exponent = Result.exponent == 0 ? 2 : Result.exponent + 1;
	Result.exponent -= (1 << (exp - 1));
	Result.flagInf = Result.flagOutBit = Result.lastOutBit = 0;
	Result = normals(Result, 1, mant);
	Result.sign = (argument >>= exp);
	return Result;
}

void printInf(int8_t sign)
{
	sign == 1 ? printf("-inf\n") : printf("inf\n");
}

void printNull(int8_t sign, int8_t mant)
{
	sign == 1 ? putchar('-') : sign;
	mant == 10 ? printf("0x0.000p+0\n") : printf("0x0.000000p+0\n");
}

Number getFlag(Number Result, Number *Number1, Number *Number2)
{
	Result.flagInf = Number1->flagInf | Number2->flagInf;
	Result.lastOutBit = Number1->lastOutBit | Number2->lastOutBit;
	Result.flagOutBit = Number1->flagOutBit | Number2->flagOutBit;
	return Result;
}

int8_t checkInfNaN(Number *Result, int8_t mant)
{
	if (Result->exponent >= (mant == 10 ? 16 : 128))
	{
		return Result->mantissa % (1 << mant) == 0 ? Result->sign == 0 ? 1 : -1 : 0;
	}
	return 3;
}

int8_t checkNull(Number *Result, int8_t mant)
{
	if (checkInfNaN(Result, mant) == 3 && Result->mantissa == 0)
	{
		return Result->sign == 1 ? -1 : 1;
	}
	return 0;
}

void roundings(Number *Numbers, int8_t rounding)
{
	if (rounding == 1 && Numbers->lastOutBit == 1)
	{
		if (Numbers->flagOutBit == 1)
		{
			Numbers->mantissa++;
		}
		else
		{
			Numbers->mantissa += Numbers->mantissa & 1;
		}
	}
	else if (Numbers->flagInf && (rounding == 2 && Numbers->sign == 0 || rounding == 3 && Numbers->sign == 1))
	{
		Numbers->mantissa++;
	}
}

Number checkEscape(Number Result, Number *Numbers, int16_t cnt)
{
	int8_t count = 0;
	for (int8_t i = 0; i < cnt; i++)
	{
		Result.lastOutBit = Numbers->mantissa & 1;
		if (Result.lastOutBit == 1)
		{
			Result.flagInf = 1;
			count++;
		}
		Numbers->mantissa >>= 1;
	}
	Result.flagOutBit = count - Result.lastOutBit > 0 ? 1 : 0;
	return Result;
}

Number subtract(Number *Number1, Number *Number2, int8_t mant);

Number add(Number *Number1, Number *Number2, int8_t mant)
{
	Number Result;
	if (abs(checkInfNaN(Number1, mant)) == 1 || abs(checkInfNaN(Number2, mant)) == 1)
	{
		if (abs(checkInfNaN(Number1, mant)) == 1 && abs(checkInfNaN(Number2, mant)) == 1 && Number1->sign != Number2->sign)
		{
			printf("nan\n");
		}
		else
		{
			abs(checkInfNaN(Number1, mant)) == 1 ? printInf(Number1->sign) : printInf(Number2->sign);
		}
		Result.sign = 50;
		return Result;
	}
	else if (abs(checkNull(Number1, mant)) == 1 || abs(checkNull(Number2, mant)) == 1)
	{
		if (abs(checkNull(Number1, mant)) == 1 && abs(checkNull(Number2, mant)) == 1)
		{
			printNull(checkNull(Number1, mant) == -1 && checkNull(Number2, mant) == -1, mant);
			Result.sign = 50;
		}
		else
		{
			Result = abs(checkNull(Number1, mant)) == 1 ? *Number2 : *Number1;
		}
		return Result;
	}
	if (Number1->sign == Number2->sign && Number1->sign == 1)
	{
		Number1->sign = Number2->sign = 0;
		Result = add(Number1, Number2, mant);
		Result.sign = 1;
		return Result;
	}
	else if (Number1->sign != Number2->sign)
	{
		if (Number1->sign == 1)
		{
			Number1->sign = 0;
			return subtract(Number2, Number1, mant);
		}
		else
		{
			Number2->sign = 0;
			return subtract(Number1, Number2, mant);
		}
	}
	Number *Buff1 = Number1->exponent >= Number2->exponent ? Number1 : Number2;
	Number *Buff2 = Number1->exponent >= Number2->exponent ? Number2 : Number1;
	Result = getFlag(Result, Number1, Number2);
	if (Buff1->exponent < 38 + Buff2->exponent)
	{
		Result.exponent = Buff2->exponent;
		Result.mantissa = Buff2->mantissa + (Buff1->mantissa << (Buff1->exponent - Buff2->exponent));
	}
	else
	{
		Result.exponent = Buff1->exponent;
		Result.mantissa = Buff1->mantissa + (Buff2->mantissa >> (Buff1->exponent - Buff2->exponent));
		Result = checkEscape(Result, Buff2, Buff1->exponent - Buff2->exponent);
	}
	Result.sign = Buff1->sign;
	Result = normals(Result, 1, mant);
	return Result;
}

Number subtract(Number *Number1, Number *Number2, int8_t mant)
{
	Number Result;
	Result.sign = 0;
	if (abs(checkInfNaN(Number1, mant)) == 1 || abs(checkInfNaN(Number2, mant)) == 1)
	{
		int8_t inf1 = checkInfNaN(Number1, mant);
		int8_t inf2 = checkInfNaN(Number2, mant);
		if (inf1 == inf2)
		{
			printf("nan\n");
		}
		else if (abs(inf1) == 1)
		{
			printInf(Number1->sign);
		}
		else
		{
			printInf(Number2->sign != 1);
		}
		Result.sign = 50;
		return Result;
	}
	else if (abs(checkNull(Number1, mant)) == 1 || abs(checkNull(Number2, mant)) == 1)
	{
		if (abs(checkNull(Number1, mant)) == 1 && abs(checkNull(Number2, mant)) == 1)
		{
			printNull(Number1->sign == 1 && Number2->sign == 0, mant);
			Result.sign = 50;
		}
		else
		{
			if (abs(checkNull(Number1, mant)) == 1)
			{
				Result = *Number2;
				Result.sign = Number2->sign == 0;
			}
			else
			{
				Result = *Number1;
			}
		}
		return Result;
	}
	Result = getFlag(Result, Number1, Number2);
	if (Number1->sign == Number2->sign && Number1->sign == 1)
	{
		Number1->sign = Number2->sign = 0;
		return subtract(Number2, Number1, mant);
	}
	if (Number1->sign != Number2->sign)
	{
		int8_t buffSign = Number1->sign;
		Number1->sign = Number2->sign = 0;
		Result = add(Number1, Number2, mant);
		Result.sign = buffSign;
		return Result;
	}
	if (Number1->exponent < Number2->exponent)
	{
		Result = subtract(Number2, Number1, mant);
		Result.sign = (Result.sign + 1) % 2;
		return Result;
	}
	Result.exponent = Number1->exponent < 39 + Number2->exponent ? Number2->exponent : Number1->exponent;
	uint64_t newMant1, newMant2;
	int8_t flag = 1;
	if (Number1->exponent < 39 + Number2->exponent)
	{
		newMant2 = Number2->mantissa;
		newMant1 = (Number1->mantissa << (Number1->exponent - Number2->exponent));
	}
	else
	{
		newMant2 = Number1->mantissa;
		newMant1 = (Number2->mantissa >> (Number1->exponent - Number2->exponent));
		if (Number2->mantissa != 0 && newMant1 == 0)
		{
			Result.mantissa = Number1->mantissa - 1;
			Result.flagInf = Result.flagOutBit = Result.lastOutBit = 1;
			Result.sign = flag = 0;
		}
		else
		{
			Result = checkEscape(Result, Number2, Number1->exponent - Number2->exponent);
		}
	}
	if (flag)
	{
		Result.mantissa = newMant2 >= newMant1 ? newMant2 - newMant1 : newMant1 - newMant2;
		Result.sign = newMant2 >= newMant1 ? 1 : 0;
	}
	if (Result.flagInf)
	{
		Result.mantissa--;
	}
	Result = normals(Result, 1, mant);
	return Result;
}

Number multiply(Number *Number1, Number *Number2, int8_t mant)
{
	Number Result = *Number1;
	if (abs(checkInfNaN(Number1, mant)) == 1 || abs(checkInfNaN(Number2, mant)) == 1)
	{
		if (abs(checkNull(Number1, mant)) == 1 || abs(checkNull(Number2, mant)) == 1)
		{
			printf("nan\n");
		}
		else
		{
			printInf(Number1->sign != Number2->sign);
		}
		Result.sign = 50;
	}
	else if (abs(checkNull(Number1, mant)) == 1 || abs(checkNull(Number2, mant)) == 1)
	{
		printNull(Number1->sign != Number2->sign, mant);
		Result.sign = 50;
	}
	else
	{
		Result = getFlag(Result, Number1, Number2);
		Result.exponent = Number1->exponent + Number2->exponent;
		Result.mantissa = Number1->mantissa * Number2->mantissa;
		(Result.mantissa >> (2 * (mant + 1) - 1)) >= 1 ? Result.exponent++ : Result.exponent;
		Result.sign = Number1->sign ^ Number2->sign;
		Result = normals(Result, 0, mant);
	}
	return Result;
}

Number divide(Number *Number1, Number *Number2, int8_t mant)
{
	Number Result;
	if (abs(checkInfNaN(Number1, mant)) == 1 || abs(checkInfNaN(Number2, mant)) == 1 ||
		abs(checkNull(Number1, mant)) == 1 || abs(checkNull(Number2, mant)) == 1)
	{
		if (abs(checkInfNaN(Number1, mant)) == 1 && abs(checkInfNaN(Number2, mant)) == 1 ||
			abs(checkNull(Number1, mant)) == 1 && abs(checkNull(Number2, mant)) == 1)
		{
			printf("nan\n");
		}
		else
		{
			if (abs(checkInfNaN(Number1, mant)) == 1 || abs(checkNull(Number1, mant)) != 1)
			{
				printInf(Number1->sign != Number2->sign);
			}
			else
			{
				printNull(Number1->sign != Number2->sign, mant);
			}
		}
		Result.sign = 50;
		return Result;
	}
	Result = getFlag(Result, Number1, Number2);
	Result.exponent = Number1->exponent - Number2->exponent;
	Result.mantissa = (Number1->mantissa << (62 - mant)) / Number2->mantissa;
	Result.exponent = Result.mantissa >> (62 - mant) < 1 ? Result.exponent - 1 : Result.exponent;
	Result = normals(Result, 0, mant);
	Result.sign = Number1->sign ^ Number2->sign;
	return Result;
}

Number printImaginary(Number *Numbers, int8_t rounding, int8_t mant)
{
	if (rounding == 1 && checkInfNaN(Numbers, mant) == 0)
	{
		printInf(Numbers->sign);
		Numbers->sign = 50;
	}
	else if ((rounding == 2 || rounding == 3) && Numbers->exponent >= (mant == 10 ? 16 : 128))
	{
		Numbers->sign == 1 ? putchar('-') : mant;
		if (Numbers->sign == 0 && rounding == 2 || Numbers->sign == 1 && rounding == 3)
		{
			printf("inf\n");
		}
		else
		{
			mant == 23 ? printf("0x1.fffffep+127\n") : printf("0x1.ffcp+15\n");
		}
		Numbers->sign = 50;
	}
	else if ((rounding == 1 || rounding == 3) && Numbers->exponent <= (mant == 10 ? -24 : -149))
	{
		Numbers->sign == 1 ? putchar('-') : mant;
		mant == 23 ? printf("0x1.000000p-149\n") : printf("0x1.000p-24\n");
		Numbers->sign = 50;
	}
	else if (rounding == 0 && Numbers->exponent >= (mant == 10 ? 16 : 128))
	{
		Numbers->sign == 1 ? putchar('-') : mant;
		mant == 23 ? printf("0x1.fffffep+127\n") : printf("0x1.ffcp+15\n");
		Numbers->sign = 50;
	}
	return *Numbers;
}

void printNumber(Number *Result, char format, int8_t mant)
{
	putchar('.');
	int8_t a = format == 'h' ? 2 : 1;
	int8_t b = format == 'h' ? 3 : 6;
	uint64_t mantissa = (Result->mantissa - (1 << mant)) << a;
	int8_t cnt = 0;
	while (mantissa > 0)
	{
		cnt++;
		mantissa /= 16;
	}
	for (int8_t i = 0; i < b - cnt; i++)
	{
		if (i == b - 1)
		{
			break;
		}
		putchar('0');
	}
	printf("%" PRIx64, (Result->mantissa - (1 << mant)) << a);
}

int main(int argc, char *argv[])
{
	if (argc < 4 || argc > 6 || argc == 5)
	{
		fprintf(stderr, "Incorrect number of arguments");
		return ERROR_ARGUMENTS_INVALID;
	}
	uint64_t first_argument, second_argument;
	char format;
	int8_t rounding, operation, cnt = 0;
	cnt += sscanf(argv[1], "%c", &format);
	cnt += sscanf(argv[2], "%hhd", &rounding);
	cnt += sscanf(argv[3], "%" PRIi64, &first_argument);
	int8_t exp = format == 'f' ? 8 : 5;
	int8_t mant = format == 'f' ? 23 : 10;
	if (cnt != 3)
	{
		fprintf(stderr, "The arguments could not be read\n");
		return ERROR_ARGUMENTS_INVALID;
	}
	if (format != 'f' && format != 'h')
	{
		fprintf(stderr, "Invalid format\n");
		return ERROR_ARGUMENTS_INVALID;
	}
	if (rounding < 0 || rounding > 3)
	{
		fprintf(stderr, "Invalid roundings\n");
		return ERROR_ARGUMENTS_INVALID;
	}
	Number Result = createNumber(first_argument, exp, mant);
	Number *res1 = &Result;
	if (argc == 6)
	{
		cnt += sscanf(argv[4], "%c", &operation);
		cnt += sscanf(argv[5], "%" PRIi64, &second_argument);
		if (cnt != 5)
		{
			fprintf(stderr, "The arguments could not be read\n");
			return ERROR_ARGUMENTS_INVALID;
		}
		Number Number2 = createNumber(second_argument, exp, mant);
		Number *res2 = &Number2;
		if (checkInfNaN(res1, mant) == 0 || checkInfNaN(res2, mant) == 0)
		{
			printf("nan\n");
			return SUCCESS;
		}
		else
		{
			switch (operation)
			{
			case '+':
				Result = add(res1, res2, mant);
				break;
			case '-':
				Result = subtract(res1, res2, mant);
				break;
			case '*':
				Result = multiply(res1, res2, mant);
				break;
			case '/':
				Result = divide(res1, res2, mant);
				break;
			default:
				fprintf(stderr, "Invalid operation\n");
				return ERROR_ARGUMENTS_INVALID;
			}
			if (Result.sign == 50)
			{
				return SUCCESS;
			}
			Result = printImaginary(&Result, rounding, mant);
		}
	}
	Number *res = &Result;
	roundings(res, rounding);
	if (Result.sign == 50)
	{
		return SUCCESS;
	}
	if (mant == 10 && res->exponent >= 16 || mant == 23 && res->exponent >= 128)
	{
		if (res->mantissa - (1 << mant) == 0 || mant == 10 && res->exponent > 16 || mant == 23 && res->exponent > 128)
		{
			printInf(res->sign);
		}
		else
		{
			printf("nan\n");
		}
	}
	else if (abs(checkNull(res, mant)) == 1)
	{
		printNull(res->sign, mant);
	}
	else
	{
		res->sign == 1 ? putchar('-') : exp;
		printf("0x1");
		printNumber(res, format, mant);
		res->exponent >= 0 ? printf("p+%hd\n", res->exponent) : printf("p%hd\n", res->exponent);
	}
	return SUCCESS;
}
