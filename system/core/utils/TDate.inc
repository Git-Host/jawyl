<?php
/**
 * @package CoreRuntimeClasses
 * @subpackage Utilities
 * @category Core
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

class TDateTimeInterval {
	private $_di_;
	
	public function __construct(DateInterval $i){
		$this->_di_ = $i;
	}
	
	public function DateIntervalObject(){
		return $this->_di_;
	}
	
	public function Value($accuracy){
		if (!$accuracy)
			$accuracy = 6;
		$result = array();
		
		foreach ((array)$this->_di_ as $nm=>$value){
			if ($value != 0){
				$result[$nm] = $value;
				$accuracy--;	
			}
			if ($accuracy == 0)
				break;
		}		
		return $result;
	}
}


/**
 * @package CoreRuntimeClasses
 * @subpackage Utilities
 * @category Core
 * Datetime wrapper.
 * @property int $Day day part of date
 * @property int $Month month part of date
 * @property int $Year year part of date
 * @property int $Hour hour part of date
 * @property int $Minutes minutes part of date
 * @property int $Seconds seconds part of date
 * @property-read int $WeekDay number of day of the week represented by date, returns 0 for Sunday through 6 for Saturday
 * @property-read int $YearDay number of day of the year represented by date
 * @property-read TDate $LastMonthDate last day of date month
 * 
 */
	class TDate {
/*
 * @todo wrap DateTime class by TDate
 * 
 * */		
		private $_timestamp_;
		
		const DP_DAY = 0;
		const DP_MONTH = 1;
		const DP_WEEK = 2;
		const DP_YEAR = 3;
		const DP_HOUR = 4;
		const DP_MINUTE = 5;
		const DP_SECOND = 6;
		
		public function __get($nm){
			$date = getdate($this->_timestamp_);
			switch ($nm){
				case "Day":return $date["mday"];break;
				case "Month":return $date["mon"];break;
				case "Year":return $date["year"];break;
				case "Hour":return $date["hours"];break;
				case "Minutes":return $date["minutes"];break;
				case "Seconds":return $date["seconds"];break;
				case "Week":return 0 + date("W",$this->_timestamp_);break;
				case "WeekDay":return ($date["wday"] == 0)?7:$date["wday"];break;
				case "YearDay":$date["yday"];break;
				case "LastMonthDate":return new TDate(mktime(0,0,0,$date["mon"] + 1,0,$date["year"]));break;
			}
		}
		
		public function __set($nm,$value){
			switch ($nm){
				case "Day":$this->_timestamp_ = mktime($this->Hour,$this->Minutes,$this->Seconds,$this->Month,$value,$this->Year);break;
				case "Month":$this->_timestamp_ = mktime($this->Hour,$this->Minutes,$this->Seconds,$value,$this->Day,$this->Year);break;
				case "Year":$this->_timestamp_ = mktime($this->Hour,$this->Minutes,$this->Seconds,$this->Month,$this->Day,$value);break;
				case "Hour":$this->_timestamp_ = mktime($value,$this->Minutes,$this->Seconds,$this->Month,$this->Day,$this->Year);break;
				case "Minutes":$this->_timestamp_ = mktime($this->Hour,$value,$this->Seconds,$this->Month,$this->Day,$this->Year);break;
				case "Seconds":$this->_timestamp_ = mktime($this->Hour,$this->Minutes,$value,$this->Month,$this->Day,$this->Year);break;
			}
		}

/**
 * adds period to date, changes date object itself.
 * @param int $value period length, when negative period is subtracted
 * @param int $type period units type, must be one of TDate constants
 * @return TDate
 */		
		public function Add($value,$type = TDate::DP_DAY){
			$day = $this->Day;
			$month = $this->Month;
			$year = $this->Year;
			$hour = $this->Hour;
			$minutes = $this->Minutes;
			$seconds = $this->Seconds; 
			switch ($type){
				case self::DP_DAY:$day = $value + $this->Day;break;
				case self::DP_MONTH:$month = $value + $this->Month;break;
				case self::DP_YEAR:$tyear = $value + $this->Year;break;
				case self::DP_HOUR:$hour = $value + $this->Hour;break;
				case self::DP_MINUTE:$minutes = $value + $this->Minutes;break;
				case self::DP_SECOND:$seconds = $value + $this->Seconds;break;
				case self::DP_WEEK:$day = $this->Day + $value*7;break;
				default:{
					throw new TCoreException(TCoreException::ERR_BAD_VALUE);
					return null;
				}break;
			}
			return new TDate(array("mday"=>$day,"mon"=>$month,"year"=>$year,"hours"=>$hour,"minutes"=>$minutes,"seconds"=>$seconds));
		}
		
		public function Diff(TDate $date, $type = TDate::DP_DAY,$accurate = false){
			$diff = $this->TimeStamp() - $date->TimeStamp();
			switch ($type){
				case self::DP_MINUTE:$diff = $diff/60;break;
				case self::DP_HOUR:$diff = $diff/3600;break;
				case self::DP_DAY:$diff = $diff/86400;break;
				case self::DP_WEEK:$diff = $diff/604800;break;
/*
 * @todo: make months calculation more accurate
 */				
				case self::DP_MONTH:$diff = $diff/2592000;break;
				case self::DP_YEAR:$diff = $diff/31536000;break;
			}
			if (!$accurate)
				$diff = round($diff);
			return $diff;	
		}
		
		public function Interval(TDate $date){
			return new TDateTimeInterval($this->DateTimeObject()->diff($date->DateTimeObject()));
		}
		
/**
 * gets a timestamp from string according to format. 
 * Yet only numeric date parts of formats of php date() function are supported.
 * Those are 'd','j','m','n','y','Y','G','H','i','s'.
 * @param string $time a datetime value to parse
 * @param string $format a format to parse by
 * @return int 
 */
		protected function fromString($time,$format){
			$parse_string = preg_replace('/([\\\^\$\.\|\?\*\+\(\)\{\}])/','\\\$1',$format);
			$parse_string = preg_replace('/[jn]/','(\d)',$parse_string);
			$parse_string = preg_replace('/[dmyGHis]/','(\d{2})',$parse_string);
			$parse_string = preg_replace('/[Y]/','(\d{4})',$parse_string);
			$parse_string = $parse_string;
			preg_match_all('/d|j|m|n|Y|y|G|H|i|s/',$format,$params);
			preg_match_all('/'.$parse_string.'/',$time,$test,PREG_SET_ORDER);
			
			if (count($test) == 0) return null;
			if (count($test[0]) < 1) return null;
			$datetime = array();
			for ($i  = 1; $i < count($test[0]); $i++)
				$datetime[$i - 1] = $test[0][$i];

			$day = null;
			$month = null;
			$year = null;
			$hour = 0;
			$minute = 0;
			$second = 0;
			if (count($datetime) == count($params[0])){
				for ($i = 0; $i < count($datetime); $i++){
					switch ($params[0][$i]){
						case 'd':$day = $datetime[$i];break;
						case 'j':$day = $datetime[$i];break;
						case 'm':$month = $datetime[$i];break;
						case 'n':$month = $datetime[$i];break;
						case 'Y':$year = $datetime[$i];break;
						case 'y':$year = $datetime[$i];break;
						case 'G':$hour = $datetime[$i];break;
						case 'H':$hour = $datetime[$i];break;
						case 'i':$minute = $datetime[$i];break;
						case 's':$second = $datetime[$i];break;
					}
				}
				if (!is_null($year) && !is_null($month) && !is_null($day))
					return mktime($hour,$minute,$second,$month,$day,$year);
			}
			return null;
		} 
		
/**
 * constructor. creates a TDate object from almost any value.
 * @param mixed $date value of any php base type.
 * If int is specified it is treated as timestamp.
 * If float is specified it is rounded to int and treated as timestamp
 * If array is specified it is checked for "seconds", "minutes","hours","mday","mon" and "year" members.
 * If they are present then date is created according this information. If only last three members are present, missing are set to zero, and date is created.
 * If necessary information is not specified then array is treated as ordinal.
 * For array of three elements they are treated as month, day and year.
 * For array of four elements they are treated as month, day, year and seconds.
 * For array of five elements they are treated as month, day, year, minutes and seconds.
 * For array of five elements they are treated as month, day, year, hours, minutes and seconds.
 * If string is specified constructor tries to parse it according to current application locale format.
 * If parsing fails then php strtotime function is used for convertion.
 * @see TDate::fromString() 
 */		
		public function __construct($date = null){
			$this->timestamp = null;
			if (is_array($date)){
				if (isset($date["seconds"]) 
				&& isset($date["minutes"])
				&& isset($date["hours"])
				&& isset($date["mday"])
				&& isset($date["mon"])
				&& isset($date["year"])
				) 
				$this->_timestamp_ = mktime($date["hours"], $date["minutes"], $date["seconds"], $date["mon"], $date["mday"], $date["year"]);
				else if (isset($date["mday"]) && isset($date["mon"]) && isset($date["year"])) 
				$this->_timestamp_ = mktime(0, 0, 0, $date["mon"], $date["mday"], $date["year"]);
				else {
					$values = array_values($date);
					switch (count($values)){
						case 3: $this->_timestamp_ = mktime(0,0,0,$values[0],$values[1],$values[2]);break;
						case 4: $this->_timestamp_ = mktime(0,0,$values[3],$values[0],$values[1],$values[2]);break;
						case 5: $this->_timestamp_ = mktime(0,$values[3],$values[4],$values[0],$values[1],$values[2]);break;
						case 6: $this->_timestamp_ = mktime($values[3],$values[4],$values[5],$values[0],$values[1],$values[2]);break;
					}
				}   
			}
			elseif (is_double($date))
				$this->_timestamp_ = round($date);
			elseif (is_int($date))
				$this->_timestamp_ = $date;
			elseif (is_string($date)){
				$this->_timestamp_ = $this->fromString($date,TI18n::Locale()->DateFormat());
				if (is_null($this->_timestamp_))
					$this->_timestamp_ = strtotime($date);	
			} 
			elseif ($date instanceof TDate)
				$this->_timestamp_ = $date->TimeStamp();
			
			if ((is_null($this->_timestamp_)) && (!is_null($date))) 
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);	
			
			if (is_null($this->_timestamp_))
				$this->_timestamp_ = time();
		}

/**
 * gets datetime string representation according to format
 * @param string $format format supported by php date() function
 * @return string 
 */		
		public function ToString($format){
			if (is_null($format)) 
				return TI18n::Locale()->FormatDateTime($this->_timestamp_);	
			return date($format,$this->_timestamp_);
		}
		
/**
 * gets datetime string representation according to current application locale
 * @return string 
 */		
		public function __toString(){
			return _dt($this->_timestamp_);	
		}

/**
 * gets timestamp value of datetime
 * @return int 
 */		
		public function TimeStamp(){
			return $this->_timestamp_;
		}
		
/**
 * gets timestamp value of date part of datetime (not including time portion)
 * @return int 
 */		
		public function DateTimeStamp(){
			return mktime(0,0,0,$this->Month,$this->Day,$this->Year);
		}
		
		public function DateTimeObject(){
			$dt = new DateTime();
			$dt->setTimeStamp($this->_timestamp_);
			return $dt;
		} 
	}
?>