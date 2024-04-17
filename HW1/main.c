#include <stdint.h>
#include <stdio.h>

uint16_t len_cnt(uint32_t cnt)
{
	uint16_t len = 1;
	while (cnt / 10 > 0)
	{
		len++;
		cnt /= 10;
	}
	return len;
}

void result_table(uint32_t n_start, uint32_t n_end, uint16_t len, uint16_t len_n, uint64_t fin, int8_t align)
{
	for (uint32_t i = n_start; i <= n_end; i++)
	{
		fin = i != 0 ? ((uint64_t)fin * i) % INT32_MAX : 1;
		uint16_t lci = len_cnt(i), lcf = len_cnt((uint32_t)fin);
		switch (align)
		{
		case 0:
			int32_t a = len_n - lci, b = len - lcf;
			printf("|%*s%d%*s|%*s%llu%*s|\n", (a >> 1) + (1 & a), "", i, a >> 1, "", (b >> 1) + (1 & b), "", fin, b >> 1, "");
			break;
		case -1:
			printf("| %d%*s| %llu%*s|\n", i, len_n - lci - 1, "", fin, len - lcf - 1, "");
			break;
		default:
			printf("|%*s%d |%*s%llu |\n", len_n - lci - 1, "", i, len - lcf - 1, "", fin);
			break;
		}
	}
}

void line_table(uint16_t len, uint16_t len_n)
{
	putchar('+');
	for (int32_t i = 0; i < len_n; i++) {putchar('-');}
	putchar('+');
	for (int32_t i = 0; i < len; i++) {putchar('-');}
	printf("+\n");
}

void print_table(uint32_t n_start, uint32_t n_end, uint16_t len, uint16_t len_n, uint32_t fin, int8_t align)
{
	line_table(len, len_n);
	putchar('|');
	switch (align)
	{
	case 0:
		printf("%*s%*s%*s%*s", len_n / 2 + 1, "n", len_n - len_n / 2, "|", len / 2 + 1 + len % 2, "n!", len / 2 + 1, "|\n");
		break;
	case -1:
		printf("%*s%*s%*s%*s", 2, "n", len_n - 1, "|", 3, "n!", len - 1, "|\n");
		break;
	default:
		printf("%*s%*s%*s%*s", len_n - 1, "n", 2, "|", len - 1, "n!", 3, "|\n");
		break;
	}
	line_table(len, len_n);
	if (n_start > n_end)
	{
		result_table(n_start, UINT16_MAX, len, len_n, fin, align);
		result_table(0, n_end, len, len_n, 1, align);
	}
	else
	{
		result_table(n_start, n_end, len, len_n, fin, align);
	}
	line_table(len, len_n);
}

int main()
{
	int32_t j, n_start, n_end;
	uint32_t fin = 1, buf, cnt = 1;
	int8_t align;
	scanf_s("%d%d%hhd", &n_start, &n_end, &align);
	j = n_start;
	if (n_start < 0 || n_end < 0 || n_end > UINT16_MAX || n_start > UINT16_MAX || align < -1 || align > 1)
	{
		return 1;
	}
	uint16_t len, len_n;
	for (int32_t i = 1; i < n_start; i++)
	{
		fin = ((uint64_t)fin * i) % INT32_MAX;
	}
	buf = fin;
	while (j != n_end + 1)
	{
		fin = ((uint64_t)fin * j) % INT32_MAX;
		cnt = cnt < fin ? fin : cnt;
		j = (j + 1) % INT16_MAX;
	}
	len_n = n_start > n_end ? 7 : len_cnt(n_end) + 2;
	len = n_start > n_end ? 12 : len_cnt(cnt) + 2;
	len = len == 3 ? 4 : len;
	print_table(n_start, n_end, len, len_n, buf, align);
	return 0;
}