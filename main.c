#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_DEFAULT_CAPACITY 32

#define STR_ROUND_TO_POWER_OF_TWO($Val) \
    ($Val)--;                                               \
	($Val) |= ($Val) >> (size_t) 1;                           \
	($Val) |= ($Val) >> (size_t) 2;                           \
	($Val) |= ($Val) >> (size_t) 4;                           \
	($Val) |= ($Val) >> (size_t) 8;                           \
	($Val) |= ($Val) >> (size_t) 16;                          \
	($Val)++

#define STR_CLAMP($Val, $Min, $Max) (($Val) < ($Min)) \
	? (($Val) = ($Min))                                       \
		: (($Val) > ($Max))                                    \
		? (($Val) = ($Max))                                    \
			: ($Val)

#define STR_MIN($Lhs, $Rhs) (($Lhs) < ($Rhs)) ? ($Lhs) : ($Rhs)

#define FOR($Ty, $Container) \
	for($Ty* it = ($Container).data; \
		it != ($Container).data + ($Container).size; \
		++it)

#define FOR_REVERSE($Ty, $Container) \
	for($Ty* it = ($Container).data + ($Container).size - 1; \
		it != ($Container).data - 1; \
		--it)

#define FOR_ITER($Ty, $IterBegin, $IterEnd) \
	for($Ty* it = ($IterBegin); \
		it != ($IterEnd); \
		++it)

#define FOR_ITER_REVERSE($Ty, $IterBegin, $IterEnd) \
	for($Ty* it = ($IterBegin); \
		it != ($IterEnd) - 1; \
		--it)

typedef struct String {
    char* data;
    size_t capacity;
    size_t size;
} string_t;

typedef struct StringView {
	const char* data;
	size_t size;
} string_view_t;

string_t str_create_from_capacity(size_t capacity) {
    STR_ROUND_TO_POWER_OF_TWO(capacity);
    string_t arr = {
        .data = malloc(capacity),
        .capacity = capacity,
        .size = 0
    };

    return arr;
}

string_t str_create_from_literal(const char* lit) {
    const size_t lit_size = strlen(lit);
    size_t capacity = lit_size;
    STR_ROUND_TO_POWER_OF_TWO(capacity);
    string_t str = {
        .data = malloc(capacity),
        .capacity = capacity,
        .size = lit_size
    };

    FOR_ITER(const char, lit, lit + lit_size) {
    	const size_t index = it - lit;
    	str.data[index] = *it;
    }

    return str;
}

string_t str_create_empty() {
    return str_create_from_capacity(STR_DEFAULT_CAPACITY);
}

void str_free(string_t* str) {
    free(str->data);
    str->capacity = 0;
    str->size = 0;
}

string_t str_copy(const string_t src) {
	string_t dst = {
			.data = malloc(src.capacity),
			.capacity = src.capacity,
			.size = src.size
	};

	FOR(char, src) {
		const size_t index = it - src.data;
		dst.data[index] = *it;
	}

	return dst;
}

void str_realloc(string_t* str, size_t new_capacity) {
    str->data = realloc(str->data, new_capacity);
    str->capacity = new_capacity;
    str->size = STR_CLAMP(str->size, 0, new_capacity);
}

void str_push(string_t* str, const char val) {
    if (str->size >= str->capacity) {
        str_realloc(str, str->capacity * 2);
    }

    str->data[str->size++] = val;
}

char str_pop(string_t* str) {
    return str->data[str->size--];
}

void str_append(string_t* dst, string_t* src) {
    const size_t left = dst->capacity - dst->size;

    if (left < src->size) {
        size_t new_capacity = src->size - left + dst->capacity;
        STR_ROUND_TO_POWER_OF_TWO(new_capacity);
        str_realloc(dst, new_capacity);
    }

    FOR(char, (*src)) {
        str_push(dst, *it);
    }
}

void str_append_literal(string_t* dst, const char* src) {
	const size_t left = dst->capacity - dst->size;
	const size_t src_size = strlen(src);

	if (left < src_size) {
		size_t new_capacity = src_size - left + dst->capacity;
		STR_ROUND_TO_POWER_OF_TWO(new_capacity);
		str_realloc(dst, new_capacity);
	}

	FOR_ITER(const char, src, src + src_size) {
		str_push(dst, *it);
	}
}

string_t str_substr(string_t* str, size_t offset, size_t len) {
	size_t capacity = len;
	STR_ROUND_TO_POWER_OF_TWO(capacity);
	string_t substr = {
			.data = malloc(capacity),
			.capacity = capacity,
			.size = len
	};

	FOR_ITER(char, str->data + offset, str->data + offset + len) {
		const size_t index = it - (str->data + offset);
		substr.data[index] = *it;
	}

	return substr;
}

string_view_t str_substr_view(const string_t* str, size_t offset, size_t len) {
	string_view_t substr = {
			.data = str->data + offset,
			.size = len
	};

	return substr;
}

char lowercase(char chr) {
	if (chr >= 'A' && chr <= 'Z') return 'a' + (chr - 'A');

	return chr;
}

char uppercase(char chr) {
	if (chr >= 'a' && chr <= 'z') return 'A' + (chr - 'a');

	return chr;
}

int is_alphabetic(char chr) {
	chr = lowercase(chr);
	if (chr >= 'a' && chr <= 'z') return 1;

	return 0;
}

void str_lowercase(string_t str) {
	FOR(char, str) {
		const size_t index = it - str.data;
		str.data[index] = lowercase(str.data[index]);
	}
}

void str_uppercase(string_t str) {
	FOR(char, str) {
		const size_t index = it - str.data;
		str.data[index] = uppercase(str.data[index]);
	}
}

int str_compare(const string_t lhs, const string_t rhs) {
	const size_t min_size = STR_MIN(lhs.size, rhs.size);
	for (int i = 0; i < min_size; ++i) {
		const char lchr = lowercase(lhs.data[i]);
		const char rchr = lowercase(rhs.data[i]);

		if (lchr > rchr) {
			return -1;
		} else if (lchr < rchr) {
			return 1;
		} else {
			continue;
		}
	}

	if (lhs.size == rhs.size) return 0;
	else if (lhs.size > rhs.size) return -1;
	else return 1;
}

int str_compare_literal(const string_t lhs, const char* rhs) {
	const size_t rhs_size = strlen(rhs);
	const size_t min_size = STR_MIN(lhs.size, rhs_size);
	for (int i = 0; i < min_size; ++i) {
		const char lchr = lowercase(lhs.data[i]);
		const char rchr = lowercase(rhs[i]);

		if (lchr > rchr) {
			return -1;
		} else if (lchr < rchr) {
			return 1;
		} else {
			continue;
		}
	}

	if (lhs.size == rhs_size) return 0;
	else if (lhs.size > rhs_size) return -1;
	else return 1;
}

int str_compare_view(const string_t lhs, const string_view_t rhs) {
	const size_t min_size = STR_MIN(lhs.size, rhs.size);
	for (int i = 0; i < min_size; ++i) {
		const char lchr = lowercase(lhs.data[i]);
		const char rchr = lowercase(rhs.data[i]);

		if (lchr > rchr) {
			return -1;
		} else if (lchr < rchr) {
			return 1;
		} else {
			continue;
		}
	}

	if (lhs.size == rhs.size) return 0;
	else if (lhs.size > rhs.size) return -1;
	else return 1;
}

int str_equal(const string_t lhs, const string_t rhs) {
	if (lhs.size != rhs.size) return 0;
	for (int i = 0; i < lhs.size; ++i) {
		if (lhs.data[i] != rhs.data[i]) return 0;
	}

	return 1;
}

int str_equal_literal(const string_t lhs, const char* rhs) {
	const size_t rhs_size = strlen(rhs);
	if (lhs.size != rhs_size) return 0;
	for (int i = 0; i < lhs.size; ++i) {
		if (lhs.data[i] != rhs[i]) return 0;
	}

	return 1;
}

int str_equal_view(const string_t lhs, const string_view_t rhs) {
	if (lhs.size != rhs.size) return 0;
	for (int i = 0; i < lhs.size; ++i) {
		if (lhs.data[i] != rhs.data[i]) return 0;
	}

	return 1;
}

void str_print(const string_t str) {
	FOR(char, str) {
		putchar(*it);
	}
}

void str_view_print(const string_view_t str) {
	FOR(const char, str) {
		putchar(*it);
	}
}

string_t str_input(char delim) {
	string_t str = str_create_empty();
	for (char input = getchar(); input != delim; input = getchar()) {
		str_push(&str, input);
	}

	return str;
}

int main() {
	string_t str1 = str_create_from_literal("abcdef");
	str_print(str1);

	puts("");

	printf("\"abcdef\" cmp \"abcdef\": %d", str_compare_literal(str1, "abcdef"));
	puts("");

	printf("\"abcdef\" cmp \"bcdefg\": %d", str_compare_literal(str1, "bcdefg"));
	puts("");

	string_t str2 = str_create_from_literal("bcdefg");
	printf("\"bcdefg\" cmp \"abcdef\": %d", str_compare_literal(str2, "abcdef"));
	puts("");

	string_view_t str_view1 = str_substr_view(&str1, 3, 3);
	str_view_print(str_view1);

	puts("");

	string_t str3 = str_substr(&str1, 1, 3);
	str_print(str3);

	puts("");

	str_append(&str1, &str3);
	str_print(str1);

	puts("");

	str_append_literal(&str1, "appended");
	str_print(str1);

	puts("");

	str_free(&str1);
	str_free(&str2);
	str_free(&str3);

	string_t str4 = str_input('\n');
	str_print(str4);

	str_free(&str4);

    return 0;
}
