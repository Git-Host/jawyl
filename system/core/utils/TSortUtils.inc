<?php

class TSortUtils {
	private static function _fast_sort_iteration(array &$a, $from, $to, $cmp){
		$i = $from;
		$j = $to;
		$dir = false;
		while ($i < $j){
			$cmp_result = call_user_func_array($cmp,array($a[$i],$a[$j]));
			if ($cmp_result > 0){
				$tmp = $a[$i];
				$a[$i] = $a[$j];
				$a[$j] = $tmp;
				$dir = !$dir;
			}
			if ($dir)
				$j--;
			else
				$i++;
		}
		if ($from < $i - 1)
			self::_fast_sort_iteration($a, $from, $i - 1, $cmp);
		if ($to > $j + 1)
			self::_fast_sort_iteration($a, $j+1, $to, $cmp);
	}
	
	private static function _simple_cmp($a,$b){
		return ($a < $b)?-1:(($a > $b)?1:0);
	}
	
	
	public static function FastSort(array &$a, $cmp = null){
		if (!is_callable($cmp))
			$cmp = array('TSortUtils','_simple_cmp');
		self::_fast_sort_iteration($a, 0, count($a) - 1, $cmp);
	}
}