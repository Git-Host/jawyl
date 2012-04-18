<?php
TApplication::Import('common::enum');

final class TOperationType extends TEnumeration {
   	const O_ADD = 15;
   	const O_SUBTRACT = 16;
   	const O_MUL = 17;
   	const O_DIV = 18;
   	const O_MOD = 19;
   	const O_AND = 20;
   	const O_OR = 21;
   	const O_IFELSE = 22;
   	const O_CASE = 23;
   	const O_STRLEN = 24;
   	const O_STRPOS = 25;
   	const O_STRCONCAT = 26;
   	const O_STRLOWER = 27;
   	const O_STRUPPER = 28;
   	const O_SUBSTR = 29;
   	const O_STRREPLACE = 30;
   	const O_STRLTRIM = 31;
   	const O_STRRTRIM = 32;
   	const O_STRTRIM = 33;
   	const O_ABS = 34;
   	const O_ACOS = 35;
   	const O_ASIN = 36;
   	const O_ATAN = 37;
   	const O_COS = 38;
   	const O_SIN = 39;
   	const O_TAN = 40;
   	const O_CEIL = 41;
   	const O_FLOOR = 42;
   	const O_ROUND = 43;
   	const O_POW = 44;
   	const O_EXP = 45;
   	const O_LN = 46;
   	const O_LOG = 47;
   	const O_SQRT = 48;
   	const O_RAND = 49;
   	const O_DATE_CUR = 50;
   	const O_DATE_ADD = 51;
   	const O_DATE_SUBT = 52;
   	const O_DATE_DIFF = 53;
   	const O_DATE_MAKE = 54;
   	const O_DATE_DAY = 55;
   	const O_DATE_MONTH = 56;
   	const O_DATE_YEAR = 57;
   	const O_DATE_WEEK = 58;
   	const O_DATE_WKDAY = 59;
   	const O_TIME_CUR = 60;
   	const O_TIME_ADD = 61;
   	const O_TIME_SUBT = 62;
   	const O_TIME_DIFF = 63;
   	const O_TIME_MAKE = 64;
   	const O_TIME_HOUR = 65;
   	const O_TIME_MIN = 66;
   	const O_TIME_SEC = 67;
   	const O_TIME_MKSEC = 68;
}

final class TConditionType extends TEnumeration {
	const C_EQUAL = 0;
	const C_NOT_EQUAL = 1;
	const C_LESS = 2;
	const C_MORE = 3;
	const C_LESS_OR_EQUAL = 4;
	const C_MORE_OR_EQUAL = 5;
	const C_IN = 6;
	const C_NOT_IN = 7;
	const C_AND = 8;
	const C_OR = 9;
	const C_LIKE = 10;
	const C_NOT_LIKE = 11;
	const C_REGEXP = 12;
	const C_IS = 13;
	const C_NOT = 14;
}

final class TSortingType extends TEnumeration {
	const SORT_ASC = 0;
	const SORT_DESC = 1;
}