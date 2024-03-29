<?php
/**
 * @package L10n
 * @subpackage Native
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha
 */

/**
 * @package L10n
 * @subpackage Native
 * @category System
 * Class of Jawyl native xml based locale
 */
class TIntrinsicLocale extends TLocale {
	/**
	 * @var string
	 */
	protected $description = "";
	/**
	 * @var array
	 */
	protected $symbols = array();
	/**
	 * @var array
	 */
	protected $formats = array();
	
	protected $months = array();
	protected $shortMonths = array();
	
	protected $weekDays = array();
	protected $shortWeekDays = array();
	protected $minWeekDays = array();
	
	protected $firstDay;
	
	/**
	 * @see ILocale::SetLanguage()
	 * @param string $language
	 */
	public function SetLanguage($language){
		$filename = $this->Application()->PrivateDir()."/locales/".$language.".xml";
		if (!file_exists($filename)) $filename = $this->Application()->SysPath()."/locales/".$language.".xml";
		if (file_exists($filename)){
			$dom = new DOMDocument();
			$dom->Load($filename);
			if (TConvertions::ConvertToBoolean($this->Validate))
				if (!$dom->schemaValidate($this->Application()->SysPath()."/schemas/locale.xsd"))
					throw new TCoreException(TCoreException::ERR_CONF_INCORRECT);
				
				
			$xpath = new DOMXPath($dom);
				
			$description = $xpath->query("/locale/description");
			if ($description->length > 0)
			$this->description = $description->item(0)->nodeValue;

			$symbols = $xpath->query("/locale/symbols/*");
			foreach ($symbols as $symbol)
			$this->symbols[$symbol->tagName] = $symbol->nodeValue;

			$formats = $xpath->query("/locale/formats/*");
			foreach ($formats as $format)
			$this->formats[$format->tagName] = $format->nodeValue;
			
			$months = $xpath->query("/locale/months/*");
			foreach ($months as $month){
				$this->months[$month->getAttribute("order") - 1] = $month->nodeValue;
				$this->shortMonths[$month->getAttribute("order") - 1] = $month->getAttribute("shortname");
			}
				
			$wd = $xpath->query("/locale/weekdays");
			$this->firstDay = $wd->Item(0)->getAttribute("firstday");
			$wd = $xpath->query("/locale/weekdays/*");
			foreach ($wd as $d){
				$this->weekDays[$d->getAttribute("order") - 1] = $d->nodeValue;
				$this->shortWeekDays[$d->getAttribute("order") - 1] = $d->getAttribute("shortname");
				$this->minWeekDays[$d->getAttribute("order") - 1] = $d->getAttribute("minname");
			}
		}
	}

	/**
	 * @see ILocale::Description()
	 * @return string
	 */
	public function Description(){
		return $this->description;
	}

	/**
	 * @see ILocale::Symbols()
	 * @return array
	 */
	public function Symbols(){
		return $this->symbols;
	}

	/**
	 * @see ILocale::DateFormat()
	 * @return string
	 */
	public function DateFormat(){
		if (!key_exists("date",$this->formats)) return "c";
		$format = $this->formats["date"];
		$format = str_replace('{ds}',$this->symbols["dateseparator"],$format);
		return $format;
	}

	/**
	 * @see ILocale::DateTimeFormat()
	 * @return string
	 */
	public function DateTimeFormat(){
		if (!key_exists("datetime",$this->formats)) return "c";
		$format = $this->formats["datetime"];
		$format = str_replace('{ds}',$this->symbols["dateseparator"],$format);
		$format = str_replace('{ts}',$this->symbols["timeseparator"],$format);
		return $format;
	}

	/**
	 * @see ILocale::NumberFormat()
	 * @return string
	 */
	public function NumberFormat(){
            if (!key_exists("number",$this->formats)) return "";
            $format = $this->formats["number"];
            $format = str_replace('{dig}',$this->symbols["digseparator"],$format);
            $format = str_replace('{dec}',$this->symbols["decseparator"],$format);
            return $format;
	}
        
     /**
      * @param string $str
      * @return float
      */                
     public function StrToFloat($str){
          return floatval(preg_replace('/[^\d\.]/','',str_replace($this->symbols["decseparator"],'.',$str))); 
     }


	/**
	 * @see ILocale::CurrencyFormat()
	 * @return string
	 */
	public function CurrencyFormat(){
		if (!key_exists("currency",$this->formats)) return "";
		$format = $this->formats["currency"];
		$format = str_replace('{dig}',$this->symbols["digseparator"],$format);
		$format = str_replace('{dec}',$this->symbols["decseparator"],$format);
		$format = str_replace('{dec}',$this->symbols["decseparator"],$format);
		$format = str_replace('{lc}',$this->symbols["localcurrency"],$format);
		$format = str_replace('{ic}',$this->symbols["intercurrency"],$format);
		return $format;
	}

	
	/**
	 * formats number
	 * @param number $number
	 * @param string $format
	 * @return string
	 */
	protected function doFormatNumber($number, $format){
		//return $number;
		if ((count($this->formats) == 0) && (count($this->symbols) == 0)) return $number;
		preg_match('/\\A[#0]*\\{dig\\}([#0]*)(?:\\{dec\\}0*#*)?\\s*\\Z/',$format,$test);
		$digcount = strlen($test[1]);
		preg_match('/\\A[#0]*\\{dig\\}[#0]*\\{dec\\}(0*)#*\\s*\\Z/',$format,$test);
		$reqdeccount = strlen($test[1]);
		preg_match('/\\A[#0]*\\{dig\\}[#0]*\\{dec\\}0*(#*)\\s*\\Z/',$format,$test);
		$optdeccount = strlen($test[1]);
		$format = preg_replace('/\\{dig\\}/','',$format);
		$format = preg_replace('/\\{dec\\}[0]*/','',$format);
		preg_match('/\\A[^0]*([0]*)[^0]*\\Z/',$format,$test);
		$requiredcount = strlen($test[1]);
			
		if ($number > floor($number))
			$strnum = number_format($number,$reqdeccount+$optdeccount,"|","");
		else
			$strnum = number_format($number,0,"|","");
				
		$parts = explode("|",$strnum);
		if (strlen($parts[0]) < $requiredcount)
			$parts[0] = str_repeat("0",$requiredcount - strlen($parts[0])).$parts[0];
		$intpart = trim(strrev(chunk_split(strrev($parts[0]),$digcount,$this->symbols["digseparator"])));

		$decpart = "";
		if (count($parts) > 1) $decpart = $parts[1];
		if (strlen($decpart) < $reqdeccount)
			$decpart = $decpart.str_repeat("0",$reqdeccount - strlen($decpart));
		if (strlen($decpart) > 0)
			return $intpart.$this->symbols["decseparator"].$decpart;
		return $intpart;
	}

	/**
	 * @see ILocale::FormatNumber()
	 * @param number $number
	 * @return string
	 */
	public function FormatNumber($number){
		return $this->doFormatNumber($number,$this->formats["number"]);
	}

	/**
	 * @see ILocale::FormatDate()
	 * @param int|TDate $date
	 * @return string
	 */
	public function FormatDate($date){
		if (!$date) return '';
		if ($date instanceof TDate)
			$timestamp = $date->TimeStamp();
		else 
			$timestamp = (int)$date;
			
		if ((count($this->formats) == 0) && (count($this->symbols) == 0))
			return date("C",$timestamp);
			
		$format = $this->formats["date"];
		$format = str_replace('{ds}',$this->symbols["dateseparator"],$format);
		return date($format,$timestamp);
	}

	/**
	 * @see ILocale::FormatTime()
	 * @param int|TDate $timestamp
	 * @return string
	 */
	public function FormatTime($timestamp){
		if (!$timestamp) return '';
		if ($timestamp instanceof TDate)
			$timestamp = $timestamp->TimeStamp();
		
		if ((count($this->formats) == 0) && (count($this->symbols) == 0))
		return date("C",$timestamp);
			
		$format = $this->formats["time"];
		$format = str_replace('{ts}',$this->symbols["timeseparator"],$format);
		return date($format,$timestamp);
	}

	/**
	 * @see ILocale::FormatDateTime()
	 * @param int|TDate $timestamp
	 * @return string
	 */
	public function FormatDateTime($timestamp){
		if (!$timestamp) return '';
		if ($timestamp instanceof TDate)
			$timestamp = $timestamp->TimeStamp();
		
		if ((count($this->formats) == 0) && (count($this->symbols) == 0))
			return date(DATE_W3C,$timestamp);
			
		$format = $this->formats["datetime"];
		$format = str_replace('{ds}',$this->symbols["dateseparator"],$format);
		$format = str_replace('{ts}',$this->symbols["timeseparator"],$format);
		return date($format,$timestamp);
	}

	/**
	 * @see ILocale::FormatCurrency()
	 * @param number $number
	 * @return string
	 */
	public function FormatCurrency($number){
		if ((count($this->formats) == 0) && (count($this->symbols) == 0))
		return $number;
			
		$format = $this->formats["currency"];
		preg_match('/\\A([#0]*\\{dig\\}[#0]*(?:\\{dec\\}0*)?).*\\Z/',$format,$test);
		$num = $this->doFormatNumber($number,$test[1]);
		preg_match('/\\A[#0]*\\{dig\\}[#0]*(?:\\{dec\\}0*)?(.*)\\Z/',$format,$test);
		$format = str_replace('{lc}',$this->symbols["localcurrency"],$test[1]);
		$format = str_replace('{ic}',$this->symbols["intercurrency"],$format);
		return $num.$format;
	}
	
	public function MonthNames($type = ILocale::NT_NORMAL){
		switch ($type){
			case ILocale::NT_NORMAL:return $this->months;break;
			case ILocale::NT_MIN:
			case ILocale::NT_SHORT:return $this->shortMonths;break;
		}	
	}
		
	public function WeekDayNames($type = ILocale::NT_NORMAL){
		switch ($type){
			case ILocale::NT_NORMAL:return $this->weekDays;break;
			case ILocale::NT_MIN:return $this->minWeekDays;break;
			case ILocale::NT_SHORT:return $this->shortWeekDays;break;
		}	
	}
	
	public function FirstWeekDay(){
		return $this->firstDay;
	}
}

?>